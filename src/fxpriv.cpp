/********************************************************************************
*                                                                               *
*              P r i v a t e   I n t e r n a l   F u n c t i o n s              *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: fxpriv.cpp,v 1.16 2002/01/18 22:43:08 jeroen Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxpriv.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXObject.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXWindow.h"


/*
  Notes:
*/


/*******************************************************************************/

// X11
#ifndef WIN32


// Send request for selection info
Atom fxsendrequest(Display *display,Window window,Atom selection,Atom prop,Atom type,FXuint time){
  FXuint loops=1000;
  XEvent ev;
  XConvertSelection(display,selection,type,prop,window,time);
  while(!XCheckTypedWindowEvent(display,window,SelectionNotify,&ev)){
    if(loops==0){ fxwarning("timed out\n"); return None; }
    fxsleep(10000);     // Don't burn too much CPU here:- the other guy needs it more....
    loops--;
    }
  return ev.xselection.property;
  }


// Reply to request for selection info
Atom fxsendreply(Display *display,Window window,Atom selection,Atom prop,Atom target,FXuint time){
  XEvent se;
  se.xselection.type=SelectionNotify;
  se.xselection.send_event=TRUE;
  se.xselection.display=display;
  se.xselection.requestor=window;
  se.xselection.selection=selection;
  se.xselection.target=target;
  se.xselection.property=prop;
  se.xselection.time=time;
  XSendEvent(display,window,True,NoEventMask,&se);
  XFlush(display);
  return prop;
  }


// Send types via property
Atom fxsendtypes(Display *display,Window window,Atom prop,FXDragType* types,FXuint numtypes){
  if(types && numtypes){
    XChangeProperty(display,window,prop,XA_ATOM,32,PropModeReplace,(unsigned char*)types,numtypes);
    return prop;
    }
  return None;
  }


// Send data via property
Atom fxsenddata(Display *display,Window window,Atom prop,Atom type,FXuchar* data,FXuint size){
  unsigned long maxtfrsize,tfrsize,tfroffset;
  int mode;
  if(data && size){
    maxtfrsize=4*XMaxRequestSize(display);
    mode=PropModeReplace;
    tfroffset=0;
    while(size){
      tfrsize=size;
      if(tfrsize>maxtfrsize) tfrsize=maxtfrsize;
      XChangeProperty(display,window,prop,type,8,mode,&data[tfroffset],tfrsize);
      mode=PropModeAppend;
      tfroffset+=tfrsize;
      size-=tfrsize;
      }
    return prop;
    }
  return None;
  }


// Read type list from property
Atom fxrecvtypes(Display *display,Window window,Atom prop,FXDragType*& types,FXuint& numtypes){
  unsigned long numitems,bytesleft;
  unsigned char *ptr;
  int actualformat;
  Atom actualtype;
  types=NULL;
  numtypes=0;
  if(prop){
    if(XGetWindowProperty(display,window,prop,0,1024,TRUE,XA_ATOM,&actualtype,&actualformat,&numitems,&bytesleft,&ptr)==Success){
      if(actualtype==XA_ATOM && actualformat==32 && numitems>0){
        if(FXMALLOC(&types,Atom,numitems)){
          memcpy(types,ptr,sizeof(Atom)*numitems);
          numtypes=numitems;
          }
        }
      XFree(ptr);
      }
    return prop;
    }
  return None;
  }


// Receive data via property
Atom fxrecvdata(Display *display,Window window,Atom prop,Atom,FXuchar*& data,FXuint& size){
  unsigned long  maxtfrsize,tfrsize,tfroffset,bytes_after;
  unsigned char *ptr;
  Atom actualtype;
  int actualformat;
  data=NULL;
  size=0;
  if(prop){
    maxtfrsize=4*XMaxRequestSize(display);
    if(XGetWindowProperty(display,window,prop,0,0,False,AnyPropertyType,&actualtype,&actualformat,&tfrsize,&bytes_after,&ptr)==Success){
      if(ptr) XFree(ptr);
      if(FXMALLOC(&data,FXuchar,bytes_after+1)){    // One extra byte!
        size=bytes_after;
        tfroffset=0;
        while(bytes_after){
          if(XGetWindowProperty(display,window,prop,tfroffset>>2,maxtfrsize>>2,False,AnyPropertyType,&actualtype,&actualformat,&tfrsize,&bytes_after,&ptr)!=Success) break;
          tfrsize*=(actualformat>>3);
          if(tfroffset+tfrsize>size){ tfrsize=size-tfroffset; bytes_after=0; }
          memcpy(&data[tfroffset],ptr,tfrsize);
          tfroffset+=tfrsize;
          XFree(ptr);
          }
        XDeleteProperty(display,window,prop);
        size=tfroffset;
        data[size]='\0';                            // Append a '\0' just past the end!
        }
      }
    return prop;
    }
  return None;
  }


