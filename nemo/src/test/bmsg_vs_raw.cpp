// Standard Includes -----------------------------------------------------------
#include <stdio.h>

// System Includes -------------------------------------------------------------
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>

#include <Message.h>

// Macros ----------------------------------------------------------------------

// Types -----------------------------------------------------------------------
struct msg_t {
	int		code;
	char	text[4096];
};

// Globals ---------------------------------------------------------------------

// Function Prototypes ---------------------------------------------------------

// Main ------------------------------------------------------------------------
int main(void) {

	int mq = msgget(ftok("/dev/mem"), IPC_CREATE|666);
	msg_t msg;
	msg.code = 1;
	
	for(int i = 0; i < 100; i++)
		msgsnd(mq, &msg, 3072, 0);

	for(int i = 0; i < 100; i++) {
		BMessage *bmsg = new BMessage(1);
		bmsg->AddData("data", B_RAW_TYPE, msg.text, 3072, false, 0);
		char *buf = new char[bmsg->FlattenedSize()];
		bmsg->Flatten(buf);
		delete bmsg;
		msgsnd();
	}
}

// Functions -------------------------------------------------------------------
