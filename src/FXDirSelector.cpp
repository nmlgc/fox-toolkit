/********************************************************************************
*                                                                               *
*              D i r e c t o r y   S e l e c t i o n   W i d g e t              *
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
* $Id: FXDirSelector.cpp,v 1.18 2002/02/05 03:59:27 fox Exp $                *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXFile.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXFont.h"
#include "FXGIFIcon.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXMenuButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXTopWindow.h"
#include "FXDialogBox.h"
#include "FXScrollbar.h"
#include "FXTextField.h"
#include "FXScrollArea.h"
#include "FXTreeList.h"
#include "FXTreeListBox.h"
#include "FXVerticalFrame.h"
#include "FXHorizontalFrame.h"
#include "FXDirList.h"
#include "FXList.h"
#include "FXListBox.h"
#include "FXDriveBox.h"
#include "FXDirSelector.h"


/*
  Notes:
  - Need a button to quickly hop to home directory.
  - Need a button to hop to current working directory.
  - Keep list of recently visited places.
  - Need button to hide/show hidden directories.
  - Need option to show files and directories, instead of only
    directories.
*/

/*******************************************************************************/

// Map
FXDEFMAP(FXDirSelector) FXDirSelectorMap[]={
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_DIRNAME,FXDirSelector::onCmdName),
  FXMAPFUNC(SEL_OPENED,FXDirSelector::ID_DIRLIST,FXDirSelector::onCmdOpened),
  FXMAPFUNC(SEL_COMMAND,FXDirSelector::ID_DRIVEBOX,FXDirSelector::onCmdDriveChanged),
  };


// Implementation
FXIMPLEMENT(FXDirSelector,FXPacker,FXDirSelectorMap,ARRAYNUMBER(FXDirSelectorMap))


// Make directory selector widget
FXDirSelector::FXDirSelector(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXPacker(p,opts,x,y,w,h){
  FXString currentdirectory=FXFile::getCurrentDirectory();
  target=tgt;
  message=sel;
  new FXLabel(this,"&Directory name:",NULL,JUSTIFY_LEFT|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  FXHorizontalFrame *contents=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
  FXVerticalFrame *leftside=new FXVerticalFrame(contents,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
  dirname=new FXTextField(leftside,25,this,ID_DIRNAME,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);
  FXVerticalFrame *buttons=new FXVerticalFrame(contents,PACK_UNIFORM_WIDTH,0,0,0,0, 0,0,0,0);
  accept=new FXButton(buttons,"&Accept",NULL,NULL,0,FRAME_RAISED|FRAME_THICK,0,0,0,0,20,20);
  cancel=new FXButton(buttons,"&Cancel",NULL,NULL,0,FRAME_RAISED|FRAME_THICK,0,0,0,0,20,20);
#ifndef WIN32
  drivebox=NULL;
#else
  drivebox=new FXDriveBox(leftside,5,this,ID_DRIVEBOX,FRAME_SUNKEN|FRAME_THICK|LAYOUT_SIDE_BOTTOM|LAYOUT_BOTTOM|LAYOUT_FILL_X,0,0,0,0);
  drivebox->setDrive(currentdirectory);
#endif
  FXHorizontalFrame *frame=new FXHorizontalFrame(leftside,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0,0,0,0,0);
  dirbox=new FXDirList(frame,0,this,ID_DIRLIST,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES|TREELIST_BROWSESELECT);
  dirbox->setDirectory(currentdirectory);
  dirname->setText(currentdirectory);
  dirbox->setFocus();
  }


// Set directory
void FXDirSelector::setDirectory(const FXString& path){
  if(drivebox) drivebox->setDrive(path);        // FXDriveBox will ignore if the second char is not ':'
  dirname->setText(path);
  dirbox->setDirectory(path);
  }


// Return directory
FXString FXDirSelector::getDirectory() const {
  return dirname->getText();
  }


// Change Directory List style
void FXDirSelector::setDirBoxStyle(FXuint style){
  dirbox->setListStyle(style);
  }


// Return Directory List style
FXuint FXDirSelector::getDirBoxStyle() const {
  return dirbox->getListStyle();
  }


// Typed in new directory name, open path in the tree
long FXDirSelector::onCmdName(FXObject*,FXSelector,void*){
  if(drivebox) drivebox->setDrive(dirname->getText());        // May have typed a new drive letter
  dirbox->setDirectory(dirname->getText());
  return 1;
  }


// Opened an item, making it the current one
long FXDirSelector::onCmdOpened(FXObject*,FXSelector,void* ptr){
  const FXTreeItem* item=(const FXTreeItem*)ptr;
  dirname->setText(dirbox->getItemPathname(item));
  return 1;
  }


// Changed the drive, making it the current one
long FXDirSelector::onCmdDriveChanged(FXObject*,FXSelector,void* ptr){
  if(drivebox){
    FXString path=FXString((char*)ptr);
#ifdef WIN32
    path.append(PATHSEPSTRING);        // Add the backslash to out drive letter
#endif
    dirname->setText(path);
    dirbox->setDirectory(path);
    }
  return 1;
  }


// Save data
void FXDirSelector::save(FXStream& store) const {
  FXPacker::save(store);
  store << dirbox;
  store << drivebox;
  store << dirname;
  store << accept;
  store << cancel;
  }


// Load data
void FXDirSelector::load(FXStream& store){
  FXPacker::load(store);
  store >> dirbox;
  store >> drivebox;
  store >> dirname;
  store >> accept;
  store >> cancel;
  }


// Clean up
FXDirSelector::~FXDirSelector(){
  dirbox=(FXDirList*)-1;
  drivebox=(FXDriveBox*)-1;
  dirname=(FXTextField*)-1;
  accept=(FXButton*)-1;
  cancel=(FXButton*)-1;
  }
