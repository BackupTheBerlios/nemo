// System Includes -------------------------------------------------------------
#include <BeBuild.h>
#include <InterfaceDefs.h>
#include <PropertyInfo.h>
#include <Handler.h>
#include <Looper.h>
#include <Application.h>
#include <Window.h>
//#include <View.h>
#include <MenuBar.h>
#include <String.h>
//#include <Screen.h>
#include <Button.h>
#include <MessageQueue.h>
#include <MessageRunner.h>
#include <Roster.h>

// Project Includes ------------------------------------------------------------
#include <AppMisc.h>
#include <PortLink.h>
#include <PortMessage.h>
#include <Session.h>
#include <ServerProtocol.h>
#include <TokenSpace.h>
#include <MessageUtils.h>
//#include <WindowAux.h>

// Standard Includes -----------------------------------------------------------
#include <stdio.h>
#include <math.h>
//#include <posix/math.h>

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------
#define DEBUG_WINDOW 1
// Constructors
//------------------------------------------------------------------------------
BWindow::BWindow(BRect frame,const char* title,window_type type,uint32 flags):BLooper( title )
{
	InitData(frame,title,type,flags);
}

//------------------------------------------------------------------------------

//Destructor
BWindow::~BWindow()
{
	// TODO: release other dinamicaly alocated objects

	// disable pulsing
	//FIXME:SetPulseRate( 0 );

	delete session;
	delete_port( receive_port );
}


//------------------------------------------------------------------------------

void BWindow::Quit()
{
	if (!IsLocked())
	{
		const char* name = Name();
		if (!name)
			name = "no-name";
		//ERROR - you must Lock a looper before calling Quit()
	}

	// Try to lock
	if (!Lock())
	{
		return;
	}

	while (!IsHidden())
	{ 
		Hide(); 
	}

	// ... also its children
	//FIXME:detachTopView();
	// tell app_server, this window will finish execution
	stopConnection();
	if (fFlags & B_QUIT_ON_WINDOW_CLOSE)be_app->PostMessage( B_QUIT_REQUESTED );

	BLooper::Quit();
}

//------------------------------------------------------------------------------

void BWindow::Close()
{
	Quit();
}

//------------------------------------------------------------------------------

void BWindow::SetTitle(const char* title){
	
	if (!title) return;

	if (fTitle)
	{
		delete fTitle;
		fTitle = NULL;
	}
	
	fTitle = strdup( title );

	// we will change BWindow's thread name to "w>window_title"	
	int32 length;
	length = strlen( fTitle );
	
	char *threadName;
	threadName = new char[32];
	strcpy(threadName,"w>");
	strncat(threadName,fTitle,(length>=29) ? 29: length);

		// if the message loop has been started...
	if (Thread() != B_ERROR )
	{
		SetName( threadName );
		rename_thread( Thread(), threadName );
			// we notify the app_server so we can actually see the change
		Lock();
		session->WriteInt32(AS_WINDOW_TITLE);
		session->WriteString( fTitle );
		session->Sync( );
		Unlock();
	}
	else
	{
		SetName( threadName );
	}
}

//------------------------------------------------------------------------------

status_t BWindow::SetType(window_type type)
{

	int32 rCode;

	Lock();
		//Using AS_SET_LOOK because there is no AS_SET_TYPE
		session->WriteInt32( AS_SET_LOOK );
		session->WriteInt32( (int32)type );
		session->Sync();
		session->ReadInt32( &rCode );
	Unlock();
	
	if (rCode == SERVER_TRUE)
	{
		fType = type;
		return B_OK;
	}

	return B_ERROR;
}

//------------------------------------------------------------------------------

window_type BWindow::Type() const
{
	return fType;
}

//------------------------------------------------------------------------------

void BWindow::Activate(bool active)
{
	if (IsHidden()) return;

	Lock();
		session->WriteInt32( AS_ACTIVATE_WINDOW );
		session->WriteBool( active );
		session->Sync();
	Unlock();
}

//------------------------------------------------------------------------------

bool BWindow::IsActive() const
{
	return fActive;
}

//------------------------------------------------------------------------------

status_t BWindow::SendBehind(const BWindow* window)
{
	if (!window) return B_ERROR;
	
	int32 rCode;

	Lock();
		session->WriteInt32( AS_SEND_BEHIND );
		session->WriteInt32( _get_object_token_(window) );	
		session->Sync();
		session->ReadInt32( &rCode );
	Unlock();
	
	return rCode == SERVER_TRUE? B_OK : B_ERROR;
}

