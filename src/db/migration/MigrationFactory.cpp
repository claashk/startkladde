#include "MigrationFactory.h"

// Autogenerated migration lists
#include "build/migrations_headers.h"
#include "src/i18n/notr.h"
#include "src/db/migration/MigrationBuilder.h"

#define MIGRATION_LIST "build/migrations.h"

/*
 * This class uses automatically generated code. This file is NOT autogenerated
 * completely in order to allow IDEs to parse the file. Instead, the file
 * migrations.h is autogenerated to contain lines like
 *   MIGRATION (20100214140000, initial)
 * You can use this file by defining the preprocessor macro MIGRATION and then
 * including migrations.h. You should undefine the macro afterwards, so it can
 * be redefined later.
 * Note that the C preprocessor does not allow preprocessor directives in the
 * expansion of macros, so this method cannot be used to include the header
 * files of the individual migrations. Thus, another file, migrations_headers.h
 * is generated which includes the indiviadual headers. This file is simply
 * included here.
 * Note also that the build dependency of this file on the migrations relies on
 * build/migrations_headers.h as qmake does not recognize build/migrations.h as
 * being included into this file.
 */

MigrationFactory *MigrationFactory::instance_;

MigrationFactory::MigrationFactory ()
{
}

MigrationFactory &MigrationFactory::instance ()
{
	if (!instance_)
		instance_=new MigrationFactory ();

	return *instance_;
}

MigrationFactory::~MigrationFactory ()
{
}

/**
 * Lists the versions of all available migrations, sorted in ascending order.
 *
 * @return a QList of migration versions
 */
QList<quint64> MigrationFactory::availableVersions () const
{
	QList<quint64> versions;

#	define MIGRATION(version, name) versions << version ## ll;
#	include MIGRATION_LIST
#	undef MIGRATION

	return versions;
}

/**
 * Determines the latest available migration. This is the last entry of the
 * list returned by #availableVersions, unless no migrations exist.
 *
 * @return the latest available migration, or 0 if no migrations exist
 */
quint64 MigrationFactory::latestVersion () const
{
	QList<quint64> versions=availableVersions ();

	if (versions.empty ())
		return 0;
	else
		return versions.last ();
}



/**
 * Creates a new Migration instance of the given version.
 *
 * The caller takes ownership of the Migration.
 *
 * @param database the database to create the migration for
 * @param version the version of the migration to generate. Must have been
 *                obtained from the same MigrationFactory instance.
 * @return a newly allocated instance of Migration for the given version
 * @throw NoSuchMigrationException if there is no migration with the given
 *        version
 */

Migration *MigrationFactory::createMigration (Interface &interface, const quint64 version) const
{
	switch (version)
	{
#		define MIGRATION(m_version, m_name) case m_version ## ll: \
			return new Migration_ ## m_version ## _ ## m_name (interface);
#		include MIGRATION_LIST
#		undef MIGRATION
	}

	throw NoSuchMigrationException (version);
}

/**
 * Determines the name of a given migration version.
 *
 * @param version the version of the migration to determine the name of. Must
 *                have been obtained from the same MigrationFactory instance.
 * @return the name of the migration with the given version
 * @throw NoSuchMigrationException if there is no migration with the given
 *        version
 */
QString MigrationFactory::migrationName (quint64 version) const
{

	switch (version)
	{
#		define MIGRATION(m_version, m_name) case m_version ## ll: return #m_name;
#		include MIGRATION_LIST
#		undef MIGRATION
	}


	throw NoSuchMigrationException (version);
}

QList<quint64> MigrationFactory::new_availableVersions () const
{
	return builders.keys ();
}

QString MigrationFactory::new_migrationName (quint64 version) const
{
	return builders.value (version)->getName ();
}

MigrationFactory::Registration::Registration (MigrationBuilder *builder)
{
	std::cout << "Adding migration builder for " << builder->getName () << std::endl;
	MigrationFactory::instance ().builders.insert (builder->getVersion (), builder);
}
