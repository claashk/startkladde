/*
 * Short term plan:
 *   - integrate db creation into the gui
 *   - add a version to the database
 *   - add migrations
 *     - how to store migrations?
 *     - can we do rails-like schema generation from migrations?
 *   - Standardize enum handling: store the database value internally (or use the
 *     numeric value in the database?); and have an "unknown" type (instead of "none")
 *     - this should also allow preserving unknown types in the database
 *   - add "object used" to Database
 *   - add ping to Database
 *   - generate string lists from other data instead of explicit query (but still
 *     cache explicitly) note that we still have to query for accounting notes and
 *     airfields because we don't get a complete flight list
 *   - fix error reporting (db.lastError (), e. g. delete objects)
 *   - remove DbColumn, DbTable
 *   - make sure "", 0 and NULL are read correctly
 *   - Migration management:
 *     - script to create migration
 *     - autogenerate factory
 *     - .pro file
 *
 * Improvements:
 *   - split this class into parts (generic, orm, specific)
 *   - move (static) methods like selectDistinctColumnQuery to QueryGenerator (but see below)
 *   - make a class Query, a la Query ("select * from foo where id=?") << 42
 *   - allow specifying an "exclude" value to selectDistinctColumnQuery (requires
 *     Query class)
 *   - move selectDistinctColumnQuery to Query
 *   - move specialized queries generation to model classes (e. g. flight prepared)
 *   - maybe we would like to select additional columns, like
 *     (landing_time-takeoff_time as duration) for some conditions
 *   - Flight should be an entity
 *   - Allow tables without id column (e. g. habtm join tables), but tables
 *     must have at least one column
 *   - dbTableName as static constant instead of function
 *   - make sure (using a dump of an old db) that the migration path is OK
 *
 * Medium term plan:
 *   - add some abstraction to the query list generation
 *   - add a version number
 *   - add migrations
 *     - one possibility: with create 0, up/down, and create current
 *     - another: autogenerate current from 0 and ups
 *     - look at rails (or other framework supporting migrations)
 *   - add support for sqlite
 *   - add indexes
 *   - add foreign key constraints
 *   - database checks (?)
 *     - show what is wrong/will be fixed
 *     - rails migration like specification
 *
 * Long term plan:
 *   - Shold there be a thread safe database class?
 *   - use a memory SQLite for local storage (datastorage functionality)
 *   - merge data storage and database methods, so we can use a (local)
 *     database directly (w/o local cache)
 *       - we'd probably have abstract DataStorage and inherited Database and
 *         CachingDataStorage which uses ad Database for access and one (in
 *         memory) for local caching
 *       - question: can we afford using a query on a memory database e. g.
 *         for list of first names rather than maintaining the list explicitly?
 *         should be fast enough (does sqlite memory table have index?)
 *         airfields are more interesting, b/c there are many flights
 *   - local disk caching
 */

#include "Database.h"

#include <iostream>
#include <cassert>

#include <QDateTime>

#include "src/db/DatabaseInfo.h"
#include "src/config/Options.h"
#include "src/text.h"
#include "src/model/Person.h"
#include "src/model/Plane.h"
#include "src/model/Flight.h"
#include "src/model/LaunchType.h"


// TODO: check if the slow part is the loop; if yes, add ProgressMonitor

// TODO: error when query fails

/*
 * Here's an idea for specifying conditions, or parts thereof:
 * Condition ("foo=? and bar=?") << 42 << "baz";
 */

// ***************
// ** Constants **
// ***************

const QString Database::dataTypeBinary    = "blob";
const QString Database::dataTypeBoolean   = "tinyint(1)";
const QString Database::dataTypeDate      = "date";
const QString Database::dataTypeDatetime  = "datetime";
const QString Database::dataTypeDecimal   = "decimal";
const QString Database::dataTypeFloat     = "float";
const QString Database::dataTypeInteger   = "int(11)";
const QString Database::dataTypeString    = "varchar(255)";
const QString Database::dataTypeText      = "text";
const QString Database::dataTypeTime      = "time";
const QString Database::dataTypeTimestamp = "datetime";
const QString Database::dataTypeCharacter = "varchar(1)"; // Non-Rails
const QString Database::dataTypeId        = dataTypeInteger;


