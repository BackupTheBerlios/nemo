//------------------------------------------------------------------------------
//	Copyright (c) 2001-2002, OpenBeOS
//
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//
//	File Name:		ServerWindow.cpp
//	Author:			DarkWyrm <bpmagic@columbus.rr.com>
//	Description:	Shadow BWindow class
//  
//------------------------------------------------------------------------------
#include <AppDefs.h>
#include <Rect.h>
#include <string.h>
#include <stdio.h>
//#include <View.h>	// for B_XXXXX_MOUSE_BUTTON defines
#include <Message.h>
#include <GraphicsDefs.h>
#include <PortLink.h>
#include <Session.h>

#include "AppServer.h"

//#include <Layer.h>
//#include "RootLayer.h"
#include "ServerWindow.h"
#include "ServerApp.h"
#include "ServerProtocol.h"
//#include "WinBorder.h"
//#include "Desktop.h"
#include "TokenHandler.h"
//#include "Utils.h"
//#include "DisplayDriver.h"
//#include "ServerPicture.h"
//#include "CursorManager.h"
//#include "Workspace.h"

#define DFBCHECK(x...)                                         \
  {                                                            \
    DFBResult err = x;                                         \
                                                               \
    if (err != DFB_OK)                                         \
      {                                                        \
        return B_ERROR;                                        \
        DirectFBErrorFatal( #x, err );                         \
      }                                                        \
  }

IDirectFBSurface* ServerWindow::body=NULL;
IDirectFBSurface* ServerWindow::head=NULL;
IDirectFBSurface* ServerWindow::shead=NULL;
IDirectFBSurface* ServerWindow::close_button=NULL;
IDirectFBSurface* ServerWindow::sclose_button=NULL;
IDirectFBSurface* ServerWindow::ok_button=NULL;
IDirectFBSurface* ServerWindow::sok_button=NULL;

//#define DEBUG_SERVERWINDOW
//#define DEBUG_SERVERWINDOW_MOUSE
//#define DEBUG_SERVERWINDOW_KEYBOARD

//! TokenHandler object used to provide IDs for all windows in the system
TokenHandler win_token_handler;

//! Active winborder - used for tracking windows during moves, resizes, and tab slides
WinBorder *active_winborder=NULL;

template<class Type> Type
read_from_buffer(int8 **_buffer)
{
	Type *typedBuffer = (Type *)(*_buffer);
	Type value = *typedBuffer;

	typedBuffer++;
	*_buffer = (int8 *)(typedBuffer);

	return value;
}

static int8 *read_pattern_from_buffer(int8 **_buffer)
{
	int8 *pattern = *_buffer;

	*_buffer += AS_PATTERN_SIZE;

	return pattern;
}

template<class Type> void
write_to_buffer(int8 **_buffer, Type value)
{
	Type *typedBuffer = (Type *)(*_buffer);

	*typedBuffer = value;
	typedBuffer++;

	*_buffer = (int8 *)(typedBuffer);
}

/*!
	\brief Contructor
	
	Does a lot of stuff to set up for the window - new decorator, new winborder, spawn a 
	monitor thread.
*/
ServerWindow::ServerWindow(BRect rect, const char *string, int32 fType,
	uint32 wflags, ServerApp *winapp,  port_id winport,
	port_id looperPort, port_id replyport/*, uint32 index*/, int32 handlerID)
{
	//STRACE(("ServerWindow(%s)::ServerWindow()\n",string? string: "NULL"));
	fServerApp			= winapp;

	if(string)
		fTitle.SetTo(string);
	fFrame			= rect;
	fFlags			= wflags;
	fHandlerToken		= handlerID;
	fClientLooperPort	= -1;
//	fWorkspaces		= index;
//	fClientTeamID	= winapp->fTeamID;
//	fWorkspace		= NULL;
	fToken			= win_token_handler.GetToken();
	hidden			= true;
	active			= false;
	created			= false;

	
	// fClientWinPort is the port to which the app awaits messages from the server
	fClientWinPort			= winport;

	// fMessagePort is the port to which the app sends messages for the server
	fMessagePort		= create_port(30,fTitle.String());

	fSession= new BSession(fMessagePort, fClientWinPort);
	
	// Send a reply to our window - it is expecting fMessagePort port.

	fSession->WriteInt32( AS_CREATE_WINDOW );
	fSession->WriteData( &fMessagePort, sizeof(port_id) );
	fSession->Sync();

	// Wait for top_view data and create ServerWindow's top most Layer
	int32			vToken;
	BRect			vFrame;
	uint32			vResizeMode;
	uint32			vFlags;
	char*			vName = NULL;
	
	//PortMessage pmsg;
	//pmsg.ReadFromPort(fMessagePort);
	int32 code;
	fSession->ReadInt32(&code);
	if(code != AS_LAYER_CREATE_ROOT)
		debugger("SERVER ERROR: ServerWindow(xxx): NO top_view data received!\n");

	/*
	pmsg.Read<int32>(&vToken);
	pmsg.Read<BRect>(&vFrame);
	pmsg.Read<uint32>(&vResizeMode);
	pmsg.Read<uint32>(&vFlags);
	pmsg.ReadString(&vName);
	*/
	fSession->ReadInt32(&vToken);
	fSession->ReadRect(&vFrame);
	fSession->ReadUInt32(&vResizeMode);
	fSession->ReadUInt32(&vFlags);
	vName = fSession->ReadString();

//	fTopLayer		= new Layer(vFrame, vName, vToken, vResizeMode, vFlags, this);
	delete vName;

//	cl = fTopLayer;

	// Create the window
	CreateWindow ();	
			
	// NOTE: this MUST be before the monitor thread is spawned!
//	desktop->AddWinBorder(fWinBorder);

	// Spawn our message-monitoring fMonitorThreadID
/*	fMonitorThreadID	= spawn_thread(MonitorWin, "TODO: name thread", B_NORMAL_PRIORITY, this);
	if(fMonitorThreadID != B_NO_MORE_THREADS && fMonitorThreadID != B_NO_MEMORY)
		resume_thread(fMonitorThreadID);*/
}

//!Tears down all connections with the user application, kills the monitoring thread.
ServerWindow::~ServerWindow(void)
{
//STRACE(("*ServerWindow (%s):~ServerWindow()\n",fTitle.String()));

//	desktop->fGeneralLock.Lock();

//	desktop->RemoveWinBorder(fWinBorder);
	//STRACE(("SW(%s) Successfuly removed from the desktop\n", fTitle.String()));
	if(fSession){
		delete fSession;
		fSession = NULL;
	}
	/*if (fWinLink){
		delete fWinLink;
		fWinLink = NULL;
	}*/
/*	
	if (fWinBorder){
		delete fWinBorder;
		fWinBorder = NULL;
	}
*/	
/*	cl		= NULL;
	if (fTopLayer)
		delete fTopLayer;*/

//	desktop->fGeneralLock.Unlock();
	//STRACE(("#ServerWindow(%s) will exit NOW!!!\n", fTitle.String()));
}

void ServerWindow::DispatchMessage(int32 code)
{
	switch(code)
	{
	/********** BWindow Messages ***********/
		case AS_QUIT_WINDOW:
		{
			if (created){
				window->Destroy(window);
				window = NULL;
				created = false;
			}
			break;
		}
		case AS_SEND_BEHIND:
		{
			// TODO
			// DFBResult err = x;
			fSession->WriteInt32 (AS_SEND_BEHIND);
			fSession->WriteInt32 (SERVER_TRUE);
			fSession->Sync();
			break;
		}
		case AS_ACTIVATE_WINDOW:
		{
			bool act;
			fSession->ReadBool (&act);
			active = act;
			break;
		}
		case AS_SHOW_WINDOW:
		{
			Show();
			break;
		}
		case AS_HIDE_WINDOW:
		{
			Hide();
			break;
		}
		case AS_WINDOW_TITLE:
		{
			char *title = fSession->ReadString();
			fTitle = BString(title);
			if (created)	
				UpdateTitle (title);
			break;
		}
		case AS_SET_LOOK:
		{
			fSession->ReadInt32 (&fType);
			if (!created)
				break;
			char title[fTitle.CountChars()+1];
			fTitle.CopyInto(title, 0, fTitle.CountChars());
			
			fSession->WriteInt32 (AS_SEND_BEHIND);
			if (UpdateTitle (title)==B_ERROR)
				fSession->WriteInt32 (SERVER_FALSE);
			else
				fSession->WriteInt32 (SERVER_TRUE);
			fSession->Sync();
			break;
		}
		case B_MINIMIZE:
		{
			Hide();
			break;
		}
		case B_WINDOW_MOVE_TO:
		{
			// TODO: Implement
			break;
		}
	/* Graphic Messages */
		case AS_SET_HIGH_COLOR:
		{
			break;
		}
		case AS_SET_LOW_COLOR:
		{
			break;
		}
		case AS_SET_VIEW_COLOR:
		{
			break;
		}
		case AS_STROKE_ARC:
		{
			break;
		}
		case AS_STROKE_BEZIER:
		{
			break;
		}
		case AS_STROKE_ELLIPSE:
		{
			break;
		}
		case AS_STROKE_LINE:
		{
				break;
		}
		case AS_STROKE_LINEARRAY:
		{
			break;
		}
		case AS_STROKE_POLYGON:
		{
			break;
		}
		case AS_STROKE_RECT:
		{
			break;
		}
		case AS_STROKE_ROUNDRECT:
		{
			break;
		}
		case AS_STROKE_SHAPE:
		{
			break;
		}
			case AS_STROKE_TRIANGLE:
		{
			break;
		}
		case AS_FILL_ARC:
		{
			break;
		}
		case AS_FILL_BEZIER:
		{
			break;
		}
		case AS_FILL_ELLIPSE:
		{
				break;
		}
		case AS_FILL_POLYGON:
		{
			break;
		}
		case AS_FILL_RECT:
		{
			break;
		}
		case AS_FILL_REGION:
		{
			break;
		}
		case AS_FILL_ROUNDRECT:
		{
			break;
		}
		case AS_FILL_SHAPE:
		{
			break;
		}
		case AS_FILL_TRIANGLE:
		{
			break;
		}
		case AS_MOVEPENBY:
		{
			break;
		}
		case AS_MOVEPENTO:
		{
			break;
		}
		case AS_SETPENSIZE:
		{
			break;
		}
		case AS_DRAW_STRING:
		{
			break;
		}
		case AS_SET_FONT:
		{
			break;
		}
		case AS_SET_FONT_SIZE:
		{
			break;
		}
		default:
				//printf("ServerWindow %s received unexpected code - message offset %lx\n",fTitle.String(), msg->Code() - SERVER_TRUE);
			break;
		
	}
}

/*!
	\brief Requests an update of the specified rectangle
	\param rect The area to update, in the parent's coordinates
	
	This could be considered equivalent to BView::Invalidate()
*/
void ServerWindow::RequestDraw(BRect rect)
{
//STRACE(("ServerWindow %s: Request Draw\n",fTitle.String()));
	BMessage		msg;
	
	msg.what		= _UPDATE_;
	msg.AddRect("_rect", rect);
	
	// TODO: implement this function
	//SendMessageToClient(&msg);
}

//! Requests an update for the entire window
void ServerWindow::RequestDraw(void)
{
	RequestDraw(fFrame);
}

//! Hides the window's WinBorder
void ServerWindow::Hide(void)
{
/*
	if(fWinBorder->IsHidden())
		return;

	STRACE(("ServerWindow %s: Hide\n",fTitle.String()));
	if(fWinBorder){
		RootLayer	*rl		= fWinBorder->GetRootLayer();
		Workspace	*ws		= NULL;

		desktop->fGeneralLock.Lock();
		STRACE(("ServerWindow(%s)::Hide() - General lock acquired\n", fWinBorder->GetName()));

		rl->fMainLock.Lock();
		STRACE(("ServerWindow(%s)::Hide() - Main lock acquired\n", fWinBorder->GetName()));

		fWinBorder->Hide();

		int32		wksCount= rl->WorkspaceCount();
		for(int32 i = 0; i < wksCount; i++){
			ws		= rl->WorkspaceAt(i+1);
			if (ws->FrontLayer() == fWinBorder){
				ws->HideSubsetWindows(fWinBorder);
				ws->SetFocusLayer(ws->FrontLayer());
			}
			else{
				if (ws->FocusLayer() == fWinBorder){
					ws->SetFocusLayer(fWinBorder);
				}
				else{
					ws->Invalidate();
				}
			}
		}
		rl->fMainLock.Unlock();
		STRACE(("ServerWindow(%s)::Hide() - Main lock released\n", fWinBorder->GetName()));

		desktop->fGeneralLock.Unlock();
		STRACE(("ServerWindow(%s)::Hide() - General lock released\n", fWinBorder->GetName()));
	}
*/
if (hidden)
	return;
if (created && active){
	window->LowerToBottom(window);
}

}
//---------------------------------------------------------------------------------
//--------------------------wael work----------------------
//---------------------------------------------------------------------------------
status_t ServerWindow::CreateWindow (IDirectFBDisplayLayer *layer){

  DFBFontDescription font_dsc;
  DFBWindowDescription wdsc;  
  IDirectFBSurface *wsurface = NULL;
  DFBSurfaceDescription dsc;
  
//  char *title;TODO: I need to convert the fTitle from BString to char *
  char title[fTitle.CountChars()+1];
  fTitle.CopyInto(title, 0, fTitle.CountChars());
  DFBRectangle frame;
  frame.x=(int)fFrame.LeftTop().x;
  frame.y=(int)fFrame.LeftTop().y;
  frame.w=(int)fFrame.IntegerWidth();
  frame.h=(int)fFrame.IntegerHeight();
    
  int width1, width2, width3;
  int height1, height2, height3;

  //create window inside the primary layer  
  wdsc.flags = (DFBWindowDescriptionFlags)(DWDESC_CAPS | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_POSX | DWDESC_POSY | DWDESC_SURFACE_CAPS);
  wdsc.caps = DWCAPS_ALPHACHANNEL;
  wdsc.width = frame.w;
  wdsc.height = frame.h;
  wdsc.posx = frame.x;
  wdsc.posy = frame.y;
  wdsc.surface_caps = DSCAPS_FLIPPING;
  DFBCHECK(layer->CreateWindow(layer, &wdsc, &window));
  
  //Get the window surface and clear it
  DFBCHECK(window->GetSurface(window, &wsurface));  
  DFBCHECK(wsurface->Clear (wsurface, 0x00, 0x00, 0x00, 0x00));
  DFBCHECK(window->SetOpaqueRegion (window, 0, 0, frame.h, frame.w));
  
  //Set the window options to be transparent
  DFBWindowOptions options;
  window->GetOptions (window, &options);
  
  options = (DFBWindowOptions) ( options | DWOP_SHAPED );
  DFBCHECK(window->SetOptions (window, options));  
  wsurface->SetSrcColorKey(wsurface, R, G, B);
  wsurface->SetBlittingFlags(wsurface, DSBLIT_SRC_COLORKEY);  
        
  if (fType==B_FLOATING_WINDOW){
  	DFBCHECK (shead->GetSize(shead, &width1, &height1));
	DFBCHECK (sok_button->GetSize(sok_button, &width2, &height2));
	DFBCHECK (sclose_button->GetSize(sclose_button, &width3, &height3));

	DFBRectangle rect;
	rect.x = 0;
	rect.y = 0;
	rect.h = height1;
	rect.w = frame.w-width2-width3;
	DFBCHECK (wsurface->StretchBlit (wsurface, shead, NULL, &rect));
	DFBCHECK (wsurface->Blit (wsurface, sok_button, NULL, rect.w, 0));
  	DFBCHECK (wsurface->Blit (wsurface, sclose_button, NULL, rect.w+width2, 0));	  	

  	// Draw the window title
  	font_dsc.flags = DFDESC_HEIGHT;
  	font_dsc.height = 10;
  	font_dsc.width = 10;  
  	DFBCHECK (app_server->dfb->CreateFont (app_server->dfb, "./decker.ttf", &font_dsc, &font));
  	DFBCHECK (wsurface->SetFont (wsurface, font));
  	DFBCHECK (wsurface->SetColor (wsurface, 0xff, 0x0, 0x0, 0xFF));
	int size = 0;
	DFBRectangle rect1;		
  	DFBCHECK (font->GetStringExtents (font, title, -1, NULL, &rect1));
	if (rect1.w > rect.w){
		do{
			size++;
			DFBCHECK (font->GetStringExtents (font, title, size, NULL, &rect1));
		}while (rect1.w < rect.w);
		size--;
	}  		  	
	DFBCHECK (wsurface->DrawString (wsurface, title, size-1, 5, 2*height1/3, DSTF_LEFT));	
		
	rect.x = 0;
	rect.y = height1;
	rect.w = frame.w;
	rect.h = frame.h-height1;
	DFBCHECK (wsurface->StretchBlit (wsurface, body, NULL, &rect));	
  }
  else if (fType==B_TITLED_WINDOW){
  	DFBCHECK (head->GetSize(head, &width1, &height1));
	DFBCHECK (ok_button->GetSize(ok_button, &width2, &height2));
	DFBCHECK (close_button->GetSize(close_button, &width3, &height3));

	DFBRectangle rect;
	rect.x = 0;
	rect.y = 0;
	rect.h = height1;
	rect.w = frame.w-width2-width3;
	DFBCHECK (wsurface->StretchBlit (wsurface, head, NULL, &rect));
	DFBCHECK (wsurface->Blit (wsurface, ok_button, NULL, rect.w, 0));
  	DFBCHECK (wsurface->Blit (wsurface, close_button, NULL, rect.w+width2, 0));
	
	// Draw the window title
  	font_dsc.flags = DFDESC_HEIGHT;
  	font_dsc.height = 15;
  	font_dsc.width = 15;  
  	DFBCHECK (app_server->dfb->CreateFont (app_server->dfb, "./decker.ttf", &font_dsc, &font));
  	DFBCHECK (wsurface->SetFont (wsurface, font));
  	DFBCHECK (wsurface->SetColor (wsurface, 0x0, 0x10, 0xfa, 0xFF));
	int size = 0;
	DFBRectangle rect1;		
  	DFBCHECK (font->GetStringExtents (font, title, -1, NULL, &rect1));
	if (rect1.w > rect.w){
		do{
			size++;
			DFBCHECK (font->GetStringExtents (font, title, size, NULL, &rect1));
		}while (rect1.w < rect.w);
		size-=2;
	}  		
	DFBCHECK (wsurface->DrawString (wsurface, title, -1, 6, 2*height1/3, DSTF_LEFT));	
		
	rect.x = 0;
	rect.y = height1;
	rect.w = frame.w;
	rect.h = frame.h-height1;
  	DFBCHECK (wsurface->StretchBlit (wsurface, body, NULL, &rect));  	
  }
  else if (fType==B_MODAL_WINDOW){
  }
  created = true;

}
//---------------------------------------------------------------------------------
status_t ServerWindow::Show (){

if (created){
  IDirectFBSurface *wsurface = NULL;
  
  DFBCHECK(window->SetStackingClass(window, DWSC_MIDDLE));
  DFBCHECK(window->RaiseToTop(window));
  DFBCHECK(window->GetSurface(window, &wsurface));  
  DFBCHECK (wsurface->Flip (wsurface, NULL, DSFLIP_WAITFORSYNC));
  window->SetOpacity (window, 0xff);
  hidden = false;
  active = true;
}
}
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

status_t ServerWindow::UpdateTitle (char *title){
	DFBFontDescription font_dsc;
	IDirectFBSurface *wsurface = NULL;
	int wdth;
	int ht;
	int width1, width2, width3;
	int height1, height2, height3;
		
	DFBCHECK(window->GetSurface(window, &wsurface));  
	DFBCHECK(window->GetSize(window, &wdth, &ht));
	
  if (fType==B_FLOATING_WINDOW){
  	DFBCHECK (shead->GetSize(shead, &width1, &height1));
	DFBCHECK (sok_button->GetSize(sok_button, &width2, &height2));
	DFBCHECK (sclose_button->GetSize(sclose_button, &width3, &height3));

	DFBRectangle rect;
	rect.x = 0;	rect.y = 0;	rect.h = height1;	rect.w = wdth-width2-width3;
	DFBCHECK (wsurface->StretchBlit (wsurface, shead, NULL, &rect));
	DFBCHECK (wsurface->Blit (wsurface, sok_button, NULL, rect.w, 0));
  	DFBCHECK (wsurface->Blit (wsurface, sclose_button, NULL, rect.w+width2, 0));	  		
	
  	// Draw the window title
  	font_dsc.flags = DFDESC_HEIGHT;
  	font_dsc.height = 10;
  	font_dsc.width = 10;  
  	DFBCHECK (app_server->dfb->CreateFont (app_server->dfb, "./decker.ttf", &font_dsc, &font));
  	DFBCHECK (wsurface->SetFont (wsurface, font));
  	DFBCHECK (wsurface->SetColor (wsurface, 0xff, 0x0, 0x0, 0xFF));  	
	DFBCHECK (wsurface->DrawString (wsurface, title, -1, 5, 2*height1/3, DSTF_LEFT));	
  }
  else if (fType==B_TITLED_WINDOW){
  	DFBCHECK (head->GetSize(head, &width1, &height1));
	DFBCHECK (ok_button->GetSize(ok_button, &width2, &height2));
	DFBCHECK (close_button->GetSize(close_button, &width3, &height3));

	DFBRectangle rect;
	rect.x = 0;	rect.y = 0;	rect.h = height1;	rect.w = wdth-width2-width3;
	DFBCHECK (wsurface->StretchBlit (wsurface, head, NULL, &rect));
	DFBCHECK (wsurface->Blit (wsurface, ok_button, NULL, rect.w, 0));
  	DFBCHECK (wsurface->Blit (wsurface, close_button, NULL, rect.w+width2, 0));	
  	
	// Draw the window title
  	font_dsc.flags = DFDESC_HEIGHT;
  	font_dsc.height = 15;
  	font_dsc.width = 15;  
  	DFBCHECK (app_server->dfb->CreateFont (app_server->dfb, "./decker.ttf", &font_dsc, &font));
  	DFBCHECK (wsurface->SetFont (wsurface, font));
  	DFBCHECK (wsurface->SetColor (wsurface, 0x0, 0x10, 0xfa, 0xFF));
	DFBCHECK (wsurface->DrawString (wsurface, title, -1, 6, 2*height1/3, DSTF_LEFT));	
  }	
}

