/*------------------------------------------------------------------------------
/ team.c
/
/ DESCRIPTION:
/	Team functions of the kernel_server
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 17/2/2004
/-----------------------------------------------------------------------------*/

/* Standard Includes ---------------------------------------------------------*/
#include <signal.h>
#include <string.h>

/* System Includes -----------------------------------------------------------*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include <Errors.h>

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

/*----------------------------------------------------------------------------*/
void
handle_team_msg(krnl_msg_t *msg) {

	krnl_team_msg_t reply;
	status_t error;

	/* decode function */
	switch(msg->cmd) {
		case P_KRNL_TEAM_ADD:
			error = sys_add_team(&(msg->team_msg), &reply);
			break;

		case P_KRNL_TEAM_DELETE:
			error = sys_delete_team(&(msg->team_msg), &reply);
			break;

		case P_KRNL_TEAM_KILL:
			error = sys_kill_team(&(msg->team_msg), &reply);
			break;

		case P_KRNL_TEAM_INFO:
			error = sys_get_team_info(&(msg->team_msg), &reply);
			break;

		case P_KRNL_TEAM_NEXT_INFO:
			error = sys_get_next_team_info(&(msg->team_msg), &reply);
			break;

		default:
			error = B_BAD_VALUE;
	}

	/* reply to requester */
	reply.cmd = P_KRNL_REPLY;
	reply.error = error;
	if(error != B_OK)
		DBG(OUT("Error performing team request\n"));

	if(msgsnd(msg->team_msg.remote, &reply, sizeof(reply), 0) == -1)
		DBG(OUT("Error while replying to remote requester\n"));
}
/*----------------------------------------------------------------------------*/
status_t
sys_add_team(krnl_team_msg_t *msg, krnl_team_msg_t *reply) {

	sys_team_info_t *t_info = NULL;

	/* does the given team id already exist? */
	acquire_sem_priv(_team_list_sem);
	t_info = _team_list;
	while(t_info) {
		if(t_info->info.team == msg->info.team) {
			release_sem_priv(_team_list_sem);
			return B_NAME_IN_USE;
		}
		t_info = t_info->next;
	}

	t_info = (sys_team_info_t*) malloc(sizeof(sys_team_info_t));
	if(!t_info) {
		release_sem_priv(_team_list_sem);
		return B_NO_MORE_TEAMS;
	}

	/* append new team to global team list */
	memcpy(&(t_info->info), &(msg->info), sizeof(team_info));
	t_info->_libroot_port = msg->_libroot_port;
	t_info->_libroot_thread_sem = msg->_libroot_thread_sem;
	t_info->next = _team_list;
	_team_list = t_info;

	/* all went fine */
	release_sem_priv(_team_list_sem);

	DBG(OUT("Created team %d\n", t_info->info.team));
	return B_OK;
}
/*----------------------------------------------------------------------------*/
status_t
sys_delete_team(krnl_team_msg_t *msg, krnl_team_msg_t *reply) {

	sys_team_info_t *curr_team = NULL;
	sys_team_info_t **prev_team = NULL;

	acquire_sem_priv(_team_list_sem);
	curr_team = _team_list;
	prev_team = &_team_list;
	while(curr_team) {
		if(curr_team->info.team == msg->info.team) {
			/* team found...remove it from list */
			*prev_team = curr_team->next;
			release_sem_priv(_team_list_sem);
			free(curr_team);

			DBG(OUT("Deleted team %d\n", msg->info.team));
			return B_OK;
		}
		prev_team = &(curr_team->next);
		curr_team = curr_team->next;
	}

	release_sem_priv(_team_list_sem);
	return B_BAD_TEAM_ID;
}
/*----------------------------------------------------------------------------*/
status_t
sys_kill_team(krnl_team_msg_t *msg, krnl_team_msg_t *reply) {

	sys_team_info_t *curr_team = NULL;
	sys_team_info_t **prev_team = NULL;
	sys_port_info_t *curr_port = NULL;
	sys_port_info_t **prev_port = NULL;
	sys_sem_info_t *curr_sem = NULL;
	sys_sem_info_t **prev_sem = NULL;

	acquire_sem_priv(_team_list_sem);
	curr_team = _team_list;
	prev_team = &_team_list;
	while(curr_team) {
		if(curr_team->info.team == msg->info.team) {
			/* team found...remove it from list */
			*prev_team = curr_team->next;
			release_sem_priv(_team_list_sem);

			if(kill((pid_t)(msg->info.team), SIGKILL) == -1) {
				free(curr_team);
				return B_BAD_TEAM_ID;
			}

			/* remove objects allocated by libroot that were
				to be removed on normal termination */
			msgctl(curr_team->_libroot_port, IPC_RMID, NULL);
			semctl(curr_team->_libroot_thread_sem, 0, IPC_RMID);

			/* remove ports that belong to the team just killed */
			acquire_sem_priv(_port_list_sem);
			curr_port = _port_list;
			prev_port = &_port_list;
			while(curr_port) {
				if(curr_port->info.team == msg->info.team) {
					msgctl(curr_port->info.port, IPC_RMID, NULL);
					*prev_port = curr_port->next;
					free(curr_port);
					curr_port = *prev_port;
					continue;
				}
				prev_port = &(curr_port->next);
				curr_port = curr_port->next;
			}
			release_sem_priv(_port_list_sem);

			/* remove sems that belong to the team just killed */
			acquire_sem_priv(_sem_list_sem);
			curr_sem = _sem_list;
			prev_sem = &_sem_list;
			while(curr_sem) {
				if(curr_sem->info.team == msg->info.team) {
					semctl(curr_sem->info.sem, 0, IPC_RMID);
					*prev_sem = curr_sem->next;
					free(curr_sem);
					curr_sem = *prev_sem;
					continue;
				}
				prev_sem = &(curr_sem->next);
				curr_sem = curr_sem->next;
			}
			release_sem_priv(_sem_list_sem);

			DBG(OUT("Killed team %d\n", msg->info.team));
			free(curr_team);
			return B_OK;
		}
		prev_team = &(curr_team->next);
		curr_team = curr_team->next;
	}

	release_sem_priv(_team_list_sem);
	return B_BAD_TEAM_ID;
}
/*----------------------------------------------------------------------------*/
status_t
sys_get_team_info(krnl_team_msg_t *msg, krnl_team_msg_t *reply) {

	sys_team_info_t *t_info = NULL;

	acquire_sem_priv(_team_list_sem);
	t_info = _team_list;
	while(t_info) {
		if(t_info->info.team == msg->info.team) {
			memcpy(&(reply->info), &(t_info->info), sizeof(team_info));
			release_sem_priv(_team_list_sem);

			DBG(OUT("Retrieved info about team %d\n", t_info->info.team));
			return B_OK;
		}

		t_info = t_info->next;
	}

	release_sem_priv(_team_list_sem);
	return B_BAD_VALUE;
}
/*----------------------------------------------------------------------------*/
status_t
sys_get_next_team_info(krnl_team_msg_t *msg, krnl_team_msg_t *reply) {

	static sys_team_info_t *t_info = NULL;

	acquire_sem_priv(_team_list_sem);

	/* if cookie = 0, reset team pointer to first team */
	if(!msg->cookie)
		t_info = _team_list;

	while(t_info) {
			memcpy(&(reply->info), &(t_info->info), sizeof(team_info));
			DBG(OUT("Retrieved info about team %d\n", t_info->info.team));

			t_info = t_info->next;
			release_sem_priv(_team_list_sem);
			return B_OK;
	}

	release_sem_priv(_team_list_sem);
	return B_BAD_VALUE;
}
/*----------------------------------------------------------------------------*/