/*******************************************************************************/


// Change PRIMARY selection data
void FXApp::selectionSetData(FXWindow*,FXDragType,FXuchar* data,FXuint size){
  FXFREE(&ddeData);
  ddeData=data;
  ddeSize=size;
  }


// Retrieve PRIMARY selection data
void FXApp::selectionGetData(FXWindow* window,FXDragType type,FXuchar*& data,FXuint& size){
  FXID answer;
  data=NULL;
  size=0;
  if(selectionWindow){
    event.type=SEL_SELECTION_REQUEST;
    event.target=type;
    ddeData=NULL;
    ddeSize=0;
    selectionWindow->handle(this,MKUINT(0,SEL_SELECTION_REQUEST),&event);
    data=ddeData;
    size=ddeSize;
    ddeData=NULL;
    ddeSize=0;
    FXTRACE((100,"Window %d requested SELECTION DATA of type %d from local; got %d bytes\n",window->id(),type,size));
    }
  else{
    FXTRACE((100,"Sending SELECTION request from %d\n",window->id()));
    answer=fxsendrequest((Display*)display,window->id(),XA_PRIMARY,ddeAtom,type,event.time);
    fxrecvdata((Display*)display,window->id(),answer,type,data,size);
    FXTRACE((100,"Window %d requested SELECTION DATA of type %d from remote; got %d bytes\n",window->id(),type,size));
    }
  }


// Retrieve PRIMARY selection types
void FXApp::selectionGetTypes(FXWindow* window,FXDragType*& types,FXuint& numtypes){
  FXID answer;
  types=NULL;
  numtypes=0;
  if(selectionWindow){
    FXMEMDUP(&types,FXDragType,xselTypeList,xselNumTypes);
    numtypes=xselNumTypes;
    }
  else{
    answer=fxsendrequest((Display*)display,window->id(),XA_PRIMARY,ddeAtom,ddeTargets,event.time);
    fxrecvtypes((Display*)display,window->id(),answer,types,numtypes);
    }
  }


/*******************************************************************************/



// Change CLIPBOARD selection data
void FXApp::clipboardSetData(FXWindow*,FXDragType,FXuchar* data,FXuint size){
  FXFREE(&ddeData);
  ddeData=data;
  ddeSize=size;
  }


// Retrieve CLIPBOARD selection data
void FXApp::clipboardGetData(FXWindow* window,FXDragType type,FXuchar*& data,FXuint& size){
  FXID answer;
  data=NULL;
  size=0;
  if(clipboardWindow){
    event.type=SEL_CLIPBOARD_REQUEST;
    event.target=type;
    ddeData=NULL;
    ddeSize=0;
    clipboardWindow->handle(this,MKUINT(0,SEL_CLIPBOARD_REQUEST),&event);
    data=ddeData;
    size=ddeSize;
    ddeData=NULL;
    ddeSize=0;
    FXTRACE((100,"Window %d requested CLIPBOARD DATA of type %d from local; got %d bytes\n",window->id(),type,size));
    }
  else{
    FXTRACE((100,"Sending CLIPBOARD request from %d\n",window->id()));
    answer=fxsendrequest((Display*)display,window->id(),xcbSelection,ddeAtom,type,event.time);
    fxrecvdata((Display*)display,window->id(),answer,type,data,size);
    FXTRACE((100,"Window %d requested CLIPBOARD DATA of type %d from remote; got %d bytes\n",window->id(),type,size));
    }
  }


// Retrieve CLIPBOARD selection types
void FXApp::clipboardGetTypes(FXWindow* window,FXDragType*& types,FXuint& numtypes){
  FXID answer;
  types=NULL;
  numtypes=0;
  if(clipboardWindow){
    FXMEMDUP(&types,FXDragType,xcbTypeList,xcbNumTypes);
    numtypes=xselNumTypes;
    }
  else{
    answer=fxsendrequest((Display*)display,window->id(),xcbSelection,ddeAtom,ddeTargets,event.time);
    fxrecvtypes((Display*)display,window->id(),answer,types,numtypes);
    }
  }


/*******************************************************************************/


// Change DND selection data
void FXApp::dragdropSetData(FXWindow*,FXDragType,FXuchar* data,FXuint size){
  FXFREE(&ddeData);
  ddeData=data;
  ddeSize=size;
  }


