/***********************************************************************
 * AUTHOR: Marcus Overhagen
 *   FILE: OldSubscriber.cpp
 *  DESCR: 
 ***********************************************************************/
#include <OldSubscriber.h>
#include "debug.h"

/*************************************************************
 * public BSubscriber
 *************************************************************/

BSubscriber::BSubscriber(const char *name)
{
	UNIMPLEMENTED();
}


BSubscriber::~BSubscriber()
{
	UNIMPLEMENTED();
}


status_t
BSubscriber::Subscribe(BAbstractBufferStream *stream)
{
	UNIMPLEMENTED();

	return B_ERROR;
}


status_t
BSubscriber::Unsubscribe()
{
	UNIMPLEMENTED();

	return B_ERROR;
}


subscriber_id
BSubscriber::ID() const
{
	UNIMPLEMENTED();

	return 0;
}


const char *
BSubscriber::Name() const
{
	UNIMPLEMENTED();
	return NULL;
}


void
BSubscriber::SetTimeout(bigtime_t microseconds)
{
	UNIMPLEMENTED();
}


bigtime_t
BSubscriber::Timeout() const
{
	UNIMPLEMENTED();

	return 0;
}


status_t
BSubscriber::EnterStream(subscriber_id neighbor,
						 bool before,
						 void *userData,
						 enter_stream_hook entryFunction,
						 exit_stream_hook exitFunction,
						 bool background)
{
	UNIMPLEMENTED();

	return B_ERROR;
}


status_t
BSubscriber::ExitStream(bool synch)
{
	UNIMPLEMENTED();

	return B_ERROR;
}


bool
BSubscriber::IsInStream() const
{
	UNIMPLEMENTED();

	return false;
}

/*************************************************************
 * protected BSubscriber
 *************************************************************/

status_t
BSubscriber::_ProcessLoop(void *arg)
{
	UNIMPLEMENTED();

	return B_ERROR;
}


status_t
BSubscriber::ProcessLoop()
{
	UNIMPLEMENTED();

	return B_ERROR;
}


BAbstractBufferStream *
BSubscriber::Stream() const
{
	UNIMPLEMENTED();
	return NULL;
}

/*************************************************************
 * private BSubscriber
 *************************************************************/

void
BSubscriber::_ReservedSubscriber1()
{
	UNIMPLEMENTED();
}


void
BSubscriber::_ReservedSubscriber2()
{
	UNIMPLEMENTED();
}


void
BSubscriber::_ReservedSubscriber3()
{
	UNIMPLEMENTED();
}


