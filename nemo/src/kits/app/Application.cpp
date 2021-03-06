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
//	File Name:		Application.cpp
//	Author:			Erik Jaesler (erik@cgsoftware.com)
//	Description:	BApplication class is the center of the application
//					universe.  The global be_app and be_app_messenger
//					variables are defined here as well.
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <new>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// System Includes -------------------------------------------------------------
#include <AppFileInfo.h>
#include <Application.h>
#include <AppMisc.h>
//#include <Cursor.h>
//#include <Entry.h>
//#include <File.h>
#include <Locker.h>
//#include <Path.h>
//#include <PropertyInfo.h>
#include <RegistrarDefs.h>
#include <Roster.h>
#include <RosterPrivate.h>
//#include <Window.h>

// Project Includes ------------------------------------------------------------
#include <LooperList.h>
#include <ObjectLocker.h>
#include <AppServerLink.h>
#include <ServerProtocol.h>
#include <PortLink.h>
#include <PortMessage.h>

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Debugging -------------------------------------------------------------------
#include "nemo_debug.h"
#if DEBUG_LIBBE
	#define OUT(x...)	fprintf(LIBBE_LOG, "BApplication: "x);
	#define DBG(x)		x;
#else
	#define DBG(x)		;
#endif

// Uncomment this to run without the registrar - used only for app_server development!
#define RUN_WITHOUT_REGISTRAR


// Globals ---------------------------------------------------------------------
BApplication	*be_app = NULL;
BMessenger		be_app_messenger;

/*BResources*	BApplication::_app_resources = NULL;
BLocker		BApplication::_app_resources_lock("_app_resources_lock");
*/
/*property_info gApplicationPropInfo[] =
{
	{
		"Window",
			{},
			{B_INDEX_SPECIFIER, B_REVERSE_INDEX_SPECIFIER},
			NULL, 0,
			{},
			{},
			{}
	},
	{
		"Window",
			{},
			{B_NAME_SPECIFIER},
			NULL, 1,
			{},
			{},
			{}
	},
	{
		"Looper",
			{},
			{B_INDEX_SPECIFIER, B_REVERSE_INDEX_SPECIFIER},
			NULL, 2,
			{},
			{},
			{}
	},
	{
		"Looper",
			{},
			{B_ID_SPECIFIER},
			NULL, 3,
			{},
			{},
			{}
	},
	{
		"Looper",
			{},
			{B_NAME_SPECIFIER},
			NULL, 4,
			{},
			{},
			{}
	},
	{
		"Name",
			{B_GET_PROPERTY},
			{B_DIRECT_SPECIFIER},
			NULL, 5,
			{B_STRING_TYPE},
			{},
			{}
	},
	{
		"Window",
			{B_COUNT_PROPERTIES},
			{B_DIRECT_SPECIFIER},
			NULL, 5,
			{B_INT32_TYPE},
			{},
			{}
	},
	{
		"Loopers",
			{B_GET_PROPERTY},
			{B_DIRECT_SPECIFIER},
			NULL, 5,
			{B_MESSENGER_TYPE},
			{},
			{}
	},
	{
		"Windows",
			{B_GET_PROPERTY},
			{B_DIRECT_SPECIFIER},
			NULL, 5,
			{B_MESSENGER_TYPE},
			{},
			{}
	},
	{
		"Looper",
			{B_COUNT_PROPERTIES},
			{B_DIRECT_SPECIFIER},
			NULL, 5,
			{B_INT32_TYPE},
			{},
			{}
	},
	{}
};*/

// argc/argv
extern const int __libc_argc;
extern const char * const *__libc_argv;

/*class BMenuWindow : public BWindow
{
};*/
//------------------------------------------------------------------------------

enum {
	NOT_IMPLEMENTED	= B_ERROR,
};

// prototypes of helper functions
static const char* looper_name_for(const char *signature);
static status_t check_app_signature(const char *signature);

//------------------------------------------------------------------------------
BApplication::BApplication(const char* signature)
			: BLooper(looper_name_for(signature)),
			  fAppName(NULL),
			  fServerFrom(-1),
			  fServerTo(-1),
			  fPulseRate(0),
			  fInitialWorkspace(0),
