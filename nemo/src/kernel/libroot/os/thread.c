/*------------------------------------------------------------------------------
/ thread.c
/
/ DESCRIPTION:
/	the functions implemented here provide access to the threading
/	capabilities of the kernel
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 4/2/2004 
------------------------------------------------------------------------------*/

/* Standard Includes ---------------------------------------------------------*/
//#include <errno.h>
#include <malloc.h>
#include <string.h>

/* System Includes -----------------------------------------------------------*/
#include <pthread.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>

#include <OS.h>

/* Private Includes ----------------------------------------------------------*/
#include "libroot.h"

/* Debugging -----------------------------------------------------------------*/
#include "nemo_debug.h"
#if DEBUG_LIBROOT
	#define OUT(x...)	fprintf(LIBROOT_LOG, "libroot: "x);
	#define DBG(x)		x;
#else
	#define DBG(x)		;
#endif

/* Types ---------------------------------------------------------------------*/
typedef	void*(*pthread_func)(void*);

typedef struct thread_func_arg_t {
	sys_thread_info_t	*th_info;
	thread_func			func;
	void				*arg;
} thread_func_arg_t;

/* Globals -------------------------------------------------------------------*/
sys_thread_info_t	*_thread_list		= NULL;
sem_id				_thread_list_sem	= -1;

/*----------------------------------------------------------------------------*/
void* start_routine(void *func_arg_struct) {

	thread_func_arg_t *func_arg = (thread_func_arg_t*)func_arg_struct;
	thread_func func = func_arg->func;
	void *arg = func_arg->arg;
	sys_thread_info_t *th_info = func_arg->th_info;
	struct sembuf sb = {0, -1, 0};
	int32 error;
	
	/* change the cancellation type of the thread to make it
	 * respond immediately to cancellation requests
	 */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	/* suspend this thread */
	semop(th_info->sem, &sb, 1);

	/* thread resumed, semaphore no longer needed */
	semctl(th_info->sem, 0, IPC_RMID);
	th_info->sem = -1;

	/* run the actual start routine once resumed */
	error = func(arg);
	exit_thread(error);
}
/*----------------------------------------------------------------------------*/
/* given an entry in the global thread list, this function frees the
 * entry and performs all necessary cleanup
 */
void
delete_thread(sys_thread_info_t *th_info) {

	void *rv = NULL;
	
	/* if the thread was never resumed or joined then we have to
	 * remove the associated semaphore manually
	 */
	if(th_info->sem != -1)
		semctl(th_info->sem, 0, IPC_RMID);

	/* join the thread to free its resources */
	pthread_join((pthread_t)(th_info->info.thread), &rv);
	
	free(th_info);
}
/*----------------------------------------------------------------------------*/
thread_id
spawn_thread(thread_func entry, const char *name, int32 priority, void *data)
{
	pthread_t id;
	thread_func_arg_t func_arg = {NULL, entry, data};
	
	sys_thread_info_t *th_info =
		(sys_thread_info_t*) malloc(sizeof(sys_thread_info_t));
	
	if(!th_info)
		return B_NO_MORE_THREADS;
	
	/* NOTE: some of the info in the thread_info struct are not set */
	th_info->info.team = (team_id)getpid();
	sprintf(th_info->info.name, "%s", name? name : "no name");

	/* TODO: in order to support the behaviour described in the BeBook,
	 * this semaphore is created here to stop the thread just after it's
	 * created. It won't be needed once we figure out how to suspend threads.
	 */
	th_info->sem = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
	semctl(th_info->sem, 0, SETVAL, 0);
	func_arg.th_info = th_info;
	
	/* add the new thread to the thread list */
	if(acquire_sem(_thread_list_sem) == B_OK ) {	
		/* TODO: use a customized pthread_attr to set the thread properties
		 */
		if(pthread_create(&id, NULL, start_routine, &func_arg) != 0) {
			free(th_info);
			/* TODO: the man page of pthread_create() states that the function
			 * returns EAGAIN if no more thread ids are available, and EAGAIN (!!)
			 * if there's not enough resources for the new thread.
			 * Be's implementation distinguishes between the two cases by returning
			 * B_NO_MORE_THREADS or B_NO_MEMORY, but since the pthread
			 * implementation doesn't make this distinction, we will always 
			 * return B_NO_MORE_THREADS */
			release_sem(_thread_list_sem);
			return B_NO_MORE_THREADS;
		}
		
		/* add the new thread to the thread list */
		th_info->info.thread = (thread_id)id;
		th_info->next = _thread_list;
		_thread_list = th_info;
		
		release_sem(_thread_list_sem);
	}

	DBG(OUT("Created thread %d (%s)\n", id, name? name : "no name"));
	return (thread_id)id;
}
/*----------------------------------------------------------------------------*/
status_t
kill_thread(thread_id thread)
{
	sys_thread_info_t *curr_th = NULL;
	sys_thread_info_t **prev_th = NULL;
	int status;
	
	if(acquire_sem(_thread_list_sem) == B_OK) {
		curr_th = _thread_list;
		prev_th = &_thread_list;
		
		while(curr_th) {
			if(curr_th->info.thread == thread) {
				/* thread found, remove it... */
				*prev_th = curr_th->next;
				status = pthread_cancel((pthread_t)thread);
				DBG(OUT("Killed thread %d (%s)\n", thread, curr_th->info.name));
				delete_thread(curr_th);
				release_sem(_thread_list_sem);
				if(status != 0) {
					switch(errno) {
						case EINVAL:
						case ESRCH: return B_BAD_THREAD_ID;
					}
				}
				return B_OK;
			}
			
			prev_th = &(curr_th->next);
			curr_th = curr_th->next;
		}
		
		release_sem(_thread_list_sem);
	}
	
	return B_BAD_THREAD_ID;
}
/*----------------------------------------------------------------------------*/
/* TODO: currently, resume_thread() is assumed to be called just once after
 * thread creation
 */
