#include "sk_flight_table.h"
#include <qapplication.h>
#include <qfontmetrics.h>

#include "src/color.h"

sk_flight_table::sk_flight_table (sk_db *_db, QWidget *parent, const char *name)/*{{{*/
	:sk_table (parent, name)
	/*
	 * Constructs a sk_flight_table instance.
	 * Parameters:
	 *   - parent, name: passed on to base class constructor.
	 */
{
	db=_db;

	setNumCols (tabellenspalten);

	QHeader *table_header=horizontalHeader ();

	set_table_column (table_header, tbl_idx_registration,       "Kennz.",             "D-WWWW (WW)");
	set_table_column (table_header, tbl_idx_flugzeug_typ,       "Typ",                "DR-400/180");
	set_table_column (table_header, tbl_idx_flug_typ,           "Flugtyp",            "Normal");
	set_table_column (table_header, tbl_idx_pilot,              "Pilot/FS",           "Herrmann, Martin");
	set_table_column (table_header, tbl_idx_begleiter,          "Begleiter/FL",       "Herrmann, Martin");
	set_table_column (table_header, tbl_idx_startart,           "Startart",           "XXX");
	set_table_column (table_header, tbl_idx_startzeit,          "Start",              "  Starten  ");
	set_table_column (table_header, tbl_idx_landezeit,          "Landung",            "  Landen  ");
	set_table_column (table_header, tbl_idx_flugdauer,          "Dauer",              "00:00");
	set_table_column (table_header, tbl_idx_landungen,          "Ldg.",               "00");
	set_table_column (table_header, tbl_idx_startort,           "Startort",           "Rheinstetten");
	set_table_column (table_header, tbl_idx_zielort,            "Zielort",            "Rheinstetten");
	set_table_column (table_header, tbl_idx_bemerkungen,        "Bemerkungen",        "Seilriss�bung");
	set_table_column (table_header, tbl_idx_abrechnungshinweis, "Abrechnungshinweis", "Bezahlt");
	set_table_column (table_header, tbl_idx_editierbar,         "Editierbar",         "Nein");
	set_table_column (table_header, tbl_idx_datum,              "Datum",              "0000-00-00");
	set_table_column (table_header, tbl_idx_id_display,         "ID",                 "9999");


	gelandete_ausblenden=true;
	weggeflogene_gekommene_anzeigen=true;
	fehlerhafte_immer=true;

	// MURX because there are problems with big fonts/resolutions
	sk_flug flug;
	flug.gestartet=true;
	flug.gelandet=false;
	flug.modus=fmod_lokal;

	set_flight  (-1, &flug, 0, false);
	adjustRow (0);
	row_height=rowHeight (0);
	setNumRows (0);
}/*}}}*/

QWidget *sk_flight_table::beginEdit ( int row, int col, bool replace )/*{{{*/
	/*
	 * Returns NULL to signify that the table data cannot be edited.
	 * Paramters:
	 *   - see QT documentation.
	 * Return value:
	 *   - see QT documentation.
	 */
{
	return NULL;
}/*}}}*/






db_id sk_flight_table::id_from_row (int row)/*{{{*/
	/*
	 * Gets the ID saved in a given row.
	 * Parameters:
	 *   - the number of the row.
	 * Return value:
	 *   - the ID saved in that row.
	 */
{
	return id_from_cell (row, tbl_idx_id);
}/*}}}*/

db_id sk_flight_table::schleppref_from_row (int row)/*{{{*/
	/*
	 * Gets the !!Schleppref saved in a given row.
	 * Parameters:
	 *   - the number of the row.
	 * Return value:
	 *   - the !!Schleppref saved in that row.
	 */
{
	return id_from_cell (row, tbl_idx_schleppref);
}/*}}}*/



int sk_flight_table::row_from_id (db_id id)/*{{{*/
	/*
	 * Gets the row where a given ID is saved.
	 * Parameters:
	 *   - the ID to search for.
	 * Return value:
	 *   - the row where this ID is saved.
	 */
{
	return row_from_column_id (id, tbl_idx_id);
}/*}}}*/

int sk_flight_table::row_from_sref (db_id sref)/*{{{*/
	/*
	 * Gets the row where a given !!Schleppref is saved.
	 * Parameters:
	 *   - the !!Schleppref to search for.
	 * Return value:
	 *   - the row where this !!Schleppref is saved.
	 */
{
	return row_from_column_id (sref, tbl_idx_schleppref);
}/*}}}*/

