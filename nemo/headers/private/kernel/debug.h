/*
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef _KERNEL_DEBUG_H
#define _KERNEL_DEBUG_H


#include <kernel.h>


struct kernel_args;

extern int dbg_register_file[2][14]; /* XXXmpetit -- must be made generic */

int  dbg_init(struct kernel_args *ka);
int  dbg_init2(struct kernel_args *ka);
char dbg_putch(char c);
void dbg_puts(const char *s);
bool dbg_set_serial_debug(bool new_val);
bool dbg_get_serial_debug(void);

/* special return codes for kernel debugger command function*/
#define  B_KDEBUG_CONT   2
#define  B_KDEBUG_QUIT   3


#if DEBUG 
#	define ASSERT(x) \
	if (x) {} else { panic("ASSERT FAILED (%s:%d): %s\n", __FILE__, __LINE__, #x); }
#else 
#	define ASSERT(x) 
#endif

#endif	/* _KERNEL_DEBUG_H */
