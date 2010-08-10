#include "SkTableView.h"

#include <QSettings>
#include <QFont>
#include <QFontMetrics>
#include <QHeaderView>
#include <QApplication>
#include <QStyle>
#include <QKeyEvent>

#include "src/gui/widgets/TableButton.h"
#include "src/itemDataRoles.h"
#include "src/concurrent/threadUtil.h" // Required for assert (isGuiThread ());
#include "src/model/objectList/ColumnInfo.h"
#include "src/util/color.h"
#include "src/util/qString.h"

#include <iostream>
#include <cassert>

SkTableView::SkTableView (QWidget *parent):
	QTableView (parent),
	autoResizeRows (false),
	settingButtons (false)
{
	// Use a style sheet rather than a palette because a style may ignore the
	// palette, while a style sheet is guaranteed to be honored.

	// Note: somehow, gradients don't seem to work here
	setStyleSheet ("selection-background-color: #3F3F3F;");
}

SkTableView::~SkTableView ()
{
}

void SkTableView::setModel (QAbstractItemModel *model)
{
	QTableView::setModel (model);

	QObject::disconnect (this, SLOT (layoutChanged ()));
	connect (model, SIGNAL (layoutChanged ()), this, SLOT (layoutChanged ()));
}

void SkTableView::layoutChanged ()
{
	// This happens when the SortFilterModel filter settings are changed
	if (autoResizeRows)
		resizeRowsToContents ();
}

void SkTableView::updateButtons (int row)
{
	// TODO: this should not happen, but when opening the person or plane
	// editor, it sometimes does (note that the models have no buttons in this
	// case).
//	assert (!settingButtons);
	if (settingButtons) return;

	QAbstractItemModel *m=model ();
	int columns=m->columnCount ();

	for (int column=0; column<columns; ++column)
	{
		QModelIndex index=m->index (row, column);

		if (m->data (index, isButtonRole).toBool ())
		{
			QString buttonText=m->data (index, buttonTextRole).toString ();
			TableButton *button=new TableButton (index, buttonText);
			QObject::connect (button, SIGNAL (clicked (QPersistentModelIndex)), this, SIGNAL (buttonClicked (QPersistentModelIndex)));

			// Avoid recursive calls, see above
			settingButtons=true;
			setIndexWidget (index, button);
			settingButtons=false;
		}
		else
		{
			// Avoid recursive calls, see above
			settingButtons=true;
			setIndexWidget (index, NULL);
			settingButtons=false;
		}
	}
}

//void SkTableView::rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
//{
//}

void SkTableView::rowsInserted (const QModelIndex &parent, int start, int end)
{
	QTableView::rowsInserted (parent, start, end);

	for (int i=start; i<=end; ++i)
		updateButtons (i);

	// If a row is inserted, the rows after that get renumbered. A row that
	// gets an index that did not exist before may be resized to the default
	// size (at least with 4.3.4). This may be a bug in Qt.
	// Workaround: resize all rows, not only the ones that were inserted.

	resizeRowsToContents ();

//	if (autoResizeRows)
//		for (int i=start; i<=end; ++i)
//			resizeRowToContents (i);
}

void SkTableView::dataChanged (const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	assert (isGuiThread ());

	QTableView::dataChanged (topLeft, bottomRight);

	if (settingButtons) return;

	for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
		updateButtons (i);

	if (autoResizeRows)
		for (int i=topLeft.row (); i<=bottomRight.row (); ++i)
			resizeRowToContents (i);
}

void SkTableView::reset ()
{
	// Strange things happening here:
	// Calling QTableView::reset here seems to *sometimes* cause a segfault
	// (when refreshing), probably related to the table buttons, like so:
	//
	// #0  0x01098b9c in QObject::disconnect(QObject const*, char const*, QObject const*, char const*)
	// #1  0x00a4b1f9 in QAbstractItemView::reset() [the following call]
	// #2  0x080ac6ed in SkTableView::reset [this method]
	//
	// This problem is hard to reproduce. On some runs of the program, it does
	// not appear at all. Restarting the program may help in reproducing the
	// problem.
	//
	// Note that this is the case even if the button signal is not connected.
	// Note also that QAbstractItemView::reset does not seem to call
	// QObject::disconnect at all.
	//
	// Now, the funny thing is: calling QTableView::reset does not even seem to
	// be necessary here, the table is still refreshed and all of the buttons
	// are deleted. Also, not calling QTableView::reset seems to fix the
	// problem.
//	QTableView::reset (); // DO NOT CALL!

	// Set up the buttons
	int rows=model ()->rowCount ();
	for (int row=0; row<rows; ++row)
		updateButtons (row);

	if (autoResizeRows)
		resizeRowsToContents ();
}

