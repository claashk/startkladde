/*
 * TODO:
 *   - allow setting some settings by both the config file and the command line
 */



#include "Settings.h"

#include <iostream>

#include <QSettings>

#include "src/util/qString.h"
#include "src/util/qList.h"
#include "src/plugin/info/InfoPlugin.h"
#include "src/plugin/factory/PluginFactory.h"
#include "src/plugins/info/test/TestPlugin.h"
#include "src/plugins/info/metar/MetarPlugin.h"
#include "src/plugins/info/sunset/SunsetTimePlugin.h"
#include "src/plugins/info/sunset/SunsetCountdownPlugin.h"
#include "src/plugins/weather/WetterOnlineImagePlugin.h"
#include "src/plugins/weather/WetterOnlineAnimationPlugin.h"
#include "src/i18n/notr.h"


Settings *Settings::theInstance=NULL;

/*
 * Notes:
 *   - don't store a QSettings instance in the class. Constructing and
 *     destroying QSettings instances is "very fast" (according to the
 *     documentation) and QSettings is only reentrant, not thread safe.
 */



Settings::Settings ():
	// Settings which are read from the configuration file do not have to be
	// initialized at this point because they are set to default values in
	// readSettings. All settings which are only set by command line have to be
	// initialized.
	enableDebug (false), coloredLabels (false), displayQueries (false),
	noFullScreen (false), enableShutdown (false),
	overrideDatabaseName (false)
{
	readSettings ();
}

Settings::~Settings ()
{
}

Settings &Settings::instance ()
{
	if (!theInstance) theInstance=new Settings ();
	return *theInstance;
}

QStringList Settings::readArgs (const QStringList &args)
{
	QStringList unprocessed=args;
	unprocessed.removeFirst (); // remove argv[0]

	while (!unprocessed.isEmpty ())
	{
		QString arg=unprocessed.first ();

		if (arg.startsWith (notr ("-")))
		{
			unprocessed.removeFirst ();

			if (arg==notr ("-q"))
				displayQueries=true;
			else if (arg==notr ("--colored-labels"))
				coloredLabels=true;
			else if (arg==notr ("--no-full-screen"))
				noFullScreen=true;
			else if (arg==notr ("--enable-shutdown"))
				enableShutdown=true;
			else if (arg==notr ("--database-name"))
			{
				if (!unprocessed.empty ())
				{
					overrideDatabaseName=true;
					overrideDatabaseNameValue=unprocessed.takeFirst ();

					// Overwrite the stored value in case it has already been
					// read from the settings store.
					databaseInfo.database=overrideDatabaseNameValue;
				}
				else
					std::cout << notr ("No database specified after --database-name") << std::endl;
			}
			else
				std::cout << notr ("Unrecognized option ") << arg << std::endl;
		}
		else
		{
			return unprocessed;
		}
	}

	return unprocessed;
}

void Settings::save ()
{
	writeSettings ();

	// Read back the changes so we can be sure we have the same state as when
	// reading the setups on startup.
	readSettings ();

	emit changed ();
}

