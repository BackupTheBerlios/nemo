//==========================================================================
// ABOUT ///////////////////////////////////////////////////////////////////
//==========================================================================

//==========================================================================
// INCLUDE /////////////////////////////////////////////////////////////////
//==========================================================================
#include <stdio.h>

#include <Application.h>
#include <Button.h>
//#include <TextControl.h>
#include <Window.h>

//==========================================================================
// MACRO DEFS //////////////////////////////////////////////////////////////
//==========================================================================
//#define BUTTON_MSG	'btnm'

//==========================================================================
// CLASS TWindow ///////////////////////////////////////////////////////////
//==========================================================================
/*class TWindow: public BWindow {

public:
//==========================================================================
	TWindow()
		: BWindow(BRect(100, 100, 200, 200),
			"BWindow Test",
			B_TITLED_WINDOW,
			0) {
		
		fTextControl = new BTextControl(BRect(10, 10, 90, 30),
			"Text Control",
			NULL,
			"waiting...",
			NULL,
			B_FOLLOW_LEFT_RIGHT);
		
		fButton = new BButton(BRect(20, 60, 80, 90),
			"The one and only button",
			"click me!",
			new BMessage(BUTTON_MSG),
			B_FOLLOW_BOTTOM|B_FOLLOW_LEFT_RIGHT);
		
		AddChild(fTextControl);
		AddChild(fButton);
		
		fCounter = 0;
	}
//==========================================================================	
	void	MessageReceived(BMessage *msg) {
		
		switch(msg->what) {
			case BUTTON_MSG: {
				char str[100];
				sprintf(str, "clicked %d time(s)", ++fCounter);
				fTextControl->SetText(str);
			} break;
			
			case B_QUIT_REQUESTED: {
				be_app->Quit();
			} break;
			
			default: BWindow::MessageReceived(msg);
		}
	}
//==========================================================================
	bool QuitRequested() {
		be_app->PostMessage(B_QUIT_REQUESTED);
		return BWindow::QuitRequested();
	}
//==========================================================================	
private:
	BTextControl	*fTextControl;
	BButton			*fButton;
	int				fCounter;
};*/

//==========================================================================
// CLASS TApplication //////////////////////////////////////////////////////
//==========================================================================
class TApplication: public BApplication {

public:
//==========================================================================
	TApplication() : BApplication("application/x-vnd.BWindowTest") {
	}
//==========================================================================			
	void	ReadyToRun() {
		
		BWindow *wnd = new BWindow(BRect(100, 100, 200, 200),
			"BWindow Test",
			B_TITLED_WINDOW,
			0);
		wnd->Show();	
	}
};

//==========================================================================
// MAIN ////////////////////////////////////////////////////////////////////
//==========================================================================
int main(void) {

	TApplication *app = new TApplication();
	app->Run();
	delete app;
	
	return 0;
}