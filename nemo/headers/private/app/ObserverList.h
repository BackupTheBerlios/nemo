#ifndef _OBSERVERLIST_H
#define _OBSERVERLIST_H

// Standard Includes -----------------------------------------------------------
#include <vector>
#include <map>

// System Includes -------------------------------------------------------------
#include <OS.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------
#define B_OBSERVE_WHAT_CHANGE		"be:observe_change_what"
#define B_OBSERVE_ORIGINAL_WHAT		"be:observe_orig_what"

// Globals ---------------------------------------------------------------------
const uint32 B_OBSERVER_OBSERVE_ALL	= 0xffffffff;

class BHandler;
class BMessage;
class BMessenger;

using namespace std;
typedef map<unsigned long, vector<BHandler*> >	THandlerObserverMap;
typedef map<unsigned long, vector<BMessenger> >	TMessengerObserverMap;

//------------------------------------------------------------------------------
namespace BPrivate {

//------------------------------------------------------------------------------
class BObserverList
{
public:
					BObserverList(void);
					~BObserverList(void);
					
		status_t	SendNotices(unsigned long, BMessage const *);
		status_t	StartObserving(BHandler *, unsigned long);
		status_t	StartObserving(const BMessenger&, unsigned long);
		status_t	StopObserving(BHandler *, unsigned long);
		status_t	StopObserving(const BMessenger&, unsigned long);
		bool		IsEmpty();

private:
		THandlerObserverMap		fHandlerMap;
		TMessengerObserverMap	fMessengerMap;
};
//------------------------------------------------------------------------------

} // namespace BPrivate
//------------------------------------------------------------------------------

#endif // _OBSERVERLIST_H
