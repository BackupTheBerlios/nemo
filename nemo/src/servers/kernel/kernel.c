/*------------------------------------------------------------------------------
/ kernel.c
/
/ Description:
/	This file, along with the rest of files in src/servers/kernel, represent
/	the kernel_server binary. Applications communicate with the kernel_server
/	via IPC to get access to system-wide recognizable objects, like ports,
/	semaphores, shared memory areas, etc...
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 31/1/2004
/-----------------------------------------------------------------------------*/

/* Standard Includes ---------------------------------------------------------*/
#include <stdlib.h>

/* System Includes -----------------------------------------------------------*/
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

/* Private Includes ----------------------------------------------------------*/
#include "kernel.h"

/* Debugging -----------------------------------------------------------------*/
#include "nemo_debug.h"
#if DEBUG_KERNEL
	#define OUT(x...)	fprintf(KERNEL_LOG, "kernel_server: "x);
	#define DBG(x)		x;
#else
	#define DBG(x)		;
#endif

/* Globals -------------------------------------------------------------------*/
int	_kernel_port_id	= -1;

sys_area_info_t	*_area_list	= NULL;
sys_port_info_t	*_port_list	= NULL;
sys_sem_info_t	*_sem_list	= NULL;
sys_team_info_t	*_team_list	= NULL;

int	_area_list_sem = -1;
int _port_list_sem = -1;
int _sem_list_sem = -1;
int _team_list_sem = -1;

/*----------------------------------------------------------------------------*/
int main(int argc, char **argv) {

	/*	make sure proper termination takes place on interruption/abortion */
	signal(SIGINT, sig_catcher);
	signal(SIGQUIT, sig_catcher);
	signal(SIGABRT, sig_catcher);
	signal(SIGTERM, sig_catcher);

	/* initialization */
	if(init() < 0) {
		DBG(OUT("Error during initialization...bailing out\n"));
		cleanup();
		exit(-1);
	}

	/* listen to incoming requests */
	listen();

	/* terminate */
	cleanup();
	exit(0);
}
/*----------------------------------------------------------------------------*/
void
sig_catcher(int x) {

	cleanup();
	exit(0);
}
/*----------------------------------------------------------------------------*/
status_t
init(void) {

	DBG(OUT("Initializing...\n"));

	/* create kernel_server message queue */
	if((_kernel_port_id =
		msgget(P_KRNL_PORT_KEY, IPC_CREAT|IPC_EXCL|0666)) == -1) {
		DBG(OUT("Error creating message queue\n"));
		return B_ERROR;
	}

	/* create semaphores to synchronize access to the lists */
	_area_list_sem = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
	_port_list_sem = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
	_sem_list_sem = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
	_team_list_sem = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);

	if(_area_list_sem < 0 || _port_list_sem < 0 || _sem_list_sem < 0
		|| _team_list_sem < 0) {
		DBG(OUT("Error creating semaphore\n"));
		return B_ERROR;
	}

	semctl(_area_list_sem, 0, SETVAL, 1);
	semctl(_port_list_sem, 0, SETVAL, 1);
	semctl(_sem_list_sem, 0, SETVAL, 1);
	semctl(_team_list_sem, 0, SETVAL, 1);

	return B_OK;
}
/*----------------------------------------------------------------------------*/
void
cleanup(void) {

	sys_area_info_t *area;
	sys_port_info_t *port;
	sys_sem_info_t	*sem;
	sys_team_info_t *team;

	DBG(OUT("Cleaning up...\n"));

	/* acquire all sems to make sure we're not interrupting other operations */
	acquire_sem_priv(_area_list_sem);
	acquire_sem_priv(_port_list_sem);
	acquire_sem_priv(_sem_list_sem);
	acquire_sem_priv(_team_list_sem);

	/* close kernel_server message queue */
	msgctl(_kernel_port_id, IPC_RMID, NULL);

	/* free lists */
	while(_area_list) {
		area = _area_list;
		_area_list = _area_list->next;
		free(area);
	} /* areas */

	while(_port_list) {
		port = _port_list;
		_port_list = _port_list->next;
		free(port);
	} /* ports */

	while(_sem_list) {
		sem = _sem_list;
		_sem_list = _sem_list->next;
		free(sem);
	} /* semaphores */

	while(_team_list) {
		team = _team_list;
		_team_list = _team_list->next;
		free(team);
	} /* teams */

	/* delete all sems */
	semctl(_area_list_sem, 0, IPC_RMID);
	semctl(_port_list_sem, 0, IPC_RMID);
	semctl(_sem_list_sem, 0, IPC_RMID);
	semctl(_team_list_sem, 0, IPC_RMID);

	DBG(OUT("Terminated sucessfully\n"));
}
/*----------------------------------------------------------------------------*/
status_t
acquire_sem_priv(int semid) {

	struct sembuf sb = {0, -1, 0};

	if(semop(semid, &sb, 1) == -1)
		return B_ERROR;

	return B_OK;
}
/*----------------------------------------------------------------------------*/
status_t
release_sem_priv(int semid) {

	struct sembuf sb = {0, 1, 0};

	if(semop(semid, &sb, 1) == -1)
		return B_ERROR;

	return B_OK;
}
/*----------------------------------------------------------------------------*/
status_t
listen(void) {

	krnl_msg_t msg;
	DBG(OUT("Listening to incoming requests at port %d\n", _kernel_port_id));

	for(;;) {
		if(msgrcv(_kernel_port_id, &msg, sizeof(msg), 0, 0) == -1) {
			DBG(OUT("Error while reading message\n"));
		}

		/* decode function type */
		switch(msg.cmd & P_KRNL_FN_TYPE_MASK) {
			/*case P_KRNL_AREA:
				handle_area_msg(&msg);
				break;*/

			case P_KRNL_PORT:
				handle_port_msg(&msg);
				break;

			case P_KRNL_SEM:
				handle_sem_msg(&msg);
				break;

			case P_KRNL_TEAM:
				handle_team_msg(&msg);
				break;

			/*case P_KRNL_THREAD:
				handle_thread_msg(&msg);
				break;

			case P_KRNL_TIME:
				handle_time_msg(&msg);
				break;

			case P_KRNL_ALARM:
				handle_alarm_msg(&msg);
				break;

			case P_KRNL_DBG:
				handle_debug_msg(&msg);
				break;

			case P_KRNL_IMAGE:
				handle_image_msg(&msg);
				break;

			case P_KRNL_SYSTEM:
				handle_system_msg(&msg);
				break;*/

			/* TODO: app_server should issue this command at exit */
			case P_KRNL_QUIT:
				DBG(OUT("kernel_server: Received Quit command\n"));
				return B_OK;

			default:
				DBG(OUT("kernel_server: Unrecognizable message received\n"));
		}
	}
}
/*----------------------------------------------------------------------------*/
