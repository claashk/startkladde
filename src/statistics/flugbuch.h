#ifndef flugbuch_h
#define flugbuch_h

/*
 * flugbuch
 * Martin Herrmann
 * 2004-09-28
 */

#include <QString>

// XXX
#include <q3ptrlist.h>
#define QPtrList Q3PtrList
#define QPtrListIterator Q3PtrListIterator
#include <QDateTime>

#include "src/flight_list.h"
#include "src/db/sk_db.h"
#include "src/model/Flight.h"
#include "src/model/Plane.h"
#include "src/model/Person.h"
#include "src/time/sk_time_t.h"

class flugbuch_entry
{
	public:
		enum flight_instructor_mode { fim_no, fim_strict, fim_loose };

		flugbuch_entry ();

		QDate tag;
		QString muster;
		QString registration;
		QString flugzeugfuehrer;
		QString begleiter;
		QString startart;
		QString ort_start;
		QString ort_landung;
		sk_time_t zeit_start;
		sk_time_t zeit_landung;
		sk_time_t flugdauer;
		QString bemerkung;

		bool invalid;

		QString tag_string () const;
		QString zeit_start_string (bool no_letters=false) const;
		QString zeit_landung_string (bool no_letters=false) const;
		QString flugdauer_string () const;
};

void make_flugbuch_day (QPtrList<flugbuch_entry> &fb, sk_db *db, QDate date);
void make_flugbuch_person (QPtrList<flugbuch_entry> &fb, sk_db *db, QDate date, Person *person, QPtrList<Flight> &flights, flugbuch_entry::flight_instructor_mode fim=flugbuch_entry::fim_no);

#endif

