#include "flugbuch.h"

#include "src/model/startart_t.h"

flugbuch_entry::flugbuch_entry ()/*{{{*/
{
	invalid=false;
}/*}}}*/

string flugbuch_entry::tag_string () const/*{{{*/
{
	return q2std (tag.toString ("yyyy-MM-dd"));
}
/*}}}*/

string flugbuch_entry::zeit_start_string (bool no_letters) const/*{{{*/
{
	return zeit_start.to_string ("%H:%M", tz_utc, 0, no_letters);
}
/*}}}*/

string flugbuch_entry::zeit_landung_string (bool no_letters) const/*{{{*/
{
	return zeit_landung.to_string ("%H:%M", tz_utc, 0, no_letters);
}
/*}}}*/

string flugbuch_entry::flugdauer_string () const/*{{{*/
{
	return flugdauer.to_string ("%H:%M", tz_timespan);
}
/*}}}*/



void make_flugbuch_person (QPtrList<flugbuch_entry> &fb, sk_db *db, QDate date, sk_person *person, QPtrList<sk_flug> &flights, flugbuch_entry::flight_instructor_mode fim)/*{{{*/
	// flights may contain flights which don't belong to the person
	// TODO pass list of planes here?
	// TODO this is slow because it needs to query the database for persons
	// TODO startart dito
	// pass an invalid/null date to ignore.
{
	// TODO! this should use flight_data

	flight_list interesting_flights; interesting_flights.setAutoDelete (false);

	// We use only flights where both the person and the date matches. Make a
	// list of these flights ("interesting flights").
	for (QPtrListIterator<sk_flug> flight (flights); *flight; ++flight)
	{
		// First condition: person matches.
		// This means that the person given is the pilot, or (in case of
		// certain flight instructor modes, the flight instructor, which is the
		// copilot).
		bool person_match=false;
		if ((*flight)->pilot==person->id) person_match=true;
		else if (fim==flugbuch_entry::fim_loose && (*flight)->begleiter==person->id) person_match=true;
		else if (fim==flugbuch_entry::fim_strict && (*flight)->begleiter==person->id && (*flight)->flugtyp==ft_schul_2) person_match=true;

		// Second condition: date matches.
		// This means that, if the date is given, it must match the flight's
		// effective date.
		bool date_match=false;
		if (!date.isValid ()) date_match=true;
		else if ((*flight)->effdatum ()==date) date_match=true;

		if (person_match && date_match)
			interesting_flights.append (*flight);
	}
	interesting_flights.sort ();

	// Iterate over all interesting flights, generating logbook entries.
	for (QPtrListIterator<sk_flug> flight (interesting_flights); *flight; ++flight)
	{
		// TODO Move to flugbuch_entry class
		flugbuch_entry *fb_entry=new flugbuch_entry;

		// Get additional data
		// TODO error checking
		sk_flugzeug fz; db->get_plane (&fz, (*flight)->flugzeug);

		// The person we are checking may either be pilot (regular) or copilot
		// (flight instructor). Thus, both of these may be a different person.

		sk_person pilot, begleiter;

		// If the pilot is the person we're checking, copy it. If not, get it
		// from the database.
		if ((*flight)->pilot==person->id)
			pilot=*person;
		else
			// TODO error checking
			db->get_person (&pilot, (*flight)->pilot);

		// Same for copilot
		if ((*flight)->begleiter==person->id)
			begleiter=*person;
		else
			// TODO error checking
			db->get_person (&begleiter, (*flight)->begleiter);

		startart_t sa; db->get_startart (&sa, (*flight)->startart);

		fb_entry->tag=(*flight)->effdatum ();
		fb_entry->muster=fz.typ;
		fb_entry->registration=fz.registration;
		fb_entry->flugzeugfuehrer=pilot.name ();
		fb_entry->begleiter=begleiter.name ();
		fb_entry->startart=sa.get_logbook_string ();
		fb_entry->ort_start=(*flight)->startort;
		fb_entry->ort_landung=(*flight)->zielort;
		fb_entry->zeit_start=(*flight)->startzeit;
		fb_entry->zeit_landung=(*flight)->landezeit;
		fb_entry->flugdauer=(*flight)->flugdauer ();
		fb_entry->bemerkung=(*flight)->bemerkungen;

		if (!(*flight)->finished ())
		{
			fb_entry->invalid=true;
		}

		fb.append (fb_entry);
	}
}
/*}}}*/

void make_flugbuch_day (QPtrList<flugbuch_entry> &fb, sk_db *db, QDate date)/*{{{*/
	// Make all flugbuchs for one day
{
	// TODO error handling

	QPtrList<sk_person> persons; persons.setAutoDelete (true);
	// Find out which persons had flights today
	db->list_persons_date (persons, &date);

	// Sort the persons
	// TODO this uses manual selection sort. Better use the heap sort provided
	// by QPtrList.
	persons.setAutoDelete (false);
	QPtrList<sk_person> sorted_persons; sorted_persons.setAutoDelete (true);
	while (!persons.isEmpty ())
	{
		sk_person *smallest=NULL;
		// Find the smallest element.
		for (QPtrListIterator<sk_person> person (persons); *person; ++person)
		{
			if (!smallest)
				// No smallest entry set yet (first element in list)
				smallest=*person;
			else if ((*person)->nachname<smallest->nachname)
				// Last name smaller
				smallest=*person;
			else if ((*person)->nachname==smallest->nachname && (*person)->vorname<smallest->vorname)
				// Last name identical, first name smaller
				smallest=*person;
		}
		sorted_persons.append (smallest);
		persons.remove (smallest);
	}

	QPtrList<sk_flug> flights; flights.setAutoDelete (true);
	// We need all flights of that date anyway. For speed, we don't query the
	// database for each person but retrieve all flights here.
	db->list_flights_date (flights, &date);

	for (QPtrListIterator<sk_person> person (sorted_persons); *person; ++person)
	{
		// TODO emit progress
		make_flugbuch_person (fb, db, date, *person, flights);
	}
}
/*}}}*/

