/********************************************************************************
*                                                                               *
*              T h e   P a t h F i n d e r   F i l e   B r o w s e r            *
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
* $Id: PathFinder.cpp,v 1.10 1998/10/22 16:22:23 jvz Exp $                    *
********************************************************************************/
#include "xincs.h"
#include "fx.h"
#include "FXDebugTarget.h"
#include "PathFinder.h"
#include "icons.h"
#include <stdio.h>
#include <stdlib.h>


  
/*******************************************************************************/

// Map
FXDEFMAP(PathFinderApp) PathFinderAppMap[]={
  FXMAPFUNC(SEL_COMMAND,PathFinderApp::ID_QUIT,PathFinderApp::onQuit),
  FXMAPFUNC(SEL_COMMAND,PathFinderApp::ID_DIRECTORY,PathFinderApp::onCmdDirectory),
  FXMAPFUNC(SEL_DOUBLECLICKED,PathFinderApp::ID_FILELIST,PathFinderApp::onCmdItemDblClicked),
  FXMAPFUNC(SEL_CLICKED,PathFinderApp::ID_FILELIST,PathFinderApp::onCmdItemClicked),
  FXMAPFUNC(SEL_COMMAND,PathFinderApp::ID_ABOUT,PathFinderApp::onCmdAbout),
  };


// Object implementation
FXIMPLEMENT(PathFinderApp,FXApp,PathFinderAppMap,ARRAYNUMBER(PathFinderAppMap))


/*******************************************************************************/
  
