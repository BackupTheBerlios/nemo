/*------------------------------------------------------------------------------
/ debug.c
/
/ DESCRIPTION:
/	debugging functions in the kernel
/
/ AUTHOR: Mahmoud Al Gammal
/ DATE: 10/2/2004
------------------------------------------------------------------------------*/

/* Standard Includes ---------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

/* System Includes -----------------------------------------------------------*/

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

/* Globals -------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*	TODO: this function should throw the calling thread into the debugger.
	right now, it just prints the given message & exits the app
*/
void debugger(const char *message) {
	
	printf("debugger: %s\n", message);
	exit(0);
}
/*----------------------------------------------------------------------------*/
