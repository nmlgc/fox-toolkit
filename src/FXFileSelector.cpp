/********************************************************************************
*                                                                               *
*                      F i l e   S e l e c t i o n   O b j e c t                *
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
* $Id: FXFileSelector.cpp,v 1.41 1998/10/22 22:43:45 jvz Exp $               *
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
#include "FXTextField.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXHorizontalFrame.h"
#include "FXMatrix.h"
#include "FXCanvas.h"
#include "FXShell.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXHeader.h"
#include "FXIconList.h"
#include "FXFileList.h"
#include "FXFileSelector.h"


/*
  To do:
  - Icons get created, but never deleted; who owns the icons?
  - Instead of all those enums in each window, we really DO need a global
    list of window messages (otherwise, they may conflict too easily!)
*/

/*******************************************************************************/

const unsigned char tbuplevel[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0xff,0xff,0x00,0xff,0xff,0xff,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x32,0x84,0x8f,0x79,0xc1,0xac,0x18,0xc4,0x90,0x22,0x3c,0xd0,
  0xb2,0x76,0x78,0x7a,0xfa,0x0d,0x56,0xe4,0x05,0x21,0x35,0x96,0xcc,0x29,0x62,0x92,
  0x76,0xa6,0x28,0x08,0x8e,0x35,0x5b,0x75,0x28,0xfc,0xba,0xf8,0x27,0xfb,0xf5,0x36,
  0x44,0xce,0xe5,0x88,0x44,0x14,0x00,0x00,0x3b
  };

const unsigned char tbnewfolder[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0xff,0xff,0x00,0xff,0xff,0xff,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x30,0x84,0x8f,0xa9,0x19,0xeb,0xbf,0x1a,0x04,0xf2,0x54,0x1a,
  0xb2,0x69,0x1c,0x05,0x31,0x80,0x52,0x67,0x65,0xe6,0xb9,0x51,0x54,0xc8,0x82,0xe2,
  0xf5,0xb5,0x2d,0xb9,0xba,0xf2,0xb8,0xd9,0xfa,0x55,0xcb,0x22,0xa3,0x9b,0x31,0x4e,
  0x44,0xde,0x24,0x51,0x00,0x00,0x3b
  };

const unsigned char tbbigicons[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x80,0xff,0xff,0xff,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x22,0x84,0x8f,0xa9,0xab,0xe1,0x9c,0x82,0x78,0xd0,0xc8,0x59,
  0xad,0xc0,0xd9,0xd1,0x0c,0x1a,0xc3,0x48,0x1e,0x5e,0x28,0x7d,0xd0,0x15,0x80,0xac,
  0x7b,0x86,0x21,0x59,0xca,0x46,0x01,0x00,0x3b
  };
  
const unsigned char tbdetails[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x27,0x84,0x8f,0xa9,0xbb,0xe1,0x01,0x5e,0x74,0xac,0x8a,0x8b,
  0xb3,0x16,0x75,0xf1,0x49,0x49,0x52,0xa7,0x7c,0x0f,0x24,0x52,0x64,0x62,0xa6,0xa8,
  0xba,0x1e,0x6d,0x48,0x43,0xb1,0x6c,0x9c,0xe0,0x7e,0x1b,0x05,0x00,0x3b
  };
  
  
const unsigned char tblist[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x80,0xff,0xff,0xff,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x2a,0x84,0x8f,0xa9,0x8b,0x11,0xea,0xa0,0x78,0xa3,0x82,0x30,
  0x41,0x1d,0x37,0x36,0xcf,0x84,0x22,0x03,0x1e,0xa5,0x81,0x51,0x56,0xaa,0xad,0xa7,
  0xf3,0x8c,0xf2,0x7c,0x76,0x92,0xca,0xb1,0x5b,0x17,0x9b,0xf5,0x6c,0x28,0x00,0x00,
  0x3b
  };
  
/*******************************************************************************/

// Map
FXDEFMAP(FXFileSelector) FXFileSelectorMap[]={
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_FILEFILTER,FXFileSelector::onCmdFilter),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_FILENAME,FXFileSelector::onCmdName),
  FXMAPFUNC(SEL_DOUBLECLICKED,FXFileSelector::ID_FILELIST,FXFileSelector::onCmdItemOpened),
  FXMAPFUNC(SEL_CLICKED,FXFileSelector::ID_FILELIST,FXFileSelector::onCmdItemSelected),
  };


// Object implementation
FXIMPLEMENT(FXFileSelector,FXPacker,FXFileSelectorMap,ARRAYNUMBER(FXFileSelectorMap))


