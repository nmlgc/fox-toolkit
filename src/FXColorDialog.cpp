/********************************************************************************
*                                                                               *
*                           C o l o r   D i a l o g                             *
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
* $Id: FXColorDialog.cpp,v 1.8 1998/09/03 05:01:25 jeroen Exp $                  *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "fxkeys.h"
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
#include "FXColorWell.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXCanvas.h"
#include "FXShell.h"
#include "FXTopWindow.h"
#include "FXDialogBox.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXList.h"
#include "FXTextField.h"
#include "FXSlider.h"
#include "FXColorSelector.h"
#include "FXColorDialog.h"


/*
  To do:
  - Should we also have a color selector?
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXColorDialog,FXDialogBox,NULL,0)


// Separator item
FXColorDialog::FXColorDialog(FXApp* a,const char* name,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXDialogBox(a,name,opts|DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE,x,y,w,h,4,4){
  colorbox=new FXColorSelector(this,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  colorbox->acceptButton()->setTarget(this);
  colorbox->acceptButton()->setSelector(FXDialogBox::ID_ACCEPT);
  colorbox->cancelButton()->setTarget(this);
  colorbox->cancelButton()->setSelector(FXDialogBox::ID_CANCEL);
  }


// Change RGBA color 
void FXColorDialog::setRGBA(FXColor clr){
  colorbox->setRGBA(clr);
  }


// Retrieve RGBA color
FXColor FXColorDialog::getRGBA() const {
  return colorbox->getRGBA();
  }

// Cleanup
FXColorDialog::~FXColorDialog(){
  colorbox=(FXColorSelector*)-1;
  }