status_t
resume_thread(thread_id thread)
{
	struct sembuf sb = {0, 1, 0};
	sys_thread_info_t *th_info = NULL;

	if(acquire_sem(_thread_list_sem) == B_OK) {
		th_info = _thread_list;
    	while(th_info) {
    		if(th_info->info.thread == thread) {
    			if(th_info->sem == -1) {
       				release_sem(_thread_list_sem);
    				return B_BAD_THREAD_STATE;
        		}
    			semop(th_info->sem, &sb, 1);
    			DBG(OUT("Resumed thread %d (%s)\n", thread, th_info->info.name));
				release_sem(_thread_list_sem);
    			return B_OK;
    		}
    		th_info = th_info->next;
    	}
     	release_sem(_thread_list_sem);
     }

	return B_BAD_THREAD_ID;
}
/*----------------------------------------------------------------------------*/
/*	TODO: suspending/resuming threads...oh my
*/
status_t
suspend_thread(thread_id thread)
{
	/*if(pthread_kill((pthread_t)thread, SIGSTOP) != 0) {
		switch(errno) {
			case EINVAL:
			case ESRCH: return B_BAD_THREAD_ID;
		}
	}

	DBG(OUT("Suspended thread %d\n", thread));
	return B_OK;*/

	DBG(OUT("PANIC!! suspend_thread() isn't implemented yet\n"));
	return B_ERROR;
}
/*----------------------------------------------------------------------------*/
thread_id
find_thread(const char *name)
{
	sys_thread_info_t *th = NULL;
	thread_id id;
	
	if(!name) {
		return (thread_id)pthread_self();
	}
	
	if(acquire_sem(_thread_list_sem) == B_OK) {
		th = _thread_list;
		
		while(th) {
			if(!strcmp(name, th->info.name)) {
				/* thread found, return its id */
				id = th->info.thread;
				release_sem(_thread_list_sem);
				DBG(OUT("Found thread %d (%s)\n", id, name));
				return id;
			}
			th = th->next;
		}
		
		release_sem(_thread_list_sem);
	}
	
	return B_NAME_NOT_FOUND;
}
/*----------------------------------------------------------------------------*/
status_t
rename_thread(thread_id thread, const char *name)
{
	sys_thread_info_t *th = NULL;
	
	if(acquire_sem(_thread_list_sem) == B_OK) {
		th = _thread_list;
		
		while(th) {
			if(th->info.thread == thread) {
				DBG(OUT("Thread %d (%s) renamed to (%s)\n", thread, th->info.name, name));
				sprintf(th->info.name, "%s", name);
				release_sem(_thread_list_sem);
				return B_OK;
			}
			th = th->next;
		}
		
		release_sem(_thread_list_sem);
	}
	
	return B_BAD_THREAD_ID;
}
/*----------------------------------------------------------------------------*/
status_t
set_thread_priority(thread_id thread, int32 priority)
{
	/* TODO: set_thread_priority not implemented */
	DBG(OUT("set_thread_priority() is not implemented\n"));
	return B_ERROR;
}
/*----------------------------------------------------------------------------*/
void
exit_thread(status_t status)
{
	sys_thread_info_t *curr_th = NULL;
	sys_thread_info_t **prev_th = NULL;
	int id = pthread_self();

	if(acquire_sem(_thread_list_sem) == B_OK) {
		curr_th = _thread_list;
		prev_th = &_thread_list;

		while(curr_th) {
			if(curr_th->info.thread == (thread_id)id) {
				*prev_th = curr_th->next;
				DBG(OUT("Thread %d (%s) exiting with error code %d\n", id, curr_th->info.name, status));
				free(curr_th);
				release_sem(_thread_list_sem);
				break;
			}

			prev_th = &(curr_th->next);
			curr_th = curr_th->next;
		}
		
		release_sem(_thread_list_sem);
	}
	
	pthread_exit((void*)status);
}
/*----------------------------------------------------------------------------*/
status_t
wait_for_thread(thread_id thread, status_t *thread_return_value)
{
	void *rv;
	struct sembuf sb = {0, 1, 0};
	
	sys_thread_info_t	*curr_th = NULL;
	sys_thread_info_t	**prev_th = NULL;
	
	if(acquire_sem(_thread_list_sem) == B_OK) {
		curr_th = _thread_list;
		prev_th = &_thread_list;
		
		while(curr_th) {
			if(curr_th->info.thread == thread) {
				/* resume thread if it's not running yet */
				if(curr_th->sem != -1)
					semop(curr_th->sem, &sb, 1);
				
				/* remove thread from thread list as it's soon going to die */
				*prev_th = curr_th->next;
				free(curr_th);
				release_sem(_thread_list_sem);
				
				if(pthread_join((pthread_t)thread, &rv) != 0) {
					/* due to the incompatibility between the meanings of the errors
					 * returned by pthread_join() & wait_for_thread() the return values
					 * of our wait_for_thread() will be limited to:
					 * 		B_OK on success
					 * 		B_BAD_THREAD_ID if the given thread id doesn't exist
					 * 		B_ERROR on all other errors (like a thread waiting for itself,
					 * 			or if the given thread is detached or is being joined by
					 * 			another thread)
					 */
					switch(errno) {
						case ESRCH: return B_BAD_THREAD_ID;
						case EDEADLK:
						case EINVAL:
							return B_ERROR;
					}
				}
				*thread_return_value = (status_t)rv;
				DBG(OUT("Thread %d joined. Return value is %d\n", thread, rv));
				return B_OK;
			}
			
			prev_th = &(curr_th->next);
			curr_th = curr_th->next;
		}
		
		release_sem(_thread_list_sem);
	}
	
	return B_BAD_THREAD_ID;
}
/*----------------------------------------------------------------------------*/
status_t
on_exit_thread(void (*callback)(void *), void *data)
{
	DBG(OUT("on_exit_thread(): not implemented yet\n"));
	return B_ERROR;
}
/*----------------------------------------------------------------------------*/
status_t
get_thread_info(thread_id thread, thread_info *info)
{
	sys_thread_info_t *th = NULL;
	
	if(!info)
		return B_BAD_VALUE;
	
	if(acquire_sem(_thread_list_sem) == B_OK) {
		th = _thread_list;
		
		while(th) {
			if(th->info.thread == thread) {
				memcpy(info, &(th->info), sizeof(thread_info));
				release_sem(_thread_list_sem);
				DBG(OUT("Retrieved info about thread %d (%s)\n", thread, info->name));
				
				return B_OK;
			}
			th = th->next;
		}
		
		release_sem(_thread_list_sem);
	}
	
	return B_BAD_VALUE;
}
/*----------------------------------------------------------------------------*/
/*	TODO: resolve hazard when the static pointer is pointing at a node
	deleted by another function...hint: the cookie can be used for that
*/
status_t
get_next_thread_info(team_id team, int32 *cookie, thread_info *info)
{
	static sys_thread_info_t *th = NULL;

	if(!info)
		return B_BAD_VALUE;
	
	if(acquire_sem(_thread_list_sem) == B_OK) {
		if(!(*cookie)) {
			th = _thread_list;
			*cookie = 1;
		}
		
		while(th) {
			if(th->info.team == team) {
				memcpy(info, &(th->info), sizeof(thread_info));
				DBG(OUT("Retrieved info about thread %d (%s)\n", th->info.thread, th->info.name));
				
				th = th->next;
				release_sem(_thread_list_sem);
				return B_OK;
			}
			
			th = th->next;
		}
		
		release_sem(_thread_list_sem);
	}
	
	return B_BAD_VALUE;
}
/*----------------------------------------------------------------------------*/
/*status_t
send_data(thread_id thread, int32 code, const void *buffer, size_t bufferSize)
{
	// ToDo: send_data()
	return B_ERROR;
}
*/
/*----------------------------------------------------------------------------*/
/*status_t
receive_data(thread_id *sender, void *buffer, size_t bufferSize)
{
	// ToDo: receive_data()
	return B_ERROR;
}*/
/*----------------------------------------------------------------------------*/
/*bool
has_data(thread_id thread)
{
	// ToDo: has_data()
	return false;
}*/
/*----------------------------------------------------------------------------*/
status_t
snooze(bigtime_t timeout)
{
	DBG(OUT("Thread %d going to sleep for %ld usecs\n", pthread_self(), timeout));
	
	if(usleep((long)timeout) != 0)
	switch(errno) {
		case EINVAL:
		case EINTR: return B_INTERRUPTED;
	}
	
	return B_OK;
}
/*----------------------------------------------------------------------------*/
/*status_t
snooze_until(bigtime_t timeout, int timeBase)
{
	return snooze_etc(timeout, timeBase, B_ABSOLUTE_TIMEOUT);
}*/
/*----------------------------------------------------------------------------*/