bool sk_flight_table::row_is_flight (int row)/*{{{*/
	/*
	 * Check if a column of the table contains a flight.
	 * Parameters:
	 *   - row: a row number.
	 * Return value:
	 *   true if the row contains a flight.
	 */
{
	if (id_from_row (row)>0)
	{
		return true;
	}
	if (schleppref_from_row (row)>0)
	{
		return true;
	}
	return false;
}/*}}}*/



sk_button *sk_flight_table::set_button_or_text (int row, int column, bool set_button, QString text, QColor bg, db_id data)/*{{{*/
	/*
	 * Sets a cell to a button or a text.
	 * Parameters:
	 *   - row, column: the coordinates of the cell to set.
	 *   - set_button: whether to set a button (true) or a text (false).
	 *   - text: the text or button caption to set.
	 *   - bg: the background color for the text.
	 *   - data: the additional data to save in the cell.
	 */
{
	// TODO: die hier als set_cell_button, clearCellWidget in ::set_cell

	// Hier muss unbedingt clearCell stehen, sonst ist Text, der durch einen
	// Button/Text ersetzt wurde, beim n�chsten �berfahren mit dem cursor
	// wieder sichtbar. M�glicherweise ein Bug in sk_table::set_cell () oder
	// in QT.
	clearCell (row, column);
	clearCellWidget (row, column);

	if (set_button)
	{
		sk_button *button=new sk_button (data, text, this, "Tabellenbutton");
		setCellWidget (row, column, button);
		return button;
	}
	else
	{
		set_cell (row, column, text, bg);
		return NULL;
	}
}/*}}}*/

sk_button *sk_flight_table::set_button_or_text (int row, int column, bool set_button, string text, QColor bg, db_id data)/*{{{*/
	/*
	 * Overloaded function using a QString instead of a std::string.
	 */
{
	return set_button_or_text (row, column, set_button, std2q (text), bg, data);
}/*}}}*/

sk_button *sk_flight_table::set_button_or_text (int row, int column, bool set_button, const char *text, QColor bg, db_id data)/*{{{*/
	/*
	 * Overloaded function using a char* instead of a std::string.
	 */
{
	return set_button_or_text (row, column, set_button, QString (text), bg, data);
}/*}}}*/



void sk_flight_table::set_cell_by_type (int row, int column, zell_typ typ, char *button_text, string zeit_text, QColor bg, db_id data, const char *signal_target)/*{{{*/
	/*
	 * Sets a cell according to a cell type given.
	 * Parameters:
	 *   - row, column: the coordinates of the cell to set.
	 *   - typ: the type of the cell to set.
	 *   - button_text: the caption if typ==zt_button.
	 *   - zeit_text: the text if typ==zt_zeit.
	 *   - bg: the background color for txt entries.
	 *   - data: the additional data to save.
	 *   - signal_target: the slot to connect the button click if
	 *     typ==zt_button.
	 */
{
	sk_button *but;

	switch (typ)
	{
		case zt_program_error:
			log_error ("Zelltyp 'zt_program_error' in sk_flight_table::set_cell_by_type ()");
			set_button_or_text (row, column, false, "!!!", bg, 0);
			break;
		case zt_unhandled:
			log_error ("Zelltyp 'zt_unhandled' in sk_flight_table::set_cell_by_type ()");
			set_button_or_text (row, column, false, "!!!", bg, 0);
			break;
		case zt_empty:
			set_button_or_text (row, column, false, "", bg, 0);
			break;
		case zt_n_a:
			set_button_or_text (row, column, false, "-", bg, 0);
			break;
		case zt_button:
			if (button_text)
			{
				but=set_button_or_text (row, column, true, button_text, bg, data);
				QObject::connect (but, SIGNAL (clicked (db_id)), this, signal_target);
			}
			else
			{
				log_error ("Button ohne Buttontext in sk_flight_table::set_cell_by_type ()");
				set_button_or_text (row, column, false, "!!!", bg, 0);
			}
			break;
		case zt_time:
			set_button_or_text (row, column, false, zeit_text, bg, 0);
			break;
		case zt_missing: case zt_invalid:
			set_button_or_text (row, column, false, "???", bg, 0);
			break;
		default:
			log_error ("Unbehandelter Zelltyp in sk_flight_table::set_cell_by_type ()");
			set_button_or_text (row, column, false, "!!!", bg, 0);
			break;
	}
}/*}}}*/



