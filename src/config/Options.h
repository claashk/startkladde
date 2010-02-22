#ifndef OPTIONS_H_
#define OPTIONS_H_

// Adding an option: see doc/internal/Options

#include <QString>
#include <QStringList>
#include <QList>
#include <qdatetime.h>
#include <qfile.h>

#include "src/db/DatabaseInfo.h"

class ShellPlugin;
class LaunchMethod;

class Options
{
	public:
		Options ();
		bool parse_arguments (int argc, char *argv[]);
		bool read_config_file (QString filename, QList<ShellPlugin *> *plugins);
		bool read_config_files (QList<ShellPlugin *> *plugins, int argc, char *argv[]);
		static void display_options (QString prefix);
		void do_display ();
		bool need_display ();
		bool address_is_local (const QString &address) const;
		QString find_plugin_file (const QString &filename, QString *dir=NULL, QString *basename=NULL) const;

		// Configuration
		QString config_file;
		bool display_help;
		bool show_version;

		// Connection
		DatabaseInfo databaseInfo;
		QString server_display_name;
		QString root_name;
		QString root_password;
		QString sk_admin_name;
		QString sk_admin_password;

		// Local Options
		bool demosystem;
		QString title;
		QString diag_cmd;
		QString ort;
		QString style;

		// Networking Options
		QStringList local_hosts;

		// Debugging
		bool debug;
		bool display_queries;
		bool colorful;

		// Database
		bool record_towpilot;

		// Plugin Entity
		QStringList plugin_paths;

		// Weather plugin
		QString weather_plugin;
		int weather_height;
		int weather_interval;
		QString weather_dialog_plugin;
		QString weather_dialog_title;
		int weather_dialog_interval;

		// Non-Options
		QStringList non_options;

		static bool silent;

		QList<LaunchMethod> configuredLaunchMethods;
};

extern Options opts;

#endif
