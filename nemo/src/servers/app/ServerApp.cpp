/*#include <AppDefs.h>
#include <List.h>
#include <String.h>
#include <PortLink.h>
#include <PortMessage.h>
#include <PortQueue.h>
#include <SysCursor.h>

#include <Session.h>
*/
#include <ServerApp.h>

//#include <stdio.h>
//#include <linux/sched.h>	// for find_task_by_pid()

/*#include <string.h>
#include <ScrollBar.h>
#include <ServerProtocol.h>

#include "BitmapManager.h"
#include "CursorManager.h"
#include "Desktop.h"
#include "DisplayDriver.h"
#include "FontServer.h"
#include "ServerApp.h"
#include "ServerWindow.h"
#include "ServerCursor.h"
#include "ServerBitmap.h"
#include "ServerPicture.h"
#include "ServerConfig.h"
#include "WinBorder.h"
#include "LayerData.h"
#include "Utils.h"
*/
#if DEBUG
	#include "nemo_debug.h"
#endif

//=============================================================================

ServerApp::ServerApp(	port_id app_port,
						port_id server_app_port,
						port_id app_server_port,
						pid_t process_id,
						int32 handler_id,
						const char *app_sig) {

	sprintf(mSignature, "%s", app_sig? app_sig:"application/x-vnd.unregistered-application");

	mAppPort = app_port;
	mServerAppPort = server_app_port;
	mAppServerPort = app_server_port;
	
	mTargetID = process_id;
	
	// token ID of the BApplication's BHandler object. Used for BMessage target specification
//	handlerToken = handler_id;

//	winList = new BList(0);
//	bmpList = new BList(0);
//	picList = new BList(0);
	mIsActive = false;
	mIsQuitting = false;

//	ServerCursor *defaultc=cursormanager->GetCursor(B_CURSOR_DEFAULT);
	
//	_appcursor = (defaultc)?new ServerCursor(defaultc):NULL;
//	lock = create_sem(1,"ServerApp sem");

//	driver = GetGfxDriver(ActiveScreen());
//	cursorHidden = false;
}

//=============================================================================
//
ServerApp::~ServerApp()
{
/*	STRACE(("ServerApp %s:~ServerApp()\n",_signature.String()));
	int32 i;
	
	ServerWindow *tempwin;
	for(i=0;i<_winlist->CountItems();i++)
	{
		tempwin=(ServerWindow*)_winlist->ItemAt(i);
		if(tempwin)
			delete tempwin;
	}
	_winlist->MakeEmpty();
	delete _winlist;

	ServerBitmap *tempbmp;
	for(i=0;i<_bmplist->CountItems();i++)
	{
		tempbmp=(ServerBitmap*)_bmplist->ItemAt(i);
		if(tempbmp)
			delete tempbmp;
	}
	_bmplist->MakeEmpty();
	delete _bmplist;

	ServerPicture *temppic;
	for(i=0;i<_piclist->CountItems();i++)
	{
		temppic=(ServerPicture*)_piclist->ItemAt(i);
		if(temppic)
			delete temppic;
	}
	_piclist->MakeEmpty();
	delete _piclist;

	delete _applink;
	_applink=NULL;
	if(_appcursor)
		delete _appcursor;
*/
	// Kill the monitor thread if it exists
	mIsQuitting = true;
	int32 retval;
	// TODO: fix this code
	if(wait_for_thread(mMonitorThread, &retval) != B_OK) {
		#ifndef DEBUG_SERVERAPP
			fprintf(SERVERAPP_LOG, "error terminating the monitor thread of \"%s\"\n", mSignature);
			// TODO: find a way to kill the monitor thread
			// TODO: what if pthread_join() blocked?
		#endif
	}

/*	cursormanager->RemoveAppCursors(_signature.String());
	delete_sem(_lock);*/
	
	Terminate();
}

//=============================================================================

void ServerApp::Terminate() {
	
	// close the server app's message port
	if(delete_port(mServerAppPort) != B_OK) {
		#if DEBUG_SERVERAPP
			fprintf(SERVERAPP_LOG, "error closing server app's message queue (%s)\n", errno2string(errno));
		#endif
	}
}

//=============================================================================
bool ServerApp::Run()
{
	// Unlike a BApplication, a ServerApp is *supposed* to return immediately
	// when its Run() function is called.
	if((mMonitorThread = spawn_thread(MonitorApp, "Monitor Thread", B_NORMAL_PRIORITY, this)) <= 0) {
		#if DEBUG_SERVERAPP
			fprintf( SERVERAPP_LOG, "server_app: error spawning the app monitor thread\n" );
		#endif
		return false;		
	}

	return true;
}

//=============================================================================

bool ServerApp::PingTarget() {
	
	// TODO: use find_task_by_pid()
	if(/*find_task_by_pid(mTargetID) == NULL*/false) {

		// app died, do all necessary cleanup
		app_server_msg msg;
		msg.what = AS_DELETE_APP;
		msg.delete_app.process_id = mTargetID;
		if(write_port(mAppServerPort, msg.what, &msg, sizeof(delete_app_msg) - 4) != B_OK) {
			#if DEBUG_SERVERAPP
				fprintf(SERVERAPP_LOG, "error sending message to app_server (%s)\n", errno2string(errno));
			#endif
			return false;
		}
		
		/*_applink->SetPort(serverport);
		_applink->SetOpCode(AS_DELETE_APP);
		_applink->Attach(&_monitor_thread,sizeof(thread_id));
		_applink->Flush();*/
		return false;
	}
	return true;
}

