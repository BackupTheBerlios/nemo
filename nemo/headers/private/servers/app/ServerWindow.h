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
//	File Name:		ServerWindow.h
//	Author:			DarkWyrm <bpmagic@columbus.rr.com>
//	Description:	Shadow BWindow class
//  
//------------------------------------------------------------------------------
#ifndef _SERVERWIN_H_
#define _SERVERWIN_H_

// Error checker for DirectFB functions
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


#include <SupportDefs.h>
#include <GraphicsDefs.h>
#include <OS.h>
#include <Locker.h>
#include <Rect.h>
#include <String.h>
#include <Window.h>
#include <PortMessage.h>
#include <directfb.h>
//#include "FMWList.h"

class BString;
class BMessenger;
class BPoint;
class BMessage;
class ServerApp;
class Decorator;
class PortLink;
class WinBorder;
class BSession;
//class Layer;

#define AS_UPDATE_DECORATOR 'asud'
#define AS_UPDATE_COLORS 'asuc'
#define AS_UPDATE_FONTS 'asuf'

/*!
	\class ServerWindow ServerWindow.h
	\brief Shadow BWindow class
	
	A ServerWindow handles all the intraserver tasks required of it by its BWindow. There are 
	too many tasks to list as being done by them, but they include handling View transactions, 
	coordinating and linking a window's WinBorder half with its messaging half, dispatching 
	mouse and key events from the server to its window, and other such things.
*/
class ServerWindow
{
public:
	ServerWindow(BRect rect, const char *string, int32 fType,
		uint32 wflags, ServerApp *winapp,  port_id winport,
		port_id looperPort, port_id replyport/*, uint32 index */,int32 handlerID);
	ServerWindow(){};
	~ServerWindow(void);

	status_t Show(void);
	void Hide(void);
	bool IsHidden(void){return hidden;};
	
	void RequestDraw(BRect rect);
	void RequestDraw(void);
	void DispatchMessage(int32 code);
	status_t UpdateTitle (char *title);
	
	BRect Frame(void);
	
static IDirectFBSurface* body;
static IDirectFBSurface* head;
static IDirectFBSurface* shead;
static IDirectFBSurface* close_button;
static IDirectFBSurface* sclose_button;
static IDirectFBSurface* ok_button;
static IDirectFBSurface* sok_button;
	
protected:	
	friend class ServerApp;
	//friend class WinBorder;
	friend class Screen; 
//	friend class Layer;
	
	
	BString fTitle;
	int32 fType,fFlags;
	bool hidden;
	bool active;
	bool created;
	
	ServerApp *fServerApp;
	
	team_id fClientTeamID;
	thread_id fMonitorThreadID;

	port_id fMessagePort;
	port_id fClientWinPort;
	port_id fClientLooperPort;

	BRect fFrame;
	uint32 fToken;
	int32 fHandlerToken;
	
	BSession *fSession;
	
	IDirectFBWindow *window;
	IDirectFBFont *font;
	
	status_t CreateWindow (IDirectFBDisplayLayer *layer = AppServer::layer);
	status_t UpDateTitle (char *title);
};



void ActivateWindow(ServerWindow *oldwin,ServerWindow *newwin);

#endif
