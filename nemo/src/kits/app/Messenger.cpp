//------------------------------------------------------------------------------
//	Copyright (c) 2001-2002, OpenBeOS
//
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//
//	File Name:		Messenger.h
//	Author:		Ingo Weinhold (bonefish@users.sf.net)
//	Description:	BMessenger delivers messages to local or remote targets.
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <new>
#include <stdio.h>
#include <string.h>

// System Includes -------------------------------------------------------------
#include <Application.h>
#include <Handler.h>
#include <LooperList.h>
#include <Looper.h>
#include <Message.h>
#include <Messenger.h>
#include <OS.h>
//#include <Roster.h>

// Project Includes ------------------------------------------------------------
#include <AppMisc.h>
#include <MessageUtils.h>
#include <ObjectLocker.h>
#include <TokenSpace.h>

// Local Includes --------------------------------------------------------------

// Debugging -------------------------------------------------------------------
#include "nemo_debug.h"
#if DEBUG_LIBBE
	#define OUT(x...)	fprintf(LIBBE_LOG, "BMessenger: "x);
	#define DBG(x)		x;
#else
	#define DBG(x)		;
#endif

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

using BPrivate::gDefaultTokens;
using BPrivate::gLooperList;
using BPrivate::BLooperList;
using BPrivate::BObjectLocker;

enum {
	NOT_IMPLEMENTED	= B_ERROR,
};

