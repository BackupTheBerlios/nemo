// Standard Includes -----------------------------------------------------------
#include <stdio.h>

// System Includes -------------------------------------------------------------
#include <Application.h>
#include <Handler.h>
#include <Looper.h>
#include <OS.h>

// Macros ----------------------------------------------------------------------

// Types -----------------------------------------------------------------------
class TApplication: public BApplication {
public:

//------------------------------------------------------------------------------
	TApplication() : BApplication("Test App") {
	};

//------------------------------------------------------------------------------
	virtual	~TApplication() {
	}
//------------------------------------------------------------------------------
	void ReadyToRun() {
		printf("TApplication: In ReadyToRun()\n");
		thread_id th = spawn_thread(test_func, "Test Function", B_NORMAL_PRIORITY, NULL);
		resume_thread(th);
	}
//------------------------------------------------------------------------------
	void MessageReceived(BMessage *message) {
		printf("TApplication: got message %d\n", message->what);
		BApplication::MessageReceived(message);
	}
//------------------------------------------------------------------------------
	static int32 test_func(void *arg) {
		while(true) {
			int i;
			printf("\nEnter message code to send to be_app (-1 to exit): ");
			scanf("%d", &i);
			if(i == -1) {
				be_app->PostMessage(B_QUIT_REQUESTED);
				break;
			}
			be_app->PostMessage(i);
		}
	}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------	
};

// Globals ---------------------------------------------------------------------

// Function Prototypes ---------------------------------------------------------

// Main ------------------------------------------------------------------------
int main(void) {

	TApplication *app = new TApplication;
	app->Run();
	delete app;
	
	return 0;
}

// Functions -------------------------------------------------------------------