// ******************
// ** Construction **
// ******************

Database::Database ()
{
    db=QSqlDatabase::addDatabase ("QMYSQL");
}

bool Database::open (const DatabaseInfo &dbInfo)
{
	std::cout << QString ("Connecting to %1@%2:%3")
		.arg (dbInfo.username, dbInfo.server, dbInfo.database) << std::endl;

    db.setHostName     (dbInfo.server  );
    db.setUserName     (dbInfo.username);
    db.setPassword     (dbInfo.password);
    db.setPort         (dbInfo.port    );
    db.setDatabaseName (dbInfo.database);

    bool result=db.open ();
	if (!result) return false;

//    QSqlQuery query (db);
//    query.prepare ("show variables like 'char%'");
//    executeQuery (query);
//
//    while (query.next())
//    {
//    	QString name=query.value(0).toString ();
//    	QString value=query.value(1).toString ();
//    	std::cout << QString ("%1=%2").arg (name).arg (value) << std::endl;
//    }

	return result;
}

void Database::close ()
{
	db.close ();
}

Database::~Database()
{
}


// ******************
// ** Very generic **
// ******************

QStringList Database::listStrings (QSqlQuery query)
{
	QStringList stringList;

	while (query.next ())
		stringList.append (query.value (0).toString ());

	return stringList;
}

QStringList Database::listStrings (QString query)
{
	QSqlQuery q (query, db);
	executeQuery (q);
	return listStrings (q);
}

QString Database::selectDistinctColumnQuery (QString table, QString column, bool excludeEmpty)
{
	// "select distinct column from table"
	QString query=QString ("select distinct %1 from %2").arg (column, table);

	// ..." where column!=''"
	if (excludeEmpty) query+=QString (" where %1!=''").arg (column);
	return query;
}

QString Database::selectDistinctColumnQuery (QStringList tables, QStringList columns, bool excludeEmpty)
{
	QStringList parts;

	foreach (QString table, tables)
		foreach (QString column, columns)
			parts << selectDistinctColumnQuery (table, column, excludeEmpty);

	return parts.join (" union ");
}

QString Database::selectDistinctColumnQuery (QStringList tables, QString column, bool excludeEmpty)
{
	return selectDistinctColumnQuery (tables, QStringList (column), excludeEmpty);
}

QString Database::selectDistinctColumnQuery (QString table, QStringList columns, bool excludeEmpty)
{
	return selectDistinctColumnQuery (QStringList (table), columns, excludeEmpty);
}


QSqlQuery &Database::executeQuery (QSqlQuery &query)
{
	if (!query.exec ())
		throw QueryFailedException (query.lastError ());

	return query;
}

QSqlQuery Database::executeQuery (QString queryString)
{
	QSqlQuery query (queryString, db);
	return executeQuery (query);
}

bool Database::queryHasResult (QString queryString)
{
	QSqlQuery query=executeQuery (queryString);
	return query.size()>0;
}


// *************************
// ** Schema manipulation **
// *************************

void Database::createTable (QString name)
{
	std::cout << QString ("Creating table %1").arg (name);

	QString queryString=QString (
		"CREATE TABLE %1 ("
		"id int(11) NOT NULL AUTO_INCREMENT,"
		"PRIMARY KEY (id)"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci"
		)
		.arg (name);

	executeQuery (queryString);
}

void Database::createTableLike (QString like, QString name)
{
	std::cout << QString ("Creating table %1 like %2").arg (name, like);

	QString queryString=
		QString ("CREATE TABLE %1 LIKE %2")
		.arg (name, like);

	executeQuery (queryString);
}

void Database::dropTable (QString name)
{
	std::cout << QString ("Dropping table %1").arg (name);

	QString queryString=
		QString ("DROP TABLE %1")
		.arg (name);

	executeQuery (queryString);
}

void Database::addColumn (QString table, QString name, QString type)
{
	std::cout << QString ("Adding column %1.%2...").arg (table, name);

	QString queryString=
		QString ("ALTER TABLE %1 ADD COLUMN %2 %3")
		.arg (table, name, type);

	executeQuery (queryString);
}