int sk_flight_table::insert_row_for_flight (sk_flug *f)/*{{{*/
	/*
	 * Insert a row at the correct position for a given flight.
	 * Parameters:
	 *   - f: the flight to look at.
	 *   - return value: the number of the row inserted.
	 */
{
	// TODO this function needs to look at the times saved in the
	// table. It also has to handle tow flights correctly (insert
	// under the towed flight).
	int row;

	if (f->vorbereitet ())
	{
		// Flight is prepared. Append to the end.
		// The order of multiple prepared flights is undefined.
		row=numRows ();
	}
	else
	{
		// Flight has happened.
		row=numRows ();
		for (int r=0; r<numRows (); r++)
		{
			// TODO this fails for coming prepared flights because
			// they don't have a start button (but we can't look at
			// the land button unless we know the mode).
			if (cellWidget (r, tbl_idx_startzeit)!=0)
			{
				// widget ==> start button ==> prepared
				row=r; break;
			}
		}
	}

	insertRows (row);
	return row;
}/*}}}*/

void sk_flight_table::set_flight (int row, sk_flug *f, db_id id, bool set_schlepp)/*{{{*/
	/*
	 * Writes a given flight to a given row in the table.
	 * Parameters:
	 *   - row: The number of the row. If 0, a line is created at the correct
	 *     position, as defined in insert_row_for_flight ().
	 *   - *f: the flight to enter.
	 *   - id: The database ID of the flight.	// TODO read from f?
	 *   - set_schlepp: wheter to set the !!Schleppflug (true) or the actual
	 *     flight (false).
	 */
{
	if (row<0) row=insert_row_for_flight (f);

	// Startart lesen
	startart_t startart;
	// TODO error checking (ex. startart invalid)
	db->get_startart (&startart, f->startart);

	// Determine the plane to be shown
	sk_flugzeug fz;
	sk_flugzeug sfz;
	bool fz_ok=(db->get_plane (&fz, f->flugzeug)==db_ok);
	bool sfz_ok=(db_ok==db->get_towplane (&sfz, startart, f->towplane));
	sk_flugzeug *eff_plane=NULL;
	bool eff_plane_ok=false;

	if (set_schlepp)
	{
		eff_plane=&sfz;
		eff_plane_ok=sfz_ok;
	}
	else
	{
		eff_plane=&fz;
		eff_plane_ok=fz_ok;
	}

	QColor bg;

	// Zustand des Flugs
	bool eff_gestartet=f->gestartet;
	bool eff_gelandet=set_schlepp?f->sfz_gelandet:f->gelandet;
	bool eff_fehlerhaft=set_schlepp?f->schlepp_fehlerhaft (&fz, &sfz, &startart):f->fehlerhaft (&fz, &sfz, &startart);
	sk_time_t eff_startzeit=f->startzeit;
	sk_time_t eff_landezeit=set_schlepp?f->landezeit_schleppflugzeug:f->landezeit;
	sk_time_t eff_flugdauer=set_schlepp?f->schleppflugdauer ():f->flugdauer ();
	bool editierbar=f->editierbar;

	// Modus
	flug_modus eff_modus=set_schlepp?f->modus_sfz:f->modus;
	bool eff_starts_here=starts_here (eff_modus);
	bool eff_lands_here=lands_here (eff_modus);

	// Signale f�r die Buttons
	const char *start_signal=
		SIGNAL (signal_button_start (db_id));
	const char *landung_signal=set_schlepp?
		SIGNAL (signal_button_schlepplandung (db_id)):
		SIGNAL (signal_button_landung (db_id));

	// Hintergrundfarbe
	bg=flug_farbe (eff_modus, eff_fehlerhaft, set_schlepp, eff_gestartet, eff_gelandet);

	// Referenzitems
	sk_table_item *id_item, *sref_item, *startzeit_item;

	// Wenn gespeicherte Startzeit existiert, l�schen
	startzeit_item=(sk_table_item *)item (row, tbl_idx_startzeit);
	if (startzeit_item)
	{
		delete (sk_time_t *)startzeit_item->get_data ();
		startzeit_item->set_data (NULL);
	}

	// Bestimmen, was in den Zeit-Zellen steht
	zell_typ start_zelle=zt_unhandled;
	zell_typ landung_zelle=zt_unhandled;
	zell_typ dauer_zelle=zt_unhandled;

	// TODO: Ankreuzfelder, evtl auch ein Makro, das
	// vorbereitet/fliegt/gelandet/nur_gelandet nimmt, oder eines, das
	// start/landung/dauer nimmt, dazu in jedem fall modus,schlepp/flug.
	// TODO: every check of started must also check starts_here

	// �bersichtstabelle:
	// X: Flug {Start, Land, Dauer}, Schlepp {Start, Land, Dauer}
	// Y: Lokal {-,/,\,/\}, Kommt {...}, Geht {...}

	// TODO unhandled cases with flights without modus

	string startbutton_text="!";
	string landebutton_text="!";

	if (
			eff_modus==fmod_lokal && !set_schlepp	||
			eff_modus==fmod_lokal && set_schlepp	||
			eff_modus==fmod_geht && set_schlepp)
	{
		/*
		 * SL:	Start		Landung		Dauer
		 * --:	"Starten"	""			""
		 * /-:	"xx:xx"		"Landen"	"xx:xx"
		 *             		"Ende"
		 * -\:	"???"  		"xx:xx"		"???"
		 * /\:	"xx:xx"		"xx:xx"		"xx:xx"
		 */
		start_zelle=eff_gestartet?zt_time:eff_gelandet?zt_missing:zt_button;
		landung_zelle=eff_gelandet?zt_time:eff_gestartet?zt_button:zt_empty;
		dauer_zelle=eff_gestartet?zt_time:eff_gelandet?zt_missing:zt_empty;
		startbutton_text="Starten";
		landebutton_text=(set_schlepp && !eff_lands_here)?"Ende":"Landen";
	}
	else if (
			eff_modus==fmod_kommt && set_schlepp)
	{
		// Programmfehler: kommende Schlepps werden nicht geschrieben
		start_zelle=zt_program_error;
		landung_zelle=zt_program_error;
		dauer_zelle=zt_program_error;
	}
	else if (
			eff_modus==fmod_kommt && !set_schlepp)
	{
		/*
		 * SL:	Start	Landung		Dauer
		 * --:	"-"		"Landen"	"-"
		 * /-:	"-"		"Landen"	"-"
		 * -\:	"-"		"xx:xx"		"-"
		 * /\:	"-"		"xx:xx"		"-"
		 */
		start_zelle=zt_n_a;
		landung_zelle=eff_gelandet?zt_time:zt_button;
		dauer_zelle=zt_n_a;
		landebutton_text="Landen";
	}
	else if (
			eff_modus==fmod_geht && !set_schlepp)
	{
		/*
		 * SL:	Start		Landung	Dauer
		 * --:	"Starten"	"-"		"-"
		 * /-:	"xx:xx"		"-"		"-"
		 * -\:	"Starten"	"-"		"-"
		 * /\:	"xx:xx"		"-"		"-"
		 */
		start_zelle=eff_gestartet?zt_time:zt_button;
		landung_zelle=zt_n_a;
		dauer_zelle=zt_n_a;
		startbutton_text="Starten";
	}

	// Special error handling
	// Schlepps, die noch nicht gestartet sind, stehen nicht in der Tabelle
	if (set_schlepp && eff_starts_here && !eff_gestartet)
		start_zelle=landung_zelle=dauer_zelle=zt_program_error;

	// If not editable or display date is not today, display no buttons
	if (!editierbar || anzeigedatum!=QDate::currentDate ())
	{
		if (start_zelle==zt_button) start_zelle=zt_missing;
		if (landung_zelle==zt_button) landung_zelle=zt_missing;
		if (dauer_zelle==zt_button) dauer_zelle=zt_missing;
	}

	// Set the time cells
	set_cell_by_type (row, tbl_idx_startzeit, start_zelle, (char *)startbutton_text.c_str (), eff_startzeit.table_string (), bg, id, start_signal);
	set_cell_by_type (row, tbl_idx_landezeit, landung_zelle, (char *)(landebutton_text.c_str ()), eff_landezeit.table_string (), bg, id, landung_signal);
	string fdstring=eff_flugdauer.table_string (tz_none);
	set_cell_by_type (row, tbl_idx_flugdauer, dauer_zelle, NULL, fdstring.c_str (), bg, id, NULL);

	if (set_schlepp)
	{
		// Schleppflug eintragen

		// Flugzeug aus der Datenbank lesen
		// TODO error handling
		if (startart.is_airtow ())
		{
			if (opts.record_towpilot)
			{
				sk_person towpilot;
				string towpilot_eintrag;

				// Eintrag f�r den Schlepppiloten festlegen
				if (id_invalid (f->towpilot))
					towpilot_eintrag=f->unvollst_towpilot_name ();
				else if (db->get_person (&towpilot, f->towpilot)!=0)
					towpilot_eintrag="???";
				else
					towpilot_eintrag=towpilot.tabelle_name ();

				set_cell (row, tbl_idx_pilot, towpilot_eintrag, bg);
			}
			else
			{
				set_cell (row, tbl_idx_pilot, "-", bg);
			}
			set_cell (row, tbl_idx_begleiter, "-", bg);
			set_cell (row, tbl_idx_zielort, f->zielort_sfz, bg);
			set_cell (row, tbl_idx_bemerkungen, "(siehe geschleppter Flug)", bg);
			set_cell (row, tbl_idx_abrechnungshinweis, "(siehe geschleppter Flug)", bg);
			if (lands_here (f->modus_sfz))
				set_cell (row, tbl_idx_landungen, f->sfz_gelandet?"1":"0", bg);
			else
				set_cell (row, tbl_idx_landungen, "-", bg);
			// CONFIGURATION ls_tablle vs. ls_kurz
			set_cell (row, tbl_idx_flug_typ, flugtyp_string (ft_schlepp, ls_tabelle), bg);
			startart_t ss; bool ss_ok=(db->get_startart_by_type (&ss, sat_self)==db_ok);
			set_cell (row, tbl_idx_startart, ss_ok?ss.get_short_description ():"?", bg);
			set_cell (row, tbl_idx_id_display, "("+QString::number (id)+")", bg);
		}
	}
	else
	{
		// Flug eintragen
		sk_person pilot, begleiter;
		string pilot_eintrag;
		string begleiter_eintrag;

		// Eintrag f�r den Piloten festlegen
		if (id_invalid (f->pilot))
			pilot_eintrag=f->unvollst_pilot_name ();
		else if (db->get_person (&pilot, f->pilot)!=0)
			pilot_eintrag="???";
		else
			pilot_eintrag=pilot.tabelle_name ();

		// TODO hier f->begleiter->bezeichnung () verwenden?
		if (f->flugtyp==ft_gast_privat)
			begleiter_eintrag="(Gast P)";
		else if (f->flugtyp==ft_gast_extern)
			begleiter_eintrag="(Gast E)";
		else if (f->flugtyp==ft_schul_1)
			begleiter_eintrag="-";
		else if (id_invalid (f->begleiter))
			// TODO nicht unbedingt unvollst�ndig, vielleicht auch einfach nicht vorhanden.
			begleiter_eintrag=f->unvollst_begleiter_name ();
		else if (db->get_person (&begleiter, f->begleiter)!=0)
			begleiter_eintrag="???";
		else
			begleiter_eintrag=begleiter.tabelle_name ();

		set_cell (row, tbl_idx_pilot, pilot_eintrag, bg);
		set_cell (row, tbl_idx_begleiter, begleiter_eintrag, bg);

		set_cell (row, tbl_idx_zielort, f->zielort, bg);
		set_cell (row, tbl_idx_bemerkungen, f->bemerkungen, bg);
		set_cell (row, tbl_idx_abrechnungshinweis, f->abrechnungshinweis, bg);
		if (lands_here (f->modus))
			set_cell (row, tbl_idx_landungen, QString::number (f->landungen), bg);
		else
			set_cell (row, tbl_idx_landungen, "-", bg);

		// CONFIGURATION ls_tablle vs. ls_kurz
		set_cell (row, tbl_idx_flug_typ, f->typ_string (ls_tabelle), bg);
		if (!starts_here (f->modus))
			set_cell (row, tbl_idx_startart, "-", bg);
		else if (id_invalid (f->startart))
			set_cell (row, tbl_idx_startart, "--", bg);
		else if (startart.ok)
		{
			if (sfz_ok && startart.is_airtow () && !startart.towplane_known ())
			{
				// An unknown airtow. Display the toplane's registration.
				set_cell (row, tbl_idx_startart, sfz.registration, bg);
			}
			else
			{
				// A known startart. Display its short description.
				set_cell (row, tbl_idx_startart, startart.get_short_description (), bg);
			}
		}
		else
			set_cell (row, tbl_idx_startart, "?", bg);

		set_cell (row, tbl_idx_id_display, QString::number (id), bg);
	}

	// Gemeinsames f�r Flug und Schleppflug
	string registration_string;
	string typ_string;
	if (eff_plane_ok)
	{
		registration_string=eff_plane->tabelle_name (set_schlepp);
		typ_string=eff_plane->typ;
	}
	else
	{
		registration_string="???";
		typ_string="???";
	}

	set_cell (row, tbl_idx_registration, registration_string, bg);
	set_cell (row, tbl_idx_flugzeug_typ, typ_string, bg);
	set_cell (row, tbl_idx_startort, f->startort, bg);
	set_cell (row, tbl_idx_editierbar, f->editierbar?"Ja":"Nein", bg);
	QDate dat=f->effdatum (tz_utc);
	QString dat_string;
	// MURX: isNull tut nicht. Man soll keine QT-Klassen verwenden.
	if (!f->happened () || (dat.year ()==1970 && dat.month ()==dat.day ()==1))
		dat_string="-";
	else
	{
		// MURX: hier sollte ein table_string () hin. Man soll keine QT-Klassen verwenden.
		QString y_string=QString::number (dat.year ());
		QString m_string=QString::number (dat.month ());
		QString d_string=QString::number (dat.day ());
		if (m_string.length ()<2) m_string="0"+m_string;
		if (d_string.length ()<2) d_string="0"+d_string;
		dat_string=y_string+"-"+m_string+"-"+d_string;
	}

	set_cell (row, tbl_idx_datum, dat_string, bg);

	// Startzeit in der Startzeit-Zelle abspeichern
	startzeit_item=(sk_table_item *)item (row, tbl_idx_startzeit);
	if (startzeit_item)
	{
		if (f->gestartet)
			startzeit_item->set_data (new sk_time_t (eff_startzeit));
		else
			startzeit_item->set_data (NULL);
	}

	// ID/Schleppref eintragen
	id_item=(sk_table_item *)item (row, tbl_idx_id);
	sref_item=(sk_table_item *)item (row, tbl_idx_schleppref);
	if (id_item) id_item->set_id (set_schlepp?0:id);
	if (sref_item) sref_item->set_id (set_schlepp?id:0);

	update_row_time (row);

	// MURX
	setRowHeight (row, row_height);
}/*}}}*/

