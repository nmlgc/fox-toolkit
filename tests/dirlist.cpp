/********************************************************************************
*                                                                               *
*                  D i r e c t o r y   L i s t   C o n t r o l                  *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* $Id: dirlist.cpp,v 1.4 1998/08/31 23:32:55 jvz Exp $                       *
********************************************************************************/
#include "fx.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>




/*******************************************************************************/


// Mini application object
class FXDirListApp : public FXApp {
  FXDECLARE(FXDirListApp)
protected:
  FXMainWindow*      mainwindow;
  FXMenuBar*         menubar;
  FXMenuPane*        filemenu;
  FXMenuPane*        helpmenu;
  FXDirList*         contents;
public:
  long onCmdAbout(FXObject*,FXSelector,void*);
public:
  enum{
    ID_ABOUT=FXApp::ID_LAST,
    ID_LAST
    };
public:
  FXDirListApp();
  void create();
  };


  
/*******************************************************************************/
  
// Map
FXDEFMAP(FXDirListApp) FXDirListAppMap[]={
  FXMAPFUNC(SEL_COMMAND, FXDirListApp::ID_ABOUT, FXDirListApp::onCmdAbout),
  };


// Object implementation
FXIMPLEMENT(FXDirListApp,FXApp,FXDirListAppMap,ARRAYNUMBER(FXDirListAppMap))


// Make some windows
FXDirListApp::FXDirListApp(){
  
  // Make main window
  mainwindow=new FXMainWindow(this,"Directory List",DECOR_ALL,0,0,800,600);
  
  // Make menu bar
  menubar=new FXMenuBar(mainwindow,LAYOUT_FILL_X);
  filemenu=new FXMenuPane(this);
    new FXMenuCommand(filemenu,"&Quit",this,ID_QUIT,MENU_DEFAULT);
    new FXMenuTitle(menubar,"&File",filemenu);
  helpmenu=new FXMenuPane(this);
    new FXMenuCommand(helpmenu,"&About FOX...",this,ID_ABOUT,0);
    new FXMenuTitle(menubar,"&Help",helpmenu,LAYOUT_RIGHT);

  // Make contents
  contents=new FXDirList(mainwindow,NULL,0,HSCROLLING_OFF|TREELIST_ROOT_BOXES|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0);
  
  }
  


// About
long FXDirListApp::onCmdAbout(FXObject*,FXSelector,void*){
  showModalInformationBox(MBOX_OK,"About FOX","FOX is a really, really cool C++ library!");
  return 1;
  }


// Start
void FXDirListApp::create(){
  FXApp::create();
  mainwindow->show();
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){

  // Make application
  FXDirListApp* application=new FXDirListApp;
  
  // Open display
  application->init(argc,argv);

  // Create app  
  application->create();
  
  // Run
  application->run();
  }


