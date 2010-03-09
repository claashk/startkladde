#include "PersonEditorPane.h"

#include "src/db/cache/Cache.h"
#include "src/model/Person.h"
#include "src/text.h"

/*
 * TODO: disallow person name changes; allow merges only
 */

// ******************
// ** Construction **
// ******************

PersonEditorPane::PersonEditorPane (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent):
	ObjectEditorPane<Person> (mode, cache, parent)
{
	ui.setupUi(this);

	fillData ();

	ui.lastNameInput->setFocus ();
}

PersonEditorPane::~PersonEditorPane()
{

}

template<> ObjectEditorPane<Person> *ObjectEditorPane<Person>::create (ObjectEditorWindowBase::Mode mode, Cache &cache, QWidget *parent)
{
	return new PersonEditorPane (mode, cache, parent);
}


// ***********
// ** Setup **
// ***********

void PersonEditorPane::fillData ()
{
	// Clubs
	ui.clubInput->addItems (cache.getClubs ());
	ui.clubInput->setCurrentText ("");
}


// ******************************
// ** ObjectEditorPane methods **
// ******************************

void PersonEditorPane::objectToFields (const Person &person)
{
	originalId=person.getId ();

	ui.lastNameInput->setText (person.lastName);
	ui.firstNameInput->setText (person.firstName);
	ui.clubInput->setCurrentText (person.club);
	ui.commentsInput->setText (person.comments);
}

Person PersonEditorPane::determineObject ()
{
	Person person;

	person.setId (originalId);

	person.lastName=ui.lastNameInput->text ();
	person.firstName=ui.firstNameInput->text ();
	person.club=ui.clubInput->currentText ();
	person.comments=ui.commentsInput->text ();

	// Error checks

	if (eintrag_ist_leer (person.lastName))
		errorCheck ("Es wurde kein Nachname angegeben.",
			ui.lastNameInput);

	if (eintrag_ist_leer (person.firstName))
		errorCheck ("Es wurde kein Vorname angegeben.",
			ui.firstNameInput);


	return person;
}