bool Database::tableExists (QString name)
{
	// Using addBindValue does not seem to work here
	QString queryString=
		QString ("SHOW TABLES LIKE '%1'")
		.arg (name);

	return queryHasResult (queryString);
}


// *********
// ** ORM **
// *********

template<class T> QList<T> Database::getObjects (QString condition, QList<QVariant> conditionValues)
{
	QSqlQuery query (db);
	query.setForwardOnly (true);

	QString queryString=QString ("select %1 from %2")
		.arg (T::selectColumnList (), T::dbTableName ());

	if (!condition.isEmpty ())
		queryString+=" where "+condition;

	query.prepare (queryString);

	foreach (const QVariant &conditionValue, conditionValues)
		query.addBindValue (conditionValue);

	query.exec ();

    return T::createListFromQuery (query);
}

template<class T> int Database::countObjects ()
{
	QString q="select count(*) from "+T::dbTableName ();
	QSqlQuery query (q, db);

	query.next ();
    return query.value (0).toInt ();
}

template<class T> bool Database::objectExists (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("select count(*) from "+T::dbTableName ()+" where id=?");
	query.addBindValue (id);
	query.exec ();

	query.next ();
	return query.value (0).toInt ()>0;
}

template<class T> T Database::getObject (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("select "+T::selectColumnList ()+" from "+T::dbTableName ()+" where id=?");
	query.addBindValue (id);
	query.exec ();

	if (!query.next ()) throw NotFoundException ();

	return T::createFromQuery (query);
}

template<class T> int Database::deleteObject (db_id id)
{
	QSqlQuery query (db);
	query.prepare ("delete from "+T::dbTableName ()+" where id=?");
	query.addBindValue (id);
	query.exec ();

	return query.numRowsAffected ();
}

template<class T> db_id Database::createObject (T &object)
{
	QSqlQuery query (db);
	query.prepare ("insert into "+T::dbTableName ()+" "+T::insertValueList ());
	object.bindValues (query);
	query.exec ();

	object.id=query.lastInsertId ().toLongLong ();

	return object.id;
}

template<class T> int Database::updateObject (const T &object)
{
	QSqlQuery query (db);
	query.prepare ("update "+T::dbTableName ()+" set "+object.updateValueList ()+" where id=?");
	object.bindValues (query);
	query.addBindValue (object.id);

	query.exec ();

	return query.numRowsAffected ();
}

// Instantiate the class templates
// Classes have to provide:
//   - ::dbTableName ();
//   - ::QString selectColumnList ();
//   - ::createFromQuery (const QSqlQuery &query);
//   - ::insertValueList ();
//   - ::updateValueList ();
//   - bindValues (QSqlQuery &q) const;
//   - ::createListFromQuery (QSqlQuery &query);

template QList<Person> Database::getObjects           (QString condition, QList<QVariant> conditionValues);
template QList<Plane > Database::getObjects           (QString condition, QList<QVariant> conditionValues);
template QList<Flight> Database::getObjects           (QString condition, QList<QVariant> conditionValues);

template int           Database::countObjects<Person> ();
template int           Database::countObjects<Plane > ();
template int           Database::countObjects<Flight> ();

template bool          Database::objectExists<Person> (db_id id);
template bool          Database::objectExists<Plane > (db_id id);
template bool          Database::objectExists<Flight> (db_id id);

template Person        Database::getObject            (db_id id);
template Plane         Database::getObject            (db_id id);
template Flight        Database::getObject            (db_id id);

template int           Database::deleteObject<Person> (db_id id);
template int           Database::deleteObject<Plane > (db_id id);
template int           Database::deleteObject<Flight> (db_id id);

template db_id         Database::createObject         (Person &object);
template db_id         Database::createObject         (Plane  &object);
template db_id         Database::createObject         (Flight &object);

template int           Database::updateObject         (const Person &object);
template int           Database::updateObject         (const Plane  &object);
template int           Database::updateObject         (const Flight &object);

// Legacy launch type
template<> QList<LaunchType> Database::getObjects (QString condition, QList<QVariant> conditionValues)
{
	assert (condition.isEmpty ());
	(void)conditionValues;
	return launchTypes.values ();
}

