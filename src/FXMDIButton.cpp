/********************************************************************************
*                                                                               *
*             M u l t i p l e  D o c u m e n t   B u t t o n                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Library General Public                   *
* License as published by the Free Software Foundation; either                  *
* version 2 of the License, or (at your option) any later version.              *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Library General Public License for more details.                              *
*                                                                               *
* You should have received a copy of the GNU Library General Public             *
* License along with this library; if not, write to the Free                    *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            *
*********************************************************************************
* $Id: FXMDIButton.cpp,v 1.3 1998/09/09 22:07:02 jvz Exp $                    *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXFont.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXGIFIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXMenu.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXMDIButton.h"
#include "FXMDIChild.h"
#include "FXMDIClient.h"

  
/*******************************************************************************/



// Map
FXDEFMAP(FXMDIButton) FXMDIButtonMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMDIButton::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXMDIButton,FXButton,FXMDIButtonMap,ARRAYNUMBER(FXMDIButtonMap))



// Make minimize button
FXMDIButton::FXMDIButton(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXButton(p,NULL,NULL,tgt,sel,opts,x,y,w,h,3,3,2,2){
  }


FXint FXMDIButton::getDefaultWidth(){
  return padleft+padright+8+(border<<1);
  }


FXint FXMDIButton::getDefaultHeight(){
  return padtop+padbottom+8+(border<<1);
  }

  
// Handle repaint 
long FXMDIButton::onPaint(FXObject*,FXSelector,void*){
  FXint xx,yy;
  clearArea(0,0,width,height);
  drawFrame(0,0,width,height);
  xx=(width-8)/2;
  yy=(height-8)/2;
  if(state){ ++xx; ++yy; }
  if(isEnabled())
    setForeground(textColor);
  else
    setForeground(shadowColor);
  switch(options&MDIBTN_MASK){
    case MDIBTN_DELETE:
      drawLine(xx,  yy,  xx+8,yy+8);    // Fix these on non-broken X server
      drawLine(xx+1,yy,  xx+8,yy+7);
      drawLine(xx,  yy+1,xx+7,yy+8);
      drawLine(xx+8,yy,  xx,  yy+8);
      drawLine(xx+8,yy+1,xx+1,yy+8);
      drawLine(xx+7,yy,  xx,  yy+7);
      break;
    case MDIBTN_MINIMIZE:
      fillRectangle(xx,yy+6,8,2);
      break;
    case MDIBTN_MAXIMIZE:
      fillRectangle(xx,yy,8,2);
      drawRectangle(xx,yy,8,8);
      break;
    case MDIBTN_RESTORE:
      fillRectangle(xx+3,yy,6,2);
      drawRectangle(xx+3,yy,6,5);
      clearArea(xx,yy+3,6,5);
      fillRectangle(xx,yy+3,6,2);
      drawRectangle(xx,yy+3,6,5);
      break;
    }
  return 1;
  }


// Get MDI button style
FXuint FXMDIButton::getMDIButtonStyle() const {
  return (options&MDIBTN_MASK); 
  }


// Set MDI button style
void FXMDIButton::setMDIButtonStyle(FXuint style){
  FXuint opts=(options&~MDIBTN_MASK) | (style&MDIBTN_MASK);
  if(options!=opts){
    options=opts;
    update(0,0,width,height);
    }
  }

/*******************************************************************************/


FXIMPLEMENT(FXMDIMenu,FXMenuPane,NULL,0)


// Convenience constructor
FXMDIMenu::FXMDIMenu(FXApp* a,FXObject* tgt):FXMenuPane(a){
  new FXMenuCommand(this,"&Restore\t\tRestore window.",tgt,FXWindow::ID_RESTORE,0);
//   new FXMenuCommand(this,"&Move\t\tMove window.",NULL,0,0);
//   new FXMenuCommand(this,"&Resize\t\tResize window.",NULL,0,0);
  new FXMenuCommand(this,"&Minimize\t\tMinimize window.",tgt,FXWindow::ID_MINIMIZE,0);
  new FXMenuCommand(this,"&Next\t\tNext window.",tgt,FXWindow::ID_ACTIVATE_NEXT,0);
  new FXMenuCommand(this,"&Previous\t\tPrevious window.",tgt,FXWindow::ID_ACTIVATE_PREV,0);
  new FXMenuCommand(this,"&Maximize\t\tMaximize window.",tgt,FXWindow::ID_MAXIMIZE,0);
  new FXMenuCommand(this,"&Close\t\tClose window.",tgt,FXWindow::ID_DELETE,0);
  }

