/*------------------------------------------------------------------------------
/ port.c
/
/ DESCRIPTION:
/	Port functions of the kernel_server
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 31/1/2004
/-----------------------------------------------------------------------------*/

/* Standard Includes ---------------------------------------------------------*/
#include <malloc.h>
#include <string.h>

/* System Includes -----------------------------------------------------------*/
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

/*----------------------------------------------------------------------------*/
void
handle_port_msg(krnl_msg_t *msg) {

	krnl_port_msg_t reply;
	status_t error;

	/* decode function */
	switch(msg->cmd) {
		case P_KRNL_PORT_CREATE:
			error = sys_create_port(&(msg->port_msg), &reply);
			break;

		case P_KRNL_PORT_FIND:
			error = sys_find_port(&(msg->port_msg), &reply);
			break;

		case P_KRNL_PORT_CLOSE:
			error = sys_close_port(&(msg->port_msg), &reply);
			break;

		case P_KRNL_PORT_DELETE:
			error = sys_delete_port(&(msg->port_msg), &reply);
			break;

		case P_KRNL_PORT_OWNER:
			error = sys_set_port_owner(&(msg->port_msg), &reply);
			break;

		case P_KRNL_PORT_INFO:
			error = sys_get_port_info(&(msg->port_msg), &reply);
			break;

		case P_KRNL_PORT_NEXT_INFO:
			error = sys_get_next_port_info(&(msg->port_msg), &reply);
			break;

		default:
			error = B_BAD_VALUE;
	}

	/* reply to requester */
	reply.cmd = P_KRNL_REPLY;
	reply.error = error;
	if(error != B_OK)
		DBG(OUT("Error performing port request\n"));

	if(msgsnd(msg->port_msg.remote, &reply, sizeof(reply), 0) == -1)
		DBG(OUT("Error while replying to remote requester\n"));
}
/*----------------------------------------------------------------------------*/
status_t
sys_create_port(krnl_port_msg_t *msg, krnl_port_msg_t *reply) {

	sys_team_info_t *team = NULL;
	sys_port_info_t *p_info = NULL;
	struct msqid_ds mq_info;
	int id = -1;

	/* check parameters */
	if(strlen(msg->info.name) >= B_OS_NAME_LENGTH || msg->info.capacity < 1)
		return B_BAD_VALUE;

	/* does the given team id exist? */
	acquire_sem_priv(_team_list_sem);
	team = _team_list;
	while(team) {
		if(team->info.team == msg->info.team) {
			/* team exists...proceed */
			p_info = (sys_port_info_t*) malloc(sizeof(sys_port_info_t));

			if(p_info == NULL) {
				release_sem_priv(_team_list_sem);
				return B_NO_MORE_PORTS;
			}

			/* create the message queue	*/
			if((id = msgget(IPC_PRIVATE, IPC_CREAT|0666)) == -1) {
				release_sem_priv(_team_list_sem);
				free(p_info);
				switch(errno) {
					case EACCES:
					case EEXIST:
					case EIDRM:
					case ENOENT: return B_BAD_VALUE;
					case ENOMEM:
					case ENOSPC: return B_NO_MORE_PORTS;
				}
			}

			/* TODO: set the message queue's capacity */

			/* append new port to global port list */
			memcpy(&(p_info->info), &(msg->info), sizeof(port_info));
			p_info->info.port = reply->info.port = (port_id)id;

			acquire_sem_priv(_port_list_sem);
			p_info->next = _port_list;
			_port_list = p_info;

			release_sem_priv(_team_list_sem);
			release_sem_priv(_port_list_sem);

			DBG(OUT("Created port %d (%s)\n", p_info->info.port, p_info->info.name));
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
sys_find_port(krnl_port_msg_t *msg, krnl_port_msg_t *reply) {

	sys_port_info_t *p_info = NULL;

	acquire_sem_priv(_port_list_sem);
	p_info = _port_list;
	while(p_info) {
		if(!strcmp(msg->info.name, p_info->info.name)) {
			reply->info.port = p_info->info.port;
        	release_sem_priv(_port_list_sem);

			DBG(OUT("Found port %d (%s)\n", reply->info.port, p_info->info.name));
			return B_OK;
		}

		p_info = p_info->next;
	}

    release_sem_priv(_port_list_sem);
	return B_NAME_NOT_FOUND;
}
/*----------------------------------------------------------------------------*/
status_t
sys_close_port(krnl_port_msg_t *msg, krnl_port_msg_t *reply) {

	sys_port_info_t *p_info = NULL;
	struct msqid_ds mq_info;

    acquire_sem_priv(_port_list_sem);
	p_info = _port_list;
	while(p_info) {
		if(p_info->info.port == msg->info.port) {
			mq_info.msg_perm.mode = 0444;	/* read only */

			if(msgctl((int)msg->info.port, IPC_SET, &mq_info) == -1) {
				release_sem_priv(_port_list_sem);
				switch(errno) {
					case EACCES:
					case EFAULT:
					case EIDRM:
					case EINVAL:
					case EPERM: return B_BAD_PORT_ID;
				}
			}
			release_sem_priv(_port_list_sem);

			DBG(OUT("Closed port %d (%s)\n", p_info->info.port, p_info->info.name));
			return B_OK;
		}
		p_info = p_info->next;
	}

    release_sem_priv(_port_list_sem);
	return B_BAD_PORT_ID;
}
/*----------------------------------------------------------------------------*/
status_t
sys_delete_port(krnl_port_msg_t *msg, krnl_port_msg_t *reply) {

	sys_port_info_t *curr_port = NULL;
	sys_port_info_t **prev_port = NULL;

	acquire_sem_priv(_port_list_sem);
	curr_port = _port_list;
	prev_port = &_port_list;
	while(curr_port) {
		if(curr_port->info.port == msg->info.port) {
			/* port found...remove it from list */
			*prev_port = curr_port->next;
			release_sem_priv(_port_list_sem);
			free(curr_port);

			if(msgctl((int)msg->info.port, IPC_RMID, NULL) == -1) {
				switch(errno) {
					case EACCES:
					case EFAULT:
					case EIDRM:
					case EINVAL:
					case EPERM: return B_BAD_PORT_ID;
				}
			}

			DBG(OUT("Deleted port %d\n", msg->info.port));
			return B_OK;
		}
		prev_port = &(curr_port->next);
		curr_port = curr_port->next;
	}

	release_sem_priv(_port_list_sem);
	return B_BAD_PORT_ID;
}
/*----------------------------------------------------------------------------*/
status_t
sys_set_port_owner(krnl_port_msg_t *msg, krnl_port_msg_t *reply) {

	/*	ports are owned by teams in BeOS, while they're owned by
		users in Linux. Due to this "cultural misunderstanding", this
		function can't be implemented
	*/
	return B_ERROR;
}
/*----------------------------------------------------------------------------*/
status_t
sys_get_port_info(krnl_port_msg_t *msg, krnl_port_msg_t *reply) {

	sys_port_info_t *p_info = NULL;

	acquire_sem_priv(_port_list_sem);
	p_info = _port_list;
	while(p_info) {
		if(p_info->info.port == msg->info.port) {
			memcpy(&(reply->info), &(p_info->info), sizeof(port_info));
			release_sem_priv(_port_list_sem);

			DBG(OUT("Retrieved info about port %d (%s)\n", p_info->info.port, p_info->info.name));
			return B_OK;
		}

		p_info = p_info->next;
	}

	release_sem_priv(_port_list_sem);
	return B_BAD_VALUE;
}
/*----------------------------------------------------------------------------*/
status_t
sys_get_next_port_info(krnl_port_msg_t *msg, krnl_port_msg_t *reply) {

	static sys_port_info_t *p_info = NULL;

	acquire_sem_priv(_port_list_sem);

	/* if cookie = 0, reset port pointer to first port */
	if(!msg->cookie)
		p_info = _port_list;

	while(p_info) {
		if(p_info->info.team == msg->info.team) {
			memcpy(&(reply->info), &(p_info->info), sizeof(port_info));
			DBG(OUT("Retrieved info about port %d (%s)\n", p_info->info.port, p_info->info.name));

			p_info = p_info->next;
			release_sem_priv(_port_list_sem);
			return B_OK;
		}

		p_info = p_info->next;
	}

	release_sem_priv(_port_list_sem);
	return B_BAD_VALUE;
}
/*----------------------------------------------------------------------------*/
