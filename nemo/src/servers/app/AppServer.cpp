#include <unistd.h>
//#include <string.h>

#include <List.h>

#include "AppServer.h"
#include "ServerApp.h"

#if DEBUG
	#include "nemo_debug.h"
#endif

//=============================================================================

AppServer::AppServer() {
		
	if(Init() != B_OK) {
		#if DEBUG_APPSERVER
			fprintf(APPSERVER_LOG, "app_server: failed to initialize\n");
		#endif
	}
	else {
		#if DEBUG_APPSERVER
			fprintf(APPSERVER_LOG, "app_server: initialized successfully\n");
		#endif
	}
}

//=============================================================================

AppServer::~AppServer() {
	
	Terminate();
}

//=============================================================================

thread_id AppServer::Run() {
	
	MainLoop();
	return (thread_id)getpid();
}

//=============================================================================

status_t AppServer::Init() {
	
	// create the app_server's & input server's message queues
	// TODO: message queue access permisssion (IPC_EXCL)
	if((fMessagePortID = create_port(100, AS_PORT_NAME)) < 0) {
		#if DEBUG_APPSERVER
			fprintf(APPSERVER_LOG, "app_server: error creating main message port (%s)\n", errno2string(errno));
		#endif
		return B_ERROR;
	}
	#if DEBUG_APPSERVER
		fprintf(APPSERVER_LOG, "app_server: listening to apps at port %d\n", fMessagePortID);
	#endif
	
	if((fInputPortID = create_port(100, AS_INPUT_PORT_NAME)) < 0) {
		#if DEBUG_APPSERVER
			fprintf(APPSERVER_LOG, "app_server: error creating the input server's message port (%s)\n", errno2string(errno));
		#endif
		return B_ERROR;
	}
	#if DEBUG_APPSERVER
		fprintf(APPSERVER_LOG, "app_server: listening to input_server at port %d\n", fInputPortID);
	#endif
	
	// TODO: initialize the desktop
	
	// spawn the message & input threads
	// TODO: use a customized pthread_attr_t
	fPollerThreadID = spawn_thread(Poller, "poller", B_NORMAL_PRIORITY, this);
	if(fPollerThreadID <= 0 ) {
//	if(pthread_create(&(pthread_t)fPollerThreadID, NULL, Poller, this) != 0) {
		#if DEBUG_APPSERVER
			fprintf(APPSERVER_LOG, "app_server: couldn't spawn the input poller thread (%s)\n", errno2string(errno));
		#endif
		return B_ERROR;
	}

	fPicassoThreadID = spawn_thread(Picasso, "picasso", B_NORMAL_PRIORITY, this);
	if(fPicassoThreadID <= 0) {
//	if(pthread_create(&(pthread_t)fPicassoThreadID, NULL, Picasso, this) != 0) {
		#if DEBUG_APPSERVER
			//fprintf(APPSERVER_LOG, "couldn't spawn the Picasso thread (%s)\n", errno2string(errno));
		#endif
		return B_ERROR;
	}
	
	// initialize the application list
	fAppList = new BList();
	fActiveApp = NULL;
	fIsQuitting = false;
	
	return B_OK;
}

//=============================================================================

void AppServer::Terminate() {
	
	// TODO: stop all apps
	// TODO: delete app list
	// TODO: wait till Poller & Picasso have stopped

	// remove the message queues
	if(fMessagePortID >= 0) {
		if(delete_port(fMessagePortID) != B_OK) {
			#if DEBUG_APPSERVER
				fprintf(APPSERVER_LOG, "app_server: couldn't remove main message port (%s)\n", errno2string(errno));
			#endif
		}
		#if DEBUG_APPSERVER
			fprintf(APPSERVER_LOG, "app_server: main message port removed\n");
		#endif		
	}
	
	if(fInputPortID >= 0) {
		if(delete_port(fInputPortID) != B_OK) {
			#if DEBUG_APPSERVER
				fprintf(APPSERVER_LOG, "app_server: couldn't remove input_server's message port (%s)\n", errno2string(errno));
			#endif
		}
		#if DEBUG_APPSERVER
			fprintf(APPSERVER_LOG, "app_server: input_server's message port removed\n");
		#endif				
	}
}

//=============================================================================

