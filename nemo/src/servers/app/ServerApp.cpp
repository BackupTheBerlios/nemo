//------------------------------------------------------------------------------
// ServerApp.cpp
//
// DESCRIPTION:	sever-side application class
//
// AUTHOR: Mahmoud Al Gammal
// DATE: 16/2/2004
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <string.h>

// System Includes -------------------------------------------------------------
#include <AppDefs.h>
#include <OS.h>

/*
#include <List.h>
#include <String.h>
#include <Session.h>
#include <ScrollBar.h>
*/
#include <Message.h>

// Private Includes ------------------------------------------------------------
#include "AppServer.h"
#include "PortLink.h"
#include "PortMessage.h"
#include "PortQueue.h"
#include "ServerApp.h"
#include "ServerProtocol.h"

/*#include "BitmapManager.h"
#include "CursorManager.h"
#include "Desktop.h"
#include "DisplayDriver.h"
#include "FontServer.h"
#include "ServerWindow.h"
#include "ServerBitmap.h"
#include "ServerPicture.h"
#include "ServerConfig.h"
#include "WinBorder.h"
#include "LayerData.h"
#include "Utils.h"
*/

// Debugging -------------------------------------------------------------------
#include "nemo_debug.h"
#if DEBUG_APPSERVER
	#define OUT(x...)	fprintf(APPSERVER_LOG, "server_app: "x);
	#define DBG(x)		x;
#else
	#define DBG(x)		;
#endif

