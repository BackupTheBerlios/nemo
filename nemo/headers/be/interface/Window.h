#ifndef	_WINDOW_H
#define	_WINDOW_H


// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <BeBuild.h>
#include <InterfaceDefs.h>
#include <List.h>
#include <Looper.h>
#include <Rect.h>
#include <StorageDefs.h>

// Project Includes ------------------------------------------------------------
//#include <View.h>

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------
//class PortLink;

// Globals ---------------------------------------------------------------------


// window definitions ----------------------------------------------------------
enum window_type {
	B_TITLED_WINDOW 	= 1,
	B_MODAL_WINDOW 		= 3,
	B_FLOATING_WINDOW	= 21
};

//----------------------------------------------------------------

enum {
	B_NOT_MOVABLE			= 0x00000001,
	B_NOT_CLOSABLE			= 0x00000020,
	B_NOT_ZOOMABLE			= 0x00000040,
	B_NOT_MINIMIZABLE		= 0x00004000,
	B_NOT_RESIZABLE			= 0x00000002,
	B_NOT_H_RESIZABLE		= 0x00000004,
	B_NOT_V_RESIZABLE		= 0x00000008,
	B_AVOID_FRONT			= 0x00000080,
	B_AVOID_FOCUS			= 0x00002000,
	B_WILL_ACCEPT_FIRST_CLICK	= 0x00000010,
	B_OUTLINE_RESIZE		= 0x00001000,
	B_NO_WORKSPACE_ACTIVATION	= 0x00000100,
	B_NOT_ANCHORED_ON_ACTIVATE	= 0x00020000,
	B_ASYNCHRONOUS_CONTROLS		= 0x00080000,
	B_QUIT_ON_WINDOW_CLOSE		= 0x00100000
};

//----------------------------------------------------------------

class BSession;
class BButton;
class BMenuBar;
class BMenuItem;
class BMessage;
class BMessageRunner;
class BMessenger;
class BView;

struct message;
struct _cmd_key_;
struct _view_attr_;
// BWindow class ---------------------------------------------------------------
class BWindow : public BLooper 
{

public:
	//Constructors
	BWindow(BRect frame,const char* title,window_type type,uint32 flags);
	//Destructors
	virtual	~BWindow();
	//Closing window
	virtual	void Quit();
	void Close(); //Synonym of Quit()
	//Chnging Order of the Window
	status_t SendBehind(const BWindow* window);
	bool IsFront() const;
	void Activate(bool = true);
	bool IsActive() const;
	//Show & Hide
	virtual void Show();
	virtual void Hide();
	bool IsHidden() const;
	//Properties
		//Title
	void SetTitle(const char* title);
		//Type
	status_t SetType(window_type type);
	window_type Type() const;

private:
	//((((((((((Members Variables))))))))))//
	//Window Properties
	window_type fType;//Window Type
	char *fTitle;//Window Title
	bool fActive;//is the window active or not
	short fShowLevel;//the level of the window on the screen (ZOrder)
	uint32 fFlags;//Flags
	BRect fFrame;//The Size of the window
	//Send & receive Ports	
	port_id send_port;
	port_id receive_port;
	BSession *session;
	//Handling Childrens (BViews)
	BView *top_view;
	BView *fFocus;
	BMenuBar *fKeyMenuBar;
	BButton *fDefaultButton;
	//Member Function
	void InitData(BRect frame,const char* title,window_type type,uint32 flags);
	void stopConnection();
};
#endif	// _WINDOW_H  