void sk_flight_table::update_flight (db_id id, sk_flug *f)/*{{{*/
	/*
	 * Updates the flight data in the table, add the flight if it is not in
	 * the table yet, or remove it if it is not to be shown (for example, if it
	 * is landed).
	 * Parameters:
	 *   - id: the database ID of the flight. // TODO read from f?
	 *   - f: The flight data.
	 */
{
	bool flug_anzeigen=true;
	bool schlepp_anzeigen=true;
	int row=-1;

	// TODO error handling
	startart_t startart;
	db->get_startart (&startart, f->startart);

	sk_flugzeug fz;
	db->get_plane (&fz, f->flugzeug);

	sk_flugzeug sfz;
	db->get_towplane (&sfz, startart, f->towplane);

	// ****** First, we determine whether the flight/tow is to be shown.
	{
		// Wenn schon gelandet oder weggeflogen und gestartet, gem��
		// Einstellung nicht anzeigen.
		if (gelandete_ausblenden)
		{
			// TODO das hier ist eigentlich "flug erledigt" (!=stattgefunden)
			if (f->gelandet) {
				flug_anzeigen = false;
			}
			if (f->modus==fmod_geht && f->gestartet)
				flug_anzeigen=false;
			// Bei gehenden Schleppfl�gen hat 'gelandet' die Bedeutung 'Schlepp
			// beendet', ansonsten 'Schleppflugzeug gelandet'
			if (f->sfz_gelandet)
				schlepp_anzeigen=false;
		}

		// Wenn geht oder kommt, gem�� Einstellung doch anzeigen
		if (weggeflogene_gekommene_anzeigen)
		{
			if (f->modus==fmod_geht || f->modus==fmod_kommt) flug_anzeigen=true;
			if (f->modus_sfz==fmod_geht) schlepp_anzeigen=true;
		}

		// Wenn nicht an Anzeigedatum, nicht anzeigen
		if (f->gestartet && f->effdatum (tz_utc)!=anzeigedatum) flug_anzeigen=schlepp_anzeigen=false;

		// Wenn noch nicht gestartet, Schleppflug nicht anzeigen.
		if (!f->gestartet) schlepp_anzeigen=false;

		// Wenn fehlerhaft, u. U. immer anzeigen (fehlerhafte_immer greift nur bei
		// editierbaren Fl�gen)
		if (fehlerhafte_immer && f->editierbar && f->fehlerhaft (&fz, &sfz, &startart)) flug_anzeigen=true;
		if (fehlerhafte_immer && f->editierbar && f->schlepp_fehlerhaft (&fz, &sfz, &startart)) schlepp_anzeigen=true;

		if (startart.is_airtow ())
		{
			// Pr�fung auf Fehler im Schlepp nur bei F-Schlepps
			if (fehlerhafte_immer && f->editierbar && f->schlepp_fehlerhaft (&fz, &sfz, &startart)) schlepp_anzeigen=true;
		}
		else
		{
			// Wenn kein F-Schlepp, Schleppflug nicht anzeigen
			schlepp_anzeigen=false;
		}

		// Wenn vorbereitet und nicht editierbar, nicht anzeigen.
		if (f->vorbereitet () && !f->editierbar)
			flug_anzeigen=false;

		// Vorbereitete nur bei aktuellem Anzeigedatum anzeigen.
		if (f->vorbereitet () && anzeigedatum!=QDate::currentDate ())
			flug_anzeigen=false;
	}


	// ***** Then, we (add or remove) the (flight and tow) (to or from) the
	// table, depending on what we just found out and wheter it is already
	// present.
	{
		// Pr�fen, ob der Flug bereits in der Tabelle steht.
		row=row_from_id (id);
		//sk_table_item* item0 = (sk_table_item*)item (row, 0);
		//bool timerActive = (item0 ? item0->isTimerActive() : false);
		//qDebug () << "item: " << item0 << endl;
		//if (item0)
		//	qDebug() << "timer active: " << item0->isTimerActive() << endl;
		if (flug_anzeigen /* || timerActive */ )
		{
			set_flight (row, f, id, false);
		}
		else
		{
			if (row>=0) removeRow (row);
		}

		// Pr�fen, ob der Schlepp bereits in der Tabelle steht
		row=row_from_sref (id);
		if (schlepp_anzeigen)
		{
			set_flight (row, f, id, true);
		}
		else
		{
			if (row>=0) removeRow (row);
		}
	}
}/*}}}*/