// Make some windows
PathFinderApp::PathFinderApp(){
  
  // Main window
  mainwindow=new FXMainWindow(this,"PathFinder",DECOR_ALL,0,0,800,600);
  
  // Menu bar
  menubar=new FXMenuBar(mainwindow,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  
  // File menu pane
  FXMenuPane* filemenu=new FXMenuPane(this);
  new FXMenuCommand(filemenu,"&New Folder",NULL,0);
  new FXMenuCommand(filemenu,"&Quit\t\tQuit PathFinder",this,ID_QUIT);
  new FXMenuTitle(menubar,"&File",filemenu);
    
  // Edit Menu Pane
  FXMenuPane* editmenu=new FXMenuPane(this);
  new FXMenuCommand(editmenu,"Select &All\tCtl-A\tSelect all icons");
  new FXMenuTitle(menubar,"&Edit",editmenu);
  
  // Separator
  new FXHorizontalSeparator(mainwindow,LAYOUT_SIDE_TOP|SEPARATOR_GROOVE|LAYOUT_FILL_X);
  
  // Toolbar
  toolbar=new FXHorizontalFrame(mainwindow,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, 4,4,0,0, 0,0);

  
  // Status bar
  FXStatusbar *status=new FXStatusbar(mainwindow,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER);
  
  // View Menu Pane
  FXMenuPane *viewmenu=new FXMenuPane(this);
  new FXMenuCommand(viewmenu,"&Toolbar\t\tShow or hide tool bar",toolbar,FXWindow::ID_TOGGLESHOWN);
  new FXMenuCommand(viewmenu,"&Statusbar\t\tShow or hide status bar",status,FXWindow::ID_TOGGLESHOWN);
  new FXMenuTitle(menubar,"&View",viewmenu);
  
  // Main window interior
  splitter=new FXSplitter(mainwindow,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  group1=new FXVerticalFrame(splitter,LAYOUT_FILL_Y|LAYOUT_FILL_Y, 0,0,180,0, 0,0,0,0);
  group2=new FXVerticalFrame(splitter,LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);


  // Directories
  new FXLabel(group1,"Directories",NULL,JUSTIFY_LEFT|LAYOUT_TOP|LAYOUT_FILL_X|FRAME_SUNKEN);
  subgroup1=new FXVerticalFrame(group1,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
  
  // Files
  new FXLabel(group2,"Files",NULL,JUSTIFY_LEFT|LAYOUT_TOP|LAYOUT_FILL_X|FRAME_SUNKEN);
  subgroup2=new FXVerticalFrame(group2,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

  // Directory List on the left
  dirlist=new FXDirList(subgroup1,this,ID_DIRECTORY,TREELIST_WANTSELECTION|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_RIGHT|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES);
    
  // Icon list on the right
  filelist=new FXFileList(subgroup2,this,ID_FILELIST,ICONLIST_WANTSELECTION|LAYOUT_FILL_X|LAYOUT_FILL_Y|ICONLIST_BIG_ICONS);

  pattern=new FXTextField(subgroup2,20,filelist,FXFileList::ID_SET_PATTERN,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_BOTTOM);

  // Add some toolbar buttons
  new FXButton(toolbar,"\tUp\tChange up one level.",new FXGIFIcon(this,dirup),NULL,0,FRAME_THICK|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Spacer  
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,0,0,4,20);
  
  // Switch display modes
  new FXButton(toolbar,"\tBig Icons\tShow big icons.",new FXGIFIcon(this,bigicons),filelist,FXIconList::ID_SHOW_BIG_ICONS,FRAME_THICK|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tSmall Icons\tShow small icons.",new FXGIFIcon(this,smallicons),filelist,FXIconList::ID_SHOW_MINI_ICONS,FRAME_THICK|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tDetails\tShow detail view.",new FXGIFIcon(this,details),filelist,FXIconList::ID_SHOW_DETAILS,FRAME_THICK|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  
  // Subtle plug for LINUX
  new FXButton(toolbar,"\tHello, I'm Tux...\nThe symbol for the Linux Operating System.\nAnd all it stands for.\tLinux:- the alternative operating system.",new FXGIFIcon(this,minipenguin),this,PathFinderApp::ID_ABOUT,LAYOUT_TOP|LAYOUT_RIGHT);
  
  // Arrange menu
  FXMenuPane* arrangemenu=new FXMenuPane(this);
  new FXMenuCommand(arrangemenu,"&Details\t\tShow detail view.",filelist,FXIconList::ID_SHOW_DETAILS);
  new FXMenuCommand(arrangemenu,"&Small Icons\t\tShow small icons.",filelist,FXIconList::ID_SHOW_MINI_ICONS);
  new FXMenuCommand(arrangemenu,"&Big Icons\t\tShow big icons.",filelist,FXIconList::ID_SHOW_BIG_ICONS);
  new FXMenuCommand(arrangemenu,"&Rows\t\tView row-wise.",filelist,FXIconList::ID_ARRANGE_BY_ROWS);
  new FXMenuCommand(arrangemenu,"&Columns\t\tView column-wise.",filelist,FXIconList::ID_ARRANGE_BY_COLUMNS);
  new FXMenuCommand(arrangemenu,"R&andom\t\tView random placement.",filelist,FXIconList::ID_ARRANGE_AT_RANDOM);
  new FXMenuTitle(menubar,"&Arrange",arrangemenu);
  
  // Sort menu
  FXMenuPane* sortmenu=new FXMenuPane(this);
  new FXMenuCommand(sortmenu,"&Name\t\tSort by file name.",filelist,FXFileList::ID_SORT_BY_NAME);
  new FXMenuCommand(sortmenu,"&Type\t\tSort by file type.",filelist,FXFileList::ID_SORT_BY_TYPE);
  new FXMenuCommand(sortmenu,"&Size\t\tSort by file size.",filelist,FXFileList::ID_SORT_BY_SIZE);
  new FXMenuCommand(sortmenu,"T&ime\t\tSort by modification time.",filelist,FXFileList::ID_SORT_BY_TIME);
  new FXMenuCommand(sortmenu,"&User\t\tSort by user name.",filelist,FXFileList::ID_SORT_BY_USER);
  new FXMenuCommand(sortmenu,"&Group\t\tSort by group name.",filelist,FXFileList::ID_SORT_BY_GROUP);
  new FXMenuCommand(sortmenu,"&Reverse\t\tReverse sort direction.",filelist,FXFileList::ID_SORT_REVERSE);
  new FXMenuTitle(menubar,"&Sort",sortmenu);
  
  // Help menu
  FXMenuPane* helpmenu=new FXMenuPane(this);
  new FXMenuTitle(menubar,"&Help",helpmenu,LAYOUT_RIGHT);
  new FXMenuCommand(helpmenu,"&About PathFinder...\t\tDisplay PathFinder About Panel.",this,ID_ABOUT,0);
  
  // Make a tool tip
  new FXTooltip(this,0);
  }
  

// Handle quitting
long PathFinderApp::onQuit(FXObject*,FXSelector,void*){
  if(MBOX_CLICKED_YES==showModalQuestionBox(MBOX_YES_NO,"Quiting PathFinder","Do you really want to quit PathFinder?")){
    exit(0);
    }
  return 1;
  }


// Change the directory
long PathFinderApp::onCmdDirectory(FXObject*,FXSelector,void* ptr){
  ////// Will all change /////
  FXDirItem *item=(FXDirItem*)ptr;
  FXchar path[MAXPATHLEN+1];
  if(item){
    dirlist->openItem((FXTreeItem*)item);
    dirlist->itempath(item,path);
    filelist->setDirectory(path);
    }
  return 1;
  }


// Folder item was opened
long PathFinderApp::onCmdItemDblClicked(FXObject*,FXSelector,void* ptr){
  FXchar path[MAXPATHLEN+1],name[2048],dir[MAXPATHLEN+1],*p;
  FXIconItem *item=(FXIconItem*)ptr;
  if(item){
    strcpy(name,filelist->getItemText(item));
    if((p=strchr(name,'\t'))) *p=0;
    strcpy(path,filelist->getDirectory());
    fxpathname(dir,path,name);
    
    // If directory, open the directory
    if(filelist->isDirectory(item)){
//fprintf(stderr,"FXFileSelector dir=%s\n",path);
//fprintf(stderr,"FXFileSelector name=%s\n",name);
      filelist->setDirectory(dir);
      //dirlist->???
      }
    
    // If regular file return as the selected file
    else if(filelist->isFile(item)){
      FXchar buf[1000];
      sprintf(buf,"nc -noask %s&",dir);
      system(buf);
      }
    }
  return 1;
  }


// File Item was selected
long PathFinderApp::onCmdItemClicked(FXObject*,FXSelector,void* ptr){
//   FXchar path[MAXPATHLEN+1],name[MAXPATHLEN+1],dir[MAXPATHLEN+1],*p;
//   FXIconItem *item=(FXIconItem*)ptr;
//   name[0]=0;
//   strcpy(path,filelist->getDirectory());
//   if(item){
//     strcpy(name,filelist->getItemText(item));
//     if((p=strchr(name,'\t'))) *p=0;
//     }
//   fxpathname(dir,path,name);
//   filename->setText(dir);
  return 1;
  }



// About
long PathFinderApp::onCmdAbout(FXObject*,FXSelector,void*){
  FXGIFIcon icon(this,bigpenguin);
  FXMessageBox about(this,"About PathFinder","PathFinder File Browser V0.1 (beta)\n\nUsing the FOX C++ GUI Library (http://cyberia.cfdrc.com/FOX/fox.html)\n\nCopyright (C) 1998 Jeroen van der Zijp (jvz@cfdrc.com)",&icon,MBOX_OK|DECOR_TITLE|DECOR_BORDER|DIALOG_MODAL);
  about.execute();  
  return 1;
  }


// Make application
void PathFinderApp::create(){
  FXApp::create();
  }


// Start
void PathFinderApp::start(){
  create();
  mainwindow->show();
  run();
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){
  PathFinderApp* application=new PathFinderApp;
  application->init(argc,argv);
  application->start();
  }


