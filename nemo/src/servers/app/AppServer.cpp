//------------------------------------------------------------------------------
// AppServer.cpp
//
// DESCRIPTION: The implementation of the app_server's main class + app_server's
//				main() function.
//
// AUTHOR: Mahmoud Al Gammal
// DATE: 14/2/2004
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

// System Includes -------------------------------------------------------------
#include <stdlib.h>
#include <signal.h>

#include <AppDefs.h>
#include <OS.h>
#include <List.h>

#include <directfb.h>
#include <expat.h>

// Private Includes ------------------------------------------------------------
#include "AppServer.h"
#include "PortLink.h"
#include "PortMessage.h"
#include "ServerProtocol.h"
#include "ServerApp.h"
#include "ServerWindow.h"


// Debugging -------------------------------------------------------------------
#include "nemo_debug.h"
#if DEBUG_APPSERVER
	#define OUT(x...)	fprintf(APPSERVER_LOG, "app_server: "x);
	#define DBG(x)		x;
#else
	#define DBG(x)		;
#endif

// Error checker for DirectFB functions
#define DFBCHECK(x...)                                         \
  {                                                            \
    DFBResult err = x;                                         \
                                                               \
    if (err != DFB_OK)                                         \
      {                                                        \
        return B_ERROR;                                        \
        DirectFBErrorFatal( #x, err );                         \
      }                                                        \
  }

// Globals ---------------------------------------------------------------------
AppServer *app_server = NULL;
// Static Variables Initializations
IDirectFB *AppServer::dfb=NULL;
IDirectFBSurface *AppServer::primary=NULL;
IDirectFBDisplayLayer *AppServer::layer=NULL;

//------------------------------------------------------------------------------
AppServer::AppServer() {
		
	if(Init() != B_OK) {
		DBG(OUT("Error during initialization\n"));
		Terminate();
		exit(-1);
	}
	else {
		DBG(OUT("Initialized successfully\n"));
	}
	
	//system("./wnd_test");
}
//------------------------------------------------------------------------------
AppServer::~AppServer() {
	
	Terminate();
}
//------------------------------------------------------------------------------
thread_id AppServer::Run() {
	
	MainLoop();
	return find_thread(NULL);
}
//------------------------------------------------------------------------------
status_t AppServer::Init() {

	app_server = this;
	
	// safe initialization of members comes first...
	fMsgHandlerList = NULL;
	fPollerThreadID = fPicassoThreadID = -1;
   	fMessagePortID = fInputPortID = -1;
   	fAppList = NULL;
   	fAppListSem = -1;
   	fActiveApp = NULL;
   	fActiveAppSem = -1;
   	fIsQuitting = false;

	// install message handlers
	if(InstallHandlers() != B_OK) {
		DBG(OUT("Error installing message handlers\n"));
		return B_ERROR;
	}
		
	// create the app_server's & input server's message queues
	if((fMessagePortID = create_port(100, AS_PORT_NAME)) < B_OK) {
		DBG(OUT("Error creating main message port (%s)\n", errno2string(errno)));
		return B_ERROR;
	}
	
	DBG(OUT("Listening to apps at port %d\n", fMessagePortID));
	
	
	if((fInputPortID = create_port(100, AS_INPUT_PORT_NAME)) < B_OK) {
		DBG(OUT("Error creating the input server's message port (%s)\n", errno2string(errno)));
		return B_ERROR;
	}
	
	DBG(OUT("Listening to input_server at port %d\n", fInputPortID));
	
	
	// TODO: initialize the desktop & input_server
	if (ViewGo_InitDirectFB() != B_OK){
		#if DEBUG_APPSERVER
			fprintf(APPSERVER_LOG, "Can't initialize the DirectFB, (%s)\n", errno2string(errno));
		#endif	
		return B_ERROR;
	}
	
	// Parse the XML file
	/*
	if (startParsing("./Window.xml") != B_OK){
		#if DEBUG_APPSERVER
			fprintf(APPSERVER_LOG, "Can't parse the XML file, (%s)\n", errno2string(errno));
		#endif	
		return B_ERROR;	
	}
	*/	
	
	

	// create the required semaphores
	fAppListSem = create_sem(1, "app list sem");
	fActiveAppSem = create_sem(1, "active app sem");

	// initialize the application list
	fAppList = new BList();
		
	// spawn the message & input threads
	// TODO: use customized priorities
	fPollerThreadID = spawn_thread(PollerTask, "poller", B_NORMAL_PRIORITY, NULL);
	if(fPollerThreadID <= 0 ) {		
		DBG(OUT("Couldn't spawn the input poller thread (%s)\n", errno2string(errno)));
		return B_ERROR;
	}
	resume_thread(fPollerThreadID);

	// TODO: use customized priorities
	fPicassoThreadID = spawn_thread(PicassoTask, "picasso", B_NORMAL_PRIORITY, NULL);
	if(fPicassoThreadID <= 0) {	
		DBG(OUT("Couldn't spawn the Picasso thread (%s)\n", errno2string(errno)));
		return B_ERROR;
	}
	resume_thread(fPicassoThreadID);

	return B_OK;
}
//------------------------------------------------------------------------------
status_t AppServer::InstallHandlers(void) {

	int length = AS_LAST_MSG_INDEX - AS_FIRST_MSG_INDEX + 1;
	fMsgHandlerList = new msg_handler[length];

	if(!fMsgHandlerList)
		return B_NO_MEMORY;

	// nullify
	memset(fMsgHandlerList, 0, sizeof(msg_handler) * length);

	// Application messages
	fMsgHandlerList[AS_CREATE_APP	- AS_FIRST_MSG_INDEX] = CREATE_APP_handler;
	fMsgHandlerList[AS_DELETE_APP	- AS_FIRST_MSG_INDEX] = DELETE_APP_handler;
	fMsgHandlerList[AS_QUIT_APP		- AS_FIRST_MSG_INDEX] = QUIT_APP_handler;

	return B_OK;
}
//------------------------------------------------------------------------------
void AppServer::Terminate() {
	
	// TODO: stop all apps
	// TODO: delete app list
	
	// stop children threads
	fIsQuitting = true;
	status_t error;
	
	if(fPollerThreadID != -1)
		wait_for_thread(fPollerThreadID, &error);
		
	if(fPicassoThreadID != -1)
		wait_for_thread(fPicassoThreadID, &error);

	// delete the message ports
   	if(fMessagePortID != -1)
    	delete_port(fMessagePortID);
	
   	if(fInputPortID != -1)
    	delete_port(fInputPortID);

    // delete the semaphores
    if(fAppListSem != -1)
    	delete_sem(fAppListSem);

	if(fActiveAppSem != -1)
		delete_sem(fActiveAppSem);

	// free message handler list
	delete [] fMsgHandlerList;
}
//------------------------------------------------------------------------------
void AppServer::MainLoop() {
	
	DBG(OUT("Main loop started\n"));

	PortMessage msg(AS_MAX_MSG_SIZE);
	
	// read incoming messages
	while(!fIsQuitting) {
		
		// read next message
		if(msg.ReadFromPort(fMessagePortID) < B_OK) {		
			DBG(OUT("Error while reading from main message port (%s)\n", errno2string(errno)));
			continue;
		}
		
		// decode & process message
		DispatchMessage(&msg);
	}

	DBG(OUT("Main loop terminated\n"));
}
//------------------------------------------------------------------------------
void AppServer::DispatchMessage(PortMessage *msg) {

	int32 code = msg->Code();

	// is this an app_server message?
	if(code >= AS_FIRST_MSG_INDEX && code <= AS_LAST_MSG_INDEX) {
		status_t error = fMsgHandlerList[code - AS_FIRST_MSG_INDEX](msg);
		if(error < B_OK) {
			DBG(OUT("Error handling message\n"));
		}
	}

	// handle system messages
	switch(code) {
		case B_QUIT_REQUESTED: {
   			DBG(OUT("Recieved request to quit\n"));
   			
			// announce to the world that the app_server is quitting
			fIsQuitting = true;
			
      		// stop poller
   			if(msg->WriteToPort(fInputPortID) != B_OK) {
   				DBG(OUT("Error while asking Poller to stop (%d)\n", errno2string(errno)));
   			}
      
			// TODO: stop picasso
			
			// ask all apps to quit
			DBG(OUT("Asking all apps to quit\n"));
			Broadcast(AS_QUIT_APP);
		} break;

		default: {
			DBG(if(!(code >= AS_FIRST_MSG_INDEX && code <= AS_LAST_MSG_INDEX))
				OUT("Received an unrecognized message code (%d)\n", code));
		} break;
	}
}
//------------------------------------------------------------------------------
void AppServer::Broadcast(int32 what) {

	if(acquire_sem(fAppListSem) == B_OK) {
		for(int i = 0; i < fAppList->CountItems(); i++ ) {
			ServerApp *app = (ServerApp*) fAppList->ItemAt(i);
			app->PostMessage(what);
		}
		release_sem(fAppListSem);
	}
}
//------------------------------------------------------------------------------
int32 AppServer::PollerTask(void *data) {

	DBG(OUT("Input poller thread started\n"));
	
	PortMessage msg(AS_MAX_MSG_SIZE);
	bool quit = false;
	
	while(!quit) {
		if(msg.ReadFromPort(app_server->fInputPortID) < B_OK) {	
			DBG(OUT("poller: Error receiving input message (%s)\n", errno2string(errno)));
			continue;
		}
		
		switch(msg.Code()) {
			case B_QUIT_REQUESTED: {
				DBG(OUT("poller: received request to quit\n"));
				
				// if the quit request was initiated from the input_server's
				// side, then we have to deliver the request to the app_server
				if(msg.WriteToPort(app_server->fMessagePortID) < B_OK) {
					DBG(OUT("poller: Error asking main loop to stop (%d)\n", errno2string(errno)));
				}
				quit = true;
			} break;

			case AS_INPUT_TEST_MSG: {
            	char *text = NULL;
            	msg.ReadString(&text);

            	if(!strcmp(text, "quit")) {
					PortMessage m;
					m.SetCode(B_QUIT_REQUESTED);
					m.WriteToPort(app_server->fMessagePortID);
            	}
            	else {
            		printf("got \"%s\"\n", text);
            	}
			} break;
		}
	}
	
	DBG(OUT("Input poller thread terminated\n"));

	app_server->fPollerThreadID = -1;
	exit_thread(B_OK);
}
//------------------------------------------------------------------------------
int32 AppServer::PicassoTask(void *data) {
	
	DBG(OUT("Picasso thread started\n"));

	DBG(OUT("Picasso thread terminated\n"));
	
	app_server->fPicassoThreadID = -1;
	exit_thread(B_OK);
}
// Application Message Handlers ------------------------------------------------
status_t AppServer::CREATE_APP_handler(PortMessage *msg) {

   	DBG(OUT("Received AS_CREATE_APP\n"));

    // read message data
    port_id	app_port;
    port_id	looper_port;
    team_id app_team;
    int32	app_token;
    char	*app_sig;

    msg->Read(&app_port);
    msg->Read(&looper_port);
    msg->Read(&app_team);
    msg->Read(&app_token);
    msg->ReadString(&app_sig);

    // Create the ServerApp to monitor the new BApplication
   	ServerApp *serverApp =
   		new ServerApp(app_port, looper_port, app_team, app_token, app_sig);
   	
    if(serverApp->InitCheck() != B_OK) {
		DBG(OUT("Error creating server-side app for %s\n", app_sig));
		delete app_sig;
		return B_ERROR;
    }

	// add the new server app to the list
   	acquire_sem(app_server->fAppListSem);
	app_server->fAppList->AddItem(serverApp);
   	release_sem(app_server->fAppListSem);

    // set the the new server app as the active one
	acquire_sem(app_server->fActiveAppSem);
   	app_server->fActiveApp = serverApp;
    release_sem(app_server->fActiveAppSem);
    
	// the regular app is waiting for a reply
	PortMessage reply(AS_MAX_MSG_SIZE);
	reply.SetCode(AS_SET_SERVER_PORT);
	reply.Attach<port_id>(serverApp->Port());
   	if(reply.WriteToPort(app_port) < B_OK) {
   		DBG(OUT("Error sending reply message to app \"%s\"\n", app_sig));
   		delete app_sig;
		return B_ERROR;
   	}
   	serverApp->Run();

	delete app_sig;
	return B_OK;
}
//------------------------------------------------------------------------------
status_t AppServer::DELETE_APP_handler(PortMessage *msg) {

   	/* Delete a ServerApp
   	 * Received only from the respective ServerApp when the
   	 * BApplication asks it to quit.
   	 */

    /* NOTE: i changed the contents of the AS_DELETE_APP message
     * so that it contains a pointer to the server app instead
     * of its target team id
     */
	ServerApp *serverApp = NULL;
	msg->Read<ServerApp*>(&serverApp);

	// find the ServerApp and delete it
	if(acquire_sem(app_server->fAppListSem) == B_OK) {
		int index = app_server->fAppList->IndexOf(serverApp);
		if(index < 0) {
			DBG(OUT("Error deleting ServerApp object (invalid pointer given)\n"));
			release_sem(app_server->fAppListSem);
			return B_BAD_VALUE;
		}
		serverApp = (ServerApp*) (app_server->fAppList->RemoveItem(index));
		DBG(OUT("Deleting ServerApp \"%s\"\n", serverApp->fSignature));
		delete serverApp;

		// choose another app to activate
		if(acquire_sem(app_server->fActiveAppSem) == B_OK) {
			if(app_server->fAppList->CountItems() == 0) {
				app_server->fActiveApp = NULL;
			}
			else {
				app_server->fActiveApp = (ServerApp*) (app_server->fAppList->ItemAt(index));
			}
			release_sem(app_server->fActiveAppSem);
		}
		release_sem(app_server->fAppListSem);
	}

	return B_OK;
}
//------------------------------------------------------------------------------
status_t AppServer::QUIT_APP_handler(PortMessage *msg) {
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//--------------Wael work--------------------------
//------------------------------------------------------------------------------
status_t AppServer::ViewGo_InitDirectFB() {

	DFBSurfaceDescription dsc;
	//Initialize the DirectFB  
	DFBCHECK (DirectFBInit (NULL, NULL));
  	DFBCHECK (DirectFBCreate (&dfb));
	DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_NORMAL));
	dsc.flags = DSDESC_CAPS;
	dsc.caps  = DFBSurfaceCapabilities(DSCAPS_PRIMARY | DSCAPS_FLIPPING);
	DFBCHECK (dfb->CreateSurface( dfb, &dsc, &primary ));
	DFBCHECK (primary->Clear(primary, 0x0, 0x0, 0x0, 0x0));
	 
	//Get the primary layer---------------------------------
	DFBCHECK (dfb->GetDisplayLayer(dfb, 0, &layer));
	DFBCHECK (layer->SetCooperativeLevel(layer, DLSCL_ADMINISTRATIVE));

}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void start(void *data, const char *el, const char **attr)
{
  IDirectFBImageProvider *provider=NULL;
  DFBSurfaceDescription dsc;
  //BString test=BString(el);
  
  if(bcmp(el, "Body", 4)==0){
	AppServer::dfb->CreateImageProvider (AppServer::dfb, attr[1], &provider);
  	provider->GetSurfaceDescription (provider, &dsc);
  	AppServer::dfb->CreateSurface( AppServer::dfb, &dsc, &(ServerWindow::body) );  
	ServerWindow::body->SetSrcColorKey(ServerWindow::body, R, G, B);
	ServerWindow::body->SetBlittingFlags(ServerWindow::body, DSBLIT_SRC_COLORKEY);
  	provider->RenderTo (provider, ServerWindow::body, NULL);
	provider->Release(provider);
	}
  else if(bcmp(el, "Head", 4)==0){
	AppServer::dfb->CreateImageProvider (AppServer::dfb, attr[1], &provider);
  	provider->GetSurfaceDescription (provider, &dsc);
  	AppServer::dfb->CreateSurface( AppServer::dfb, &dsc, &(ServerWindow::head) );  
	ServerWindow::head->SetSrcColorKey(ServerWindow::head, R, G, B);
	ServerWindow::head->SetBlittingFlags(ServerWindow::head, DSBLIT_SRC_COLORKEY);	
  	provider->RenderTo (provider, ServerWindow::head, NULL);
	provider->Release(provider);
	}
  else if(bcmp(el, "Shead", 5)==0){
	AppServer::dfb->CreateImageProvider (AppServer::dfb, attr[1], &provider);
  	provider->GetSurfaceDescription (provider, &dsc);
  	AppServer::dfb->CreateSurface( AppServer::dfb, &dsc, &(ServerWindow::shead) );  
	ServerWindow::shead->SetSrcColorKey(ServerWindow::shead, R, G, B);
	ServerWindow::shead->SetBlittingFlags(ServerWindow::shead, DSBLIT_SRC_COLORKEY);	
  	provider->RenderTo (provider, ServerWindow::shead, NULL);
	provider->Release(provider);
	}
	
  else if(bcmp(el, "Close", 5)==0){
	AppServer::dfb->CreateImageProvider (AppServer::dfb, attr[1], &provider);
  	provider->GetSurfaceDescription (provider, &dsc);
  	AppServer::dfb->CreateSurface( AppServer::dfb, &dsc, &(ServerWindow::close_button) );  
	ServerWindow::close_button->SetSrcColorKey(ServerWindow::close_button, R, G, B);
	ServerWindow::close_button->SetBlittingFlags(ServerWindow::close_button, DSBLIT_SRC_COLORKEY);	
  	provider->RenderTo (provider, ServerWindow::close_button, NULL);
	provider->Release(provider);
	}
  else if(bcmp(el, "Sclose", 6)==0){
	AppServer::dfb->CreateImageProvider (AppServer::dfb, attr[1], &provider);
  	provider->GetSurfaceDescription (provider, &dsc);
  	AppServer::dfb->CreateSurface( AppServer::dfb, &dsc, &(ServerWindow::sclose_button) );  
	ServerWindow::sclose_button->SetSrcColorKey(ServerWindow::sclose_button, R, G, B);
	ServerWindow::sclose_button->SetBlittingFlags(ServerWindow::sclose_button, DSBLIT_SRC_COLORKEY);	
  	provider->RenderTo (provider, ServerWindow::sclose_button, NULL);
	provider->Release(provider);
	}  
  else if(bcmp(el, "Ok", 2)==0){
	AppServer::dfb->CreateImageProvider (AppServer::dfb, attr[1], &provider);
  	provider->GetSurfaceDescription (provider, &dsc);
  	AppServer::dfb->CreateSurface( AppServer::dfb, &dsc, &(ServerWindow::ok_button) );  
	ServerWindow::ok_button->SetSrcColorKey(ServerWindow::ok_button, R, G, B);
	ServerWindow::ok_button->SetBlittingFlags(ServerWindow::ok_button, DSBLIT_SRC_COLORKEY);	  	
	provider->RenderTo (provider, ServerWindow::ok_button, NULL);
	provider->Release(provider);
	}  
  else if(bcmp(el, "Sok", 3)==0){
	AppServer::dfb->CreateImageProvider (AppServer::dfb, attr[1], &provider);
  	provider->GetSurfaceDescription (provider, &dsc);
  	AppServer::dfb->CreateSurface( AppServer::dfb, &dsc, &(ServerWindow::sok_button) );  
	ServerWindow::sok_button->SetSrcColorKey(ServerWindow::sok_button, R, G, B);
	ServerWindow::sok_button->SetBlittingFlags(ServerWindow::sok_button, DSBLIT_SRC_COLORKEY);	  	
	provider->RenderTo (provider, ServerWindow::sok_button, NULL);
	provider->Release(provider);
	}  	

}

