// Standard Includes -----------------------------------------------------------
#include <algorithm>

// System Includes -------------------------------------------------------------
#include <Handler.h>
#include <Message.h>
#include <Messenger.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include <ObserverList.h>

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

namespace BPrivate {
//------------------------------------------------------------------------------
//	#pragma mark -
//	#pragma mark _ObserverList
//	#pramga mark -
//------------------------------------------------------------------------------
BObserverList::BObserverList(void)
{
}
//------------------------------------------------------------------------------
BObserverList::~BObserverList(void)
{
}
//------------------------------------------------------------------------------
status_t BObserverList::SendNotices(unsigned long what, BMessage const* message)
{
	// Having to new a temporary is really irritating ...
	BMessage* copyMsg = NULL;
	if (message)
	{
		copyMsg = new BMessage(*message);
		copyMsg->what = B_OBSERVER_NOTICE_CHANGE;
		copyMsg->AddInt32(B_OBSERVE_ORIGINAL_WHAT, message->what);
	}
	else
	{
		copyMsg = new BMessage(B_OBSERVER_NOTICE_CHANGE);
	}

	copyMsg->AddInt32(B_OBSERVE_WHAT_CHANGE, what);

	vector<BHandler*>& handlers = fHandlerMap[what];
	for (uint32 i = 0; i < handlers.size(); ++i)
	{
		BMessenger msgr(handlers[i]);
		msgr.SendMessage(copyMsg);
	}

	vector<BMessenger>& messengers = fMessengerMap[what];
	for (uint32 i = 0; i < messengers.size(); ++i)
	{
		messengers[i].SendMessage(copyMsg);
	}

	// Gotta make sure to clean up the annoying temporary ...
	delete copyMsg;

	return B_OK;
}
//------------------------------------------------------------------------------
status_t BObserverList::StartObserving(BHandler* handler, unsigned long what)
{
	if (!handler)
	{
		return B_BAD_HANDLER;
	}

	vector<BHandler*>& handlers = fHandlerMap[what];
	vector<BHandler*>::iterator iter;
	iter = find(handlers.begin(), handlers.end(), handler);
	if (iter != handlers.end())
	{
		// TODO: verify
		return B_OK;
	}

	handlers.push_back(handler);
	return B_OK;
}
//------------------------------------------------------------------------------
status_t BObserverList::StartObserving(const BMessenger& messenger,
									   unsigned long what)
{
	vector<BMessenger>& messengers = fMessengerMap[what];
	vector<BMessenger>::iterator iter;
	iter = find(messengers.begin(), messengers.end(), messenger);
	if (iter != messengers.end())
	{
		// TODO: verify
		return B_OK;
	}

	messengers.push_back(messenger);
	return B_OK;
}
//------------------------------------------------------------------------------
status_t BObserverList::StopObserving(BHandler* handler, unsigned long what)
{
	if (handler)
	{
		vector<BHandler*>& handlers = fHandlerMap[what];
		vector<BHandler*>::iterator iter;
		iter = find(handlers.begin(), handlers.end(), handler);
		if (iter != handlers.end())
		{
			handlers.erase(iter);
			if (handlers.empty())
			{
				fHandlerMap.erase(what);
			}
			return B_OK;
		}
	}

	return B_BAD_HANDLER;
}
//------------------------------------------------------------------------------
status_t BObserverList::StopObserving(const BMessenger& messenger,
									  unsigned long what)
{
	// ???:	What if you call StartWatching(MyMsngr, aWhat) and then call
	//		StopWatchingAll(MyMsnger)?  Will MyMsnger be removed from the aWhat
	//		watcher list?  For now, we'll assume that they're discreet lists
	//		which do no cross checking; i.e., MyMsnger would *not* be removed in
	//		this scenario.
	vector<BMessenger>& messengers = fMessengerMap[what];
	vector<BMessenger>::iterator iter;
	iter = find(messengers.begin(), messengers.end(), messenger);
	if (iter != messengers.end())
	{
		messengers.erase(iter);
		if (messengers.empty())
		{
			fMessengerMap.erase(what);
		}
		return B_OK;
	}

	return B_BAD_HANDLER;
}
//------------------------------------------------------------------------------
bool BObserverList::IsEmpty()
{
	return fHandlerMap.empty() && fMessengerMap.empty();
}
//------------------------------------------------------------------------------

} // namespace BPrivate
