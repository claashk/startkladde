#include "startart_t.h"

#include <cstdlib>

// Class management
void startart_t::init ()/*{{{*/
{
	id=invalid_id;
	towplane="";
	accelerator="";
	description="---";
	short_description="-";
	logbook_string="-";
	person_required=true;
	type=sat_other;
	ok=false;
}
/*}}}*/

startart_t::startart_t ()/*{{{*/
	:stuff ()
{
	init ();
}/*}}}*/

startart_t::startart_t (int _id, startart_type _type, string _towplane, string _description, string _short_description, string _accelerator, string _logbook_string, bool _person_required)/*{{{*/
	:stuff ()
{
	id=_id;
	type=_type;
	towplane=_towplane;
	description=_description;
	short_description=_short_description;
	accelerator=_accelerator;
	logbook_string=_logbook_string;
	person_required=_person_required;
}
/*}}}*/

startart_t::startart_t (string desc)/*{{{*/
	:stuff ()
{
	init ();

	QStringList split=QStringList::split (",", std2q (desc), true);
	for (QStringList::Iterator s=split.begin (); s!=split.end (); ++s)
		*s=(*s).simplifyWhiteSpace ();
	int n=split.count ();

	if (n>=0) id=atoi (split[0]);
	if (n>=1)
	{
		if (split[1].lower ()=="winch") type=sat_winch;
		else if (split[1].lower ()=="airtow") type=sat_airtow;
		else if (split[1].lower ()=="self") type=sat_self;
		else if (split[1].lower ()=="other") type=sat_other;
		else
		{
			log_error ("Unknown startart type in startart_t::startart_t (string)");
			type=sat_other;
		}
	}
	else
	{
		type=sat_other;
	}
	if (n>=2) towplane=q2std (split[2]);
	if (n>=3) description=q2std (split[3]);
	if (n>=4) short_description=q2std (split[4]);
	if (n>=5) accelerator=q2std (split[5]);
	if (n>=6) logbook_string=q2std (split[6]);
	if (n>=7 && split[7].lower ()=="false") person_required=false;
}
/*}}}*/

// Class information
string startart_t::name () const/*{{{*/
{
	return description;
}/*}}}*/

string startart_t::tabelle_name () const/*{{{*/
{
	return short_description;
}/*}}}*/

string startart_t::text_name () const/*{{{*/
{
	return description;
}/*}}}*/

string startart_t::bezeichnung (casus c) const/*{{{*/
{
	return stuff_bezeichnung (st_startart, c);
}/*}}}*/



string startart_t::get_selector_value (int column_number) const/*{{{*/
{
	switch (column_number)
	{
		case 0: return description;
		case 1: return bemerkungen;
		case 2: return num_to_string (id);
		default: return string ();
	}
}
/*}}}*/

string startart_t::get_selector_caption (int column_number)/*{{{*/
{
	switch (column_number)
	{
		case 0: return "Bezeichnung";
		case 1: return "Bemerkungen";
		case 2: return "(ID)";
		default: return string ();
	}
}
/*}}}*/



string startart_t::list_text () const/*{{{*/
{
	if (accelerator.empty ())
		return string (" ")+description;
	else
		return string (accelerator)+string (" - ")+description;
}
/*}}}*/


string startart_type_string (startart_type t)/*{{{*/
{
	switch (t)
	{
		case sat_winch: return "Windenstart"; break;
		case sat_airtow: return "F-Schlepp"; break;
		case sat_self: return "Eigenstart"; break;
		case sat_other: return "Sonstige"; break;
	}

	return "???";
}
/*}}}*/

void startart_t::output (ostream &stream, output_format_t format)/*{{{*/
{
	stuff::output (stream, format, false, "ID", id);
	stuff::output (stream, format, false, "Bezeichnung", description);
	stuff::output (stream, format, true, "Typ", startart_type_string (type));
}
/*}}}*/