void sk_flight_table::removeRow (int row)/*{{{*/
	/*
	 * Removes a row from the table.
	 * Parameters:
	 *   - row: the number of the row to remove.
	 */
{
	clearCellWidget (row, tbl_idx_startzeit);
	clearCellWidget (row, tbl_idx_landezeit);
	sk_table::removeRow (row);
}/*}}}*/

void sk_flight_table::remove_flight (db_id id)/*{{{*/
	/*
	 * Removes a row containing a given flight from the table, also remove the
	 * row containing the !!Schleppflug, if any
	 * Parameters:
	 *   - id: the ID of the flight to remove.
	 */
{
	int row;
	row=row_from_id (id);
	if (row>=0)
		removeRow (row);
	row=row_from_sref (id);
	if (row>=0)
		removeRow (row);
}/*}}}*/



void sk_flight_table::update_row_time (int row, sk_time_t *t)/*{{{*/
	/*
	 * Updates the time displayed in a row.
	 * Parameters:
	 *   - row: the number of the row to update.
	 *   - t: the time to be regarded as current.
	 */
{
	QWidget *w=cellWidget (row, tbl_idx_landezeit);
	if (w)
	{
		// Landezeit: Widget, also Button, also noch nicht gelandet

		sk_table_item *sz_item=(sk_table_item *)item (row, tbl_idx_startzeit);
		if (sz_item)
		{
			sk_time_t *startzeit;
			startzeit=(sk_time_t *)sz_item->get_data ();

			if (startzeit)
			{
				sk_time_t flugdauer;
				flugdauer.set_to (startzeit->secs_to (t));
				setText (row, tbl_idx_flugdauer, std2q (flugdauer.table_string (tz_none)));
			}
			else
			{
				// Keine Startzeit gespeichert --> nicht gestartet (kommt)
				setText (row, tbl_idx_flugdauer, "-");
			}
		}
		else
		{
			setText (row, tbl_idx_flugdauer, "Error");
		}
	}
}/*}}}*/