//------------------------------------------------------------------------------
// constructor
/*!	\brief Creates an unitialized BMessenger.
*/
BMessenger::BMessenger()
		  : fPort(-1),
			fHandlerToken(-1),
			fTeam(-1),
			fPreferredTarget(false)
{
}
//------------------------------------------------------------------------------
// constructor
/*!	\brief Creates a BMessenger and initializes it to target the already
	running application identified by its signature and/or team ID.

	When only a signature is given, and multiple instances of the application
	are running it is undeterminate which one is chosen as the target. In case
	only a team ID is passed, the target application is identified uniquely.
	If both are supplied, the application identified by the team ID must have
	a matching signature, otherwise the initilization fails.

	\param signature The target application's signature. May be \c NULL.
	\param team The target application's team ID. May be < 0.
	\param result An optional pointer to a pre-allocated status_t into which
		   the result of the initialization is written.
*/
BMessenger::BMessenger(const char *signature, team_id team, status_t *result)
		  : fPort(-1),
			fHandlerToken(-1),
			fTeam(-1),
			fPreferredTarget(false)
{
	InitData(signature, team, result);
}
//------------------------------------------------------------------------------
// constructor
/*!	\brief Creates a BMessenger and initializes it to target the local
	BHandler and/or BLooper.

	When a \c NULL handler is supplied, the preferred handler in the given
	looper is targeted. If no looper is supplied the looper the given handler
	belongs to is used -- that means in particular, that the handler must
	already belong to a looper. If both are supplied the handler must actually
	belong to looper.

	\param handler The target handler. May be \c NULL.
	\param looper The target looper. May be \c NULL.
	\param result An optional pointer to a pre-allocated status_t into which
		   the result of the initialization is written.
*/
BMessenger::BMessenger(const BHandler *handler, const BLooper *looper,
					   status_t *result)
		  : fPort(-1),
			fHandlerToken(-1),
			fTeam(-1),
			fPreferredTarget(false)
{
	status_t error = (handler || looper ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		if (handler) {
			// BHandler is given, check/retrieve the looper.
			if (looper) {
				if (handler->Looper() != looper)
					error = B_MISMATCHED_VALUES;
			} else {
				looper = handler->Looper();
				if (looper == NULL)
					error = B_MISMATCHED_VALUES;
			}
		}
		// set port, token,...
		if (error == B_OK) {
			BObjectLocker<BPrivate::BLooperList> locker(gLooperList);
			if (locker.IsLocked() && gLooperList.IsLooperValid(looper)) {
				fPort = looper->fMsgPort;
				fHandlerToken = (handler ? _get_object_token_(handler) : -1);
				fPreferredTarget = !handler;
				fTeam = looper->Team();
			} else
				error = B_BAD_VALUE;
		}
	}
	if (result)
		*result = error;
}
//------------------------------------------------------------------------------
// copy constructor
/*!	\brief Creates a BMessenger and initializes it to have the same target
	as the supplied messemger.

	\param from The messenger to be copied.
*/
BMessenger::BMessenger(const BMessenger &from)
		  : fPort(from.fPort),
			fHandlerToken(from.fHandlerToken),
			fTeam(from.fTeam),
			fPreferredTarget(from.fPreferredTarget)
{
}
//------------------------------------------------------------------------------
// destructor
/*!	\brief Frees all resources associated with this object.
*/
BMessenger::~BMessenger()
{
}
//------------------------------------------------------------------------------
// IsTargetLocal
/*!	\brief Returns whether or not the messenger's target lives within the team
	of the caller.

	\return \c true, if the object is properly initialized and its target
			lives within the caller's team, \c false otherwise.
*/
bool
BMessenger::IsTargetLocal() const
{
	thread_info info;
	return (get_thread_info(find_thread(NULL), &info) == B_OK
			&& fTeam == info.team);
}
//------------------------------------------------------------------------------
/*!	\brief Returns the handler and looper targeted by the messenger, if the
	target is local.

	The handler is returned directly, the looper by reference. If both are
	\c NULL, the object is either not properly initialized, the target
	objects have been deleted or the target is remote. If only the returned
	handler is \c NULL, either the looper's preferred handler is targeted or
	the handler has been deleted.

	\param looper A pointer to a pre-allocated BLooper pointer into which
		   the pointer to the targeted looper is written.
	\return The BHandler targeted by the messenger.
*/
BHandler *
BMessenger::Target(BLooper **looper) const
{
	BHandler *handler = NULL;
	if (IsTargetLocal()) {
		if (!fPreferredTarget) {
			gDefaultTokens.GetToken(fHandlerToken, B_HANDLER_TOKEN,
									(void**)&handler);
		}
		if (looper)
			*looper = BPrivate::gLooperList.LooperForPort(fPort);
	} else if (looper)
		*looper = NULL;
	return handler;
}
//------------------------------------------------------------------------------
/*!	\brief Locks the BLooper targeted by the messenger, if the target is local.

	This method is a shorthand for retrieving the targeted looper via
	Target() and calling BLooper::Lock() on the looper afterwards.

	\see BLooper::Lock() for details.

	\return \c true, if the looper could be locked sucessfully, \c false, if
			the messenger is not properly initialized, the target is remote,
			or the targeted looper is invalid.
*/
bool
BMessenger::LockTarget() const
{
	BLooper *looper = NULL;
	Target(&looper);
	return (looper && looper->Lock());
}
//------------------------------------------------------------------------------
/*!	\brief Locks the BLooper targeted by the messenger, if the target is local.

	This method is a shorthand for retrieving the targeted looper via
	Target() and calling BLooper::LockWithTimeout() on the looper afterwards.

	\see BLooper::LockWithTimeout() for details.

	\return
	- \c B_OK, if the looper could be locked sucessfully,
	- \c B_BAD_VALUE, if the messenger is not properly initialized,
	  the target is remote, or the targeted looper is invalid,
	- other error codes returned by BLooper::LockWithTimeout().
*/
status_t
BMessenger::LockTargetWithTimeout(bigtime_t timeout) const
{
	BLooper *looper = NULL;
	Target(&looper);
	status_t error = (looper ? B_OK : B_BAD_VALUE);
	if (error == B_OK)
		error = looper->LockWithTimeout(timeout);
	return error;
}
//------------------------------------------------------------------------------
// SendMessage
/*! \brief Delivers a BMessage synchronously to the messenger's target.

	The method does not wait for a reply. The message is sent asynchronously.

	\param command The what field of the message to deliver.
	\param replyTo The handler to which a reply to the message shall be sent.
		   May be \c NULL.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_PORT_ID: The messenger is not properly initialized or its
	  target doesn't exist anymore.
*/
status_t
BMessenger::SendMessage(uint32 command, BHandler *replyTo) const
{
	BMessage message(command);
	return SendMessage(&message, replyTo);
}
//------------------------------------------------------------------------------
/*! \brief Delivers a BMessage synchronously to the messenger's target.

	A copy of the supplied message is sent and the caller retains ownership
	of \a message.

	The method does not wait for a reply. The message is sent asynchronously.

	\param message The message to be sent.
	\param replyTo The handler to which a reply to the message shall be sent.
		   May be \c NULL.
	\param timeout A timeout for the delivery of the message.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_PORT_ID: The messenger is not properly initialized or its
	  target doesn't exist anymore.
	- \c B_WOULD_BLOCK: A delivery timeout of 0 was supplied and the target
	  port was full when trying to deliver the message.
	- \c B_TIMED_OUT: The timeout expired while trying to deliver the
	  message.
*/
status_t
BMessenger::SendMessage(BMessage *message, BHandler *replyTo,
						bigtime_t timeout) const
{
DBG(OUT("BMessenger::SendMessage2(%.4s)\n", (char*)&message->what));
	status_t error = (message ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		BMessenger replyMessenger(replyTo);
		error = SendMessage(message, replyMessenger, timeout);
	}
DBG(OUT("BMessenger::SendMessage2() done: %lx\n", error));
	return error;
}
//------------------------------------------------------------------------------
/*! \brief Delivers a BMessage synchronously to the messenger's target.

	A copy of the supplied message is sent and the caller retains ownership
	of \a message.

	The method does not wait for a reply. The message is sent asynchronously.

	\param message The message to be sent.
	\param replyTo A messenger specifying the target for a reply to \a message.
	\param timeout A timeout for the delivery of the message.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_PORT_ID: The messenger is not properly initialized or its
	  target doesn't exist anymore.
	- \c B_WOULD_BLOCK: A delivery timeout of 0 was supplied and the target
	  port was full when trying to deliver the message.
	- \c B_TIMED_OUT: The timeout expired while trying to deliver the
	  message.
*/
status_t
BMessenger::SendMessage(BMessage *message, BMessenger replyTo,
						bigtime_t timeout) const
{
	status_t error = (message ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		// If the reply messenger is invalid use the app messenger.
		if (!replyTo.IsValid())
			replyTo = be_app_messenger;
		error = message->_send_(fPort, fHandlerToken, fPreferredTarget,
								timeout, false, replyTo);
	}
	return error;
}
//------------------------------------------------------------------------------
/*! \brief Delivers a BMessage synchronously to the messenger's target and
	waits for a reply.

	The method does wait for a reply. The reply message is copied into
	\a reply. If the target doesn't send a reply, the \c what field of
	\a reply is set to \c B_NO_REPLY.

	\param command The what field of the message to deliver.
	\param reply A pointer to a pre-allocated BMessage into which the reply
		   message will be copied.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_PORT_ID: The messenger is not properly initialized or its
	  target doesn't exist anymore.
	- \c B_NO_MORE_PORTS: All reply ports are in use.
*/
status_t
BMessenger::SendMessage(uint32 command, BMessage *reply) const
{
	BMessage message(command);
	return SendMessage(&message, reply);
}
//------------------------------------------------------------------------------
/*! \brief Delivers a BMessage synchronously to the messenger's target and
	waits for a reply.

	A copy of the supplied message is sent and the caller retains ownership
	of \a message.

	The method does wait for a reply. The reply message is copied into
	\a reply. If the target doesn't send a reply or if a reply timeout occurs,
	the \c what field of \a reply is set to \c B_NO_REPLY.

	\param message The message to be sent.
	\param reply A pointer to a pre-allocated BMessage into which the reply
		   message will be copied.
	\param deliveryTimeout A timeout for the delivery of the message.
	\param replyTimeout A timeout for waiting for the reply.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_PORT_ID: The messenger is not properly initialized or its
	  target doesn't exist anymore.
	- \c B_WOULD_BLOCK: A delivery timeout of 0 was supplied and the target
	  port was full when trying to deliver the message.
	- \c B_TIMED_OUT: The timeout expired while trying to deliver the
	  message.
	- \c B_NO_MORE_PORTS: All reply ports are in use.
*/
status_t
BMessenger::SendMessage(BMessage *message, BMessage *reply,
						bigtime_t deliveryTimeout, bigtime_t replyTimeout) const
{
	status_t error = (message && reply ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		error = message->send_message(fPort, fTeam, fHandlerToken,
									  fPreferredTarget, reply, deliveryTimeout,
									  replyTimeout);
		// Map this error for now:
		if (error == B_BAD_TEAM_ID)
			error = B_BAD_PORT_ID;
	}
	return error;
}
//------------------------------------------------------------------------------
/*!	\brief Makes this BMessenger a copy of the supplied one.

	\param from the messenger to be copied.
	\return A reference to this object.
*/
BMessenger &
BMessenger::operator=(const BMessenger &from)
{
	if (this != &from) {
		fPort = from.fPort;
		fHandlerToken = from.fHandlerToken;
		fTeam = from.fTeam;
		fPreferredTarget = from.fPreferredTarget;
	}
	return *this;
}
//------------------------------------------------------------------------------
/*!	\brief Returns whether this and the supplied messenger have the same
	target.

	\param other The other messenger.
	\return \c true, if the messengers have the same target or if both aren't
			properly initialzed, \c false otherwise.
*/
bool
BMessenger::operator==(const BMessenger &other) const
{
	// Note: The fTeam fields are not compared.
	return (fPort == other.fPort
			&& fHandlerToken == other.fHandlerToken
			&& fPreferredTarget == other.fPreferredTarget);
}
//------------------------------------------------------------------------------
/*!	\brief Returns whether the messenger's target looper does still exist.

	It is not checked whether the target handler is also still existing.

	\return \c true, if the messenger's target looper does still exist,
			\c false otherwise.
*/
bool
BMessenger::IsValid() const
{
	port_info info;
	return (fPort >= 0 && get_port_info(fPort, &info) == B_OK);
}
//------------------------------------------------------------------------------
/*!	\brief Returns the ID of the team the messenger's target lives in.

	\return The team of the messenger's target.
*/
team_id
BMessenger::Team() const
{
	return fTeam;
}


