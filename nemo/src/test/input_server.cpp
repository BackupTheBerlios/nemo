#include "AppServer.h"

#include <OS.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
  
int main( void ) {
	
	// connect to app_server
	port_id as_mqid = find_port(AS_PORT_NAME);
	if(as_mqid < 0) {
		printf("input_server: error connecting to app_server or app_server not running\n");
		exit(-1);
	} else {
		printf("input_server: connected to app_server at port %d\n", as_mqid);
	}
	
	// get app_server's input message queue
	port_id asi_mqid = find_port(AS_INPUT_PORT_NAME);
	if(asi_mqid < 0) {
		printf("input_server: error connecting to app_server's input port\n");
		exit(-1);
	} else {
		printf("input_server: app_server input port %d\n", asi_mqid);
	}
	
	// create app's message queue
	port_id is_mqid = create_port(100, "ismp");
	if(is_mqid < 0) {
		printf("input_server: error creating input_server's message port\n");
		exit(-1);
	} else {
		printf("input_server: listening at port %d\n", is_mqid);
	}
	
	printf("\ninput_server: enter commands to send to app_server:\n");
	app_server_msg msg;
	bool quit = false;
	while(true) {
		scanf("%s", msg.input.command);
		
		if(!strcmp(msg.input.command, "quit")) {
			quit = true;
			msg.what = B_QUIT_REQUESTED;
		}
		else {
			msg.what = AS_INPUT_EXAMPLE_MSG;
		}
	
		if(write_port(asi_mqid, msg.what, &msg, sizeof(input_msg) - 4) != B_OK) {
			printf("input_server: error sending input message to app_server\n");
		}
		
		if(quit) break;
	}
	
	// close the input_server's message queue
	if(delete_port(is_mqid) != B_OK) {
		printf("input_server: error closing message port\n");
	}
	
	printf("input_server: terminated.\n");
	
	return 0;
}
