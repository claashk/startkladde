#ifndef _MainWindow_h
#define _MainWindow_h

#include <cstdio>
#include <stdlib.h>
#include <QString>
#include <vector>

#include <QApplication>
#include <QDateTime>
#include <QFrame>
#include <QLayout>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>

#include "src/data_types.h"
#include "src/db/db_types.h"
#include "src/gui/settings.h"
#include "src/gui/spacing.h"
#include "src/gui/widgets/SkLabel.h"
#include "src/gui/widgets/WeatherWidget.h"
#include "src/gui/windows/WeatherDialog.h"
#include "src/logging/messages.h"
#include "src/plugins/sk_plugin.h"

class db_event;
class flight;
class options;
class sk_db;
class FlightTable;
class Flight;
class Plane;
class DateWindow;
class FlightWindow;
class StatisticsWindow;
class EntityListWindow;
class SplashScreen;

using namespace Qt;

// Info label IDs/*{{{*/
const int idx_info_time=0;
const int idx_info_utc=1;
const int idx_info_anzeige_datum=2;
const int idx_info_fluege_luft=3;
const int idx_info_fluege_gesamt=4;
const int idx_info_datenbankverbindung=5;
const int idx_info_acpi=6;
const int num_info_labels=7;
/*}}}*/

class MainWindow:public QMainWindow
{
	Q_OBJECT

	public:
		enum db_state_t { ds_uninitialized, ds_no_connection, ds_established, ds_unusable, ds_connection_lost };
		MainWindow (QWidget *parent, sk_db *_db, std::list<sk_plugin> *_plugins, const char *name, WFlags f=WType_TopLevel);
		~MainWindow ();
		void start_db ();

	protected:
		void keyPressEvent (QKeyEvent *);

	private:
		// Program menagement/*{{{*/
		bool startup_complete;
		void shutdown ();
/*}}}*/
		// UI/*{{{*/
		void update_checks ();
		void menu_enables (bool cell_change=false);
		bool display_log;
/*}}}*/
		// Other windows/*{{{*/
		::SplashScreen *ss;
		FlightWindow *flug_editor;
		EntityListWindow *flugzeug_liste;
		EntityListWindow *personen_liste;
		WeatherDialog *weatherDialog;
/*}}}*/
		// Database/*{{{*/
		sk_db *db;
		db_state_t db_state;
		QString db_error;
		bool try_initialize_db (QString reason);
		void set_connection_label (const QString &text, const QColor &color=QColor (0, 0, 0));
		// FSM
		void db_do_action ();
		void db_set_state (db_state_t new_state);
		db_state_t db_action_connect ();
		db_state_t db_action_check_connection ();
		bool db_available ();
/*}}}*/
		// Flights/*{{{*/
		void edit_flight (Flight *);
		void manipulate_flight (db_id, flight_manipulation, db_id sref=0);
		void manipulate_flight_by_row (int, flight_manipulation);
		void neuer_flug (Flight *vorlage=NULL);
		void flug_wiederholen (Flight *vorlage=NULL);
/*}}}*/
		// Table/*{{{*/
		FlightTable *tbl_fluege;
		db_id get_flight_id (int);
		void table_activated (int);
		int context_row, context_col;
		int old_row;
/*}}}*/
		// Window I/O/*{{{*/
		void simulate_key (int);
		void update_info ();
		void dbase_connect (QObject *);
/*}}}*/
		// Settings/*{{{*/
		void readSettings ();
		void writeSettings ();
/*}}}*/
		// Date management/*{{{*/
		QDate anzeigedatum;
		bool display_new_flight_date;
		bool always_use_current_date;
		/*}}}*/
		// Layout/*{{{*/
		QSplitter *split_main;
		QFrame *main_frame;
		QVBoxLayout *main_layout;

		QFrame *info_frame;
		QFrame *weather_frame;
		QLabel *lbl_info[num_info_labels];
		QLabel *lbl_info_value[num_info_labels];
		WeatherWidget *weather;
/*}}}*/
		void initActions();
		void initMenu();
		void initContextMenu();
		void initToolbar();

		// Menus/*{{{*/
		QMenu *menu_programm;
		QMenu *menu_flug, *menu_ansicht, *menu_statistik,
			*menu_datenbank, *menu_ansicht_fluege, *menu_ansicht_datum, *menu_hilfe,
			*menu_debug, *menu_demosystem;
		QMenu *menu_ctx_flug, *menu_ctx_emptyrow;
		QToolBar *toolBar;