void AppServer::MainLoop() {
	
	#if DEBUG_APPSERVER
		fprintf(APPSERVER_LOG, "app_server: main loop started...\n");
	#endif
	
	app_server_msg msg;
	ssize_t msg_len;
	
	// read application messages
	while(true) {
		
		// read next message
		if((msg_len = read_port(fMessagePortID, &(msg.what), &msg, AS_MAX_MSG_SIZE)) < 0) {
			#if DEBUG_APPSERVER
				fprintf(APPSERVER_LOG, "app_server: error while reading from main message port (%s)\n", errno2string(errno));
			#endif
		}
		
		// decode & process message
		switch(msg.what) {
			case B_QUIT_REQUESTED:
				#if DEBUG_APPSERVER
					fprintf(APPSERVER_LOG, "app_server: recieved request to quit\n");
				#endif			
				// stop poller
				if(write_port(fInputPortID, msg.what, &msg, 0) != B_OK) {
					#if DEBUG_APPSERVER
						fprintf(APPSERVER_LOG, "app_server: error while asking Poller to stop (%d)\n", errno2string(errno));
					#endif
				}
				// TODO: stop picasso
				fIsQuitting = true;
			case AS_CREATE_APP:
			case AS_DELETE_APP:
				DispatchMessage(&msg);
				break;
			default: {
				#if DEBUG_APPSERVER
					fprintf(APPSERVER_LOG, "app_server: main loop received an unknown message: %d\n", msg.what);
				#endif
				break;
			}
		}
		
		// time to stop?
		if(fIsQuitting)	{
			break;
		}
	}

	#if DEBUG_APPSERVER
		fprintf(APPSERVER_LOG, "app_server: main loop terminated.\n");
	#endif
}

//=============================================================================

void AppServer::DispatchMessage(app_server_msg *msg) {
	
	switch(msg->what) {
		case AS_CREATE_APP: {
			#if DEBUG_APPSERVER
				fprintf(APPSERVER_LOG, "app_server: received request to create app \"%s\"\n", msg->create_app.app_sig);
			#endif
			// Create the ServerApp to node monitor a new BApplication

			// TODO: sync using semaphores
			// Create the ServerApp subthread for this app
//			acquire_sem(_applist_lock);
			
			port_id server_app_port;
			if((server_app_port = create_port(100, "unnp")) < 0) {
				#if DEBUG_APPSERVER
					fprintf(APPSERVER_LOG, "app_server: error creating a message port for the server side app \"%s\"\n", msg->create_app.app_sig);
//					release_sem(_applist_lock);
				#endif
				break;
			}
				
			ServerApp *newServerApp =
				new ServerApp(	msg->create_app.app_port,
								server_app_port,
								fMessagePortID,
								msg->create_app.process_id,
								msg->create_app.handler_token,
								msg->create_app.app_sig);
			
			fAppList->AddItem(newServerApp);
			
//			release_sem(_applist_lock);

//			acquire_sem(_active_lock);
			fActiveApp = newServerApp;
			fActiveAppIndex = fAppList->CountItems() - 1;

			create_app_reply_msg reply = {AS_SET_SERVER_PORT, server_app_port};
			if(write_port(msg->create_app.app_port, reply.what, &reply, sizeof(create_app_reply_msg) - 4) != B_OK) {
				#ifndef DEBUG_APPSERVER
					fprintf(APPSERVER_LOG, "app_server: error sending reply message to app \"%s\"\n", msg->create_app.app_sig);
				#endif
			}			
//			release_sem(_active_lock);
			
			newServerApp->Run();
			#if DEBUG_APPSERVER
				fprintf(APPSERVER_LOG, "app_server: server side app \"%s\" listening at port %d\n", msg->create_app.app_sig, server_app_port);
			#endif			
		} break;
		
		case AS_DELETE_APP: {
			// Delete a ServerApp. Received only from the respective ServerApp when a
			// BApplication asks it to quit.
			
			ServerApp *serverApp;
			team_id app_pid = msg->delete_app.process_id;
			
			// Run through the list of apps and nuke the proper one
			for(int i=0; i < fAppList->CountItems(); i++) {
				serverApp = (ServerApp*) fAppList->ItemAt(i);

				if(serverApp != NULL && serverApp->mTargetID == app_pid) {
					// TODO: semaphores...
//					acquire_sem(_applist_lock);
					serverApp = (ServerApp*) fAppList->RemoveItem(i);
					if(serverApp) {
						delete serverApp;
						serverApp = NULL;
					}
					// TODO: more semaphores...
//					release_sem(_applist_lock);
//					acquire_sem(_active_lock);

					if(fAppList->CountItems() == 0)	{
						// active==-1 signifies that no other apps are running - NOT good
						fActiveAppIndex = -1;
					}
					else {
						// we actually still have apps running, so make a new one active
						fActiveAppIndex > 0? fActiveAppIndex-- : fActiveAppIndex = 0;
					}
					fActiveApp = fActiveAppIndex != -1? (ServerApp*) fAppList->ItemAt(fActiveAppIndex) : NULL;
//					release_sem(_active_lock);
					break;	// jump out of our for() loop
				}

			}
		} break;
		
		case B_QUIT_REQUESTED: {
			#if DEBUG_APPSERVER
				fprintf(APPSERVER_LOG, "app_server: asking all apps to quit\n");
			#endif
			// TODO: fix the quitting mechanism, this is just for testing
			//Broadcast(AS_QUIT_APP);
			if(fActiveApp) {
				app_server_msg a_msg;
				a_msg.what = AS_QUIT_APP;
				fActiveApp->ForwardMessage(&a_msg);
				fActiveApp->PostMessage(&a_msg);
			}
			fIsQuitting = true;
		} break;
		
		default:
			#if DEBUG_APPSERVER
				fprintf(APPSERVER_LOG, "app_server: received an unknown message (%d)\n", msg->what);
			#endif
	}
}

