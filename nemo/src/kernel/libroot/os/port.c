/*------------------------------------------------------------------------------
/ port.c
/
/ DESCRIPTION:
/	User-side port functions. Communicate with the kernel_server to carry
/	the required operation on a port.
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 31/1/2004
------------------------------------------------------------------------------*/

/* Standard Includes ---------------------------------------------------------*/
#include <string.h>

/* System Includes -----------------------------------------------------------*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
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

/* Macro Defs ----------------------------------------------------------------*/
#define	P_MAX_MSG_SIZE	1024

/*----------------------------------------------------------------------------*/
/* TODO: "capacity" is currently unused */
port_id
create_port(int32 capacity, const char *name)
{
	krnl_msg_t msg;
	
	/* check parameters */
	if(capacity < 0 || strlen(name) >= B_OS_NAME_LENGTH)
		return (port_id)B_BAD_VALUE;
	
	/* send request to kernel_server */
	msg.cmd = P_KRNL_PORT_CREATE;
	msg.port_msg.remote = _libroot_port_id;
	sprintf(msg.port_msg.info.name, name);
	msg.port_msg.info.capacity = capacity;
	msg.port_msg.info.team = getpid();
	
	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);
	
	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.port_msg.error != B_OK)
		return (port_id)(msg.port_msg.error);
	
	/* all fine, return port id */
	return (port_id)(msg.port_msg.info.port);
}
/*----------------------------------------------------------------------------*/
port_id
find_port(const char *name)
{
	krnl_msg_t msg;
	
	/* check parameters */
	if(strlen(name) >= B_OS_NAME_LENGTH)
		return (port_id)B_BAD_VALUE;
	
	/* send request to kernel_server */
	msg.cmd = P_KRNL_PORT_FIND;
	msg.port_msg.remote = _libroot_port_id;
	sprintf(msg.port_msg.info.name, name);
	
	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);
	
	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.port_msg.error != B_OK)
		return (port_id)(msg.port_msg.error);
	
	/* all fine, return port id */
	return (port_id)(msg.port_msg.info.port);
}
/*----------------------------------------------------------------------------*/
status_t
write_port(port_id port, int32 code, const void *buffer, size_t bufferSize)
{
	return write_port_etc(port, code, buffer, bufferSize, 0, 0);
}
/*----------------------------------------------------------------------------*/
/* NOTE: the message code is copied at the beginning of the
 * given "buffer". "buffer" must be a pointer to a struct {long, void*}.
 */
status_t
read_port(port_id port, int32 *code, void *buffer, size_t bufferSize)
{
	return read_port_etc(port, code, buffer, bufferSize, 0, 0);
}
/*----------------------------------------------------------------------------*/
/* NOTE:
 * - in our implementation if "flags" include B_TIMEOUT,
 * 	 "timeout" will always be considered to be zero
 */
