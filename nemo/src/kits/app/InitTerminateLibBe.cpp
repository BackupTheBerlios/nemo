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
//	File Name:		InitTerminateLibBe.cpp
//	Author(s):		Ingo Weinhold (bonefish@users.sf.net)
//	Description:	Global library initialization/termination routines.
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <stdio.h>

// System Includes -------------------------------------------------------------
#include <MessagePrivate.h>
#include <RosterPrivate.h>

// Debugging -------------------------------------------------------------------
#include "nemo_debug.h"
#if DEBUG_LIBBE
	#define OUT(x...)	fprintf(LIBBE_LOG, "libbe: "x);
	#define DBG(x)		x;
#else
	#define DBG(x)		;
#endif


//------------------------------------------------------------------------------
/*extern "C"
void __attribute__ ((constructor)) libbe_init(void)
{
	DBG("Initializing\n");

	_init_message_();
	// TODO: decide whether the roster will be kept or not
	//_init_roster_();
}
//------------------------------------------------------------------------------
extern "C"
void __attribute__ ((destructor)) libbe_fini(void)
{
	DBG("Terminating\n");

	//_delete_roster_();
	_delete_message_();
	_msg_cache_cleanup_();
}*/
//------------------------------------------------------------------------------
/* gcc's attributes refuse to work here, so as a work around the following
 * class is instantiated globally so that its constructor and destructor will
 * provide an alternative to __attribute__ (constructor) &
 * __attribute__ (destructor) functions.
 */
class InitTerminate {
public:
	InitTerminate() {
		DBG(OUT("Initializing\n"));

		_init_message_();
		// TODO: decide whether the roster will be kept or not
		//_init_roster_();
	}
	
	~InitTerminate() {
		DBG(OUT("Terminating\n"));

		//_delete_roster_();
		_delete_message_();
		_msg_cache_cleanup_();
	}
} initTerminate;
//------------------------------------------------------------------------------
