//------------------------------------------------------------------------------
// ServerApp.h
//
// DESCRIPTION:	sever-side application class
//
// AUTHOR: Mahmoud Al Gammal
// DATE: 15/2/2004
//------------------------------------------------------------------------------

#ifndef _SERVERAPP_H
#define _SERVERAPP_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <OS.h>

// Private Includes ------------------------------------------------------------
#include "AppServer.h"

// Type Defs -------------------------------------------------------------------
class AppServer;
class BMessage;
class PortLink;
class PortMessage;
class BList;
//class DisplayDriver;
//class ServerCursor;

//------------------------------------------------------------------------------
class ServerApp {

	friend	class	AppServer;
	friend	class	ServerWindow;
	
public:
    					ServerApp(
    						port_id app_port,
    						port_id looper_port,
    						team_id app_team,
    						int32 app_token,
    						char *app_sig);
    						
    					~ServerApp();

        status_t		InitCheck();
        port_id			Port();
		bool			Run();
		void			Lock();
		void			Unlock();
		bool			IsLocked();
		bool			IsActive() const;
		void			Activate(bool active);
		bool			PingTarget();
		void			PostMessage(PortMessage *msg);
		void			PostMessage(int32 what);
		void			ForwardMessage(PortMessage *msg);

private:
		void			InitData(
    						port_id app_port,
    						port_id looper_port,
    						team_id app_team,
    						int32 app_token,
    						char *app_sig);
		void			Terminate();
static	status_t		MonitorTask(void *data);
		void			DispatchMessage(PortMessage *msg);
	//	ServerBitmap*	FindBitmap(int32 token);

		status_t		fInitError;
		port_id			fLocalPort,
						fAppPort,
						fLooperPort,
						fAppServerPort;
		char			*fSignature;
		thread_id		fMonitorThreadID;
		team_id			fTeamID;
//		BList			*winList, *bmpList, *picList;
//		DisplayDriver	*driver;
		sem_id			fSemID;
		bool			fIsActive;
		bool			fIsQuitting;
		int32			fHandlerToken;
		
	// Message Handlers
		status_t		QUIT_APP_handler(PortMessage *msg);
		status_t		B_QUIT_REQUESTED_handler(PortMessage *msg);
};
//------------------------------------------------------------------------------
#endif