template<> int Database::countObjects<LaunchType> ()
{
	return launchTypes.size ();
}

template<> bool Database::objectExists<LaunchType> (db_id id)
{
	return launchTypes.contains (id);
}

template<> LaunchType Database::getObject (db_id id)
{
	if (launchTypes.contains (id)) throw NotFoundException ();
	return launchTypes[id];
}

template<> int Database::deleteObject<LaunchType> (db_id id)
{
	(void)id;
	assert (!"Thou shalt not try to delete a launch type");
	return 0;
}

template<> db_id Database::createObject (LaunchType &object)
{
	(void)object;
	assert (!"Thou shalt not try to create a launch type");
	return invalid_id;
}

template<> int Database::updateObject (const LaunchType &object)
{
	(void)object;
	assert (!"Thou shalt not try to update a launch type");
	return 0;
}

void Database::addLaunchType (const LaunchType &launchType)
{
	launchTypes.insert (launchType.get_id (), launchType);
}


// *******************
// ** Very specific **
// *******************

QStringList Database::listAirfields ()
{
	QString query="select startort from %1 where startort!='' union select zielort from %1 where zielort!=''";
	return listStrings (query.arg (Flight::dbTableName ()));

	return listStrings (selectDistinctColumnQuery (
		Flight::dbTableName (),
		QStringList () << "startort" << "zielort"
		));
}

QStringList Database::listAccountingNotes ()
{
	return listStrings (selectDistinctColumnQuery (
		Flight::dbTableName (),
		"abrechnungshinweis",
		true));
}

QStringList Database::listClubs ()
{
	return listStrings (selectDistinctColumnQuery (
		QStringList () << Plane::dbTableName() << Person::dbTableName (),
		"verein",
		true));
}

QStringList Database::listPlaneTypes ()
{
	return listStrings (selectDistinctColumnQuery (
		Plane::dbTableName (),
		"typ",
		true));
}

QList<Flight> Database::getPreparedFlights ()
{
	// The correct criterion for prepared flights is:
	// !((starts_here and started) or (lands_here and landed))
	// Resolving the flight mode, we get:
	// !( (local and (started or landed)) or (leaving and started) or (coming and landed) )

	QString condition="!( (modus=? and status&?) or (modus=? and status&?) or (modus=? and status&?) )";
	QList<QVariant> conditionValues; conditionValues
		<< "l" << (Flight::STATUS_STARTED|Flight::STATUS_LANDED)
		<< "g" << Flight::STATUS_STARTED
		<< "k" << Flight::STATUS_LANDED
		;

	return getObjects<Flight> (condition, conditionValues);
}

QList<Flight> Database::getFlightsDate (QDate date)
{
	// The correct criterion for flights on a given date is:
	// (happened and effective_date=that_date)
	// effective_date has to be calculated from takeoff time, landing time,
	// status and mode, which is compilicated. Thus, we select a superset of
	// the flights of that date and filter out the correct flights afterwards.

	// The superset criterion is:
	// (launch_date=that_date or landing_date=that_date)
	// Since the database stores the datetimes, we compare them agains the
	// first and last datetime of the date.

	QDateTime thisMidnight (date,             QTime (0, 0, 0)); // Start of day
	QDateTime nextMidnight (date.addDays (1), QTime (0, 0, 0)); // Start of next day

	QString condition="(startzeit>=? and startzeit<?) or (landezeit>=? and landezeit<?)";
	QList<QVariant> conditionValues; conditionValues
		<< thisMidnight << nextMidnight
		<< thisMidnight << nextMidnight
		;

	QList<Flight> candidates=getObjects<Flight> (condition, conditionValues);

	// For some of the selected flights, the fact that the takeoff or landing
	// time is on that day may not indicate that the flight actually happened
	// on that day. For example, if a flight is prepared (i. e. not taken off
	// nor landed), or leaving, the times may not be relevant.
	// Thus, we only keep flights which happened and where the effective date
	// is the given date.

	QList<Flight> flights;
	foreach (const Flight &flight, candidates)
		if (flight.happened () && flight.effdatum ()==date)
			flights.append (flight);

	return flights;
}