// Retrieve DND selection data
void FXApp::dragdropGetData(FXWindow* window,FXDragType type,FXuchar*& data,FXuint& size){
  FXID answer;
  data=NULL;
  size=0;
  if(dragWindow){
    event.type=SEL_DND_REQUEST;
    event.target=type;
    ddeData=NULL;
    ddeSize=0;
    dragWindow->handle(this,MKUINT(0,SEL_DND_REQUEST),&event);
    data=ddeData;
    size=ddeSize;
    ddeData=NULL;
    ddeSize=0;
    FXTRACE((100,"Window %d requested XDND DATA of type %d from local; got %d bytes\n",window->id(),type,size));
    }
  else{
    FXTRACE((100,"Sending XDND request from %d\n",window->id()));
    answer=fxsendrequest((Display*)display,window->id(),xdndSelection,ddeAtom,type,event.time);
    fxrecvdata((Display*)display,window->id(),answer,type,data,size);
    FXTRACE((100,"Window %d requested XDND DATA of type %d from remote; got %d bytes\n",window->id(),type,size));
    }
  }


// Retrieve DND selection types
void FXApp::dragdropGetTypes(FXWindow*,FXDragType*& types,FXuint& numtypes){
  FXMEMDUP(&types,FXDragType,ddeTypeList,ddeNumTypes);
  numtypes=ddeNumTypes;
  }

/*******************************************************************************/

// MSWIN

#else

// Send data via shared memory
HANDLE fxsenddata(HWND window,FXuchar* data,FXuint size){
  HANDLE hMap,hMapCopy;
  FXuchar *ptr;
  DWORD processid;
  HANDLE process;

  if(data && size){
    hMap=CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,size+sizeof(FXuint),"_FOX_DDE");
    if(hMap){
      ptr=(FXuchar*)MapViewOfFile((HANDLE)hMap,FILE_MAP_WRITE,0,0,size+sizeof(FXuint));
      if(ptr){
        *((FXuint*)ptr)=size;
        memcpy(ptr+sizeof(FXuint),data,size);
        UnmapViewOfFile(ptr);
        }
      GetWindowThreadProcessId((HWND)window,&processid);
      process=OpenProcess(PROCESS_DUP_HANDLE,TRUE,processid);
      DuplicateHandle(GetCurrentProcess(),hMap,process,&hMapCopy,FILE_MAP_ALL_ACCESS,TRUE,DUPLICATE_CLOSE_SOURCE|DUPLICATE_SAME_ACCESS);
      CloseHandle(process);
      }
    return hMapCopy;
    }
  return 0;
  }


// Receive data via shared memory
HANDLE fxrecvdata(HANDLE hMap,FXuchar*& data,FXuint& size){
  FXuchar *ptr;
  data=NULL;
  size=0;
  if(hMap){
    ptr=(FXuchar*)MapViewOfFile(hMap,FILE_MAP_READ,0,0,0);
    if(ptr){
      size=*((FXuint*)ptr);
      if(FXMALLOC(&data,FXuchar,size)){
        memcpy(data,ptr+sizeof(FXuint),size);
        }
      UnmapViewOfFile(ptr);
      }
    CloseHandle(hMap);
    return hMap;
    }
  return 0;
  }


// Send request for data
HANDLE fxsendrequest(HWND window,HWND requestor,WPARAM type){
  FXuint loops=1000;
  MSG msg;
  PostMessage((HWND)window,WM_DND_REQUEST,type,(LPARAM)requestor);
  while(!PeekMessage(&msg,NULL,WM_DND_REPLY,WM_DND_REPLY,PM_REMOVE)){
    if(loops==0){ fxwarning("timed out\n"); return 0; }
    fxsleep(10000);     // Don't burn too much CPU here:- the other guy needs it more....
    loops--;
    }
  return (HANDLE)msg.wParam;
  }


/*******************************************************************************/


// Change PRIMARY selection data
void FXApp::selectionSetData(FXWindow*,FXDragType,FXuchar* data,FXuint size){
  FXFREE(&ddeData);
  ddeData=data;
  ddeSize=size;
  }


// Retrieve PRIMARY selection data
void FXApp::selectionGetData(FXWindow* window,FXDragType type,FXuchar*& data,FXuint& size){
  data=NULL;
  size=0;
  if(selectionWindow){
    event.type=SEL_SELECTION_REQUEST;
    event.target=type;
    ddeData=NULL;
    ddeSize=0;
    selectionWindow->handle(this,MKUINT(0,SEL_SELECTION_REQUEST),&event);
    data=ddeData;
    size=ddeSize;
    ddeData=NULL;
    ddeSize=0;
    FXTRACE((100,"Window %d requested SELECTION DATA of type %d from local; got %d bytes\n",window->id(),type,size));
    }
  }



