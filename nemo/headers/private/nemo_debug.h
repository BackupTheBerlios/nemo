#ifndef _NEMO_DEBUG_H
#define _NEMO_DEBUG_H

#include <errno.h>
#include <stdio.h>

/* Debugging macros =========================================================*/

/* Kernel Kit */
#define DEBUG_KERNEL		1
#define KERNEL_LOG			stdout

/* Application Server */
#define DEBUG_APPSERVER		1
#define APPSERVER_LOG		stdout

/* ServerApp class */
#define DEBUG_SERVERAPP		1
#define SERVERAPP_LOG		stdout

/* BLocker class */
#define DEBUG_LOCKER		1
#define LOCKER_LOG			stdout

/* Debugging functions ======================================================*/

#ifdef __cplusplus
extern "C" {
#endif

const char*	errno2string(int no);
void not_implemented(FILE *log, const char *func);

#ifdef __cplusplus
}
#endif

#endif // _NEMO_DEBUG_H
