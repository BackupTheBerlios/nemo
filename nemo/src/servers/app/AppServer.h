#ifndef _APPSERVER_H
#define _APPSERVER_H

#include <OS.h>

class BList;
class ServerApp;

//=============================================================================

// TODO: these definitions & types should be removed
// from here & put in their appropriate places
// TODO: use a file specific to app_server instead
#define AS_PORT_NAME				"asap"
#define AS_INPUT_PORT_NAME			"asip"
#define B_QUIT_REQUESTED			0x00000001
#define AS_CREATE_APP				0x00000002
#define AS_CREATE_APP_REPLY			0x00000003
#define AS_DELETE_APP				0x00000004
#define AS_QUIT_APP					0x00000005
#define AS_SET_SERVER_PORT			0x00000006
#define AS_INPUT_MSG				0x00000007
#define AS_INPUT_EXAMPLE_MSG		0x00000008

//=============================================================================

struct raw_msg {
	int32	what;
	void*	buffer;
};

//=============================================================================

struct create_app_msg {
	int32	what;
	pid_t	process_id;
	int32	handler_token;
	port_id	app_port;
	char	app_sig[B_MIME_TYPE_LENGTH + 1];
};

//=============================================================================

struct create_app_reply_msg {
	int32	what;
	port_id	server_port;
};

//=============================================================================

struct delete_app_msg {
	int32	what;
	pid_t	process_id;
};

//=============================================================================

struct input_msg {
	int32	what;
//	int8	data[1];
	char	command[128];
};

//=============================================================================

union app_server_msg {
	
	int32					what;
	
	raw_msg					raw;
	create_app_msg			create_app;
	create_app_reply_msg	create_app_reply;
	delete_app_msg			delete_app;
	
	input_msg				input;
};

//=============================================================================

#define AS_MAX_MSG_SIZE		sizeof(app_server_msg)-4

//=============================================================================
// CLASS AppServer ////////////////////////////////////////////////////////////
//=============================================================================

class AppServer {

public:
					AppServer();
					~AppServer();
		
		thread_id	Run();

private:
		status_t	Init();
		void		Terminate();
		void		MainLoop();
		void		DispatchMessage(app_server_msg *msg);
		void		Broadcast(int32 what);
		void		HandleKeyMessage(int32 code, int8 *buffer);
static	int32		Poller(void *data);
static	int32		Picasso(void *data);

		thread_id	fPollerThreadID, fPicassoThreadID;
		port_id		fMessagePortID, fInputPortID;
		BList		*fAppList;
		ServerApp	*fActiveApp;
		int32		fActiveAppIndex;
		bool		fIsQuitting;
};

#endif
