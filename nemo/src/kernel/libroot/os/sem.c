/*------------------------------------------------------------------------------
/ sem.c
/
/ DESCRIPTION:
/	User-side semaphore functions. Communicate with the kernel_server to carry
/	the required operation on a semaphore.
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 2/2/2004
------------------------------------------------------------------------------*/

/* Standard Includes ---------------------------------------------------------*/
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
#if DEBUG_LIBROOT
	#define OUT(x...)	fprintf(LIBROOT_LOG, "libroot: "x);
	#define DBG(x)		x;
#else
	#define DBG(x)		;
#endif

/* Globals -------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
sem_id
create_sem(int32 count, const char *name)
{
	krnl_msg_t msg;
	
	/* check parameters */
	if(count < 0 || strlen(name) >= B_OS_NAME_LENGTH)
		return (sem_id)B_BAD_VALUE;
	
	/* send request to kernel_server */
	msg.cmd = P_KRNL_SEM_CREATE;
	msg.sem_msg.remote = _libroot_port_id;
	sprintf(msg.sem_msg.info.name, name);
	msg.sem_msg.info.count = count;
	msg.sem_msg.info.team = getpid();
	/* TODO: sem_info.latest_holder isn't used */
	
	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);

	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.sem_msg.error != B_OK)
		return (sem_id)(msg.sem_msg.error);
	
	/* all fine, return sem id */
	return (sem_id)(msg.sem_msg.info.sem);
}
/*----------------------------------------------------------------------------*/
status_t
delete_sem(sem_id id)
{
	krnl_msg_t msg;

	/* send request to kernel_server */
	msg.cmd = P_KRNL_SEM_DELETE;
	msg.sem_msg.remote = _libroot_port_id;
	msg.sem_msg.info.sem = id;
	
	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);
	
	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.sem_msg.error != B_OK)
		return msg.sem_msg.error;
	
	/* all fine */
	return B_OK;
}
/*----------------------------------------------------------------------------*/
status_t
acquire_sem(sem_id id)
{
	return acquire_sem_etc(id, 1, 0, 0);
}
/*----------------------------------------------------------------------------*/
/* TODO: the BeBook doesn't specify the behavior when "timeout" < 0
 */
status_t
acquire_sem_etc(sem_id id, int32 count, uint32 flags, bigtime_t timeout)
{
	/* NOTE: "timeout" is always assumed to be zero, if used
	 */
	struct sembuf sb = {0, -count,
		flags & (B_RELATIVE_TIMEOUT | B_ABSOLUTE_TIMEOUT)? IPC_NOWAIT : 0};
	
	if(count < 1) return B_BAD_VALUE;
	
	if(semop(id, &sb, 1) == -1) {
		switch(errno) {
			case EACCES:
			case EIDRM:
			case EINVAL: return B_BAD_SEM_ID;
			case EAGAIN: return (timeout > 0? B_TIMED_OUT : B_WOULD_BLOCK);
			case EINTR:	return B_INTERRUPTED;
			case ERANGE: return B_BAD_VALUE;
		}
	}
	
	return B_NO_ERROR;
}
/*----------------------------------------------------------------------------*/
status_t
release_sem(sem_id id)
{
	return release_sem_etc(id, 1, 0);
}
/*----------------------------------------------------------------------------*/
/* TODO: the BeBook doesn't specify the behavior when "timeout" < 0
 */
status_t
release_sem_etc(sem_id id, int32 count, uint32 flags)
{
	/* NOTE:
	 * - the only possible value of "flags", B_DO_NOT_RESCHEDULE, 
	 *	 is not supported
	 * - the original release_sem_etc() returns B_BAD_VALUE on "count" < 0
	 *   but the BeBook says nothing about the case of acquire(n) then 
	 *   release(n + m), which is most probably supported. On the other
	 *   hand, this would cause semop() to block under Linux, but since
	 *   release_sem*() should never block, we will have to prevent this
	 *   case by asserting IPC_NO_WAIT   
	 */
	struct sembuf sb = {0, count, IPC_NOWAIT};
	
	if(count < 1) return B_BAD_VALUE;
	
	if(semop(id, &sb, 1) == -1) {
		switch(errno) {
			case EACCES:
			case EIDRM:
			case EINVAL: return B_BAD_SEM_ID;
			case EINTR:	return B_INTERRUPTED;
			case EAGAIN:			
			case ERANGE: return B_BAD_VALUE;
		}
	}
	
	return B_NO_ERROR;
}
/*----------------------------------------------------------------------------*/
status_t
get_sem_count(sem_id sem, int32 *count)
{
	int val = 0;
	if((val = semctl(sem, 0, GETVAL)) == -1) {
		switch(errno) {
			case EACCES:
			case EIDRM:
			case EINVAL: return B_BAD_SEM_ID;		
		}
	}
	*count = (int32)val;
	
	return B_NO_ERROR;
}
/*----------------------------------------------------------------------------*/
status_t
set_sem_owner(sem_id sem, team_id team)
{
	/*	semaphores are owned by teams in BeOS, while they're owned by
		users in Linux. Due to this "cultural misunderstanding", this
		function can't be implemented
	*/
	return B_ERROR;
}
/*----------------------------------------------------------------------------*/
status_t
get_sem_info(sem_id sem, sem_info *info)
{
	krnl_msg_t msg;
	
	/* check parameters */
	if(info == NULL)
		return B_BAD_VALUE;

	/* send request to kernel_server */
	msg.cmd = P_KRNL_SEM_INFO;
	msg.sem_msg.remote = _libroot_port_id;
	msg.sem_msg.info.sem = sem;
	
	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);
	
	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.sem_msg.error != B_OK)
		return msg.sem_msg.error;
	
	/* all fine, return sem info */
	memcpy(info, &(msg.sem_msg.info), sizeof(sem_info));
	return B_OK;	
}
/*----------------------------------------------------------------------------*/
status_t
get_next_sem_info(team_id team, int32 *cookie, sem_info *info)
{
	krnl_msg_t msg;
	
	/* check parameters */
	if(cookie == NULL || info == NULL)
		return B_BAD_VALUE;

	/* send request to kernel_server */
	msg.cmd = P_KRNL_SEM_NEXT_INFO;
	msg.sem_msg.remote = _libroot_port_id;
	msg.sem_msg.info.team = getpid();
	msg.sem_msg.cookie = *cookie;
	
	msgsnd(_kernel_port_id, &msg, sizeof(msg), 0);
	
	/* wait for reply from kernel_server */
	msgrcv(_libroot_port_id, &msg, sizeof(msg), 0, 0);
	if(msg.sem_msg.error != B_OK)
		return msg.sem_msg.error;
	
	/* all fine, return sem info */
	*cookie = 1;
	memcpy(info, &(msg.sem_msg.info), sizeof(sem_info));
	return B_OK;
}
/*----------------------------------------------------------------------------*/
