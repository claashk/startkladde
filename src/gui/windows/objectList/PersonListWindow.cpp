/*
 * PersonListWindow.cpp
 *
 *  Created on: 25.09.2010
 *      Author: martin
 */

#include "PersonListWindow.h"

#include <iostream>

#include "src/config/Settings.h"
#include "src/gui/dialogs.h"
#include "src/util/qString.h"
#include "src/gui/windows/ObjectSelectWindow.h"
#include "src/gui/windows/ConfirmOverwritePersonDialog.h"
#include "src/gui/windows/objectEditor/ObjectEditorWindow.h"
#include "src/gui/windows/objectEditor/PersonEditorPane.h"
#include "src/gui/PasswordPermission.h"

PersonListWindow::PersonListWindow (DbManager &manager, QWidget *parent):
	ObjectListWindow<Person> (manager, parent),
	mergeAction (new QAction (this)),
	displayMedicalDataAction (new QAction (this)),
	mergePermission (databasePasswordCheck),
	viewMedicalDataPermission (databasePasswordCheck),
	changeMedicalDataPermission (databasePasswordCheck)
{
	// The model is a Person::DefaultObjectModel, as created in ObjectListWindow
	// TODO this sucks, the derived class should specify the model explicitly
	// (and get rid of validity checks)
	personModel=dynamic_cast<Person::DefaultObjectModel *> (getObjectModel ());
	if (!personModel)
		std::cerr << "Unexpected model type in PersonListWindow constructor" << std::endl;

	connect (mergeAction             , SIGNAL (triggered ()), this, SLOT (mergeAction_triggered              ()));
	connect (displayMedicalDataAction, SIGNAL (triggered ()), this, SLOT (displayMedicalDataAction_triggered ()));

	displayMedicalDataAction->setCheckable (true);
	displayMedicalDataAction->setChecked (false);

	ui.menuObject->addSeparator ();
	ui.menuObject->addAction (mergeAction);
	ui.menuObject->addAction (displayMedicalDataAction);

	mergePermission            .setPasswordRequired (Settings::instance ().protectMergePeople   );
	viewMedicalDataPermission  .setPasswordRequired (Settings::instance ().protectViewMedicals  );
	changeMedicalDataPermission.setPasswordRequired (Settings::instance ().protectChangeMedicals);

	// Display the medical data
	bool displayMedicalData=!Settings::instance ().protectViewMedicals;
	displayMedicalDataAction->setChecked (displayMedicalData);
	personModel->setDisplayMedicalData (displayMedicalData);

//	if (viewMedicalDataPermission.getPasswordRequired ())
//		if (personModel)
//			personModel->setDisplayMedicalData (false);

	setupText ();
}

PersonListWindow::~PersonListWindow ()
{
}

void PersonListWindow::setupText ()
{
	mergeAction             ->setText (tr ("&Merge"));
	displayMedicalDataAction->setText (tr ("Display &medical data"));
}

void PersonListWindow::displayMedicalDataAction_triggered ()
{
	if (!personModel) return;

	if (displayMedicalDataAction->isChecked ())
	{
		if (viewMedicalDataPermission.permit (tr ("The database password must be entered to view medical data.")))
			personModel->setDisplayMedicalData (true);
		else
			displayMedicalDataAction->setChecked (false);
	}
	else
	{
		personModel->setDisplayMedicalData (false);
	}

	// FIXME refresh the model
}

void PersonListWindow::mergeAction_triggered ()
{
	// We cannot use allowEdit because that is also used for adding and editing
	// people, and we may want to reqire a password for merging, but not for
	// editing.
	if (!mergePermission.permit (tr ("The database password must be entered to merge people.")))
		return;

	QList<Person> people=activeObjects ();
	if (people.size ()<2)
	{
		showWarning (
			tr ("Not enough people selected"),
			tr ("At least two people must be selected for merging."),
			this);
		return;
	}

	QString title=tr ("Select correct entry");

	QString text;
	if (people.size ()>2)
		text=tr ("Please select the correct entry. All other entries will be overwritten.");
	else
		text=tr ("Please select the correct entry. The other entry will be overwritten.");

	dbId correctPersonId=invalidId;
	ObjectSelectWindowBase::Result selectionResult=
		ObjectSelectWindow<Person>::select (&correctPersonId, title, text,
			people, new Person::DefaultObjectModel (), true, invalidId, false, this);

	switch (selectionResult)
	{
		case ObjectSelectWindowBase::resultOk:
			break;
		case ObjectSelectWindowBase::resultUnknown:      // fallthrough
		case ObjectSelectWindowBase::resultNew:          // fallthrough
		case ObjectSelectWindowBase::resultCancelled:    // fallthrough
		case ObjectSelectWindowBase::resultNoneSelected: // fallthrough
			return;
	}

	Person correctPerson;

	for (int i=0; i<people.size (); ++i)
	{
		if (people.at (i).getId ()==correctPersonId)
		{
			correctPerson=people.takeAt (i);
			break;
		}
	}

	// The wrong people are now in people
	bool confirmed=ConfirmOverwritePersonDialog::confirmOverwrite (correctPerson, people, this);

	if (confirmed)
		// Does not throw OperationCanceledException
		manager.mergePeople (correctPerson, people, this);
}

void PersonListWindow::prepareContextMenu (QMenu *contextMenu)
{
	ObjectListWindow<Person>::prepareContextMenu (contextMenu);

	if (activeObjectCount ()>1)
	{
		contextMenu->addSeparator ();
		contextMenu->addAction (mergeAction);
	}
}

void PersonListWindow::languageChanged ()
{
	ObjectListWindow<Person>::languageChanged ();
	setupText ();
}

int PersonListWindow::editObject (const Person &object)
{
	// TODO overly complicated, see ObjectEditorPane

	PersonEditorPaneData paneData;
	paneData.displayMedicalData=displayMedicalDataAction->isChecked ();
	paneData.viewMedicalDataPermission=&viewMedicalDataPermission;
	paneData.changeMedicalDataPermission=&changeMedicalDataPermission;

	return ObjectEditorWindow<Person>::editObject (this, manager, object, &paneData);
}
