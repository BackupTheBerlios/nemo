// Standard Includes -----------------------------------------------------------
#include <stdio.h>

// System Includes -------------------------------------------------------------
#include <Message.h>

// Private Includes ------------------------------------------------------------

// Main ------------------------------------------------------------------------
int main( void ) {

	BMessage *msg1 = new BMessage;
	BMessage *msg2 = new BMessage;
	
	msg1->AddInt32("ya rab!", 16);	

	int size = msg1->FlattenedSize();
	char *buffer = new char[size];
	msg1->Flatten(buffer, size);

	msg2->Unflatten(buffer);
	int32 l = 0;
	msg2->FindInt32("ya rab!", &l);
	printf("l = %d\n", l);
	
	delete msg1;
	delete msg2;
	delete[] buffer;
	
	return 0;
}
//------------------------------------------------------------------------------
