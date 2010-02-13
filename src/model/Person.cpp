#include "Person.h"

#include <iostream>
#include <cassert>

Person::Person ():
	Entity ()
{
	initialize ();
}

Person::Person (db_id id):
	Entity (id)
{
	initialize ();
}

void Person::initialize ()
{
}

bool Person::operator< (const Person &o) const
{
	if (nachname<o.nachname) return true;
	if (nachname>o.nachname) return false;
	if (vorname<o.vorname) return true;
	if (vorname>o.vorname) return false;
	return false;
}

QString Person::name () const
	/*
	 * Returns the name of the person in a form suitable for enumerations.
	 * Return value:
	 *   - the name.
	 */
{
	if (eintrag_ist_leer (nachname)&&eintrag_ist_leer (vorname)) return "-";
	if (eintrag_ist_leer (nachname)) return vorname;
	if (eintrag_ist_leer (vorname)) return nachname;
	return nachname+", "+vorname;
}

QString Person::full_name () const
{
	QString l=nachname; if (l.isEmpty ()) l="?";
	QString f= vorname; if (f.isEmpty ()) f="?";

	return f+" "+l;
}

QString Person::formal_name () const
{
	QString l=nachname; if (l.isEmpty ()) l="?";
	QString f= vorname; if (f.isEmpty ()) f="?";

	return l+", "+f;
}

QString Person::pdf_name () const
	/*
	 * Returns the name of the person in a form suitable for the PDF document.
	 * Return value:
	 *   - the name.
	 */
{
	return name ();
}

QString Person::tableName () const
	/*
	 * Returns the name of the person in a form suitable for the Table.
	 * Return value:
	 *   - the name.
	 */
{
	if (eintrag_ist_leer (club)) return name ();
	return name ()+" ("+club+")";
}

QString Person::textName () const
	/*
	 * Returns the name of the person in a form suitable for running text.
	 * Return value:
	 *   - the name.
	 */
{
	if (eintrag_ist_leer (nachname)&&eintrag_ist_leer (vorname)) return "-";
	if (eintrag_ist_leer (nachname)) return vorname;
	if (eintrag_ist_leer (vorname)) return nachname;
	return vorname+" "+nachname;
}

QString Person::get_selector_value (int column_number) const
{
	switch (column_number)
	{
		case 0: return nachname;
		case 1: return vorname;
		case 2: return club;
		case 3: return comments;
		case 4: return QString::number (id);
		default: return QString ();
	}
}

QString Person::get_selector_caption (int column_number)
{
	switch (column_number)
	{
		case 0: return "Nachname";
		case 1: return "Vorname";
		case 2: return "Verein";
		case 3: return "Bemerkungen";
		case 4: return "ID";
		default: return QString ();
	}
}

void Person::output (std::ostream &stream, output_format_t format)
{
	Entity::output (stream, format, false, "ID", id);
	Entity::output (stream, format, false, "Nachname", nachname);
	Entity::output (stream, format, false, "Vorname", vorname);
	Entity::output (stream, format, false, "Verein", club);
	Entity::output (stream, format, false, "Vereins-ID", club_id);
	Entity::output (stream, format, true, "Landesverbandsnummer", landesverbands_nummer);
}

// ******************
// ** ObjectModels **
// ******************

int Person::DefaultObjectModel::columnCount () const
{
	return 6;
}

QVariant Person::DefaultObjectModel::displayHeaderData (int column) const
{
	switch (column)
	{
		case 0: return "Nachname";
		case 1: return "Vorname";
		case 2: return "Verein";
		case 3: return "Landesverbandsnummer";
		case 4: return "Bemerkungen";
		// TODO remove from DefaultItemModel?
		case 5: return "ID";
	}

	assert (false);
	return QVariant ();
}

QVariant Person::DefaultObjectModel::displayData (const Person &object, int column) const
{
	switch (column)
	{
		case 0: return object.nachname;
		case 1: return object.vorname;
		case 2: return object.club;
		case 3: return object.landesverbands_nummer;
		case 4: return object.comments;
		case 5: return object.id;
	}

	assert (false);
	return QVariant ();
}

QString Person::toString () const
{
	return QString ("id=%1, lastName=%2, firstName=%3, club=%4, clubId=%5")
		.arg (id)
		.arg (nachname)
		.arg (vorname)
		.arg (club)
		.arg (club_id)
		;
}


// *******************
// ** SQL interface **
// *******************

QString Person::dbTableName ()
{
	return "person_temp";
}

QString Person::selectColumnList ()
{
	return "id,nachname,vorname,verein,vereins_id,bemerkung";
}

Person Person::createFromQuery (const QSqlQuery &q)
{
	Person p (q.value (0).toLongLong ());

	p.nachname =q.value (1).toString ();
	p.vorname  =q.value (2).toString ();
	p.club     =q.value (3).toString ();
	p.club_id  =q.value (4).toString ();
	p.comments =q.value (5).toString ();

	return p;
}

QString Person::insertValueList ()
{
	return "(nachname,vorname,verein,vereins_id,bemerkung) values (?,?,?,?,?)";
}

QString Person::updateValueList ()
{
	return "nachname=?, vorname=?, verein=?, vereins_id=?, bemerkung=?";
}

void Person::bindValues (QSqlQuery &q) const
{
	q.addBindValue (nachname);
	q.addBindValue (vorname);
	q.addBindValue (club);
	q.addBindValue (club_id);
	q.addBindValue (comments);
}


QList<Person> Person::createListFromQuery (QSqlQuery &q)
{
	QList<Person> list;

	while (q.next ())
		list.append (createFromQuery (q));

	return list;
}
