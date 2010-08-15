/*
 * Improvements:
 *   - horizontal scrolling using the scroll bar (mouse) does not update the
 *     widget focus (updateWidgetFocus is not called)
 *   - updating the data (e. g. flight duration on minute change) recreates the
 *     buttons, even if not necessary; this is probably inefficient. Also, it
 *     necessitates determining the index of the focused button
 *     (focusWidgetIndex) beforehand and focusing the widget at that index
 *     afterwards (focusWidgetAt) (from MainWindow)
 *   - on every move, the current selection has to be searched for a visible
 *     button in order to set the focus; this could possibly be made more
 *     efficient
 */
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
#include "src/util/io.h" // remove

#include <iostream>
#include <cassert>

SkTableView::SkTableView (QWidget *parent):
	QTableView (parent),
	autoResizeRows (false),
	settingButtons (false),
	coloredSelectionEnabled (false)
{
	// Use a style sheet rather than a palette because a style may ignore the
	// palette, while a style sheet is guaranteed to be honored.

	setTabKeyNavigation (false);
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
//	std::cout << "key " << e->key () << "/" << e->modifiers () << " pressed in SkTableView" << std::endl;

	switch (e->key ())
	{
		// Hack: it seems that as of Qt 4.6.2 (Ubuntu Lucid), QTableView consumes
		// the delete key, which is not passed to the parent widget (the containing
		// window). This only seems to happen with the delete key proper, not the
		// keypad delete key, even though both have a value of Qt::Key_Delete.
		// Ignore the delete key here to propagate it to the parent widget.
		case Qt::Key_Delete: e->ignore (); break;
		case Qt::Key_Left: scrollLeft (); break;
		case Qt::Key_Right: scrollRight (); break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			e->ignore (); // Don't call base, it will accept it
			break;
		default:
			e->ignore (); // Propagate to parent widget (unless the QTableView accepts it)
			QTableView::keyPressEvent (e);
	}
}

// Current cell changed - row (flight) or column
//void SkTableView::currentChanged (const QModelIndex &current, const QModelIndex &previous)
//{
//	QTableView::currentChanged (current, previous);
//}

bool SkTableView::cellVisible (const QModelIndex &index)
{
	QRect viewportRect=viewport ()->rect ();
	QRect indexRect=visualRect (index);

	return
		viewportRect.contains (indexRect.topLeft ()) &&
		viewportRect.contains (indexRect.bottomRight ())
		;
}

QWidget *SkTableView::findVisibleWidget (const QModelIndexList &indexes)
{
	foreach (const QModelIndex &index, indexes)
	{
		QWidget *widget=indexWidget (index);

		if (widget)
			if (cellVisible (index))
				return widget;
	}

	return NULL;
}

/**
 * @param button may be NULL
 * @return
 */
QPersistentModelIndex SkTableView::findButton (TableButton *button)
{
	if (!button) return QPersistentModelIndex ();

	return button->getIndex ();
}

bool SkTableView::focusWidgetAt (const QModelIndex &index)
{
	if (!index.isValid ()) return false;

	QWidget *widget=indexWidget (index);
	if (!widget) return false;

	widget->setFocus ();
	return true;
}

void SkTableView::updateWidgetFocus (const QModelIndexList &indexes)
{
	// If the current selection contains a widget, focus it if it is visible
	QWidget *widget=findVisibleWidget (indexes);
	if (widget)
		widget->setFocus ();
	else
		this->setFocus ();
}

// Selection changed - since selectionBehavior is SelectRows, this means that a
// different flight (or none) was selected
void SkTableView::selectionChanged (const QItemSelection &selected, const QItemSelection &deselected)
{
	if (coloredSelectionEnabled)
	{
		// Set up the selection colors depending on the cell background color
		QColor backgroundColor;
		if (!selected.indexes ().isEmpty ())
		{
			QModelIndex index=selected.indexes ().first ();

			backgroundColor=index.data (Qt::BackgroundRole).value<QBrush> ().color ();

			// Flight color on dark gray in selected cells
			if (backgroundColor.isValid ())
				setStyleSheet (QString
					("selection-background-color: #3F3F3F; selection-color: %1;")
					.arg (backgroundColor.name ()));
		}

		// Fake border around selected
		// WARNING: in some styles (e. g. Gnome), gradients appear as solid black
//		setStyleSheet (QString (
//		    "selection-color: #000000; "
//		    "selection-background-color: "
//		    "qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
//		    "stop: 0 %1, stop: 0.2 %2, stop: 0.8 %3, stop: 1 %4);")
//		    .arg (QString ("#000000"))
//		    .arg (backgroundColor.name ())
//		    .arg (backgroundColor.name ())
//		    .arg (QString ("#000000")));

		// Vertical gradient in selected cells
		// WARNING: in some styles (e. g. Gnome), gradients appear as solid black
//		setStyleSheet (QString (
//		    "selection-color: #000000; "
//		    "selection-background-color: "
//		    "qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
//		    "stop: 0 %1, stop: 1 %2);")
//		    .arg (backgroundColor.darker (135).name())
//		    .arg (backgroundColor.lighter (135).name()));
	}

	updateWidgetFocus (selected.indexes ());
	QTableView::selectionChanged (selected, deselected);
}

void SkTableView::scrollLeft ()
{
	// The current row is selected completely
	// FIXME a flight may not have been selected
	QList<QModelIndex> indexes=selectionModel ()->selectedIndexes ();

	// Find the first visible cell and scroll to the one before it
	QModelIndex lastIndex;
	QListIterator<QModelIndex> i (indexes);
	while (i.hasNext ())
	{
		QModelIndex index=i.next ();
		if (cellVisible (index))
		{
			if (lastIndex.isValid ())
			{
				setCurrentIndex (lastIndex);
				updateWidgetFocus (selectionModel ()->selectedIndexes ());
			}
			return;
		}

		lastIndex=index;
	}

	updateWidgetFocus (selectionModel ()->selectedIndexes ());
}

void SkTableView::scrollRight ()
{
	// The current row is selected completely
	// FIXME a flight may not have been selected
	QList<QModelIndex> indexes=selectionModel ()->selectedIndexes ();

	// Find the first visible cell and scroll to the one before it
	QModelIndex lastIndex;
	QListIterator<QModelIndex> i (indexes);
	i.toBack ();
	while (i.hasPrevious ())
	{
		QModelIndex index=i.previous ();
		if (cellVisible (index))
		{
			if (lastIndex.isValid ())
			{
				setCurrentIndex (lastIndex);
				updateWidgetFocus (selectionModel ()->selectedIndexes ());
			}
			return;
		}

		lastIndex=index;
	}
}

void SkTableView::mouseDoubleClickEvent (QMouseEvent *event)
{
	if (indexAt (event->pos ()).isValid ())
		QTableView::mouseDoubleClickEvent (event);
	else
	{
//		emit emptySpaceDoubleClicked ();
		emit doubleClicked (QModelIndex ());
		event->accept ();
	}
}