/**
 * Reads the column widths from the settings or uses defaults from a columnInfo
 *
 * The settings object has to be set to the correct section. The widths are
 * read from the value columnWidth_(name) where name is the column name from
 * columnInfo.
 *
 * If no width is stored in settings for a given column, the sample text from
 * columnInfo and the column title from the model are used to determine a
 * default width.
 *
 * @param settings the QSettings to read the widths from
 * @param columnInfo the ColumnInfo to read the default widths from
 */
void SkTableView::readColumnWidths (QSettings &settings, const ColumnInfo &columnInfo)
{
	// The column info set must have the sam number of columns as the model of
	// this table.
	assert (columnInfo.columnCount ()==model ()->columnCount ());

	for (int i=0; i<columnInfo.columnCount (); ++i)
	{
		// Determine the column name and the settings key
		QString columnName=columnInfo.columnName (i);
		QString key=QString ("columnWidth_%1").arg (columnName);

		// Determine the font metrics and the frame margin
		const QFont &font=horizontalHeader ()->font ();
		QFontMetrics metrics (font);
		QStyle *style=horizontalHeader ()->style ();
		if (!style) style=QApplication::style ();
		// Similar to QItemDelegate::textRectangle
		const int margin=style->pixelMetric (QStyle::PM_FocusFrameHMargin)+1;

		if (settings.contains (key))
		{
			// The settings contain a width for this column
			setColumnWidth (i, settings.value (key).toInt ());
		}
		else
		{
			// No width for this column in the settings. Determine the default.
			QString sampleText=columnInfo.sampleText (i);
			QString headerText=model ()->headerData (i, Qt::Horizontal).toString ();

			// The 2/4 were determined experimentally. Probably, some metric
			// should be used. For headerWidth, +2 is enough on Linux/Gnome,
			// but not on Windows XP.
			int sampleWidth=metrics.boundingRect (sampleText).width ()+2*margin+2;
			int headerWidth=metrics.boundingRect (headerText).width ()+2*margin+4;

			setColumnWidth (i, qMax (sampleWidth, headerWidth));
		}
	}
}

void SkTableView::writeColumnWidths (QSettings &settings, const ColumnInfo &columnInfo)
{
	assert (columnInfo.columnCount ()==model ()->columnCount ());

	for (int i=0; i<columnInfo.columnCount (); ++i)
	{
		QString columnName=columnInfo.columnName (i);
		QString key=QString ("columnWidth_%1").arg (columnName);
		int value=columnWidth (i);

		settings.setValue (key, value);
	}
}

void SkTableView::keyPressEvent (QKeyEvent *e)
{
	// Hack: it seems that as of Qt 4.6.2 (Ubuntu Lucid), QTableView consumes
	// the delete key, which is not passed to the parent widget (the containing
	// window). This only seems to happen with the delete key proper, not the
	// keypad delete key, even though both have a value of Qt::Key_Delete.
	// Ignore the delete key here to propagate it to the parent widget.
	switch (e->key ())
	{
		case Qt::Key_Delete: e->ignore (); break;
		default: QTableView::keyPressEvent (e);
	}
//
//	std::cout << "A key: " << e->key () << std::endl;
}

// Current cell changed - row (flight) or column
//void SkTableView::currentChanged (const QModelIndex &current, const QModelIndex &previous)
//{
//	QTableView::currentChanged (current, previous);
//}

// Selection changed - since selectionBehavior is SelectRows, this means that a
// different flight (or none) was selected
//void SkTableView::selectionChanged (const QItemSelection &selected, const QItemSelection &deselected)
//{
//	if (!selected.indexes ().isEmpty ())
//	{
//		QModelIndex index=selected.indexes ().first ();
//
//		// Set up the highlights color depending on the cell background color
//		QColor c=index.data (Qt::BackgroundRole).value<QBrush> ().color ();
//		c=interpol (0.5, c, Qt::black);
//		//c=interpol (0.5, c, Qt::white);
//		//c=QColor (63, 63, 63);
//		setStyleSheet (QString ("selection-background-color: %1;").arg (c.name ()));
//	}
//
//	QTableView::selectionChanged (selected, deselected);
//}