//			  fDraggedMessage(NULL),
//			  fPulseRunner(NULL),
			  fInitError(B_NO_INIT),
			  fReadyToRunCalled(false)
{
	InitData(signature, NULL);
}
//------------------------------------------------------------------------------
BApplication::BApplication(const char* signature, status_t* error)
			: BLooper(looper_name_for(signature)),
			  fAppName(NULL),
			  fServerFrom(-1),
			  fServerTo(-1),
			  fPulseRate(0),
			  fInitialWorkspace(0),
//			  fDraggedMessage(NULL),
//			  fPulseRunner(NULL),
			  fInitError(B_NO_INIT),
			  fReadyToRunCalled(false)
{
	InitData(signature, error);
}
//------------------------------------------------------------------------------
BApplication::~BApplication()
{
	// TODO: tell all loopers (windows) to quit and wait for them
	
#ifndef RUN_WITHOUT_REGISTRAR
	// unregister from the roster
	BRoster::Private().RemoveApp(Team());
#endif

	// tell the app_server that we're quitting
	PortMessage msg;
	msg.SetCode(B_QUIT_REQUESTED);
	msg.WriteToPort(fServerFrom);
	
	// delete the local port
	delete_port(fServerTo);
	
	// uninitialize be_app and be_app_messenger
	be_app = NULL;

// R5 doesn't uninitialize be_app_messenger.
//	be_app_messenger = BMessenger();
}
//------------------------------------------------------------------------------
/*BApplication::BApplication(BMessage* data)
			: BLooper(looper_name_for(NULL)),
			  fAppName(NULL),
			  fServerFrom(-1),
			  fServerTo(-1),
			  fServerHeap(NULL),
			  fPulseRate(0),
			  fInitialWorkspace(0),
			  fDraggedMessage(NULL),
			  fPulseRunner(NULL),
			  fInitError(B_NO_INIT),
			  fReadyToRunCalled(false)
{
}
//------------------------------------------------------------------------------
BArchivable* BApplication::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data, "BApplication"))
	{
		return NULL;
	}

	return new BApplication(data);
}
//------------------------------------------------------------------------------
status_t BApplication::Archive(BMessage* data, bool deep) const
{
	return NOT_IMPLEMENTED;
}*/
//------------------------------------------------------------------------------
status_t BApplication::InitCheck() const
{
	return fInitError;
}
//------------------------------------------------------------------------------
thread_id BApplication::Run()
{
	AssertLocked();

	if (fRunCalled)
	{
		// Not allowed to call Run() more than once
		// TODO: test
		// find out what message is actually here
		debugger("You can't call Run() on a BApplication twice");
	}

	// Note: We need a local variable too (for the return value), since
	// fTaskID is cleared by Quit().
	thread_id thread = fTaskID = find_thread(NULL);

	if (fMsgPort == B_NO_MORE_PORTS || fMsgPort == B_BAD_VALUE)
	{
		return fMsgPort;
	}

	fRunCalled = true;

	run_task();

	return thread;
}
//------------------------------------------------------------------------------
void BApplication::Quit()
{
	bool unlock = false;
	if (!IsLocked()) {
		const char* name = Name();
		if (!name)
			name = "no-name";
		printf("ERROR - you must Lock the application object before calling "
			   "Quit(), team=%ld, looper=%s\n", Team(), name);
		unlock = true;
		if (!Lock())
			return;
	}
	// Delete the object, if not running only.
	if (!fRunCalled) {
		delete this;
	} else if (find_thread(NULL) != fTaskID) {
		// We are not the looper thread.
		// We push a _QUIT_ into the queue.
		// TODO: When BLooper::AddMessage() is done, use that instead of
		// PostMessage()??? This would overtake messages that are still at
		// the port.
		// NOTE: We must not unlock here -- otherwise we had to re-lock, which
		// may not work. This is bad, since, if the port is full, it
		// won't get emptied, as the looper thread needs to lock the object
		// before dispatching messages.
		while (PostMessage(_QUIT_, this) == B_WOULD_BLOCK)
			snooze(10000);
	} else {
		// We are the looper thread.
		// Just set fTerminating to true which makes us fall through the
		// message dispatching loop and return from Run().
		fTerminating = true;
	}
	// If we had to lock the object, unlock now.
	if (unlock)
		Unlock();
}
//------------------------------------------------------------------------------
bool BApplication::QuitRequested()
{
	// No windows -- nothing to do.
	return BLooper::QuitRequested();
}
//------------------------------------------------------------------------------
void BApplication::Pulse()
{
}
//------------------------------------------------------------------------------
void BApplication::ReadyToRun()
{
}
//------------------------------------------------------------------------------
void BApplication::MessageReceived(BMessage* msg)
{
	BLooper::MessageReceived(msg);
}
//------------------------------------------------------------------------------
void BApplication::ArgvReceived(int32 argc, char** argv)
{
}
//------------------------------------------------------------------------------
void BApplication::AppActivated(bool active)
{
}
//------------------------------------------------------------------------------
void BApplication::RefsReceived(BMessage* a_message)
{
}
//------------------------------------------------------------------------------
void BApplication::AboutRequested()
{
}
//------------------------------------------------------------------------------
/*BHandler* BApplication::ResolveSpecifier(BMessage* msg, int32 index,
										 BMessage* specifier, int32 form,
										 const char* property)
{
	return NULL;	// not implemented
}
//------------------------------------------------------------------------------
void BApplication::ShowCursor()
{
	// Because we're just sending an opcode, we can skip the BSession and fake the protocol
	int32 foo=AS_SHOW_CURSOR;
	write_port(fServerTo,AS_SHOW_CURSOR,&foo,sizeof(int32));
}
//------------------------------------------------------------------------------
void BApplication::HideCursor()
{
	// Because we're just sending an opcode, we can skip the BSession and fake the protocol
	int32 foo=AS_HIDE_CURSOR;
	write_port(fServerTo,AS_HIDE_CURSOR,&foo,sizeof(int32));
}
//------------------------------------------------------------------------------
void BApplication::ObscureCursor()
{
	// Because we're just sending an opcode, we can skip the BSession and fake the protocol
	int32 foo=AS_OBSCURE_CURSOR;
	write_port(fServerTo,AS_OBSCURE_CURSOR,&foo,sizeof(int32));
}
//------------------------------------------------------------------------------
bool BApplication::IsCursorHidden() const
{
	PortMessage msg;

	BPrivate::BAppServerLink link;
	link.SetOpCode(AS_QUERY_CURSOR_HIDDEN);
	link.FlushWithReply(&msg);
	return (msg.Code()==SERVER_TRUE)?true:false;
}
//------------------------------------------------------------------------------
void BApplication::SetCursor(const void* cursor)
{
	// BeBook sez: If you want to call SetCursor() without forcing an immediate
	//				sync of the Application Server, you have to use a BCursor.
	// By deductive reasoning, this function forces a sync. =)
	BCursor Cursor(cursor);
	SetCursor(&Cursor, true);
}
//------------------------------------------------------------------------------
void BApplication::SetCursor(const BCursor* cursor, bool sync)
{
	BPrivate::BAppServerLink link;
	PortMessage msg;

	link.SetOpCode(AS_SET_CURSOR_BCURSOR);
	link.Attach<bool>(sync);
	link.Attach<int32>(cursor->m_serverToken);
	if(sync)
		link.FlushWithReply(&msg);
	else
		link.Flush();
}
//------------------------------------------------------------------------------
int32 BApplication::CountWindows() const
{
	// BeBook sez: The windows list includes all windows explicitely created by
	//				the app ... but excludes private windows create by Be
	//				classes.
	// I'm taking this to include private menu windows, thus the incl_menus
	// param is false.
	return count_windows(false);
}
//------------------------------------------------------------------------------
BWindow* BApplication::WindowAt(int32 index) const
{
	// BeBook sez: The windows list includes all windows explicitely created by
	//				the app ... but excludes private windows create by Be
	//				classes.
	// I'm taking this to include private menu windows, thus the incl_menus
	// param is false.
	return window_at(index, false);
}*/
//------------------------------------------------------------------------------
int32 BApplication::CountLoopers() const
{
	using namespace BPrivate;
	BObjectLocker<BLooperList> listLock(gLooperList);
	if (listLock.IsLocked())
	{
		return gLooperList.CountLoopers();
	}

	return B_ERROR;	// Some bad, non-specific thing has happened
}
//------------------------------------------------------------------------------
BLooper* BApplication::LooperAt(int32 index) const
{
	using namespace BPrivate;
	BLooper* looper = NULL;
	BObjectLocker<BLooperList> listLock(gLooperList);
	if (listLock.IsLocked())
	{
		looper = gLooperList.LooperAt(index);
	}

	return looper;
}
//------------------------------------------------------------------------------
bool BApplication::IsLaunching() const
{
	return !fReadyToRunCalled;
}
//------------------------------------------------------------------------------
/*status_t BApplication::GetAppInfo(app_info* info) const
{
	return be_roster->GetRunningAppInfo(be_app->Team(), info);
}*/
//------------------------------------------------------------------------------
/*BResources* BApplication::AppResources()
{
	return NULL;	// not implemented
}*/
//------------------------------------------------------------------------------
void BApplication::DispatchMessage(BMessage* message, BHandler* handler)
{
	switch (message->what) {
		/* Hook function triggers */
		
		/* TODO: uncomment this when BMessage::FindString() is implemented */
		/*case B_ARGV_RECEIVED:
		{
			// build the argv vector
			status_t error = B_OK;
			int32 argc;
			char **argv = NULL;
			if (message->FindInt32("argc", &argc) == B_OK && argc > 0) {
				argv = new char*[argc];
				for (int32 i = 0; error == B_OK && i < argc; i++)
					argv[i] = NULL;
				// copy the arguments
				for (int32 i = 0; error == B_OK && i < argc; i++) {
					const char *arg = NULL;
					error = message->FindString("argv", i, &arg);
					if (error == B_OK && arg) {
						argv[i] = new(std::nothrow) char[strlen(arg) + 1];
						if (argv[i])
							strcpy(argv[i], arg);
						else
							error = B_NO_MEMORY;
					}
				}
			}
			// call the hook
			if (error == B_OK)
				ArgvReceived(argc, argv);
			// cleanup
			if (argv) {
				for (int32 i = 0; i < argc; i++)
					delete[] argv[i];
				delete[] argv;
			}
			break;
		}*/
		case B_REFS_RECEIVED: {
			RefsReceived(message);
		} break;
		
		case B_READY_TO_RUN: {
			// TODO: Find out, whether to set fReadyToRunCalled before or
			// after calling the hook.
			ReadyToRun();
			fReadyToRunCalled = true;
		} break;
		
		default: {
			BLooper::DispatchMessage(message, handler);
		} break;
	}
}
//------------------------------------------------------------------------------
void BApplication::SetPulseRate(bigtime_t rate)
{
	fPulseRate = rate;
}
//------------------------------------------------------------------------------
/*status_t BApplication::GetSupportedSuites(BMessage* data)
{
	status_t err = B_OK;
	if (!data)
	{
		err = B_BAD_VALUE;
	}

	if (!err)
	{
		err = data->AddString("Suites", "suite/vnd.Be-application");
		if (!err)
		{
			BPropertyInfo PropertyInfo(gApplicationPropInfo);
			err = data->AddFlat("message", &PropertyInfo);
			if (!err)
			{
				err = BHandler::GetSupportedSuites(data);
			}
		}
	}

	return err;
}*/
//------------------------------------------------------------------------------
status_t BApplication::Perform(perform_code d, void* arg)
{
	return NOT_IMPLEMENTED;
}
//------------------------------------------------------------------------------
BApplication::BApplication(uint32 signature)
{
}
//------------------------------------------------------------------------------
BApplication::BApplication(const BApplication& rhs)
{
}
//------------------------------------------------------------------------------
BApplication& BApplication::operator=(const BApplication& rhs)
{
	return *this;
}
//------------------------------------------------------------------------------
/*bool BApplication::ScriptReceived(BMessage* msg, int32 index, BMessage* specifier, int32 form, const char* property)
{
	return false;	// not implemented
}*/
//------------------------------------------------------------------------------
void BApplication::run_task()
{
	task_looper();
}
//------------------------------------------------------------------------------
void BApplication::InitData(const char* signature, status_t* error)
{
	// check whether there exists already an application
	if (be_app)
		debugger("2 BApplication objects were created. Only one is allowed.");

	// check signature
	fInitError = check_app_signature(signature);
	fAppName = signature;
	/* TODO: uncomment after implementing the registrar */
	/*bool isRegistrar = (signature && !strcmp(signature, kRegistrarSignature));*/
	bool isRegistrar = false;

	// get team and thread
	team_id team = Team();
	thread_id thread = BPrivate::main_thread_for(team);

	// get app executable ref
	/*	TODO: deal with entry_refs when implemented
	*/
	/*entry_ref ref;
	if (fInitError == B_OK)
		fInitError = BPrivate::get_app_ref(&ref);*/

	/*	TODO: deal with this when BFile is implemented
	*/
	// get the BAppFileInfo and extract the information we need
	/*uint32 appFlags = B_REG_DEFAULT_APP_FLAGS;
	if (fInitError == B_OK) {
		BAppFileInfo fileInfo;
		BFile file(&ref, B_READ_ONLY);
		fInitError = fileInfo.SetTo(&file);
		if (fInitError == B_OK) {
			fileInfo.GetAppFlags(&appFlags);
			char appFileSignature[B_MIME_TYPE_LENGTH + 1];
			// compare the file signature and the supplied signature
			if (fileInfo.GetSignature(appFileSignature) == B_OK) {
				if (strcmp(appFileSignature, signature) != 0) {
					printf("Signature in rsrc doesn't match constructor arg. "
						   "(%s,%s)\n", signature, appFileSignature);
				}
			}
		}
	}*/

#ifndef RUN_WITHOUT_REGISTRAR
	// check whether be_roster is valid
	if (fInitError == B_OK && !isRegistrar
		&& !BRoster::Private().IsMessengerValid(false)) {
		printf("FATAL: be_roster is not valid. Is the registrar running?\n");
		fInitError = B_NO_INIT;
	}

	// check whether or not we are pre-registered
	bool preRegistered = false;
	app_info appInfo;
	if (fInitError == B_OK && !isRegistrar) {
		preRegistered = BRoster::Private().IsAppPreRegistered(&ref, team,
															  &appInfo);
	}
	if (preRegistered) {
		// we are pre-registered => the app info has been filled in
		// Check whether we need to replace the looper port with a port
		// created by the roster.
		if (appInfo.port >= 0 && appInfo.port != fMsgPort) {
			delete_port(fMsgPort);
			fMsgPort = appInfo.port;
		}
		else appInfo.port = fMsgPort;

		// check the signature and correct it, if necessary
		if (strcmp(appInfo.signature, fAppName))
			BRoster::Private().SetSignature(team, fAppName);

		// complete the registration
		fInitError = BRoster::Private().CompleteRegistration(team, thread,
															 appInfo.port);
	}
	else if (fInitError == B_OK) {
		// not pre-registered -- try to register the application
		team_id otherTeam = -1;

		// the registrar must not register
		if (!isRegistrar) {
			fInitError = BRoster::Private().AddApplication(signature, &ref,
				appFlags, team, thread, fMsgPort, true, NULL, &otherTeam);
		}
		if (fInitError == B_ALREADY_RUNNING) {
			// An instance is already running and we asked for
			// single/exclusive launch. Send our argv to the running app.
			// Do that only, if the app is NOT B_ARGV_ONLY.
			if (otherTeam >= 0 && __libc_argc > 1) {
				app_info otherAppInfo;
				if (be_roster->GetRunningAppInfo(otherTeam, &otherAppInfo)
					== B_OK && !(otherAppInfo.flags & B_ARGV_ONLY)) {
					// create an B_ARGV_RECEIVED message
					BMessage argvMessage(B_ARGV_RECEIVED);
					do_argv(&argvMessage);
					// replace the first argv string with the path of the
					// other application
					BPath path;
					if (path.SetTo(&otherAppInfo.ref) == B_OK)
						argvMessage.ReplaceString("argv", 0, path.Path());
					// send the message
					BMessenger(NULL, otherTeam).SendMessage(&argvMessage);
				}
			}
		} else if (fInitError == B_OK) {
			// the registrations was successful
			// Create a B_ARGV_RECEIVED message and send it to ourselves.
			// Do that even, if we are B_ARGV_ONLY.
			// TODO: When BLooper::AddMessage() is done, use that instead of
			// PostMessage().
			if (__libc_argc > 1) {
				BMessage argvMessage(B_ARGV_RECEIVED);
				do_argv(&argvMessage);
				PostMessage(&argvMessage, this);
			}
			// send a B_READY_TO_RUN message as well
			PostMessage(B_READY_TO_RUN, this);
		} else if (fInitError > B_ERRORS_END) {
			// Registrar internal errors shouldn't fall into the user's hands.
			fInitError = B_ERROR;
		}
	}
#endif	// ifndef RUN_WITHOUT_REGISTRAR

	// Notify app_server that we exist
	fServerFrom = find_port(AS_PORT_NAME);
	if(fServerFrom >= B_OK)
	{
		// Create the port so that the app_server knows where to send messages
		fServerTo = create_port(100, "a<fServerTo");
		if(fServerTo >= B_OK)
		{
			/* AS_CREATE_APP:
			 * 1) port_id	- receiver port of a regular app
			 * 2) port_id 	- looper port for this BApplication
			 * 3) team_id 	- this app's team id
			 * 4) int32		- handler token of the app
			 * 5) char*		- signature of the resular app
			 */
			PortMessage msg(AS_MAX_MSG_SIZE);
			msg.SetCode(AS_CREATE_APP);
			msg.Attach<port_id>(fServerTo);
			msg.Attach<port_id>(BLooper::fMsgPort);
			msg.Attach<team_id>(Team());
			msg.Attach<int32>(_get_object_token_(this));
			msg.AttachString(signature);
			msg.WriteToPort(fServerFrom);
			
			/* Reply to AS_CREATE_APP:
			 * AS_SET_SERVER_PORT
			 * 1) port_id	- server-side app port
			 */
			msg.ReadFromPort(fServerTo);
			msg.Read<port_id>(&fServerFrom);
		}
		else
			fInitError = (status_t)fServerTo;
	}

	// init be_app and be_app_messenger
	if (fInitError == B_OK) {
		be_app = this;
		be_app_messenger = BMessenger(NULL, this);
	}

	/* TODO: uncomment this when entry_ref & BPath are implemented */
	/*// set the BHandler's name
	if (fInitError == B_OK)
		SetName(ref.name);

	// create meta MIME
	if (fInitError == B_OK) {
		BPath path;
		if (path.SetTo(&ref) == B_OK)
			create_app_meta_mime(path.Path(), false, true, false);
	}*/

	// Return the error or exit, if there was an error and no error variable
	// has been supplied.
	if(error) {
		*error = fInitError;
	}
	else if(fInitError != B_OK) {
		exit(0);
	}
}
//------------------------------------------------------------------------------
/*void BApplication::BeginRectTracking(BRect r, bool trackWhole)
{
	BPrivate::BAppServerLink link;
	link.Attach<int32>(AS_BEGIN_RECT_TRACKING);
	link.Attach<BRect>(r);
	link.Attach<int32>(trackWhole);
	link.Flush();
}
//------------------------------------------------------------------------------
void BApplication::EndRectTracking()
{
	int32 foo=AS_END_RECT_TRACKING;
	write_port(fServerTo,AS_END_RECT_TRACKING,&foo,sizeof(int32));
}
//------------------------------------------------------------------------------
void BApplication::get_scs()
{
}
//------------------------------------------------------------------------------
void BApplication::setup_server_heaps()
{
}
//------------------------------------------------------------------------------
void* BApplication::rw_offs_to_ptr(uint32 offset)
{
	return NULL;	// not implemented
}
//------------------------------------------------------------------------------
void* BApplication::ro_offs_to_ptr(uint32 offset)
{
	return NULL;	// not implemented
}
//------------------------------------------------------------------------------
void* BApplication::global_ro_offs_to_ptr(uint32 offset)
{
	return NULL;	// not implemented
}
//------------------------------------------------------------------------------
void BApplication::connect_to_app_server()
{
}
//------------------------------------------------------------------------------
void BApplication::send_drag(BMessage* msg, int32 vs_token, BPoint offset, BRect drag_rect, BHandler* reply_to)
{
}
//------------------------------------------------------------------------------
void BApplication::send_drag(BMessage* msg, int32 vs_token, BPoint offset, int32 bitmap_token, drawing_mode dragMode, BHandler* reply_to)
{
}
//------------------------------------------------------------------------------
void BApplication::write_drag(_BSession_* session, BMessage* a_message)
{
}
//------------------------------------------------------------------------------
bool BApplication::quit_all_windows(bool force)
{
	return false;	// not implemented
}
//------------------------------------------------------------------------------
bool BApplication::window_quit_loop(bool, bool)
{
	return false;	// not implemented
}
//------------------------------------------------------------------------------
void BApplication::do_argv(BMessage* message)
{
	if (message) {
		int32 argc = __libc_argc;
		const char * const *argv = __libc_argv;
		// add argc
		message->AddInt32("argc", argc);
		// add argv
		for (int32 i = 0; i < argc; i++)
			message->AddString("argv", argv[i]);
		// add current working directory
		char cwd[B_PATH_NAME_LENGTH + 1];
		if (getcwd(cwd, B_PATH_NAME_LENGTH + 1))
			message->AddString("cwd", cwd);
	}
}
//------------------------------------------------------------------------------
uint32 BApplication::InitialWorkspace()
{
	return 0;	// not implemented
}
//------------------------------------------------------------------------------
int32 BApplication::count_windows(bool incl_menus) const
{
	using namespace BPrivate;

	// Windows are BLoopers, so we can just check each BLooper to see if it's
	// a BWindow (or BMenuWindow)
	int32 count = 0;
	BObjectLocker<BLooperList> ListLock(gLooperList);
	if (ListLock.IsLocked())
	{
		BLooper* Looper = NULL;
		for (int32 i = 0; i < gLooperList.CountLoopers(); ++i)
		{
			Looper = gLooperList.LooperAt(i);
			if (dynamic_cast<BWindow*>(Looper))
			{
				if (incl_menus || dynamic_cast<BMenuWindow*>(Looper) == NULL)
				{
					++count;
				}
			}
		}
	}

	return count;
}
//------------------------------------------------------------------------------
BWindow* BApplication::window_at(uint32 index, bool incl_menus) const
{
	using namespace BPrivate;

	// Windows are BLoopers, so we can just check each BLooper to see if it's
	// a BWindow (or BMenuWindow)
	uint32 count = 0;
	BWindow* Window = NULL;
	BObjectLocker<BLooperList> ListLock(gLooperList);
	if (ListLock.IsLocked())
	{
		BLooper* Looper = NULL;
		for (int32 i = 0; i < gLooperList.CountLoopers() && !Window; ++i)
		{
			Looper = gLooperList.LooperAt(i);
			if (dynamic_cast<BWindow*>(Looper))
			{
				if (incl_menus || dynamic_cast<BMenuWindow*>(Looper) == NULL)
				{
					if (count == index)
					{
						Window = dynamic_cast<BWindow*>(Looper);
					}
					else
					{
						++count;
					}
				}
			}
		}
	}

	return Window;
}
//------------------------------------------------------------------------------
status_t BApplication::get_window_list(BList* list, bool incl_menus) const
{
	return NOT_IMPLEMENTED;
}
//------------------------------------------------------------------------------
int32 BApplication::async_quit_entry(void* data)
{
	return 0;	// not implemented
}*/
//------------------------------------------------------------------------------