// Retrieve PRIMARY selection types
void FXApp::selectionGetTypes(FXWindow* window,FXDragType*& types,FXuint& numtypes){
  types=NULL;
  numtypes=0;
  if(selectionWindow){
    FXMEMDUP(&types,FXDragType,xselTypeList,xselNumTypes);
    numtypes=xselNumTypes;
    FXTRACE((100,"Window %d requested SELECTION TYPES of from remote; got %d types\n",window->id(),numtypes));
    }
  }

/*******************************************************************************/




// Change CLIPBOARD selection data
void FXApp::clipboardSetData(FXWindow*,FXDragType type,FXuchar* data,FXuint size){
  HGLOBAL hGlobalMemory=GlobalAlloc(GMEM_MOVEABLE,size);
  if(hGlobalMemory){
    void *pGlobalMemory=GlobalLock(hGlobalMemory);
    FXASSERT(pGlobalMemory);
    memcpy((FXchar*)pGlobalMemory,data,size);
    GlobalUnlock(hGlobalMemory);
    SetClipboardData(type,hGlobalMemory);
    FXFREE(&data);
    }
  }


// Retrieve CLIPBOARD selection data
void FXApp::clipboardGetData(FXWindow* window,FXDragType type,FXuchar*& data,FXuint& size){
  data=NULL;
  size=0;
  if(IsClipboardFormatAvailable(type)){
    if(OpenClipboard((HWND)window->id())){
      HANDLE hClipMemory=GetClipboardData(type);
      if(hClipMemory){
        size=GlobalSize(hClipMemory);
        if(FXMALLOC(&data,FXuchar,size)){
          void *pClipMemory=GlobalLock(hClipMemory);
          FXASSERT(pClipMemory);
          memcpy((void*)data,pClipMemory,size);
          GlobalUnlock(hClipMemory);
          CloseClipboard();
          FXTRACE((100,"Window %d requested CLIPBOARD DATA of type %d from remote; got %d bytes\n",window->id(),type,size));
          }
        }
      }
    }
  }



// Retrieve CLIPBOARD selection types
void FXApp::clipboardGetTypes(FXWindow* window,FXDragType*& types,FXuint& numtypes){
  FXuint count;
  types=NULL;
  numtypes=0;
  if(OpenClipboard((HWND)window->id())){
    count=CountClipboardFormats();
    if(count){
      FXMALLOC(&types,FXDragType,count);
      UINT format=0;
      while(numtypes<count && (format=EnumClipboardFormats(format))!=0){
        types[numtypes++]=format;
        }
      FXTRACE((100,"Window %d requested CLIPBOARD TYPES of from remote; got %d types\n",window->id(),numtypes));
      }
    CloseClipboard();
    }
  }

/*******************************************************************************/



// Change DND selection data
void FXApp::dragdropSetData(FXWindow*,FXDragType,FXuchar* data,FXuint size){
  FXFREE(&ddeData);
  ddeData=data;
  ddeSize=size;
  }


// Retrieve DND selection data
void FXApp::dragdropGetData(FXWindow* window,FXDragType type,FXuchar*& data,FXuint& size){
  HANDLE answer;
  data=NULL;
  size=0;
  if(dragWindow){
    event.type=SEL_DND_REQUEST;
    event.target=type;
    ddeData=NULL;
    ddeSize=0;
    dragWindow->handle(this,MKUINT(0,SEL_DND_REQUEST),&event);
    data=ddeData;
    size=ddeSize;
    ddeData=NULL;
    ddeSize=0;
    FXTRACE((100,"Window %d requested XDND DATA of type %d from local; got %d bytes\n",window->id(),type,size));
    }
  else{
    FXTRACE((100,"Sending request to %d\n",xdndSource));
    answer=fxsendrequest((HWND)xdndSource,(HWND)window->id(),(WPARAM)type);
    fxrecvdata(answer,data,size);
    FXTRACE((100,"Window %d requested XDND DATA of type %d from remote; got %d bytes\n",window->id(),type,size));
    }
  }


// Retrieve DND selection types
void FXApp::dragdropGetTypes(FXWindow*,FXDragType*& types,FXuint& numtypes){
  FXMEMDUP(&types,FXDragType,ddeTypeList,ddeNumTypes);
  numtypes=ddeNumTypes;
  }



#endif
