//
//	$Id: Locker.h,v 1.1 2004/03/19 12:58:53 fadi_edward Exp $
//
//	This is the BLocker interface for OpenBeOS.  It has been created to
//	be source and binary compatible with the BeOS version of BLocker.
//


#ifndef	_LOCKER_H
#define	_LOCKER_H

#include <OS.h>
#include <SupportDefs.h>

//=============================================================================

class BLocker {

public:
							BLocker();
							BLocker(const char *name);
							BLocker(bool benaphore_style);
							BLocker(const char *name, bool benaphore_style);
	
							// The following constructor is not documented in the BeBook
							// and is only listed here to ensure binary compatibility.
							// DO NOT USE THIS CONSTRUCTOR!
							BLocker(const char *name, bool benaphore_style, bool);

virtual						~BLocker();

		bool				Lock(void);
		status_t			LockWithTimeout(bigtime_t timeout);
		void				Unlock(void);

		thread_id			LockingThread(void) const;
		bool 				IsLocked(void) const;
		int32 				CountLocks(void) const;
		int32 				CountLockRequests(void) const;
		sem_id				Sem(void) const;

private:
		void				InitLocker(const char *name, bool benaphore_style);
		bool 				AcquireLock(bigtime_t timeout, status_t *error);

		int32				fBenaphoreCount;
		sem_id				fSemaphoreID;
		thread_id			fLockOwner;
		int32 				fRecursiveCount;
};

//=============================================================================

#endif // _LOCKER_H
