#ifndef _NEMO_DEBUG_H
#define _NEMO_DEBUG_H

/* Standard Includes ---------------------------------------------------------*/
#include <errno.h>
#include <stdio.h>

/* Debugging macros ----------------------------------------------------------*/

/* Kernel Kit */
#define DEBUG_KERNEL		1
#define KERNEL_LOG			stdout

/* libroot */
#define	DEBUG_LIBROOT		1
#define LIBROOT_LOG			stdout

/* Application Server */
#define DEBUG_APPSERVER		1
#define APPSERVER_LOG		stdout

/* libbe */
#define DEBUG_LIBBE			1
#define LIBBE_LOG			stdout

/* Debugging functions -------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

const char*	errno2string(int no);
void not_implemented(FILE *log, const char *func);

#ifdef __cplusplus
}
#endif

#endif // _NEMO_DEBUG_H
