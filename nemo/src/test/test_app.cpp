#include "AppServer.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <OS.h>

//=============================================================================

key_t	key;
port_id	as_mqid;
port_id	app_mqid;
port_id	sa_mqid;

//=============================================================================

int32 Listen( void* ) {
	
	app_server_msg msg;
	
	while(true) {
		if(read_port(app_mqid, &msg.what, &msg, AS_MAX_MSG_SIZE) < 0) {
			printf("app: looper: error receiving message\n");
			return -1;
		}
		
		switch(msg.what) {
			case AS_CREATE_APP_REPLY: {
				sa_mqid = msg.create_app_reply.server_port;
			} break;
			
			case AS_INPUT_EXAMPLE_MSG: {
				printf("app: app_server says \"%s\"\n", msg.input.command);
			} break;
			
			case AS_QUIT_APP: return B_OK;
		}
	}
}

//=============================================================================

int main( void ) {
	
	// connect to app_server
	as_mqid = find_port(AS_PORT_NAME);
	
	if(as_mqid < 0) {
		printf("app: error connecting to app_server or app_server not running\n");
		exit(-1);
	} else {
		printf("app: connected to app_server at port %d\n", as_mqid);
	}
	
	// create app's message queue
	app_mqid = create_port(100, "tapp");
	if(app_mqid < 0) {
		printf("app: error creating message port\n");
		exit(-1);
	} else {
		printf("app: listening at port %d\n", app_mqid);
	}
	
	app_server_msg msg;
	
	// run looper
	thread_id looper_t = spawn_thread(Listen, "looper", B_NORMAL_PRIORITY, NULL);
	if(looper_t <= 0) {
		printf("app: error spawning looper thread\n");
		exit(-1);
	} else {
		printf("app: looper thread started...\n");
	}
	
	// notify the app_server of our existance
	msg.what = AS_CREATE_APP;
	msg.create_app.process_id = getpid();
	msg.create_app.app_port = app_mqid;
	sprintf(msg.create_app.app_sig, "test_app");
	
	if(write_port(as_mqid, msg.what, &msg, sizeof(create_app_msg) - 4) != B_OK) {
		printf("app: error sending message to app_server\n");
    }
	
	// wait till looper stops
	status_t rv;
	wait_for_thread(looper_t, &rv);
	printf("app: looper thread stopped\n");
	
	// close the app's message queue
	if(delete_port(app_mqid) != B_OK) {
		printf("app: error closing message port\n");
	}
	
	return 0;
}
