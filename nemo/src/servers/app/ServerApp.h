#ifndef _SERVERAPP_H_
#define _SERVERAPP_H_

#include <AppServer.h>

#include <OS.h>

class AppServer;
/*class BMessage;
class PortLink;
class PortMessage;*/
class BList;
/*class DisplayDriver;
class ServerCursor;
*/
//union app_server_msg;

//=============================================================================

class ServerApp {
	
public:
					ServerApp(	port_id app_port,
								port_id server_app_port,
								port_id app_server_port,
								pid_t process_id,
								int32 handler_id,
								const char *app_sig);
					~ServerApp();

		bool		Run();
//		void		Lock();
//		void		Unlock();
//		bool		IsLocked();
		bool		IsActive() const;
		void		Activate(bool active);
		bool		PingTarget();
		void		PostMessage(app_server_msg *msg);
		void		PostMessage(int32 what);
		void		ForwardMessage(app_server_msg *msg);

//		void		SetAppCursor();

private:

static	status_t	MonitorApp(void *data);	
		void		Terminate();

protected:

friend	class		AppServer;
//friend	class		ServerWindow;

		void		DispatchMessage(app_server_msg *msg);
//	ServerBitmap*	FindBitmap(int32 token);

		port_id		mAppServerPort, mAppPort, mServerAppPort;
//		BString		signature;
		char		mSignature[B_MIME_TYPE_LENGTH + 1];
		thread_id	mMonitorThread;
//		team_id		targetID;
		team_id		mTargetID;
//		PortLink	*appLink;
//		BList		*winList, *bmpList, *picList;
//		DisplayDriver
//					*driver;
//		ServerCursor
//					*appCursor;
//		sem_id		lock;
//		bool		cursorHidden;
		bool		mIsActive;
		bool		mIsQuitting;
//		int32		handlerToken;
};

#endif
