#include "Migration_20100215000000_change_to_innodb.h"

#include <iostream>

#include "src/util/qString.h"

Migration_20100215000000_change_to_innodb::Migration_20100215000000_change_to_innodb (Db::Interface::Interface &interface):
	Migration (interface)
{
}

Migration_20100215000000_change_to_innodb::~Migration_20100215000000_change_to_innodb ()
{
}

void Migration_20100215000000_change_to_innodb::up ()
{
	changeTable ("person");
	changeTable ("person_temp");
	changeTable ("flugzeug");
	changeTable ("flugzeug_temp");
	changeTable ("flug");
	changeTable ("flug_temp");
	changeTable ("user");
}

void Migration_20100215000000_change_to_innodb::down ()
{
	// Don't change back
}

void Migration_20100215000000_change_to_innodb::changeTable (const QString &name)
{
	std::cout << "Changing table " << name << std::endl;

	executeQuery (
		QString ("ALTER TABLE %1 ENGINE=InnoDB")
		.arg (name)
	);
}
