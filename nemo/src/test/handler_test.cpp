// Standard Includes -----------------------------------------------------------
#include <stdio.h>

// System Includes -------------------------------------------------------------
#include <Handler.h>
#include <Looper.h>
#include <Message.h>
#include <MessageFilter.h>

// Macros ----------------------------------------------------------------------
#define	MESSAGE_1	'msg1'
#define	MESSAGE_2	'msg2'
#define	MESSAGE_3	'msg3'

// Types -----------------------------------------------------------------------
class THandler: public BHandler {
public:
	void MessageReceived(BMessage *message) {

		switch(message->what) {
			case MESSAGE_1: {
				printf("\n<< Handler received MESSAGE_1 >>\n");
			} break;

			case MESSAGE_2: {
				printf("\n<< Handler received MESSAGE_2 >>\n");
			} break;

			case MESSAGE_3: {
				printf("\n<< Handler received MESSAGE_3 >>\n");
			} break;	
		}
	}
};

// Globals ---------------------------------------------------------------------

// Function Prototypes ---------------------------------------------------------

// Main ------------------------------------------------------------------------
int main(void) {
	
	printf("BHandler test\n");
	
	THandler *handler = new THandler;
	BMessageFilter *filter = new BMessageFilter(MESSAGE_1);	
	BLooper *looper = new BLooper;

	if(looper->Lock()) {
		looper->AddHandler(handler);
		handler->AddFilter(filter);
		looper->Unlock();
	}
	
	thread_id th = looper->Run();

	while(true) {
		int n = -1;
		while(n < 0 || n > 3) {
			printf("\nEnter message number (1, 2, 3, or 0 to exit): ");
			fflush(stdin);
			scanf("%d", &n);
		}
		if(n == 0) {
			looper->PostMessage(B_QUIT_REQUESTED);
			break;			
		}
		switch(n) {
			case 1: {
				looper->PostMessage(MESSAGE_1, handler);
			} break;

			case 2: {
				looper->PostMessage(MESSAGE_2, handler);
			} break;

			case 3: {
				looper->PostMessage(MESSAGE_3, handler);
			} break;												
		}
	}

	status_t err;
	wait_for_thread(th, &err);
	delete handler;
	
	return 0;
}
// Functions -------------------------------------------------------------------