QList<InfoPlugin *> Settings::readInfoPlugins ()
{
	QList<InfoPlugin *> plugins;

	QSettings s;
	s.beginGroup (notr ("settings"));

	PluginFactory &factory=PluginFactory::getInstance ();

	// If no entry for infoPlugins exists, create a default set of plugins.
	// Note that if no plugins are used, there is still an entry with 0
	// elements.
	if (s.contains (notr ("infoPlugins/size")))
	{
		int n=s.beginReadArray (notr ("infoPlugins"));
		for (int i=0; i<n; ++i)
		{
			s.setArrayIndex (i);

			QString id=s.value (notr ("id")).toString ();
			InfoPlugin *plugin=factory.createPlugin<InfoPlugin> (id);

			// TODO better handling if not found
			if (plugin)
			{
				s.beginGroup (notr ("settings"));
				plugin->readSettings (s);
				s.endGroup ();

				plugins << plugin;
			}
		}
		s.endArray ();
	}
	else
	{
//		plugins.append (new TestPlugin ("Foo:"));
//		plugins.append (new TestPlugin ("Bar:", true, "TestPlugin", true));

		plugins.append (new SunsetTimePlugin      (tr ("Sunset:")           , true, notr ("sunsets.txt")));
		plugins.append (new SunsetCountdownPlugin (tr ("Time until sunset:"), true, notr ("sunsets.txt")));

		// Add METAR plugin instances for three well-known airports; when
		// translating, the airport IDs should be replaced with airports in the
		// country the translation is made for (e. g. a german translation
		// might include Frankfurt as one of the airports).
		plugins.append (new MetarPlugin (tr ("Weather:"), true, tr ("KSFO"), 15*60));
		plugins.append (new MetarPlugin (""             , true, tr ("KJFK"), 15*60));
		plugins.append (new MetarPlugin (""             , true, tr ("KFTW"), 15*60));
	}

	return plugins;
}

void Settings::writeInfoPlugins (const QList<InfoPlugin *> &plugins)
{
	QSettings s;
	s.beginGroup (notr ("settings"));

	// *** Plugins - Info
	s.beginWriteArray (notr ("infoPlugins"));
	for (int i=0; i<plugins.size (); ++i)
	{
		s.setArrayIndex (i);

		InfoPlugin *plugin=plugins[i];
		s.setValue (notr ("id"), plugin->getId ().toString ());

		s.beginGroup (notr ("settings"));
		plugin->writeSettings (s);
		s.endGroup ();
	}
	s.endArray ();

}

void Settings::readSettings ()
{
	QSettings s;
	s.beginGroup (notr ("settings"));

	// *** Database
	s.beginGroup (notr ("database"));
	databaseInfo.load (s); // Connection
	s.endGroup ();
	// If the database name has been overridden, overwrite the value. It won't
	// be written back.
	if (overrideDatabaseName)
		databaseInfo.database=overrideDatabaseNameValue;

	// *** Settings
	// UI
	s.beginGroup (notr ("language"));
	languageConfiguration.load (s);
	s.endGroup ();
	// Data
	location      =s.value (notr ("location")      , tr ("Twiddlethorpe")).toString ();
	recordTowpilot=s.value (notr ("recordTowpilot"), true            ).toBool ();
	checkMedicals =s.value (notr ("checkMedicals") , true            ).toBool ();
	// Permissions
	protectSettings      =s.value (notr ("protectSettings"      ), false).toBool ();
	protectLaunchMethods =s.value (notr ("protectLaunchMethods" ), false).toBool ();
	protectMergePeople   =s.value (notr ("protectMergePeople"   ), false).toBool ();
	protectFlightDatabase=s.value (notr ("protectFlightDatabase"), false).toBool ();
	protectViewMedicals  =s.value (notr ("protectViewMedicals"  ), false).toBool ();
	protectChangeMedicals=s.value (notr ("protectChangeMedicals"), false).toBool ();
	// Diagnostics
	enableDebug=s.value (notr ("enableDebug"), false       ).toBool ();
	diagCommand=s.value (notr ("diagCommand"), notr ("./script/netztest_xterm")).toString (); // xterm -e ./netztest &

	// *** Plugins - Weather
	// Weather plugin
	weatherPluginId      =s.value (notr ("weatherPluginId")      , WetterOnlineImagePlugin::_getId ().toString ()).toString ();
	weatherPluginCommand =s.value (notr ("weatherPluginCommand") , notr ("regenradar_wetteronline.de.rb")).toString ();
	weatherPluginEnabled =s.value (notr ("weatherPluginEnabled") , true).toBool ();
	weatherPluginHeight  =s.value (notr ("weatherPluginHeight")  , 200).toInt ();
	weatherPluginInterval=s.value (notr ("weatherPluginInterval"), 15*60).toInt ();
	// Weather dialog
	weatherWindowPluginId=s.value (notr ("weatherWindowPluginId"), WetterOnlineAnimationPlugin::_getId ().toString ()).toString ();
	weatherWindowCommand =s.value (notr ("weatherWindowCommand") , notr ("regenradar_wetteronline.de_animation.rb")).toString ();
	weatherWindowEnabled =s.value (notr ("weatherWindowEnabled") , true).toBool ();
	weatherWindowInterval=s.value (notr ("weatherWindowInterval"), 15*60).toInt ();
	weatherWindowTitle   =s.value (notr ("weatherWindowTitle")   , tr ("Weather radar (3 hours)")).toString ();

	// *** Plugins - Paths
	pluginPaths.clear ();
	if (s.contains (notr ("pluginPaths/size")))
	{
		int n=s.beginReadArray (notr ("pluginPaths"));
		for (int i=0; i<n; ++i)
		{
			s.setArrayIndex (i);
			pluginPaths << s.value (notr ("path")).toString ();
		}
		s.endArray ();
	}
	else
	{
		pluginPaths
			<< notr ("./.startkladde/plugins")
			<< notr ("./plugins")
			<< notr ("./plugins/info")
			<< notr ("./plugins/weather")
			<< notr ("/usr/lib/startkladde/plugins")
			<< notr ("/usr/lib/startkladde/plugins/info")
			<< notr ("/usr/lib/startkladde/plugins/weather")
			;
	}
}

