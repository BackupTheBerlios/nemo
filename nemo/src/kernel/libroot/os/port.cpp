/**
 * FILE NAME:	port.c
 * DESCRIPTION:	the functions implemented here provide access to the messaging
 * 				capabilities of the kernel
 * AUTHOR:		Mahmoud Al Gammal
 * DATE:		Dec 21st 2003
 */


#include <OS.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#ifdef DEBUG
	#include "nemo_debug.h"
#endif

/*===========================================================================*/

typedef struct {
	long		code;
	const void*	buffer;
} port_msg;

/*===========================================================================*/

/* TODO: "capacity" is currently unused */
port_id
create_port(int32 capacity, const char *name)
{
	/* TODO: change name<-->key mapping method 
	 */
	key_t k = 0;
	int i = 0;
	
	if(strlen(name) != 4 || capacity < 0)
		return B_BAD_VALUE;

	if(!strcmp(name, "unnp"))
		k = IPC_PRIVATE;
	else {
		/* create a key from the given name */
		for(i = 0; i < 4; i++)
			k |= (name[i] << (24 - i * 8));
	}
	
	/* create the message queue	*/
	if((i = msgget(k, IPC_CREAT|IPC_EXCL|0666)) == -1) {
		switch(errno) {
			case EACCES:
			case EEXIST:
			case EIDRM:
			case ENOENT: return B_BAD_VALUE; 
			case ENOMEM:
			case ENOSPC: return B_NO_MORE_PORTS;
		}
	}
	
	return (port_id)i;
}

/*===========================================================================*/

port_id
find_port(const char *name)
{
	/* TODO: change name<-->key mapping method 
	 */
	key_t k = 0;
	int i = 0;
	
	if(strlen(name) != 4) return B_NAME_NOT_FOUND;

	/* create a key from the given name */
	for(i = 0; i < 4; i++)
		k |= (name[i] << (24 - i * 8));
	
	if((i = msgget(k, 0666))== -1)
		return B_NAME_NOT_FOUND;
		
	return (port_id)i;
}

/*===========================================================================*/

status_t
write_port(port_id port, int32 code, const void *buffer, size_t bufferSize)
{
	return write_port_etc(port, code, buffer, bufferSize, 0, 0);
}

/*===========================================================================*/

/* NOTE: the message code is copied at the beginning of the
 * given "buffer". "buffer" must be a pointer to a struct {long, void*}.
 */
 
status_t
read_port(port_id port, int32 *code, void *buffer, size_t bufferSize)
{
	return read_port_etc(port, code, buffer, bufferSize, 0, 0);
}

/*===========================================================================*/

/* NOTE:
 * - in our implementation if "flags" include B_TIMEOUT,
 * 	 "timeout" will always be considered to be zero
 * 
 * - the first 4 bytes of "buffer" are assumed to contain the value
 * 	 of "code". "buffer" must be a pointer to a struct {long, void*}.
 */

status_t
write_port_etc(port_id port, int32 code, const void *buffer, size_t bufferSize,
	uint32 flags, bigtime_t timeout)
{
	
	if(msgsnd((int)port, buffer, bufferSize, flags & B_TIMEOUT? IPC_NOWAIT : 0) == -1) {
		switch(errno) {
			case EAGAIN: return B_WOULD_BLOCK;
			case EFAULT:
			case ENOMEM:			
			case EACCES:
			case EINTR: return B_TIMED_OUT;
			case EIDRM:
			case EINVAL: return B_BAD_PORT_ID;			
		}
	}
	
	return B_OK;
}

/*===========================================================================*/

/* NOTE:
 * - in our implementation if "flags" include B_TIMEOUT,
 * 	 "timeout" will always be considered to be zero
 * 
 * - the message code is copied to the beginning of the given "buffer".
 * 	 "buffer" must be a pointer to a struct {long, void*}.
 */
 
status_t
read_port_etc(port_id port, int32 *code, void *buffer, size_t bufferSize,
	uint32 flags, bigtime_t timeout)
{
	ssize_t size;
	
	if((size = msgrcv((int)port, buffer, bufferSize, 0, flags & B_TIMEOUT? IPC_NOWAIT : 0)) == -1) {
		switch(errno) {
			case E2BIG:
			case EACCES:
			case EFAULT:
			case EINTR: return B_TIMED_OUT;
			case ENOMSG: return B_WOULD_BLOCK;
			case EIDRM:
			case EINVAL: return B_BAD_PORT_ID;
		}
	}
	
	return size;
}

/*===========================================================================*/

/* NOTE: POSIX message queues don't keep the size of each message
 * separately, so our implementation of port_buffer_size() will always
 * return the same size when invoked on the same port, which is the max
 * message size of that port
 */
ssize_t
port_buffer_size(port_id port)
{
  	msqid_ds info;
	if(msgctl((int)port, IPC_STAT, &info) == -1) {
		switch(errno) {
			case EACCES:
			case EFAULT:
			case EIDRM:
			case EINVAL:
			case EPERM: return B_BAD_PORT_ID;
		}
	}
	
	return (ssize_t)(info.msg_qbytes);
}

/*===========================================================================*/

//ssize_t
//port_buffer_size_etc(port_id port, uint32 flags, bigtime_t timeout)
//{
//	return sys_port_buffer_size_etc(port, flags, timeout);
//}
//
/*===========================================================================*/

ssize_t
port_count(port_id port)
{
  	msqid_ds info;
	if(msgctl((int)port, IPC_STAT, &info) == -1) {
		switch(errno) {
			case EACCES:
			case EFAULT:
			case EIDRM:
			case EINVAL:
			case EPERM: return B_BAD_PORT_ID;
		}
	}
	
	return (ssize_t)(info.msg_qnum);
}

/*===========================================================================*/

//status_t
//set_port_owner(port_id port, team_id team)
//{
//	return sys_port_set_owner(port, team);
//}
//
/*===========================================================================*/

status_t
close_port(port_id port)
{
  	msqid_ds info;
	info.msg_perm.mode = 0444;	/* read only */

	if(msgctl((int)port, IPC_SET, &info) == -1) {
		switch(errno) {
			case EACCES:
			case EFAULT:
			case EIDRM:
			case EINVAL:
			case EPERM: return B_BAD_PORT_ID;
		}		
	}
	
	return B_OK;
}

/*===========================================================================*/

status_t
delete_port(port_id port)
{
	if(msgctl((int)port, IPC_RMID, NULL) == -1) {
		switch(errno) {
			case EACCES:
			case EFAULT:
			case EIDRM:
			case EINVAL:
			case EPERM: return B_BAD_PORT_ID;
		}		
	}
	
	return B_OK;
}

/*===========================================================================*/

//status_t
//_get_next_port_info(team_id team, int32 *cookie, port_info *info, size_t size)
//{
//	// size is not yet used, but may, if port_info changes
//	(void)size;
//
//	return sys_port_get_next_port_info(team, cookie, info);
//}
//
/*===========================================================================*/

//status_t
//_get_port_info(port_id port, port_info *info, size_t size)
//{
//	return sys_port_get_info(port, info);
//}
//
/*===========================================================================*/
