//------------------------------------------------------------------------------
//	$Id: MessageQueue.h,v 1.2 2004/04/15 01:31:11 mahmoudfg Exp $
//
//	This is the BMessageQueue interface for OpenBeOS.  It has been created
//  to be source and binary compatible with the BeOS version of
//  BMessageQueue.
//------------------------------------------------------------------------------

#ifndef	_MESSAGEQUEUE_H
#define	_MESSAGEQUEUE_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

class BLocker;
class BMessage;

// BMessageQueue class ---------------------------------------------------------
class BMessageQueue {
public:
						BMessageQueue();
virtual					~BMessageQueue();
	
		void			AddMessage(BMessage *message);
		void			RemoveMessage(BMessage *message);
	
		int32 			CountMessages(void) const;
		bool 			IsEmpty(void) const;
	
		BMessage*		FindMessage(int32 index) const;
		BMessage*		FindMessage(uint32 what, int32 index=0) const;
		
		bool 			Lock(void);
		void 			Unlock(void);
		bool 			IsLocked(void);

		BMessage*		NextMessage(void);

private:
						BMessageQueue(const BMessageQueue &);
						BMessageQueue &operator=(const BMessageQueue &);
	
		BMessage		*fTheQueue;
		BMessage		*fQueueTail;
		int32			fMessageCount;
		BLocker			fLocker;
};
//------------------------------------------------------------------------------

#endif // _MESSAGEQUEUE_H
