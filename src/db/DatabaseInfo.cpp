#include "DatabaseInfo.h"

#include <QSettings>

#include <iostream>

DatabaseInfo::DatabaseInfo ():
	defaultPort (true), port (0)
{
}

DatabaseInfo::~DatabaseInfo ()
{
}

DatabaseInfo::DatabaseInfo (QSettings &settings):
	defaultPort (true), port (0)
{
	load (settings);
}


QString DatabaseInfo::toString () const
{
	return QString ("%1@%2:%3").arg (username, server, database);
}

DatabaseInfo::operator QString () const
{
	return toString ();
}

QString DatabaseInfo::serverText () const
{
	if (defaultPort)
		return server;
	else
		return QString ("%1:%2").arg (server).arg (port);
}

void DatabaseInfo::load (QSettings &settings)
{
	server     =settings.value ("server"     , "localhost"  ).toString ();
	defaultPort=settings.value ("defaultPort", true         ).toBool   ();
	port       =settings.value ("port"       , 3306         ).toInt    ();
	username   =settings.value ("username"   , "startkladde").toString ();
	password   =settings.value ("password"   , "moobert"    ).toString ();
	database   =settings.value ("database"   , "startkladde").toString ();
}

void DatabaseInfo::save (QSettings &settings)
{
	settings.setValue ("server"     , server     );
	settings.setValue ("defaultPort", defaultPort);
	settings.setValue ("port"       , port       );
	settings.setValue ("username"   , username   );
	settings.setValue ("password"   , password   );
	settings.setValue ("database"   , database   );
}

/**
 * Determines whether this databaseInfo refers to a different database than
 * another one
 *
 * The databases can be identical (i. e. this method returns false) even if
 * some values are different, for example, if they are not active because they
 * belong to a different database type than selected.
 *
 * @param other
 * @return
 */
bool DatabaseInfo::different (const DatabaseInfo &other)
{
	if (server          !=other.server          ) return true;
	if (effectivePort ()!=other.effectivePort ()) return true;
	if (username        !=other.username        ) return true;
	if (password        !=other.password        ) return true;
	if (database        !=other.database        ) return true;

	return false;
}