// Private or reserved ---------------------------------------------------------
/*!	\brief Creates a BMessenger and initializes it to team, target looper port
	and handler token.

	If \a preferred is \c true, \a token is ignored.

	\param team The target's team.
	\param port The target looper port.
	\param token The target handler token.
	\param preferred \c true to rather use the looper's preferred handler
		   instead of the one specified by \a token.
*/
BMessenger::BMessenger(team_id team, port_id port, int32 token, bool preferred)
		  : fPort(-1),
			fHandlerToken(-1),
			fTeam(-1),
			fPreferredTarget(false)
{
	fTeam = team;
	fPort = port;
	fHandlerToken = token;
	fPreferredTarget = preferred;
}
//------------------------------------------------------------------------------
/*!	\brief Initializes the BMessenger object's data given the signature and/or
	team ID of a target.

	When only a signature is given, and multiple instances of the application
	are running it is undeterminate which one is chosen as the target. In case
	only a team ID is passed, the target application is identified uniquely.
	If both are supplied, the application identified by the team ID must have
	a matching signature, otherwise the initilization fails.

	\param signature The target application's signature. May be \c NULL.
	\param team The target application's team ID. May be < 0.
	\param result An optional pointer to a pre-allocated status_t into which
		   the result of the initialization is written.
*/
void
BMessenger::InitData(const char *signature, team_id team, status_t *result)
{
	/* TODO: uncomment this after implementing BRoster */
	
	/*status_t error = B_OK;
	// get an app_info
	app_info info;
	if (team < 0) {
		// no team ID given
		if (signature) {
			error = be_roster->GetAppInfo(signature, &info);
			team = info.team;
			// B_ERROR means that no application with the given signature
			// is running. But we are supposed to return B_BAD_VALUE.
			if (error == B_ERROR)
				error = B_BAD_VALUE;
		} else
			error = B_BAD_TYPE;
	} else {
		// a team ID is given
		error = be_roster->GetRunningAppInfo(team, &info);
		// Compare the returned signature with the supplied one.
		if (error == B_OK && signature && strcmp(signature, info.signature))
			error = B_MISMATCHED_VALUES;
	}
	// check whether the app flags say B_ARGV_ONLY
	if (error == B_OK && (info.flags & B_ARGV_ONLY)) {
		error = B_BAD_TYPE;
		// Set the team ID nevertheless -- that's what Be's implementation
		// does. Don't know, if that is a bug, but at least it doesn't harm.
		fTeam = team;
	}
	// init our members
	if (error == B_OK) {
		fTeam = team;
		fPort = info.port;
		fHandlerToken = 0;
		fPreferredTarget = true;
	}
	// return the error
	if (result)
		*result = error;*/
}
//------------------------------------------------------------------------------
/*!	\brief Returns whether the first one of two BMessengers is less than the
	second one.

	This method defines an order on BMessengers based on their member
	variables \c fPort, \c fHandlerToken and \c fPreferredTarget.

	\param a The first messenger.
	\param b The second messenger.
	\return \c true, if \a a is less than \a b, \c false otherwise.
*/
bool
operator<(const BMessenger &a, const BMessenger &b)
{
	// significance:
	// 1. fPort
	// 2. fHandlerToken
	// 3. fPreferredTarget
	// fTeam is insignificant
	return (a.fPort < b.fPort
			|| a.fPort == b.fPort
				&& (a.fHandlerToken < b.fHandlerToken
					|| a.fHandlerToken == b.fHandlerToken
						&& !a.fPreferredTarget
						&& b.fPreferredTarget));
}
//------------------------------------------------------------------------------
/*!	\brief Returns whether two BMessengers have not the same target.

	\param a The first messenger.
	\param b The second messenger.
	\return \c false, if \a a and \a b have the same targets or are both not
			properly initialized, \c true otherwise.
*/
bool
operator!=(const BMessenger &a, const BMessenger &b)
{
	return !(a == b);
}
//------------------------------------------------------------------------------
