/*------------------------------------------------------------------------------
/ libroot.h
/
/ DESCRIPTION:
/	Global data, initialization & termination procedures of libroot
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 1/2/2004
------------------------------------------------------------------------------*/

#ifndef _LIBROOT_H
#define _LIBROOT_H

/* Standard Includes ---------------------------------------------------------*/

/* System Includes -----------------------------------------------------------*/
#include <OS.h>

/* Private Includes ----------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* Types ---------------------------------------------------------------------*/
typedef struct sys_thread_info_t {
	thread_info	info;
	int sem;
	struct sys_thread_info_t *next;
} sys_thread_info_t;	

/* Globals -------------------------------------------------------------------*/
extern int _kernel_port_id;
extern int _libroot_port_id;

extern sys_thread_info_t *_thread_list;
extern sem_id _thread_list_sem;

/* Library initialization & finalization functions ---------------------------*/
void __attribute__ ((constructor)) libroot_init(void);
void __attribute__ ((destructor)) libroot_fini(void);

/* Private Thread Functions & Types ------------------------------------------*/
void*		start_routine(void *func_arg_struct);
void		delete_thread(sys_thread_info_t *th_info);
thread_id	get_main_thread(void);

/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* _LIBROOT_H */
