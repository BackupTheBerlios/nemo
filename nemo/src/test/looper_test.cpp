// Standard Includes -----------------------------------------------------------
#include <stdio.h>
#include <unistd.h>

// System Includes -------------------------------------------------------------
#include <AppDefs.h>
#include <OS.h>
#include <Looper.h>

// Macros ----------------------------------------------------------------------

// Types -----------------------------------------------------------------------

// Globals ---------------------------------------------------------------------
BLooper *looper = NULL;

// Function Prototypes ---------------------------------------------------------
int32 func(void *);

// Main ------------------------------------------------------------------------
int main(void) {
	
	printf("BLooper test\n");
	looper = new BLooper("looper_test");

	printf("Created looper looper_test\n");
	printf("This team id is %d\n", getpid());
	printf("looper->Team() = %d\n", looper->Team());
	printf("looper->Sem() = %d\n", looper->Sem());
	if(looper->IsLocked()) {
		printf("looper is locked %d times\n", looper->CountLocks());
		printf("looper has %d handler(s)\n", looper->CountHandlers());
	}
	else {
		printf("looper is not locked\n");
	}
	
	thread_id th = looper->Run();

	int what;
	while(true) {
		printf("Enter message code (-1 to exit): ");
		fflush(stdin);
		scanf("%d", &what);
		if(what == -1) {
			looper->PostMessage(B_QUIT_REQUESTED);
			break;
		}
		looper->PostMessage(what);
	}

	status_t err;
	wait_for_thread(th, &err);

	return 0;
}
// Functions -------------------------------------------------------------------
int32 func(void *arg) {

	printf("in func\n");
	thread_id thid = looper->Run();
	printf("looper thread = %d\n", thid);
	exit_thread(0);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
