//
//	$Id: Autolock.h,v 1.1 2004/03/19 12:58:48 fadi_edward Exp $
//
//	This is the BAutolock interface for OpenBeOS.  It has been created to
//	be source and binary compatible with the BeOS version of BAutolock.
//  To that end, all members are inline just as with the BeOS version.
//

#ifndef	_AUTOLOCK_H
#define	_AUTOLOCK_H

#include <Locker.h>
#include <Looper.h>

//=============================================================================

class BAutolock {
	
public:
inline						BAutolock(BLooper *looper);
inline						BAutolock(BLocker *locker);
inline						BAutolock(BLocker &locker);
	
inline						~BAutolock();
	
inline	bool				IsLocked(void);

private:
		BLocker				*fTheLocker;
		BLooper				*fTheLooper;
		bool				fIsLocked;
};

//=============================================================================

inline BAutolock::BAutolock(BLooper *looper) :
	fTheLocker(NULL),
	fTheLooper(looper),
	fIsLocked(looper->Lock())
{
}

//=============================================================================

inline BAutolock::BAutolock(BLocker *locker) :
	fTheLocker(locker),
	fTheLooper(NULL),
	fIsLocked(locker->Lock())
{
}

//=============================================================================

inline BAutolock::BAutolock(BLocker &locker) :
	fTheLocker(&locker),
	fTheLooper(NULL),
	fIsLocked(locker.Lock())
{
}

//=============================================================================

inline BAutolock::~BAutolock()
{
	if (fIsLocked) {
		if (fTheLooper != NULL) {
			fTheLooper->Unlock();
		} else {
			fTheLocker->Unlock();
		}
	}
}

//=============================================================================

inline bool BAutolock::IsLocked()
{
	return fIsLocked;
}

//=============================================================================

#endif // _AUTOLOCK_H