status_t
write_port_etc(port_id port, int32 code, const void *buffer, size_t bufferSize,
	uint32 flags, bigtime_t timeout)
{
	int8 temp_buf[P_MAX_MSG_SIZE + 4];
	((int32*)temp_buf)[0] = code;
	memcpy(temp_buf + 4, buffer, bufferSize);

	DBG(if(code <= 0) OUT("write_port_etc(): !!PANIC!! asked to send a message with code <= 0\n"));
	
	if(msgsnd(	(int)port,
				temp_buf,
				bufferSize,
				flags & B_TIMEOUT? IPC_NOWAIT : 0) == -1) {
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
/*----------------------------------------------------------------------------*/
/* NOTE:
 * - in our implementation if "flags" include B_TIMEOUT,
 * 	 "timeout" will always be considered to be zero
 */
status_t
read_port_etc(port_id port, int32 *code, void *buffer, size_t bufferSize,
	uint32 flags, bigtime_t timeout)
{
	ssize_t size;
	int8 temp_buf[P_MAX_MSG_SIZE + 4];
	
	if((size = msgrcv(	(int)port,
						temp_buf,
						P_MAX_MSG_SIZE,
						0,
						flags & B_TIMEOUT? IPC_NOWAIT : 0)) == -1) {
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
	
	/*	The BeBook says that "it's up to the caller to ensure that the message
		buffer is large enough to accommodate the message that's being read",
		so we will do no effort in checking whether bufferSize >= read size
		or not
	*/
	*code = *((int32*)temp_buf);
	memcpy(buffer, temp_buf + 4, size);
	
	return (status_t)size;
}
/*----------------------------------------------------------------------------*/
/* NOTE: 
 * POSIX message queues don't keep the size of each message separately,
 * so our implementation of port_buffer_size() will always returns
 * the same size when invoked on the same port, which is the max message
 * size of that port.
 *
 * Also, unlike the original behavior, this function never blocks,
 * so it shouldn't be used to wait till a message arrives at the port.
 */
ssize_t
port_buffer_size(port_id port)
{
	struct msqid_ds info;

	if(msgctl((int)port, IPC_STAT, &info) == -1) {
		switch(errno) {
			case EACCES:
			case EFAULT:
			case EIDRM:
			case EINVAL:
			case EPERM: return (ssize_t)B_BAD_PORT_ID;
		}
	}
	
	return (ssize_t)(info.msg_qbytes);
}
/*----------------------------------------------------------------------------*/
/* NOTE:
	o	timeout is always considered zero
	o	this function always returns the maximum message size, whether there
		are messages at the port or not...see port_buffer_size()
*/
ssize_t
port_buffer_size_etc(port_id port, uint32 flags, bigtime_t timeout)
{
  	struct msqid_ds info;
   
	if(msgctl((int)port, IPC_STAT, &info) == -1) {
		switch(errno) {
			case EACCES:
			case EFAULT:
			case EIDRM:
			case EINVAL:
			case EPERM: return (ssize_t)B_BAD_PORT_ID;
		}
	}
	
	return (ssize_t)(info.msg_qbytes);
}
/*----------------------------------------------------------------------------*/
ssize_t
port_count(port_id port)
{
  	struct msqid_ds info;
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
/*----------------------------------------------------------------------------*/
status_t
set_port_owner(port_id port, team_id team)
{
	/*	ports are owned by teams in BeOS, while they're owned by
		users in Linux. Due to this "cultural misunderstanding", this
		function can't be implemented
	*/
	return B_ERROR;
}
/*----------------------------------------------------------------------------*/
status_t
close_port(port_id port)
{
  	struct msqid_ds info;
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
/*----------------------------------------------------------------------------*/
status_t
delete_port(port_id port)
{
	krnl_msg_t msg;

	/* send request to kernel_server */
	msg.cmd = P_KRNL_PORT_DELETE;
	msg.port_msg.remote = _libroot_port_id;
	msg.port_msg.info.port = port;
	
	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);
	
	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.port_msg.error != B_OK)
		return msg.port_msg.error;
	
	/* all fine */
	return B_OK;
}
/*----------------------------------------------------------------------------*/
status_t
get_port_info(port_id port, port_info *info)
{
	krnl_msg_t msg;
	
	/* check parameters */
	if(info == NULL)
		return B_BAD_VALUE;

	/* send request to kernel_server */
	msg.cmd = P_KRNL_PORT_INFO;
	msg.port_msg.remote = _libroot_port_id;
	msg.port_msg.info.port = port;
	
	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);
	
	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.port_msg.error != B_OK)
		return msg.port_msg.error;
	
	/* all fine, return port info */
	memcpy(info, &(msg.port_msg.info), sizeof(port_info));
	return B_OK;
}
/*----------------------------------------------------------------------------*/
status_t
get_next_port_info(team_id team, int32 *cookie, port_info *info)
{
	krnl_msg_t msg;
	
	/* check parameters */
	if(cookie == NULL || info == NULL)
		return B_BAD_VALUE;

	/* send request to kernel_server */
	msg.cmd = P_KRNL_PORT_NEXT_INFO;
	msg.port_msg.remote = _libroot_port_id;
	msg.port_msg.info.team = getpid();
	msg.port_msg.cookie = *cookie;
	
	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);
	
	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.port_msg.error != B_OK)
		return msg.port_msg.error;
	
	/* all fine, return port info */
	*cookie = 1;
	memcpy(info, &(msg.port_msg.info), sizeof(port_info));
	return B_OK;
}
/*----------------------------------------------------------------------------*/