//------------------------------------------------------------------------------

status_t AppServer::startParsing(const char* fileName)
{
  FILE *f=fopen(fileName,"r");
  XML_Parser p=XML_ParserCreate("ISO-8859-1");
  if (! p) {
    return B_ERROR;
  }
  XML_SetStartElementHandler(p, start);
  
  for (;;) {
    int done;
    int len;

    len = fread(Buff, 1, BUFFSIZE, f);
    if (ferror(f)) {
      return B_ERROR;
    }
    done = feof(f);

    if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
      return B_ERROR;
              XML_GetCurrentLineNumber(p),
              XML_ErrorString(XML_GetErrorCode(p));
    }

    if (done)
      break;
  }
  fclose(f);
}
//------------------------------------------------------------------------------
status_t AppServer::ViewGo_CREATE_WINDOW_handler(PortMessage *msg){

	BRect frame;
	char* title;
	int32 type;
	uint32 flags;
	port_id receive_port;
	port_id fMsgPort;
	
	//read message data
	msg->Read(&frame);
	msg->Read(&type);
	msg->Read(&flags);
	msg->ReadString(&title);
	msg->Read(&receive_port, sizeof(port_id));
	msg->Read(&fMsgPort, sizeof(port_id));
	ServerWindow *wnd = new ServerWindow(frame, title, type, flags, NULL, receive_port, (port_id)0, fMsgPort, /*(uint32)0,*/ (int32)0);
	//TODO:make instance from the ServerWindow class and send this parameters to it
	//TODO:call function "CreateWindow" from the ServerWindow instance	
	wnd->Show();
}

//------------------------------------------------------------------------------
status_t AppServer::ViewGo_SHOW_WINDOW_handler(PortMessage *msg){

}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/* this is just a temporary function for testing */
void AppServer::test_exit(int x) {

	// ask the app_server nicely to quit
	PortMessage msg;
	msg.SetCode(B_QUIT_REQUESTED);
	msg.WriteToPort(app_server->fInputPortID);
}

// Main Function ---------------------------------------------------------------
int main(int argc, char **argv) {

	// TODO: check if the app_server is already running
	
	/* this is just here so we can quit the app_server gracefully instead
	 * of killing it while testing
	 */
	signal(SIGINT, AppServer::test_exit);
	
	DBG(OUT("Initializing...\n"));
	
	AppServer *app_server = new AppServer();
	app_server->Run();
	delete app_server;

	DBG(OUT("Terminated successfully\n"));
		
	return 0;
}
//------------------------------------------------------------------------------
