//------------------------------------------------------------------------------
// AppServer.h
//
// DESCRIPTION:	The main app_server class
//
// AUTHOR: Mahmoud Al Gammal
// DATE: 14/2/2004
//------------------------------------------------------------------------------

#ifndef _APPSERVER_H
#define _APPSERVER_H

#define BUFFSIZE       	8192
#define R	0xff
#define G	0x0
#define B	0xff

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <OS.h>

#include <directfb.h>

// Private Includes ------------------------------------------------------------

// Macro Defs ------------------------------------------------------------------

// Type Defs -------------------------------------------------------------------
class BList;
class PortMessage;
class ServerApp;

typedef status_t(*msg_handler)(PortMessage*);

//------------------------------------------------------------------------------
class AppServer {

friend	class		ServerApp;

public:
					AppServer();
					~AppServer();
		
		thread_id	Run();
static	void		test_exit(int x);
static IDirectFB *dfb;
static IDirectFBSurface *primary;
static IDirectFBDisplayLayer *layer;

private:
		status_t	Init();
		status_t	InstallHandlers();		
		void		Terminate();
		void		MainLoop();
		void		DispatchMessage(PortMessage *msg);
		void		Broadcast(int32 what);
		void		HandleKeyMessage(int32 code, int8 *buffer);
static	int32		PollerTask(void *data);
static	int32		PicassoTask(void *data);

		msg_handler	*fMsgHandlerList;
		thread_id	fPollerThreadID, fPicassoThreadID;
		port_id		fMessagePortID, fInputPortID;
		BList		*fAppList;
		sem_id		fAppListSem;
		ServerApp	*fActiveApp;
		sem_id		fActiveAppSem;
		bool		fIsQuitting;
		//DirectFB Work				
		char 		Buff[BUFFSIZE];
		status_t	ViewGo_InitDirectFB();
		status_t	startParsing(const char* fileName);
// Message handlers
static	status_t	CREATE_APP_handler(PortMessage *msg);
static	status_t	DELETE_APP_handler(PortMessage *msg);
static  status_t	ViewGo_CREATE_WINDOW_handler(PortMessage *msg);
static  status_t	ViewGo_SHOW_WINDOW_handler(PortMessage *msg);
static	status_t	QUIT_APP_handler(PortMessage *msg);
};

//------------------------------------------------------------------------------
extern AppServer *app_server;

//------------------------------------------------------------------------------
#endif // _APPSERVER_H