void Settings::writeSettings ()
{
	QSettings s;
	s.beginGroup (notr ("settings"));

	// *** Database
	// If the database name has been overridden, don't store the settings
	if (!overrideDatabaseName)
	{
		s.beginGroup (notr ("database"));
		databaseInfo.save (s); // Connection
		s.endGroup ();
	}

	// *** Settings
	// UI
	s.beginGroup (notr ("language"));
	languageConfiguration.save (s);
	s.endGroup ();
	// Data
	s.setValue (notr ("location")      , location      );
	s.setValue (notr ("recordTowpilot"), recordTowpilot);
	s.setValue (notr ("checkMedicals") , checkMedicals );
	// Permissions
	s.setValue (notr ("protectSettings"      ), protectSettings      );
	s.setValue (notr ("protectLaunchMethods" ), protectLaunchMethods );
	s.setValue (notr ("protectMergePeople"   ), protectMergePeople   );
	s.setValue (notr ("protectFlightDatabase"), protectFlightDatabase);
	s.setValue (notr ("protectViewMedicals"  ), protectViewMedicals  );
	s.setValue (notr ("protectChangeMedicals"), protectChangeMedicals);
	// Diagnostics
	s.setValue (notr ("enableDebug"), enableDebug);
	s.setValue (notr ("diagCommand"), diagCommand);


	// *** Plugins - Weather
	// Weather plugin
	s.setValue (notr ("weatherPluginId")      , weatherPluginId);
	s.setValue (notr ("weatherPluginCommand") , weatherPluginCommand);
	s.setValue (notr ("weatherPluginEnabled") , weatherPluginEnabled);
	s.setValue (notr ("weatherPluginHeight")  , weatherPluginHeight);
	s.setValue (notr ("weatherPluginInterval"), weatherPluginInterval);
	// Weather dialog
	s.setValue (notr ("weatherWindowPluginId"), weatherWindowPluginId);
	s.setValue (notr ("weatherWindowCommand") , weatherWindowCommand);
	s.setValue (notr ("weatherWindowEnabled") , weatherWindowEnabled);
	s.setValue (notr ("weatherWindowInterval"), weatherWindowInterval);
	s.setValue (notr ("weatherWindowTitle")   , weatherWindowTitle);

	// *** Plugins - Paths
	s.beginWriteArray (notr ("pluginPaths"));
	for (int i=0; i<pluginPaths.size (); ++i)
	{
		s.setArrayIndex (i);
		s.setValue (notr ("path"), pluginPaths.at (i));
	}
	s.endArray ();

	s.sync ();
}