void sk_flight_table::update_row_time (int row)/*{{{*/
	/*
	 * Updates the time displayed in a row, using the current time.
	 * Parameters:
	 *   - row: the number of the row to update.
	 */
{
	sk_time_t t;
	t.set_current ();
	update_row_time (row, &t);
}/*}}}*/

void sk_flight_table::update_time ()/*{{{*/
	/*
	 * If the current second is 0, update the time entries in every row.
	 */
{
	if (QTime::currentTime ().second ()==0)
	{
		for (int row=0; row<numRows (); row++)
		{
			update_row_time (row);
		}
	}
}/*}}}*/



void sk_flight_table::columnClicked (int c)/*{{{*/
	/*
	 * A column header was clicked. Sort the table by the corresponding column.
	 * Parameters:
	 *   see QT documentation.
	 */
{
	// Why isn't this inherited from sk_table?
	sortColumn (c, true, true);
}/*}}}*/


/**
  * Set the display date to a given date.
  * Parameters:
  *   - date: the new display date to set.
  */
void sk_flight_table::set_anzeigedatum (QDate date)
{
	anzeigedatum=date;
}

/**
  * readSettings
  * read persistent settings
  * column width set to zero will not be displayed
  * provide default values if the config file is not present
  */
void sk_flight_table::readSettings (QSettings& settings)
{
        settings.beginGroup ("flight_table");
        setColumnWidth (tbl_idx_registration,           settings.value ("tbl_idx_registration",       95).toInt());
        setColumnWidth (tbl_idx_flugzeug_typ,           settings.value ("tbl_idx_flugzeug_typ",       75).toInt());
        setColumnWidth (tbl_idx_flug_typ,               settings.value ("tbl_idx_flug_typ",           50).toInt());
        setColumnWidth (tbl_idx_pilot,                  settings.value ("tbl_idx_pilot",             150).toInt());
        setColumnWidth (tbl_idx_begleiter,              settings.value ("tbl_idx_begleiter",         150).toInt());
        setColumnWidth (tbl_idx_startart,               settings.value ("tbl_idx_startart",           70).toInt());
        setColumnWidth (tbl_idx_startzeit,              settings.value ("tbl_idx_startzeit",          60).toInt());
        setColumnWidth (tbl_idx_landezeit,              settings.value ("tbl_idx_landezeit",          60).toInt());
        setColumnWidth (tbl_idx_flugdauer,              settings.value ("tbl_idx_flugdauer",          50).toInt());
        setColumnWidth (tbl_idx_landungen,              settings.value ("tbl_idx_landungen",          50).toInt());
        setColumnWidth (tbl_idx_startort,               settings.value ("tbl_idx_startort",          100).toInt());
        setColumnWidth (tbl_idx_zielort,                settings.value ("tbl_idx_zielort",           100).toInt());
        setColumnWidth (tbl_idx_bemerkungen,            settings.value ("tbl_idx_bemerkungen",       200).toInt());
        setColumnWidth (tbl_idx_abrechnungshinweis,     settings.value ("tbl_idx_abrechnungshinweis", 50).toInt());
        setColumnWidth (tbl_idx_editierbar,             settings.value ("tbl_idx_editierbar",         50).toInt());
        setColumnWidth (tbl_idx_datum,                  settings.value ("tbl_idx_datum",              50).toInt());
        setColumnWidth (tbl_idx_id_display,             settings.value ("tbl_idx_id_display",         50).toInt());
        settings.endGroup ();
}

