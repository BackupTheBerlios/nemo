// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
//#include <Handler.h>
//#include <Message.h>
//#include <Messenger.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include <ObserverList.h>

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

//namespace BPrivate {
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
/*status_t BObserverList::SendNotices(unsigned long what, BMessage const* Message)
{
	// Having to new a temporary is really irritating ...
	BMessage* CopyMsg = NULL;
	if (Message)
	{
		CopyMsg = new BMessage(*Message);
		CopyMsg->what = B_OBSERVER_NOTICE_CHANGE;
		CopyMsg->AddInt32(B_OBSERVE_ORIGINAL_WHAT, Message->what);
	}
	else
	{
		CopyMsg = new BMessage(B_OBSERVER_NOTICE_CHANGE);
	}

	CopyMsg->AddInt32(B_OBSERVE_WHAT_CHANGE, what);

	vector<BHandler*>& Handlers = fHandlerMap[what];
	for (uint32 i = 0; i < Handlers.size(); ++i)
	{
		BMessenger msgr(Handlers[i]);
		msgr.SendMessage(CopyMsg);
	}

	vector<BMessenger>& Messengers = fMessengerMap[what];
	for (uint32 i = 0; i < Messengers.size(); ++i)
	{
		Messengers[i].SendMessage(CopyMsg);
	}

	// Gotta make sure to clean up the annoying temporary ...
	delete CopyMsg;

	return B_OK;
}*/
//------------------------------------------------------------------------------
/*status_t BObserverList::StartObserving(BHandler* Handler, unsigned long what)
{
	if (!Handler)
	{
		return B_BAD_HANDLER;
	}

	vector<BHandler*>& Handlers = fHandlerMap[what];
	vector<BHandler*>::iterator iter;
	iter = find(Handlers.begin(), Handlers.end(), Handler);
	if (iter != Handlers.end())
	{
		// TODO: verify
		return B_OK;
	}

	Handlers.push_back(Handler);
	return B_OK;
}*/
//------------------------------------------------------------------------------
/*status_t BObserverList::StartObserving(const BMessenger& Messenger,
									   unsigned long what)
{
	vector<BMessenger>& Messengers = fMessengerMap[what];
	vector<BMessenger>::iterator iter;
	iter = find(Messengers.begin(), Messengers.end(), Messenger);
	if (iter != Messengers.end())
	{
		// TODO: verify
		return B_OK;
	}

	Messengers.push_back(Messenger);
	return B_OK;
}*/
//------------------------------------------------------------------------------
/*status_t BObserverList::StopObserving(BHandler* Handler, unsigned long what)
{
	if (Handler)
	{
		vector<BHandler*>& Handlers = fHandlerMap[what];
		vector<BHandler*>::iterator iter;
		iter = find(Handlers.begin(), Handlers.end(), Handler);
		if (iter != Handlers.end())
		{
			Handlers.erase(iter);
			if (Handlers.empty())
			{
				fHandlerMap.erase(what);
			}
			return B_OK;
		}
	}

	return B_BAD_HANDLER;
}*/
//------------------------------------------------------------------------------
/*status_t BObserverList::StopObserving(const BMessenger& Messenger,
									  unsigned long what)
{
	// ???:	What if you call StartWatching(MyMsngr, aWhat) and then call
	//		StopWatchingAll(MyMsnger)?  Will MyMsnger be removed from the aWhat
	//		watcher list?  For now, we'll assume that they're discreet lists
	//		which do no cross checking; i.e., MyMsnger would *not* be removed in
	//		this scenario.
	vector<BMessenger>& Messengers = fMessengerMap[what];
	vector<BMessenger>::iterator iter;
	iter = find(Messengers.begin(), Messengers.end(), Messenger);
	if (iter != Messengers.end())
	{
		Messengers.erase(iter);
		if (Messengers.empty())
		{
			fMessengerMap.erase(what);
		}
		return B_OK;
	}

	return B_BAD_HANDLER;
}*/
//------------------------------------------------------------------------------
/*bool BObserverList::IsEmpty()
{
	return fHandlerMap.empty() && fMessengerMap.empty();
}*/
//------------------------------------------------------------------------------

//} // namespace BPrivate
