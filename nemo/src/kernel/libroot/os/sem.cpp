/**
 * FILE NAME:	sem.c
 * DESCRIPTION:	provides functions for semaphore manipulation
 * AUTHOR:		Mahmoud Al Gammal
 * DATE:		Dec 24th 2003
 */

#include <OS.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>

#ifdef DEBUG
	#include "nemo_debug.h"
#endif

/*===========================================================================*/

sem_id
create_sem(int32 count, const char *name)
{
	int id = 0;
	sembuf sb = {0, count, 0};
	
	if(count < 0) return (sem_id)B_BAD_VALUE;
	
	/* create the semaphore	*/
	if((id = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666)) == -1) {
		switch(errno) {
			case EACCES:
			case EEXIST:
			case ENOENT:
			case EINVAL: return (sem_id)B_BAD_VALUE;
			case ENOMEM: return (sem_id)B_NO_MEMORY;
			case ENOSPC: return (sem_id)B_NO_MORE_SEMS;
		}
	}
	
	if(semop(id, &sb, 1) == -1) {
		#if DEBUG_KERNEL
			fprintf(KERNEL_LOG, "error setting semaphore properties\n");
			return (sem_id)B_ERROR;
		#endif
	}
	
	return (sem_id)id;
}

/*===========================================================================*/

status_t
delete_sem(sem_id id)
{
	if(semctl((int)id, 0, IPC_RMID) == -1) {
		switch(errno) {
			case EACCES:
			case EPERM:			
			case EINVAL:
			case EIDRM: return B_BAD_SEM_ID;
		}
	}
	
	return B_NO_ERROR;
}

/*===========================================================================*/

status_t
acquire_sem(sem_id id)
{
	return acquire_sem_etc(id, 1, 0, 0);
}

/*===========================================================================*/

/* TODO: the BeBook doesn't specify the behavior when "timeout" < 0
 */
status_t
acquire_sem_etc(sem_id id, int32 count, uint32 flags, bigtime_t timeout)
{
	/* NOTE: "timeout" is always assumed to be zero, if used
	 */
	sembuf sb = {0, -count,
		flags & (B_RELATIVE_TIMEOUT | B_ABSOLUTE_TIMEOUT)? IPC_NOWAIT : 0};
	
	if(count < 1) return B_BAD_VALUE;
	
	if(semop(id, &sb, 1) == -1) {
		switch(errno) {
			case EACCES:
			case EIDRM:
			case EINVAL: return B_BAD_SEM_ID;
			case EAGAIN:
				if(timeout > 0) return B_TIMED_OUT;
				else return B_WOULD_BLOCK;
			case EINTR:	return B_INTERRUPTED;
			case ERANGE: return B_BAD_VALUE;
		}
	}
	
	return B_NO_ERROR;
}

/*===========================================================================*/

status_t
release_sem(sem_id id)
{
	return release_sem_etc(id, 1, 0);
}

/*===========================================================================*/

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
	sembuf sb = {0, count, IPC_NOWAIT};
	
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

/*===========================================================================*/

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
	
	*count = int32(-val);
	return B_NO_ERROR;
}

/*===========================================================================*/

//status_t
//set_sem_owner(sem_id sem, team_id team)
//{
//	return sys_set_sem_owner(sem, team);
//}

/*===========================================================================*/

//status_t
//_get_sem_info(sem_id sem, sem_info *info, size_t size)
//{
//	return sys_get_sem_info(sem, info, size);
//}

/*===========================================================================*/

//status_t
//_get_next_sem_info(team_id team, int32 *cookie, sem_info *info, size_t size)
//{
//	return sys_get_next_sem_info(team, cookie, info, size);
//}

/*===========================================================================*/