		QAction *actionSetTime;
		QAction *actionQuit;
		QAction *actionShutDown;
		QAction *actionStart, *actionLanding, *actionCtxLanding, *actionGoAround;
		QAction *actionEditFlight, *actionCopyFlight, *actionDeleteFlight, *actionSortTable;

		QAction *actionRefreshTable;
		QAction *toggleKeyboard;
		QAction *actionNewFlight;
		//QAction *toggleShowAll;
		QAction *actionDemoWeb;
		QAction *actionSetFont, *actionIncFont, *actionDecFont;
		QAction *actionSchleppRef;
		QAction *actionRestartPlugins;
		QAction *actionSuppressLanded, *actionShowDeparted, *actionShowFaulty;
		QAction *actionSetDate, *actionNewFlightNewDate, *actionNewFlightCurrDate;
		QAction *actionBordbuch, *actionFlugbuch, *actionStartart;
		QAction *actionEditPlanes, *actionEditPersons;
		QAction *actionRefreshAll; // *actionWriteCSV;
		QAction *actionInfo, *actionNetDiag;
		QAction *actionSegfault, *actionPing, *actionDisplayLog;
/*}}}*/
		// Timers/*{{{*/
		QTimer *timer_status;
		QTimer *timer_db;
/*}}}*/


		QTextEdit *log;
		std::list<sk_plugin> *plugins;
		sk_plugin *weather_plugin;

	public slots:
		void slot_db_update (db_event *event);
		void set_anzeigedatum (QDate);
		void log_message (QString message);
		void log_message (QString *message);
		void restart_all_plugins ();
		void openWeatherDialog ();

	private slots:
		void slotToggleKeyboard (bool);
		// Flight menu slots/*{{{*/
		void slot_flight_new ();
/*}}}*/
		// Program menu slots/*{{{*/
		void slot_close ();
		void slot_shutdown ();
		void slot_setdate ();
/*}}}*/
		// View menu slots/*{{{*/
		void slot_menu_ansicht_flug_gelandete (bool);
		void slot_menu_ansicht_flug_weggeflogene_gekommene (bool);
		void slot_menu_ansicht_flug_fehlerhafte (bool);
		void slot_menu_ansicht_display_new_flight_date ();
		void slot_menu_ansicht_always_use_current_date ();
		void slot_menu_ansicht_datum_einstellen ();
		//ee
		void slotSetFont ();
		void slotIncFont ();
		void slotDecFont ();
		void slot_schleppref_springen ();
		void slot_tabelle_sortieren ();
		void slot_refresh_table ();
/*}}}*/
		// Statistics menu slots/*{{{*/
		void slot_flugbuch ();
		void slot_bordbuch ();
		void slot_sastat ();
/*}}}*/
		// Database menu slots/*{{{*/
		void slot_flugzeugeditor ();
		void slot_personeneditor ();
		void slot_db_refresh_all ();
/*}}}*/
		// Debug menu slots/*{{{*/
		void slot_display_log ();
		void slot_ping_db ();
		void slot_segfault ();
/*}}}*/
		// Help menu slots/*{{{*/
		void slot_info ();
		void slot_netztest ();
/*}}}*/
		// Demo system menu slots/*{{{*/
		void slot_webinterface ();
/*}}}*/

		// Table slots/*{{{*/
		void slot_table_edit ();
		void slot_table_land ();
		void slot_table_start ();
		void slot_table_delete ();
		void slot_table_zwischenlandung ();
		void slot_table_wiederholen ();
/*}}}*/
		// Table button slots/*{{{*/
		void slot_tbut_start (db_id);
		void slot_tbut_landung (db_id);
		void slot_tbut_schlepplandung (db_id);
/*}}}*/
		// Context menu slots/*{{{*/
		void slot_context_edit ();
		void slot_context_land ();
		void slot_context_start ();
		void slot_context_delete ();
		void slot_context_zwischenlandung ();
		void slot_context_wiederholen ();
/*}}}*/
		// Mouse/Keyboard event slots/*{{{*/
		void slot_table_context (const QPoint &);
		void slot_table_double_click (int, int);
		void slot_table_key (int);
		void slot_current_changed ();
//		void slot_plugin_clicked (int);
/*}}}*/

		void update_time ();
//		void slot_menu (int);
		void setdate (QDateTime);
		void slot_timer_db ();

	signals:
		void db_change (db_event *event);
		void db_update (db_event *event);
		void status (QString);
		void progress (int, int);
		void long_operation_start ();
		void long_operation_end ();
};

#endif

