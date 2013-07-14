#include "src/io/dataStream/BackgroundDataStream.h"

#include <QDebug>
#include <QThread>

#include "src/util/signal.h"

/**
 * Creates a BackgroundDataStream for the specified stream with the specified Qt
 * parent.
 *
 * If streamOwned is true, the stream will automatically be deleted when this
 * BackgroundDataStream instance is deleted.
 *
 * The background thread will be started immediately.
 */
BackgroundDataStream::BackgroundDataStream (QObject *parent, DataStream *stream, bool streamOwned):
	QObject (parent),
	_stream (stream), _streamOwned (streamOwned)
{
	// Get the initial state of the stream. Note that we do this before moving
	// the stream to the background thread (see below).
	_state=stream->getState ();

	// Create the thread and move the stream to the thread. We have to reparent
	// the stream to NULL in order to move it to a different thread. Since the
	// stream is not necessarily thread safe, we may not directly access it any
	// more after the thread has been started.
	// The thread will be deleted automatically.
	_thread=new QThread (this);
	_stream->setParent (NULL);
	_stream->moveToThread (_thread);

	// Connect signals to the stream
	connect (this   , SIGNAL (open_stream ()),
	         _stream, SLOT   (open ()));
	connect (this   , SIGNAL (close_stream ()),
	         _stream, SLOT   (close ()));
	connect (this   , SIGNAL (setOpen_stream (bool)),
	         _stream, SLOT   (setOpen (bool)));

	// Connect signals from the stream
	// Note that the stateChanged signal connects to a slot.
	connect (_stream, SIGNAL (stateChanged        (DataStream::State)),
	         this   , SLOT   (stream_stateChanged (DataStream::State)));
	connect (_stream, SIGNAL (dataReceived (QByteArray)),
	         this   , SIGNAL (dataReceived (QByteArray)));

	_thread->start ();
}

/**
 * Stops the background thread.
 *
 * If streamOwned is true, the stream is also deleted.
 */
BackgroundDataStream::~BackgroundDataStream ()
{
	// FIXME must stop the thread before deleting it
	// _thread will be deleted automatically.

	_thread->exit (0);
	QThread::yieldCurrentThread ();
	_thread->wait (100);
	_thread->terminate ();

	// _stream has no parent; that's a requirement of moving it to the
	// background thread.
	if (_stream && _streamOwned)
		_stream->deleteLater ();
}


// ***********************
// ** Underlying stream **
// ***********************

/**
 * Note that you should not access the stream directly via the pointer returned
 * by this method, unless you know that it is thread safe. This method's purpose
 * is to check the identity of the stream.
 */
DataStream *BackgroundDataStream::getStream () const
{
	return _stream;
}


// **********************
// ** Public interface **
// **********************

/**
 * Returns the state of the wrapped stream.
 *
 * More precisely, this returns the state at the time of the last stateChanged
 * signal. The value returned by this method may be outdated if the state has
 * changed in the meantime and the corresponding stateChanged signal has not yet
 * been delivered.
 */
DataStream::State BackgroundDataStream::getState () const
{
	return _state;
}


// *********************************
// ** Forward slots to the stream **
// *********************************

/**
 * Opens the wrapped stream on the background thread.
 *
 * Note that after this method returns, the state of the data stream has not
 * necessarily changed yet.
 *
 * See also DataStream::open ()
 */
void BackgroundDataStream::open ()
{
	emit open_stream ();
}

/**
 * Closes the wrapped stream on the background thread.
 *
 * Note that after this method returns, the state of the data stream has not
 * necessarily changed yet.
 *
 * See also DataStream::open ()
 */
void BackgroundDataStream::close ()
{
	emit close_stream ();
}

/**
 * Calls DataStream::setOpen on the background thread.
 *
 * Note that after this method returns, the state of the data stream has not
 * necessarily changed yet.
 *
 * See also DataStream::open ()
 */
void BackgroundDataStream::setOpen (bool o)
{
	emit setOpen_stream (o);
}

// *****************************
// ** Signals from the stream **
// *****************************

// Note that most stream signals are simply re-emitted (connections made in the
// constructor). Some signals, however, have to be processed explicitly.

// We need to buffer the state locally so we can provide a getState method that
// does not have to query the underlying stream.
void BackgroundDataStream::stream_stateChanged (DataStream::State state)
{
	_state=state;
	// Emit the signal *after* the state has been updated.
	emit stateChanged (state);
}
