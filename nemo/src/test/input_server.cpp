// Standard Includes -----------------------------------------------------------
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// System Includes -------------------------------------------------------------
#include <AppDefs.h>
#include <OS.h>

// Private Includes ------------------------------------------------------------
#include "ServerProtocol.h"
#include "AppServer.h"
#include "PortMessage.h"

// Main ------------------------------------------------------------------------
int main( void ) {
	
	// connect to app_server
	port_id as_port = find_port(AS_PORT_NAME);
	if(as_port < 0) {
		printf("input_server: error connecting to app_server or app_server not running\n");
		exit(-1);
	} else {
		printf("input_server: connected to app_server at port %d\n", as_port);
	}
	
	// get app_server's input message queue
	port_id as_input_port = find_port(AS_INPUT_PORT_NAME);
	if(as_input_port < 0) {
		printf("input_server: error connecting to app_server's input port\n");
		exit(-1);
	} else {
		printf("input_server: app_server input port %d\n", as_input_port);
	}
	
	// create app's message queue
	port_id is_port = create_port(100, "input_server_port");
	if(is_port < 0) {
		printf("input_server: error creating input_server's message port\n");
		exit(-1);
	} else {
		printf("input_server: listening at port %d\n", is_port);
	}
	
	printf("\ninput_server: enter commands to send to app_server:\n");
	PortMessage msg;
	char text[1024];
	bool quit = false;
	while(true) {
		msg.Reset();
		scanf("%s", text);
		
		if(!strcmp(text, "quit")) {
			quit = true;
			msg.SetCode(B_QUIT_REQUESTED);
		}
		else {
			msg.SetCode(AS_INPUT_TEST_MSG);
			msg.AttachString(text);
		}
	
		if(msg.WriteToPort(as_input_port) != B_OK) {
			printf("input_server: error sending input message to app_server\n");
		}
		
		if(quit) break;
	}
	
	// close the input_server's message queue
	if(delete_port(is_port) != B_OK) {
		printf("input_server: error closing message port\n");
	}
	
	printf("input_server: terminated.\n");
	
	return 0;
}
