#include "Worker.h"

#include <iostream>

// Not explicitly used, but required for destruction of the shared pointer in
// #executeQueryResult
#include "src/db/result/Result.h"
#include "src/db/result/CopiedResult.h"
#include "src/db/Query.h"
#include "src/db/interface/DefaultInterface.h"
#include "src/concurrent/Returner.h"
#include "src/concurrent/monitor/OperationMonitor.h"

namespace Db { namespace Interface { namespace ThreadSafe
{
	// ******************
	// ** Construction **
	// ******************

	Worker::Worker (const DatabaseInfo &dbInfo):
		interface (NULL)
	{
		// For connecting the signal, we need to know that it's a
		// DefaultInterface. Afterwards, we assign it to the
		// AbstractInterface *interface;
		DefaultInterface *defaultInterface=new DefaultInterface (dbInfo);

		connect (defaultInterface, SIGNAL (databaseError (int, QString)), this, SIGNAL (databaseError (int, QString)));

		interface=defaultInterface;
	}

	Worker::~Worker ()
	{
		delete interface;
	}


	// ***************************
	// ** Connection management **
	// ***************************

	void Worker::open (Returner<bool> *returner)
	{
		returnOrException (returner, interface->open ());
	}

	void Worker::asyncOpen (Returner<bool> *returner, OperationMonitor *monitor)
	{
		// Signal end of operation when this method returns
		OperationMonitorInterface monitorInterface=monitor->interface ();
		monitorInterface.status ("Verbindung herstellen");
		returnOrException (returner, interface->open ());
	}

	void Worker::close (Returner<void> *returner)
	{
		returnVoidOrException (returner, interface->close ());
	}

	void Worker::lastError (Returner<QSqlError> *returner) const
	{
		returnOrException (returner, interface->lastError ());
	}

	void Worker::cancelConnection ()
	{
		// Called directly
		if (interface) interface->cancelConnection ();
	}


	// ******************
	// ** Transactions **
	// ******************

	void Worker::transaction (Returner<void> *returner)
	{
		returnVoidOrException (returner, interface->transaction ());
	}

	void Worker::commit (Returner<void> *returner)
	{
		returnVoidOrException (returner, interface->commit ());
	}

	void Worker::rollback (Returner<void> *returner)
	{
		returnVoidOrException (returner, interface->rollback ());
	}


	// *************
	// ** Queries **
	// *************

	void Worker::executeQuery (Returner<void> *returner, Db::Query query)
	{
		returnVoidOrException (returner, interface->executeQuery (query));
	}

	void Worker::executeQueryResult (Returner<QSharedPointer<Result::Result> > *returner, Db::Query query, bool forwardOnly)
	{
		// Option 1: copy the DefaultResult (is it allowed to access the
		// QSqlQuery from the other thread? It seems to work.)
//		returnOrException (returner, interface->executeQueryResult (query, forwardOnly));

		// Option 2: create a CopiedResult
		(void)forwardOnly;
		returnOrException (returner, QSharedPointer<Result::Result> (
			new Result::CopiedResult (
				// When copying, we can always set forwardOnly
				*interface->executeQueryResult (query, true)
			)
		));
	}

	void Worker::queryHasResult (Returner<bool> *returner, Db::Query query)
	{
		returnOrException (returner, interface->queryHasResult (query));
	}
} } }
