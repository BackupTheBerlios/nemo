/**
 * FILE NAME:	thread.c
 * DESCRIPTION:	the functions implemented here provide access to the threading
 * 				capabilities of the kernel
 * AUTHOR:		Mahmoud Al Gammal
 * DATE:		Dec 21st 2003
 */

#include <OS.h>

#include <errno.h>
#include <pthread.h>

/*#include <stdlib.h>
#include <stdio.h>

#include "tls.h"
#include "syscalls.h"
*/

//#undef thread_entry
	// thread_entry is still defined in OS.h for compatibility reasons

#if DEBUG
	#include "nemo_debug.h"
#endif

/*===========================================================================*/

/*typedef struct callback_node {
	struct callback_node *next;
	void (*function)(void *);
	void *argument;
} callback_node;*/

/*===========================================================================*/

//void _thread_do_exit_notification(void);

/*===========================================================================*/

typedef	void*(*pthread_func)(void*);

/*===========================================================================*/

/*static int32
thread_entry(thread_func entry, void *data)
{
	int32 returnCode = entry(data);

	_thread_do_exit_notification();

	return returnCode;
}*/

/*===========================================================================*/

/* TODO: unlike the original spawn_thread(), a thread created using our
 * spawn_thread() starts immediately
 */
thread_id
spawn_thread(thread_func entry, const char *name, int32 priority, void *data)
{
	pthread_t id;
	/* TODO: use a customized pthread_attr to set the thread properties
	 */
	if(pthread_create(&id, NULL, (pthread_func)entry, data) != 0) {
		/* TODO: the man page of pthread_create() states that the function
		 * returns EAGAIN if no more thread ids are available, and EAGAIN (!!)
		 * if there's not enough resources for the new thread.
		 * Be's implementation distinguishes between the two cases by returning
		 * B_NO_MORE_THREADS or B_NO_MEMORY, but since the pthread
		 * implementation doesn't make this distinction, we will always 
		 * return B_NO_MORE_THREADS */
		return B_NO_MORE_THREADS;
	}
	
	return (thread_id)id;
}

/*===========================================================================*/

status_t
kill_thread(thread_id thread)
{
	if(pthread_cancel((pthread_t)thread) != 0) {
		if(errno == ESRCH) return B_BAD_THREAD_ID;
	}
	return B_OK;
}

/*===========================================================================*/

/*status_t
resume_thread(thread_id thread)
{
	return sys_resume_thread(thread);
}*/

/*===========================================================================*/

/*status_t
suspend_thread(thread_id thread)
{
	return sys_suspend_thread(thread);
}*/

/*===========================================================================*/

/* NOTE: finding thread by name is not supported yet.
 * Currently, this function can only be used to get the current
 * thread id by passing NULL as the thread name
 */
thread_id
find_thread(const char *name)
{
	if (!name)
		return (thread_id)pthread_self();
	else
		return B_NAME_NOT_FOUND;
}

/*===========================================================================*/

/*status_t
rename_thread(thread_id thread, const char *name)
{
	// ToDo: rename_thread not implemented
	return B_ERROR;
}*/

/*===========================================================================*/

/*status_t
set_thread_priority(thread_id thread, int32 priority)
{
	// ToDo: set_thread_priority not implemented
	return B_ERROR;
}*/

/*===========================================================================*/

/*void
exit_thread(status_t status)
{
	_thread_do_exit_notification();

	sys_exit(status);
}*/

/*===========================================================================*/

status_t
wait_for_thread(thread_id thread, status_t *thread_return_value)
{
	void *rv;
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
	*thread_return_value = (status_t)(rv);
	return B_OK;
}

/*===========================================================================*/

/*void
_thread_do_exit_notification(void)
{
	callback_node *node = tls_get(TLS_ON_EXIT_THREAD_SLOT);
	callback_node *next;
	
	while (node != NULL) {
		next = node->next;

		node->function(node->argument);
		free(node);

		node = next;
	}
}*/

/*===========================================================================*/

/*status_t
on_exit_thread(void (*callback)(void *), void *data)
{
	callback_node **head = (callback_node **)tls_address(TLS_ON_EXIT_THREAD_SLOT);

	callback_node *node = malloc(sizeof(callback_node));
	if (node == NULL)
		return B_NO_MEMORY;

	node->function = callback;
	node->argument = data;

	// add this node to the list
	node->next = *head;
	*head = node;

	return B_OK;
}*/

/*===========================================================================*/

/*status_t
_get_thread_info(thread_id thread, thread_info *info, size_t size)
{
	// size parameter is not yet used (but may, if the thread_info structure ever changes)
	(void)size;
	return sys_get_thread_info(thread, info);
}*/

/*===========================================================================*/

/*status_t
_get_next_thread_info(team_id team, int32 *cookie, thread_info *info, size_t size)
{
	// size parameter is not yet used (but may, if the thread_info structure ever changes)
	(void)size;
	return sys_get_next_thread_info(team, cookie, info);
}*/

/*===========================================================================*/

/* ToDo: Those are currently defined in syscalls.S - we need some
 * 		consistency here...
 *		send_data(), receive_data(), has_data()

status_t
send_data(thread_id thread, int32 code, const void *buffer, size_t bufferSize)
{
	// ToDo: send_data()
	return B_ERROR;
}


status_t
receive_data(thread_id *sender, void *buffer, size_t bufferSize)
{
	// ToDo: receive_data()
	return B_ERROR;
}


bool
has_data(thread_id thread)
{
	// ToDo: has_data()
	return false;
}
*/

/*===========================================================================*/

/*status_t
snooze_etc(bigtime_t timeout, int timeBase, uint32 flags)
{
	return sys_snooze_etc(timeout, timeBase, flags);
}*/

/*===========================================================================*/

/*status_t
snooze(bigtime_t timeout)
{
	return snooze_etc(timeout, B_SYSTEM_TIMEBASE, B_RELATIVE_TIMEOUT);
}*/

/*===========================================================================*/

/*status_t
snooze_until(bigtime_t timeout, int timeBase)
{
	return snooze_etc(timeout, timeBase, B_ABSOLUTE_TIMEOUT);
}*/

/*===========================================================================*/