/**
  * writeSettings
  * write persistent settings
  */
void sk_flight_table::writeSettings (QSettings& settings)
{
        settings.beginGroup ("flight_table");
        settings.setValue ("tbl_idx_registration",              columnWidth (tbl_idx_registration));
        settings.setValue ("tbl_idx_flugzeug_typ",              columnWidth (tbl_idx_flugzeug_typ));
        settings.setValue ("tbl_idx_flug_typ",                  columnWidth (tbl_idx_flug_typ));
        settings.setValue ("tbl_idx_pilot",                     columnWidth (tbl_idx_pilot));
        settings.setValue ("tbl_idx_begleiter",                 columnWidth (tbl_idx_begleiter));
        settings.setValue ("tbl_idx_startart",                  columnWidth (tbl_idx_startart));
	settings.setValue ("tbl_idx_startzeit",                 columnWidth (tbl_idx_startzeit));
	settings.setValue ("tbl_idx_landezeit",                 columnWidth (tbl_idx_landezeit));
	settings.setValue ("tbl_idx_flugdauer",                 columnWidth (tbl_idx_flugdauer));
	settings.setValue ("tbl_idx_landungen",                 columnWidth (tbl_idx_landungen));
	settings.setValue ("tbl_idx_startort",                  columnWidth (tbl_idx_startort));
	settings.setValue ("tbl_idx_zielort",                   columnWidth (tbl_idx_zielort));
	settings.setValue ("tbl_idx_bemerkungen",               columnWidth (tbl_idx_bemerkungen));
	settings.setValue ("tbl_idx_abrechnungshinweis",        columnWidth (tbl_idx_abrechnungshinweis));
	settings.setValue ("tbl_idx_editierbar",                columnWidth (tbl_idx_editierbar));
	settings.setValue ("tbl_idx_datum",                     columnWidth (tbl_idx_datum));
	settings.setValue ("tbl_idx_id_display",                columnWidth (tbl_idx_id_display));
	settings.endGroup ();
}

/**
  * setFont
  * set font of table
  */
void sk_flight_table::setFont (const QFont& font)
{
	QHeader *table_header=horizontalHeader ();
	table_header->setFont(font);
	// seems not to work
	table_header->adjustHeaderSize();
}