//------------------------------------------------------------------------------

bool BWindow::IsFront() const
{
	if (IsActive()) return true;

	//if (IsModal()) return true;

	return false;
}

//------------------------------------------------------------------------------

void BWindow::Show()
{
	bool isLocked = this->IsLocked();
		
	fShowLevel--;

	if (fShowLevel == 0)
	{
		//STRACE(("BWindow(%s): sending AS_SHOW_WINDOW message...\n", Name() ));
		if ( !isLocked ) Lock();
		session->WriteInt32( AS_SHOW_WINDOW );
		session->Sync();
		if ( !isLocked) Unlock();
	}
	// if it's the fist time Show() is called... start the Looper thread.
	if ( Thread() == B_ERROR )
	{
		// normaly this won't happen, but I want to be sure!
		if ( !isLocked ) Lock();
		Run();
	}
	
}

//------------------------------------------------------------------------------

void BWindow::Hide()
{
	if (fShowLevel == 0)
	{
		Lock();
		session->WriteInt32( AS_HIDE_WINDOW );
		session->Sync();
		Unlock();
	}
	fShowLevel++;
}
//------------------------------------------------------------------------------

bool BWindow::IsHidden() const
{
	return fShowLevel > 0;
	return 0;
	
}

// PRIVATE
//--------------------Private Methods-------------------------------------------
// PRIVATE
void BWindow::InitData(BRect frame,const char* title,window_type type,uint32 flags)
{
	#if DEBUG_WINDOW
		printf("BWindow: Intializing BWindow.");
	#endif
	fTitle=NULL;
	if ( be_app == NULL )
	{
		//Error:You need a valid BApplication object before interacting with the app_server
		return;
	}
	
	#if DEBUG_WINDOW
		printf("BWindow: Title is %s ",title);
	#endif
	
	fFrame = frame;

	if (title) SetTitle( title );

	fFlags = flags;

	fActive = false;
	
	fShowLevel = 1;
	
	top_view = NULL;
	
	fFocus = NULL;
	
	fKeyMenuBar = NULL;
	
	fDefaultButton = NULL;

// TODO: other initializations!


	//Here, we will contact app_server and let him know that a window has
	//been created

	receive_port = create_port(B_LOOPER_PORT_DEFAULT_CAPACITY,"w_rcv_port");
	
	if (receive_port==B_BAD_VALUE || receive_port==B_NO_MORE_PORTS)
	{
		//Error:Could not create BWindow's receive port, used for interacting with the app_server!
		delete this;
		return;
	}

	// let app_server to know that a window has been created.
	
	session = new BSession( receive_port, be_app->fServerFrom );

	// HERE we are in BApplication's thread, so for locking we use be_app variable
	// we'll lock the be_app to be sure we're the only one writing at BApplication's server port
	
	bool locked = false;
	if ( !(be_app->IsLocked()) )
	{ 
		be_app->Lock(); 
		locked = true; 
	}
	
	/*PortMessage		pmsg;
	PortLink		link(be_app->fServerFrom);
	link.SetOpCode(AS_CREATE_WINDOW);
	link.Attach<BRect>( fFrame );
	link.Attach<int32>( (int32)fType );
	link.Attach<uint32>( fFlags );
	link.Attach<int32>( _get_object_token_(this) );
	link.Attach<port_id>( receive_port );
	link.Attach<port_id>( fMsgPort );
	link.AttachString( title );
	link.FlushWithReply(&pmsg);

	pmsg.Read<port_id>(&send_port);
	*/

	session->WriteInt32( AS_CREATE_WINDOW );
	session->WriteRect( fFrame );
	session->WriteInt32( (int32)fType );
	session->WriteUInt32( fFlags );
	//FIXME:session->WriteData( &team, sizeof(team_id));
	session->WriteInt32( _get_object_token_(this) );
	session->WriteData( &receive_port, sizeof(port_id) );
	session->WriteData( &fMsgPort, sizeof(port_id) );
	session->WriteString( title );

	session->Sync();
		// The port on witch app_server will listen for us	
	session->ReadData( &send_port, sizeof(port_id) );
	
	// unlock, so other threads can do their job.
	if( locked ) be_app->Unlock();

	session->SetSendPort(send_port);

	// build and register top_view with app_server
	//FIXME when implementing bview:BuildTopView();
}

//------------------------------------------------------------------------------

void BWindow::stopConnection()
{
	Lock();
		session->WriteInt32( B_QUIT_REQUESTED );
		session->Sync();
	Unlock();
}

//------------------------------------------------------------------------------
 