//------------------------------------------------------------------------------
ServerApp::ServerApp(port_id app_port, port_id looper_port,
	team_id app_team, int32 app_token, char *app_sig) {

	InitData(app_port, looper_port, app_team, app_token, app_sig);
}
//------------------------------------------------------------------------------
ServerApp::~ServerApp()
{	
	Terminate();
}
//------------------------------------------------------------------------------
void ServerApp::InitData(port_id app_port, port_id looper_port,
	team_id app_team, int32 app_token, char *app_sig) {

	// create a dedicated port for the new app
	fLocalPort = create_port(100, "ServerApp port");
	if(fLocalPort <= B_OK) {
		fInitError = fLocalPort;
		return;
	}
	
	// set info about the new app
	fAppPort = app_port;
	fLooperPort = looper_port;
	fTeamID = app_team;
	fAppServerPort = app_server->fMessagePortID;
	fHandlerToken = app_token;
	fSignature = app_sig?
		strdup(app_sig) : strdup("application/x-vnd.unregistered-application");

	// initialize the rest of the members
	fMonitorThreadID = -1;
	fSemID = create_sem(1, "ServerApp sem");

	fIsActive = false;
	fIsQuitting = false;

/*	winList = new BList(0);
	bmpList = new BList(0);
	picList = new BList(0);
	driver = GetGfxDriver(ActiveScreen());
*/
	fInitError = B_OK;
}
//------------------------------------------------------------------------------
void ServerApp::Terminate() {

	// Kill the monitor thread if it exists
	fIsQuitting = true;
	if(fMonitorThreadID != -1) {
		int32 retval;
		wait_for_thread(fMonitorThreadID, &retval);
	}
	
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
*/
	delete_sem(fSemID);
	delete_port(fLocalPort);
	free(fSignature);
}
//------------------------------------------------------------------------------
status_t ServerApp::InitCheck() {

	return fInitError;
}
//------------------------------------------------------------------------------
port_id ServerApp::Port() {

	return fLocalPort;
}
//------------------------------------------------------------------------------
bool ServerApp::Run()
{
	/* Unlike a BApplication, a ServerApp is *supposed* to return immediately
	 * when its Run() function is called.
	 */
	if((fMonitorThreadID = spawn_thread(
			MonitorTask, "Monitor Thread", B_NORMAL_PRIORITY, this)) < B_OK) {
		DBG(OUT("Error spawning the app monitor thread\n"));
		return false;
	}
	resume_thread(fMonitorThreadID);

	return true;
}
//------------------------------------------------------------------------------
bool ServerApp::PingTarget() {

	team_info info;
	if(get_team_info(fTeamID, &info) < B_OK) {
		// app died, do all necessary cleanup
		PortMessage msg(AS_MAX_MSG_SIZE);
		msg.SetCode(AS_DELETE_APP);
		msg.Attach<thread_id>(fMonitorThreadID);
		msg.WriteToPort(fAppServerPort);
		return false;
	}
	return true;
}
//------------------------------------------------------------------------------
void ServerApp::PostMessage(PortMessage *msg) {
	
	msg->WriteToPort(fLocalPort);
}
//------------------------------------------------------------------------------
void ServerApp::PostMessage(int32 what) {
	
	PortMessage msg(0);
	msg.SetCode(what);
	msg.WriteToPort(fLocalPort);
}
//------------------------------------------------------------------------------
void ServerApp::ForwardMessage(PortMessage *msg) {

	msg->WriteToPort(fLooperPort);	
}
//------------------------------------------------------------------------------
bool ServerApp::IsActive() const {
	
	return fIsActive;
}
//------------------------------------------------------------------------------
void ServerApp::Activate(bool active) {
	
	fIsActive = active;
}
//------------------------------------------------------------------------------
status_t ServerApp::MonitorTask(void *data) {
	
	// Message-dispatching loop for the ServerApp

	ServerApp *serverApp = (ServerApp*) data;
	PortMessage msg(AS_MAX_MSG_SIZE);
	
	DBG(OUT("Monitoring thread of app \"%s\" started...\n", serverApp->fSignature));
	
	// since we are not using a roster, hook functions will be triggered here
	/* TODO: only B_READY_TO_RUN is sent here, remember to send B_ARGV_RECEIVED
	 * and B_REFS_RECEIVED
	 */
	BMessage bmsg(B_READY_TO_RUN);
	ssize_t size = bmsg.FlattenedSize();
	char *buf = new char[size];
	bmsg.Flatten(buf, size);
	write_port(serverApp->fLooperPort, B_READY_TO_RUN, buf, size);

	while(!serverApp->fIsQuitting) {

		if(msg.ReadFromPort(serverApp->fLocalPort) < B_OK) {
			DBG(OUT("Error receiving message from app \"%s\"(%s)\n", serverApp->fSignature, errno2string(errno)));
			return B_ERROR;
		}

		serverApp->DispatchMessage(&msg);
	}

	exit_thread(B_OK);
}
//------------------------------------------------------------------------------
void ServerApp::DispatchMessage(PortMessage *msg)
{
	switch(msg->Code()) {
    	case AS_QUIT_APP:		QUIT_APP_handler(msg); break;
    	case B_QUIT_REQUESTED:	B_QUIT_REQUESTED_handler(msg); break;

    	default: {
    		DBG(OUT("\"%s\" received an unrecognized message code\n", fSignature));
    	};
	}
}
//------------------------------------------------------------------------------
/*ServerBitmap *ServerApp::FindBitmap(int32 token)
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
// Message Handlers ------------------------------------------------------------
status_t ServerApp::QUIT_APP_handler(PortMessage *msg) {

	DBG(OUT("app \"%s\" received AS_QUIT_APP\n", fSignature));

    PortMessage m(0);
    m.SetCode(B_QUIT_REQUESTED);
    m.WriteToPort(fLooperPort);
    
	return B_OK;
}
//------------------------------------------------------------------------------
status_t ServerApp::B_QUIT_REQUESTED_handler(PortMessage *msg) {

   	DBG(OUT("app \"%s\" received B_QUIT_REQUESTED\n", fSignature));
    
   	/* the BApplication sent this message when it quit.
   	 * We need to ask the app_server to delete the server-side app
   	 */
    PortMessage m(AS_MAX_MSG_SIZE);
    m.SetCode(AS_DELETE_APP);
    m.Attach<ServerApp*>(this);
    m.WriteToPort(fAppServerPort);

	// the end
	fIsQuitting = true;
	
	return B_OK;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
