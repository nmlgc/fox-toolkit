/********************************************************************************
*                                                                               *
*                           Test Icon List Widget                               *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* $Id: iconlist.cpp,v 1.19 1998/08/28 20:03:58 jvz Exp $                      *
********************************************************************************/
#include "fx.h"
#include <stdio.h>
#include <stdlib.h>




/*******************************************************************************/


// Mini application object
class IconListApp : public FXApp {
  FXDECLARE(IconListApp)
public:
  long onQuit(FXObject*,FXSelector,void*);
  long onCmdDirectory(FXObject*,FXSelector,void*);
  long onCmdPattern(FXObject*,FXSelector,void*);
protected:
  FXMainWindow*      mainwindow;
  FXMenuBar*         menubar;
  FXMenuPane*        filemenu;
  FXMenuPane*        arrangemenu;
  FXMenuPane*        sortmenu;
  FXSplitter*        splitter;
  FXVerticalFrame*   group1;
  FXVerticalFrame*   subgroup1;
  FXVerticalFrame*   group2;
  FXVerticalFrame*   subgroup2;
  FXDirList*         dirlist;
  FXFileList*        iconlist;
  FXTextField*       pattern;
public:
  enum{
    ID_QUIT=1,
    ID_DIRECTORY,
    ID_PATTERN,
    ID_LAST
    };
public:
  IconListApp();
  void create();
  void start();
  };


  
/*******************************************************************************/
  
// Map
FXDEFMAP(IconListApp) IconListAppMap[]={
  FXMAPFUNC(SEL_COMMAND,IconListApp::ID_QUIT,IconListApp::onQuit),
  FXMAPFUNC(SEL_COMMAND,IconListApp::ID_DIRECTORY,IconListApp::onCmdDirectory),
  FXMAPFUNC(SEL_COMMAND,IconListApp::ID_PATTERN,IconListApp::onCmdPattern),
  };


// Object implementation
FXIMPLEMENT(IconListApp,FXApp,IconListAppMap,ARRAYNUMBER(IconListAppMap))


// Make some windows
IconListApp::IconListApp(){
  
  // Main window
  mainwindow=new FXMainWindow(this,"Icon List Test",DECOR_ALL,0,0,800,600);
  
  // Menu bar
  menubar=new FXMenuBar(mainwindow,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  
  // File menu
  filemenu=new FXMenuPane(this);
  new FXMenuCommand(filemenu,"&Quit",this,ID_QUIT,MENU_DEFAULT);
  new FXMenuTitle(menubar,"&File",filemenu);
    
  // Status bar
  FXStatusbar *status=new FXStatusbar(mainwindow,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER);
  
  // Main window interior
  splitter=new FXSplitter(mainwindow,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  group1=new FXVerticalFrame(splitter,LAYOUT_FILL_Y|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
  group2=new FXVerticalFrame(splitter,LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);


  // Directories
  new FXLabel(group1,"Directories",NULL,LAYOUT_TOP|LAYOUT_FILL_X|FRAME_SUNKEN);
  subgroup1=new FXVerticalFrame(group1,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
  
  // Files
  new FXLabel(group2,"Files",NULL,LAYOUT_TOP|LAYOUT_FILL_X|FRAME_SUNKEN);
  subgroup2=new FXVerticalFrame(group2,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

  // Directory List on the left
  dirlist=new FXDirList(subgroup1,this,ID_DIRECTORY,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_RIGHT|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES);
    
  // Icon list on the right
  iconlist=new FXFileList(subgroup2,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y|ICONLIST_BIG_ICONS);

  pattern=new FXTextField(subgroup2,20,this,ID_PATTERN,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_BOTTOM);

  // Arrange menu
  arrangemenu=new FXMenuPane(this);
  new FXMenuCommand(arrangemenu,"&Details",iconlist,FXIconList::ID_SHOW_DETAILS);
  new FXMenuCommand(arrangemenu,"&Small Icons",iconlist,FXIconList::ID_SHOW_MINI_ICONS);
  new FXMenuCommand(arrangemenu,"&Big Icons",iconlist,FXIconList::ID_SHOW_BIG_ICONS);
  new FXMenuCommand(arrangemenu,"&Rows",iconlist,FXIconList::ID_ARRANGE_BY_ROWS);
  new FXMenuCommand(arrangemenu,"&Columns",iconlist,FXIconList::ID_ARRANGE_BY_COLUMNS);
  new FXMenuCommand(arrangemenu,"R&andom",iconlist,FXIconList::ID_ARRANGE_AT_RANDOM);
  new FXMenuTitle(menubar,"&Arrange",arrangemenu);
  
  // Sort menu
  sortmenu=new FXMenuPane(this);
  new FXMenuCommand(sortmenu,"&Name",iconlist,FXFileList::ID_SORT_BY_NAME);
  new FXMenuCommand(sortmenu,"&Type",iconlist,FXFileList::ID_SORT_BY_TYPE);
  new FXMenuCommand(sortmenu,"&Size",iconlist,FXFileList::ID_SORT_BY_SIZE);
  new FXMenuCommand(sortmenu,"T&ime",iconlist,FXFileList::ID_SORT_BY_TIME);
  new FXMenuCommand(sortmenu,"&User",iconlist,FXFileList::ID_SORT_BY_USER);
  new FXMenuCommand(sortmenu,"&Group",iconlist,FXFileList::ID_SORT_BY_GROUP);
  new FXMenuCommand(sortmenu,"&Reverse",iconlist,FXFileList::ID_SORT_REVERSE);
//  new FXMenuCommand(sortmenu,"Hide status",status,FXWindow::ID_HIDE);
//  new FXMenuCommand(sortmenu,"Show status",status,FXWindow::ID_SHOW);
  new FXMenuTitle(menubar,"&Sort",sortmenu);
  }
  

// Handle quitting
long IconListApp::onQuit(FXObject*,FXSelector,void*){
  if(MBOX_CLICKED_YES==showModalQuestionBox(MBOX_YES_NO,"Quiting IconList Application","Do you really want to quit the IconList Application?")){
    exit(0);
    }
  return 1;
  }


// Change the pattern
long IconListApp::onCmdPattern(FXObject*,FXSelector,void*){
  iconlist->setPattern(pattern->getText());
  return 1;
  }


// Change the directory
long IconListApp::onCmdDirectory(FXObject*,FXSelector,void* ptr){
  ////// Will all change /////
  FXDirItem *item=(FXDirItem*)ptr;
  FXchar path[2000];
  if(item){
    dirlist->itempath(item,path);
    iconlist->setDirectory(path);
    }
  return 1;
  }


// Make application
void IconListApp::create(){
  FXApp::create();
  }


// Start
void IconListApp::start(){
  create();
  mainwindow->show();
  run();
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){
  IconListApp* application=new IconListApp;
  application->init(argc,argv);
  application->start();
  }