//=============================================================================

void AppServer::Broadcast(int32 what) {

	// TODO: use semaphores
//	acquire_sem(_applist_lock);
	for(int i = 0; i < fAppList->CountItems(); i++ ) {
		ServerApp *app = (ServerApp*) fAppList->ItemAt(i);
		if(!app) continue;
		app->PostMessage(what);
	}
	// TODO: use semaphores	
//	release_sem(_applist_lock);
}

//=============================================================================

int32 AppServer::Poller(void *data) {

	#if DEBUG_APPSERVER
		fprintf(APPSERVER_LOG, "app_server: input poller thread started...\n");
	#endif
	
	AppServer *app_server=(AppServer*)data;
	app_server_msg msg;
	
	while(true) {
		if(read_port(app_server->fInputPortID, &(msg.what), &msg, sizeof(input_msg) - 4) < 0) {
			#if DEBUG_APPSERVER
				fprintf(APPSERVER_LOG, "app_server: poller: error receiving input message (%s)\n", errno2string(errno));
			#endif
		}
		
		switch(msg.what) {
			case B_QUIT_REQUESTED: {
				printf("app_server: poller: received request to quit\n");
				// stop the main loop
				if(write_port(app_server->fMessagePortID, msg.what, &msg, 0) != B_OK) {
					#if DEBUG_APPSERVER
						fprintf(APPSERVER_LOG, "app_server: poller: error asking main loop to stop (%d)\n", errno2string(errno));
					#endif
				}
				app_server->fIsQuitting = true;
				
			} break;
			
			case AS_INPUT_EXAMPLE_MSG: {
				printf("app_server: poller: got '%s'\n", msg.input.command);
				if(app_server->fActiveApp) {
					app_server->fActiveApp->ForwardMessage(&msg);
				}
			} break;
		}
		
		if(app_server->fIsQuitting) break;
	}
	
	#if DEBUG_APPSERVER
		fprintf(APPSERVER_LOG, "app_server: input poller thread terminated.\n");
	#endif
		
	return B_OK;
}

//=============================================================================

int32 AppServer::Picasso(void *data) {

	#if DEBUG_APPSERVER
		//fprintf(APPSERVER_LOG, "Picasso thread started\n");
	#endif
	
	#if DEBUG_APPSERVER
		//fprintf(APPSERVER_LOG, "Picasso thread exiting\n");
	#endif
		
	return B_OK;
}

//=============================================================================
int main(int argc, char **argv) {

	// TODO: check if the app_server is already running
	#if DEBUG_APPSERVER
		fprintf(APPSERVER_LOG, "app_server: starting...\n");	
	#endif

	AppServer *app_server = new AppServer();
	app_server->Run();
	delete app_server;

	#if DEBUG_APPSERVER
		fprintf(APPSERVER_LOG, "app_server: terminated.\n");
	#endif
	
	return 0;
}