//=============================================================================

void ServerApp::PostMessage(app_server_msg *msg) {
	
	// send message to self
	if(write_port(mServerAppPort, msg->what, msg, AS_MAX_MSG_SIZE) != B_OK) {
		#if DEBUG_SERVERAPP
			fprintf(SERVERAPP_LOG, "error sending message to self (%s)\n", errno2string(errno));
		#endif
	}
}

//=============================================================================

void ServerApp::PostMessage(int32 what) {
	
	// send message to self
	if(write_port(mServerAppPort, what, NULL, 0) != B_OK) {
		#if DEBUG_SERVERAPP
			fprintf(SERVERAPP_LOG, "error sending message to self (%s)\n", errno2string(errno));
		#endif
	}
}

//=============================================================================

void ServerApp::ForwardMessage(app_server_msg *msg) {
	
	if(write_port(mAppPort, msg->what, msg, AS_MAX_MSG_SIZE) != B_OK) {
		#if DEBUG_SERVERAPP
			fprintf(SERVERAPP_LOG, "error sending message to self (%s)\n", errno2string(errno));
		#endif
	}
}

//=============================================================================

bool ServerApp::IsActive() const {
	
	return mIsActive;
}

//=============================================================================

void ServerApp::Activate(bool active) {
	
	mIsActive = active;
//	SetAppCursor();
}

//=============================================================================
 
/*void ServerApp::SetAppCursor() {
	
	if(_appcursor)
		cursormanager->SetCursor(_appcursor->ID());
	else
		cursormanager->SetCursor(B_CURSOR_DEFAULT);
}*/

//=============================================================================

status_t ServerApp::MonitorApp(void *data) {
	
	// Message-dispatching loop for the ServerApp

	ServerApp *serverApp = (ServerApp*) data;
//	PortQueue msgqueue(app->_receiver);
//	PortMessage *msg;
	port_id server_app_port = serverApp->mServerAppPort;
	app_server_msg msg;
	
	#if DEBUG_SERVERAPP
		fprintf(SERVERAPP_LOG, "server_app: monitoring thread of app \"%s\" started...\n", serverApp->mSignature);
	#endif
	
	bool quit = false;
	while(true) {
	
		if(read_port(server_app_port, &(msg.what), &msg, AS_MAX_MSG_SIZE) < B_OK) {
			#if DEBUG_SERVERAPP
				fprintf(SERVERAPP_LOG, "server_app: error receiving message from app \"%s\"(%s)\n", serverApp->mSignature, errno2string(errno));
			#endif
			return B_ERROR;
		}
		
		printf("got message %d\n", msg.what);
		switch(msg.what) {
			
			case AS_QUIT_APP: {
				#if DEBUG_SERVERAPP
					fprintf(SERVERAPP_LOG, "server_app: received request to stop monitoring app \"%s\"\n", serverApp->mSignature);
				#endif
				
				// TODO: figure out a way to do this nicely
				// kill user-side app
				//kill(appServer->targetID, 9);
				//serverApp->PostMessage(B_QUIT_REQUESTED);
				
				// TODO: just for testing...
				quit = true;
				//serverApp->Terminate();
				//delete serverApp;
			} break;
			
			case B_QUIT_REQUESTED: {
				// Our BApplication sent us this message when it quit.
				// We need to ask the app_server to delete our monitor
				#if DEBUG_SERVERAPP
					fprintf(SERVERAPP_LOG, "server_app: app \"%s\" received notification to quit\n", serverApp->mSignature);
				#endif
				
				// permit app to quit
				msg.what = AS_QUIT_APP;
				if(write_port(serverApp->mAppPort, msg.what, &msg, 0) != B_OK) {
					#if DEBUG_SERVERAPP
						fprintf(SERVERAPP_LOG, "server_app: error sending message to app (%s)\n", errno2string(errno));
					#endif
				}
				
				// ask app_server to delete this server app
				msg.what = AS_DELETE_APP;
				msg.delete_app.process_id = serverApp->mTargetID;
				if(write_port(serverApp->mAppServerPort, msg.what, &msg, sizeof(delete_app_msg) - 4) != B_OK) {
					#if DEBUG_SERVER_APP
						fprintf(SERVERAPP_LOG, "server_app: error sending message to app_server (%s)\n", errno2string(errno));
					#endif
					return B_ERROR;
				}
				quit = true;
			} break;
			
			default: {
				serverApp->DispatchMessage(&msg);
			};
		}
		
		// time to stop?
		if(quit)
			return B_OK;
		
		if(serverApp->mIsQuitting)
			return B_OK;
			
	} // while true

//	exit_thread(0);
	return B_OK;
}

//=============================================================================

void ServerApp::DispatchMessage(app_server_msg *msg)
{
	
	// TODO: handle message from the app_server here
	switch(msg->what) {
		default: {
			#if DEBUG_SERVERAPP
				fprintf(SERVERAPP_LOG, "server_app: \"%s\" received a message to dispatch\n", mSignature);
			#endif
		}
	}
}

//=============================================================================

/*
ServerBitmap *ServerApp::FindBitmap(int32 token)
{
	ServerBitmap *temp;
	for(int32 i=0; i<_bmplist->CountItems();i++)
	{
		temp=(ServerBitmap*)_bmplist->ItemAt(i);
		if(temp && temp->Token()==token)
			return temp;
	}
	return NULL;
}*/

//=============================================================================