// check_app_signature
/*!	\brief Checks whether the supplied string is a valid application signature.

	An error message is printed, if the string is no valid app signature.

	\param signature The string to be checked.
	\return
	- \c B_OK: \a signature is a valid app signature.
	- \c B_BAD_VALUE: \a signature is \c NULL or no valid app signature.
*/
static
status_t
check_app_signature(const char *signature)
{
	/* TODO: uncomment after implementing BMimeType */
	/*bool isValid = false;
	BMimeType type(signature);
	if (type.IsValid() && !type.IsSupertypeOnly()
		&& BMimeType("application").Contains(&type)) {
		isValid = true;
	}
	if (!isValid) {
		printf("bad signature (%s), must begin with \"application/\" and "
			   "can't conflict with existing registered mime types inside "
			   "the \"application\" media type.\n", signature);
	}
	return (isValid ? B_OK : B_BAD_VALUE);*/
	return B_OK;
}
//------------------------------------------------------------------------------
// looper_name_for
/*!	\brief Returns the looper name for a given signature.

	Normally this is "AppLooperPort", but in case of the registrar a
	special name.

	\return The looper name.
*/
static
const char*
looper_name_for(const char *signature)
{
	/* TODO: uncomment after implementing BRoster */
	/*if (signature && !strcmp(signature, kRegistrarSignature))
		return kRosterPortName;*/
	return "AppLooperPort";
}
//------------------------------------------------------------------------------

/*
 * $Log $
 *
 * $Id  $
 *
 */
