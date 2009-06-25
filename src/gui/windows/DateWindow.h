#ifndef _DateWindow_h
#define _DateWindow_h

#include <QDateEdit>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDialog>
#include <QLabel>
#include <QPushButton>

#include "src/gui/spacing.h"

/*
 * A date or (date and time) editor window.
 */
class DateWindow:public QDialog
{
	Q_OBJECT

	public:
		DateWindow (QWidget *parent, QDate datum);
		DateWindow (QWidget *parent, QDateTime dt);

	protected:
//		void resizeEvent (QResizeEvent *);

	private:
		QPushButton *but_ok, *but_cancel;
		QDateEdit *edit_datum;
		QTimeEdit *edit_zeit;
		QLabel *lbl_datum, *lbl_zeit;
		void init_common ();

	public slots:
		void edit_date ();
		void edit_datetime ();
		int exec ();

	private slots:
		void slot_ok ();
		void slot_cancel ();

	signals:
		void date_accepted (QDate);	// Emitted when the OK button is pressed.
		void datetime_accepted (QDateTime);	// Emitted when the OK button is pressed.
};

#endif

