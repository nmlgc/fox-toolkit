/********************************************************************************
*                                                                               *
*                    F i l e   S e l e c t i o n   D i a l o g                  *
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
* $Id: FXFileDialog.cpp,v 1.18 1998/10/22 22:43:44 jvz Exp $                 *
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
#include "FXButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXCanvas.h"
#include "FXShell.h"
#include "FXTopWindow.h"
#include "FXDialogBox.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXHeader.h"
#include "FXIconList.h"
#include "FXFileList.h"
#include "FXTextField.h"
#include "FXFileSelector.h"
#include "FXFileDialog.h"



/*
  To do:
  - Add API's for changing filters.
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXFileDialog,FXDialogBox,NULL,0)


// Separator item
FXFileDialog::FXFileDialog(FXApp* a,const char* name,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXDialogBox(a,name,opts|DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE,x,y,w,h,4,4){
  filebox=new FXFileSelector(this,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  filebox->acceptButton()->setTarget(this);
  filebox->acceptButton()->setSelector(FXDialogBox::ID_ACCEPT);
  filebox->cancelButton()->setTarget(this);
  filebox->cancelButton()->setSelector(FXDialogBox::ID_CANCEL);
  }


// Set file name
void FXFileDialog::setFilename(const FXchar* path){
  filebox->setFilename(path);
  }


// Get filename
const FXchar* FXFileDialog::getFilename() const {
  return filebox->getFilename();
  }


// Will change...
void FXFileDialog::setPattern(const FXchar* ptrn){
  filebox->setPattern(ptrn);
  }


// Will change...
const FXchar* FXFileDialog::getPattern() const {
  return filebox->getPattern();
  }


// Cleanup
FXFileDialog::~FXFileDialog(){
  filebox=(FXFileSelector*)-1;
  }