// Separator item
FXFileSelector::FXFileSelector(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXPacker(p,opts,x,y,w,h){
  char dir[MAXPATHLEN+1];
  target=tgt;
  message=sel;
  FXHorizontalFrame *buttons=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  FXMatrix *fields=new FXMatrix(this,3,MATRIX_BY_COLUMNS|LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);
  new FXLabel(fields,"&File Name:",NULL,JUSTIFY_LEFT);
  filename=new FXTextField(fields,25,this,ID_FILENAME,LAYOUT_FILL_COLUMN|LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);
  accept=new FXButton(fields,"&Accept",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0,20,20);
  new FXLabel(fields,"File F&ilter:",NULL,JUSTIFY_LEFT);
  filefilter=new FXTextField(fields,25,this,ID_FILEFILTER,LAYOUT_FILL_COLUMN|LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);
  cancel=new FXButton(fields,"&Cancel",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0,20,20);
  FXHorizontalFrame *frame=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0,0,0,0,0);
  filebox=new FXFileList(frame,this,ID_FILELIST,ICONLIST_MINI_ICONS|ICONLIST_WANTSELECTION|ICONLIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(buttons,"Directory:",NULL);
  updiricon=new FXGIFIcon(getApp(),tbuplevel);
  newdiricon=new FXGIFIcon(getApp(),tbnewfolder);
  listicon=new FXGIFIcon(getApp(),tblist);
  iconsicon=new FXGIFIcon(getApp(),tbbigicons);
  detailicon=new FXGIFIcon(getApp(),tbdetails);
  new FXButton(buttons,"\tUp\tMove up to higher directory.",updiricon,filebox,FXFileList::ID_DIRECTORY_UP);
  new FXButton(buttons,"\tNew Directory\tCreate new directory.",newdiricon,filebox,FXFileList::ID_DIRECTORY_NEW);
  new FXButton(buttons,"\tList\tDisplay directory with small icons.",listicon,filebox,FXFileList::ID_SHOW_MINI_ICONS);
  new FXButton(buttons,"\tIcons\tDisplay directory with big icons.",iconsicon,filebox,FXFileList::ID_SHOW_BIG_ICONS);
  new FXButton(buttons,"\tDetails\tDisplay detailed directory listing.",detailicon,filebox,FXFileList::ID_SHOW_DETAILS);
  filefilter->setText("*");
  filebox->setPattern("*");
  fxabspath(dir,NULL,NULL);
  filebox->setDirectory(dir);
  filename->setText(dir);
  }


// Starts the timer
void FXFileSelector::create(){
  FXPacker::create();
  }


// Change the pattern
long FXFileSelector::onCmdFilter(FXObject*,FXSelector,void*){
  filebox->setPattern(filefilter->getText());
  return 1;
  }


// Change the name
long FXFileSelector::onCmdName(FXObject*,FXSelector,void*){
  FXchar dir[MAXPATHLEN+1];
  fxdirpart(dir,filename->getText());
  filebox->setDirectory(dir);
  return 1;
  }


// Folder item was opened
long FXFileSelector::onCmdItemOpened(FXObject*,FXSelector,void* ptr){
  FXchar path[MAXPATHLEN+1],name[2048],dir[MAXPATHLEN+1],*p;
  FXIconItem *item=(FXIconItem*)ptr;
  if(item){
    
    // If directory, open the directory
    if(filebox->isDirectory(item)){
      strcpy(path,filebox->getDirectory());
      strcpy(name,filebox->getItemText(item));
      if((p=strchr(name,'\t'))) *p=0;
//fprintf(stderr,"FXFileSelector dir=%s\n",path);
//fprintf(stderr,"FXFileSelector name=%s\n",name);
      fxpathname(dir,path,name);
      filename->setText(dir);
      filebox->setDirectory(dir);
      }
    
    // If regular file return as the selected file
    else if(filebox->isFile(item)){
      FXObject *tgt=accept->getTarget();
      FXSelector sel=accept->getSelector();
      if(tgt) tgt->handle(accept,MKUINT(sel,SEL_COMMAND),ptr);
      }
    }
  return 1;
  }


// File Item was selected
long FXFileSelector::onCmdItemSelected(FXObject*,FXSelector,void* ptr){
  FXchar path[MAXPATHLEN+1],name[MAXPATHLEN+1],dir[MAXPATHLEN+1],*p;
  FXIconItem *item=(FXIconItem*)ptr;
  name[0]=0;
  strcpy(path,filebox->getDirectory());
  if(item){
    strcpy(name,filebox->getItemText(item));
    if((p=strchr(name,'\t'))) *p=0;
    }
  fxpathname(dir,path,name);
  filename->setText(dir);
  return 1;
  }


// Set file name
void FXFileSelector::setFilename(const FXchar* path){
  FXchar abspath[MAXPATHLEN+1],dirpath[MAXPATHLEN+1];
  fxabspath(abspath,NULL,path);
  fxdirpart(dirpath,abspath);
  filebox->setDirectory(dirpath);
  filename->setText(abspath);
  }


// Get filename
const FXchar* FXFileSelector::getFilename() const {
  return filename->getText();
  }


// Will change...
void FXFileSelector::setPattern(const FXchar* ptrn){
  filefilter->setText(ptrn);
  filebox->setPattern(ptrn);
  }


// Will change...
const FXchar* FXFileSelector::getPattern() const {
  return filebox->getPattern();
  }


// Will change...
void FXFileSelector::setDirectory(const FXchar* path){
  FXchar abspath[MAXPATHLEN+1];
  fxabspath(abspath,NULL,path);
  filebox->setDirectory(abspath);
  filename->setText(abspath);
  }


// Will change...
const FXchar* FXFileSelector::getDirectory() const {
  return filebox->getDirectory();
  }


// Cleanup; icons must be explicitly deleted
FXFileSelector::~FXFileSelector(){
  delete updiricon;
  delete newdiricon;
  delete listicon;
  delete detailicon;
  delete iconsicon;
  }


