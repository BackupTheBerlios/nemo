/*------------------------------------------------------------------------------
/ libroot.c
/
/ DESCRIPTION:
/	Global data, initialization & termination procedures of libroot
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 1/2/2004
------------------------------------------------------------------------------*/

/* Standard Includes ---------------------------------------------------------*/
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

/* System Includes -----------------------------------------------------------*/
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <unistd.h>

/* Private Includes ----------------------------------------------------------*/
#include "kernel.h"
#include "libroot.h"

/* Debugging -----------------------------------------------------------------*/
#include "nemo_debug.h"
#if DEBUG_LIBROOT
	#define OUT(x...)	fprintf(LIBROOT_LOG, "libroot: "x);
	#define DBG(x)		x;
#else
	#define DBG(x)		;
#endif

/* Globals -------------------------------------------------------------------*/
int	_kernel_port_id		= -1;
int	_libroot_port_id	= -1;
int _fini_called 		= 0;

/*----------------------------------------------------------------------------*/
/*	library initialization; finds the kernel_server's message queue id,
	and creates a message queue	for this team
*/
void __attribute__ ((constructor)) libroot_init(void) {
	
	krnl_msg_t msg;
	sys_thread_info_t *main_thread;
	struct sembuf sb = {0, 1, 0};
	
	/* make sure finalization takes place in case we get a SIGINT */
	signal(SIGINT, (void(*)(int))libroot_fini);
	atexit(libroot_fini);

	DBG(OUT("Team %d starting...\n", getpid()));
	DBG(OUT("Connecting to kernel_server\n"));
	
	/* get kernel_server port */
	_kernel_port_id = msgget(P_KRNL_PORT_KEY, 0666);
	if(_kernel_port_id < 0) {
		DBG(OUT("Can't connect to kernel_server or kernel_server not running\n"));
		exit(-1);
	}

	/* create a new message queue for kernel_server replies */
	_libroot_port_id = msgget(IPC_PRIVATE, IPC_CREAT|0666);
	if(_libroot_port_id < 0) {
		DBG(OUT("Error creating message queue for kernel_server replies\n"));
		exit(-1);
	}

	/* create a semaphore to synchronize access to the thread list */
	if((_thread_list_sem = semget(IPC_PRIVATE, 1, IPC_CREAT|0666)) == -1) {
		DBG(OUT("Couldn't create thread list semaphore\n"));
		exit(-1);
	}
	semop(_thread_list_sem, &sb, 1);

	/* the main thread is added as the first thread in the thread list */
	main_thread = (sys_thread_info_t*) malloc(sizeof(sys_thread_info_t));
	if(!main_thread) {
		DBG(OUT("Error allocating memory for main thread\n"));
		exit(-1);
	}
	main_thread->info.thread = (thread_id)pthread_self();
	main_thread->info.team = (team_id)getpid();
	sprintf(main_thread->info.name, "main thread");
	main_thread->next = NULL;
	_thread_list = main_thread;
			
	/* add this team to the kernel_server's team list*/
	msg.cmd = P_KRNL_TEAM_ADD;
	msg.team_msg.remote = _libroot_port_id;
	msg.team_msg.info.team = (team_id)getpid();
	msg.team_msg.info.uid = getuid();
	msg.team_msg.info.gid = getgid();
	msg.team_msg._libroot_port = _libroot_port_id;
	msg.team_msg._libroot_thread_sem = _thread_list_sem;

	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);

	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.team_msg.error != B_OK) {
		DBG(OUT("Couldn't add team to kernel_server\n"));
		exit(-1);
	}
	
	DBG(OUT("Successfully initialized\n"));		
}
/*----------------------------------------------------------------------------*/
/* library finalization */
void __attribute__ ((destructor)) libroot_fini(void) {
	
	sys_thread_info_t *th;
	krnl_msg_t msg;
	
	/* finalization shouldn't be carried out more than once */
	if(_fini_called) return;
	_fini_called = 1;
	
	DBG(OUT("Cleaning up...\n"));
	
	/* empty thread list */
	DBG(OUT("Killing blocked threads, if any\n"));
	while(_thread_list) {
		th = _thread_list;
		_thread_list = _thread_list->next;
		if(pthread_self() != th->info.thread) {
			pthread_cancel(th->info.thread);
			DBG(OUT("Killed thread %d (%s)\n", th->info.thread, th->info.name));
		}
		delete_thread(th);
	}
	
	/* delete the thread list semaphore */
	if(_thread_list_sem != -1) {
		if(semctl(_thread_list_sem, 0, IPC_RMID) == -1)
			DBG(OUT("Error while removing _thread_list_sem\n"));
	}
		
	/* remove team from kernel_server */
	if(_kernel_port_id != -1 && _libroot_port_id != -1) {
		DBG(OUT("Asking kernel_server to delete this team\n"));
		msg.cmd = P_KRNL_TEAM_DELETE;
		msg.team_msg.remote = _libroot_port_id;
		msg.team_msg.info.team = (team_id)getpid();

		msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);

		/* wait for reply from kernel_server */
		msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
		if(msg.team_msg.error != B_OK)
			DBG(OUT("Couldn't remove team from kernel_server\n"));
	}

	/* remove libroot's message queue */
	if(_libroot_port_id != -1) {
		if(msgctl(_libroot_port_id, IPC_RMID, NULL) == -1)
			DBG(OUT("Error while removing message queue\n"));
	}

	DBG(OUT("Team terminated successfully\n"));
	exit(0);
}
/*----------------------------------------------------------------------------*/
/* returns the main thread id for this team */
thread_id get_main_thread(void) {

	sys_thread_info_t *th_info = NULL;
	thread_id id = B_BAD_TEAM_ID;

	if(acquire_sem(_thread_list_sem) == B_OK) {
		th_info = _thread_list;
    	while(th_info) {
    		if(!strcmp(th_info->info.name, "main thread")) {
    			id = th_info->info.thread;
				break;
    		}
    		th_info = th_info->next;
    	}
     	release_sem(_thread_list_sem);
     }

	return id;
}
/*----------------------------------------------------------------------------*/
