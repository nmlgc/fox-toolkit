/********************************************************************************
*                                                                               *
*                         M e s s a g e   B o x e s                             *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXMessageBox.cpp,v 1.12 1998/08/14 22:00:04 jvz Exp $                  *
********************************************************************************/
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXHorizontalFrame.h"
#include "FXVerticalFrame.h"
#include "FXShell.h"
#include "FXTopWindow.h"
#include "FXDialogBox.h"
#include "FXMessageBox.h"


/*
  Notes:
  - Want justify modes for the message box so the label can be aligned 
    different ways.
*/

// Padding for message box buttons
#define HORZ_PAD 30
#define VERT_PAD 2


/*******************************************************************************/

// Map
FXDEFMAP(FXMessageBox) FXMessageBoxMap[]={
  FXMAPFUNCS(SEL_COMMAND,FXMessageBox::ID_CLICKED_YES,FXMessageBox::ID_CLICKED_SAVE,FXMessageBox::onCmdClicked),
  };



// Object implementation
FXIMPLEMENT(FXMessageBox,FXDialogBox,FXMessageBoxMap,ARRAYNUMBER(FXMessageBoxMap))


// Create message box
FXMessageBox::FXMessageBox(FXApp* a,const char* name,const char* text,FXIcon* ic,FXuint opts,FXint x,FXint y):
  FXDialogBox(a,name,opts|DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE,x,y,0,0){
  FXuint btns=(opts&MBOX_BUTTON_MASK);
  FXVerticalFrame* content=new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame* info=new FXHorizontalFrame(content,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10);
  new FXLabel(info,NULL,ic,ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(info,text,NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXHorizontalSeparator(content,SEPARATOR_GROOVE|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);
  FXHorizontalFrame* buttons=new FXHorizontalFrame(content,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,10,10,10,10);
  if(btns==MBOX_OK){
    new FXButton(buttons,"&OK",NULL,this,ID_CLICKED_OK,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    }
  else if(btns==MBOX_OK_CANCEL){
    new FXButton(buttons,"&OK",NULL,this,ID_CLICKED_OK,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,"&Cancel",NULL,this,ID_CLICKED_CANCEL,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    }
  else if(btns==MBOX_YES_NO){
    new FXButton(buttons,"&Yes",NULL,this,ID_CLICKED_YES,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,"&No",NULL,this,ID_CLICKED_NO,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    }
  else if(btns==MBOX_YES_NO_CANCEL){
    new FXButton(buttons,"&Yes",NULL,this,ID_CLICKED_YES,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,"&No",NULL,this,ID_CLICKED_NO,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,"&Cancel",NULL,this,ID_CLICKED_CANCEL,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    }
  else if(btns==MBOX_QUIT_CANCEL){
    new FXButton(buttons,"&Quit",NULL,this,ID_CLICKED_QUIT,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,"&Cancel",NULL,this,ID_CLICKED_CANCEL,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    }
  else if(btns==MBOX_QUIT_SAVE_CANCEL){
    new FXButton(buttons,"&Quit",NULL,this,ID_CLICKED_QUIT,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,"&Save",NULL,this,ID_CLICKED_SAVE,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,"&Cancel",NULL,this,ID_CLICKED_CANCEL,FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    }
  }


// Close dialog with a cancel
long FXMessageBox::onCmdClicked(FXObject*,FXSelector sel,void*){
  hide();
  state=SELID(sel)-ID_CLICKED_YES+MBOX_CLICKED_YES;
  return 1;
  }

