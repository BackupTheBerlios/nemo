/*------------------------------------------------------------------------------
/ sem.c
/
/ DESCRIPTION:
/	Semaphore functions of the kernel_server
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 1/2/2004
/-----------------------------------------------------------------------------*/

/* Standard Includes ---------------------------------------------------------*/
#include <malloc.h>
#include <string.h>

/* System Includes -----------------------------------------------------------*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/* Private Includes ----------------------------------------------------------*/
#include "kernel.h"
#include "libroot.h"

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
handle_sem_msg(krnl_msg_t *msg) {

	krnl_sem_msg_t reply;
	status_t error;

	/* decode function */
	switch(msg->cmd) {
		case P_KRNL_SEM_CREATE:
			error = sys_create_sem(&(msg->sem_msg), &reply);
			break;

		case P_KRNL_SEM_DELETE:
			error = sys_delete_sem(&(msg->sem_msg), &reply);
			break;

		case P_KRNL_SEM_OWNER:
			error = sys_set_sem_owner(&(msg->sem_msg), &reply);
			break;

		case P_KRNL_SEM_INFO:
			error = sys_get_sem_info(&(msg->sem_msg), &reply);
			break;

		case P_KRNL_SEM_NEXT_INFO:
			error = sys_get_next_sem_info(&(msg->sem_msg), &reply);
			break;

		default:
			error = B_BAD_VALUE;
	}

	/* reply to requester */
	reply.cmd = P_KRNL_REPLY;
	reply.error = error;
	if(error != B_OK)
		DBG(OUT("Error performing semaphore request\n"));

	if(msgsnd(msg->sem_msg.remote, &reply, sizeof(reply), 0) == -1)
		DBG(OUT("Error while replying to remote requester\n"));
}
/*----------------------------------------------------------------------------*/
status_t
sys_create_sem(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply) {

	sys_team_info_t *team = NULL;
	sys_sem_info_t *s_info = NULL;
	int id = -1;

	/* check parameters */
	if(strlen(msg->info.name) >= B_OS_NAME_LENGTH || msg->info.count < 0)
		return B_BAD_VALUE;

	/* does the given team id exist? */
	acquire_sem_priv(_team_list_sem);
	team = _team_list;
	while(team) {
		if(team->info.team == msg->info.team) {
			/* team exists...proceed */
			s_info = (sys_sem_info_t*) malloc(sizeof(sys_sem_info_t));

			if(s_info == NULL) {
				release_sem_priv(_team_list_sem);
				return B_NO_MEMORY;
			}

			/* create the semaphore	*/
			if((id = semget(IPC_PRIVATE, 1, IPC_CREAT|0666)) == -1) {
				release_sem_priv(_team_list_sem);
				free(s_info);
				switch(errno) {
					case EACCES:
					case EEXIST:
					case ENOENT:
					case EINVAL: return B_BAD_VALUE;
					case ENOMEM: return B_NO_MEMORY;
					case ENOSPC: return B_NO_MORE_SEMS;
				}
			}

			if(semctl(id, 0, SETVAL, msg->info.count) == -1) {
				DBG(OUT("Error setting semaphore properties\n"));
				return B_ERROR;
			}

			/* append new semaphore to global semaphore list */
			memcpy(&(s_info->info), &(msg->info), sizeof(sem_info));
			s_info->info.sem = reply->info.sem = (sem_id)id;

			acquire_sem_priv(_sem_list_sem);
			s_info->next = _sem_list;
			_sem_list = s_info;

			release_sem_priv(_team_list_sem);
			release_sem_priv(_sem_list_sem);

			DBG(OUT("Created semaphore %d\n", s_info->info.sem));
			return B_OK;
		}

		team = team->next;
	}

	/* team doesn't exist */
	release_sem_priv(_team_list_sem);
	return B_ERROR;
}
/*----------------------------------------------------------------------------*/
status_t
sys_delete_sem(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply) {

	sys_sem_info_t *curr_sem = NULL;
	sys_sem_info_t **prev_sem = NULL;

	acquire_sem_priv(_sem_list_sem);
	curr_sem = _sem_list;
	prev_sem = &_sem_list;
	while(curr_sem) {
		if(curr_sem->info.sem == msg->info.sem) {
			/* semaphore found...remove it from list */
			*prev_sem = curr_sem->next;
			release_sem_priv(_sem_list_sem);
			free(curr_sem);

			if(semctl((int)msg->info.sem, 0, IPC_RMID) == -1) {
				switch(errno) {
					case EACCES:
					case EPERM:
					case EINVAL:
					case EIDRM: return B_BAD_SEM_ID;
				}
			}

			DBG(OUT("Deleted semaphore %d\n", msg->info.sem));
			return B_OK;
		}
		prev_sem = &(curr_sem->next);
		curr_sem = curr_sem->next;
	}

	release_sem_priv(_sem_list_sem);
	return B_BAD_SEM_ID;
}
/*----------------------------------------------------------------------------*/
status_t
sys_set_sem_owner(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply) {

	/*	semaphoress are owned by teams in BeOS, while they're owned by
		users in Linux. Due to this "cultural misunderstanding", this
		function can't be implemented
	*/
	return B_ERROR;
}
/*----------------------------------------------------------------------------*/
status_t
sys_get_sem_info(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply) {

	sys_sem_info_t *s_info = NULL;

	acquire_sem_priv(_sem_list_sem);
	s_info = _sem_list;
	while(s_info) {
		if(s_info->info.sem == msg->info.sem) {
			memcpy(&(reply->info), &(s_info->info), sizeof(sem_info));
			release_sem_priv(_sem_list_sem);

			DBG(OUT("Retrieved info about semaphore %d\n", s_info->info.sem));
			return B_OK;
		}

		s_info = s_info->next;
	}

	release_sem_priv(_sem_list_sem);
	return B_BAD_VALUE;
}
/*----------------------------------------------------------------------------*/
status_t
sys_get_next_sem_info(krnl_sem_msg_t *msg, krnl_sem_msg_t *reply) {

	static sys_sem_info_t *s_info = NULL;

	acquire_sem_priv(_sem_list_sem);

	/* if cookie = 0, reset sem pointer to first sem */
	if(!msg->cookie)
		s_info = _sem_list;

	while(s_info) {
		if(s_info->info.team == msg->info.team) {
			memcpy(&(reply->info), &(s_info->info), sizeof(sem_info));
			DBG(OUT("Retrieved info about semaphore %d\n", s_info->info.sem));

			s_info = s_info->next;
			release_sem_priv(_sem_list_sem);
			return B_OK;
		}

		s_info = s_info->next;
	}

	release_sem_priv(_sem_list_sem);
	return B_BAD_VALUE;
}
/*----------------------------------------------------------------------------*/
