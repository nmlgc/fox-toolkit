/********************************************************************************
*                                                                               *
*                                 Test MDI Widgets                              *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* $Id: mditest.cpp,v 1.9 1998/09/21 18:45:55 jvz Exp $                        *
********************************************************************************/
#include "fx.h"
#include <stdio.h>
#include <stdlib.h>



/*******************************************************************************/


// Mini application object
class FXMDIApp : public FXApp {
  FXDECLARE(FXMDIApp)
    
protected:
  FXMainWindow      *mainwindow;
  FXMenuBar         *menubar;
  FXMDIClient       *mdiclient;               // MDI Client area
  FXMDIMenu         *mdimenu;                 // MDI Window Menu
  FXGIFIcon         *mdiicon;                 // MDI Icon
  
public:
  
  // We define additional ID's, starting from the last one used by the base class+1.
  // This way, we know the ID's are all unique for this particular target.
  enum {
    ID_ABOUT=FXApp::ID_LAST,
    ID_NEW,
    };
    
  // Message handlers
  long onCmdAbout(FXObject*,FXSelector,void*);
  long onCmdNew(FXObject*,FXSelector,void*);
  
public:
  FXMDIApp();
  void create();
  };


  
/*******************************************************************************/
  
  
const unsigned char penguin[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x12,0x00,0xf2,0x00,0x00,0xb2,0xc0,0xdc,
  0x80,0x80,0x80,0x00,0x00,0x00,0xc0,0xc0,0xc0,0x10,0x10,0x10,0xff,0xff,0xff,0xe0,
  0xa0,0x08,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x12,0x00,0x00,0x03,
  0x53,0x08,0xba,0x21,0x12,0x2b,0xc6,0xe6,0x9e,0x94,0x62,0x64,0x77,0xa3,0x20,0x4e,
  0x21,0x74,0x8b,0x60,0x9c,0x1a,0xa9,0x98,0xa8,0x45,0xb2,0x85,0x38,0x76,0x4f,0x6c,
  0xbb,0x93,0x60,0xdb,0x0d,0xe4,0xd9,0x83,0x1d,0xe7,0x57,0x18,0x04,0x6f,0xb8,0x4c,
  0xec,0x88,0x9c,0x01,0x0c,0x47,0x66,0xac,0xa2,0x38,0x19,0x76,0x36,0x83,0xc3,0xf0,
  0xb4,0x5e,0x77,0x03,0xaf,0xf8,0x7b,0x13,0x77,0xad,0xd3,0xad,0x75,0x61,0xa5,0x54,
  0x02,0x27,0x45,0x02,0x00,0x3b
  };


// Map
FXDEFMAP(FXMDIApp) FXMDIAppMap[]={
  //__Message_Type_____________ID________________________Message_Handler_____
  FXMAPFUNC(SEL_COMMAND,  FXMDIApp::ID_ABOUT,          FXMDIApp::onCmdAbout),
  FXMAPFUNC(SEL_COMMAND,  FXMDIApp::ID_NEW,            FXMDIApp::onCmdNew),
  };


// Object implementation
FXIMPLEMENT(FXMDIApp,FXApp,FXMDIAppMap,ARRAYNUMBER(FXMDIAppMap))


// Make some windows
FXMDIApp::FXMDIApp(){
  FXMDIChild *mdichild;
  
  // Main window
  mainwindow=new FXMainWindow(this,"MDI Widget Test",DECOR_ALL,0,0,800,600);
  
  // Menubar
  menubar=new FXMenuBar(mainwindow,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  
  // Status bar
  new FXStatusbar(mainwindow,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER);
  
  // MDI Client
  mdiclient=new FXMDIClient(mainwindow,menubar,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  
  // Icon for MDI Child
  mdiicon=new FXGIFIcon(this,penguin);

  // Make MDI Menu
  mdimenu=new FXMDIMenu(this,mdiclient);
  
  // Test
  mdichild=new FXMDIChild(mdiclient,"TEST1",mdiicon,mdimenu,LAYOUT_FIX_X|LAYOUT_FIX_Y,10,10,300,200);
  new FXScrollWindow(mdichild,HSCROLLER_ALWAYS|VSCROLLER_ALWAYS);
  
  mdichild=new FXMDIChild(mdiclient,"TEST2",mdiicon,mdimenu,LAYOUT_FIX_X|LAYOUT_FIX_Y,20,20,300,200);
  new FXScrollWindow(mdichild,HSCROLLER_ALWAYS|VSCROLLER_ALWAYS);
  
  mdichild=new FXMDIChild(mdiclient,"TEST3",mdiicon,mdimenu,LAYOUT_FIX_X|LAYOUT_FIX_Y,30,30,300,200);
  new FXButton(mdichild,"Hello",NULL,NULL,0,FRAME_THICK|FRAME_RAISED|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y);
  
  // File menu
  FXMenuPane *filemenu=new FXMenuPane(this);
  new FXMenuCommand(filemenu,"&New\t\tCreate new document.",this,ID_NEW);
  new FXMenuCommand(filemenu,"&Open\t\tOpen document.",NULL,0);
  new FXMenuCommand(filemenu,"&Quit\t\tQuit application.",this,ID_QUIT,0);
  new FXMenuTitle(menubar,"&File",filemenu);
    
  // Window menu
  FXMenuPane *windowmenu=new FXMenuPane(this);
  new FXMenuCommand(windowmenu,"Tile &Horizontally",mdiclient,FXWindow::ID_TILE_HORIZONTAL);
  new FXMenuCommand(windowmenu,"Tile &Vertically",mdiclient,FXWindow::ID_TILE_VERTICAL);
  new FXMenuCommand(windowmenu,"C&ascade",mdiclient,FXWindow::ID_CASCADE);
  new FXMenuCommand(windowmenu,"&Close",mdiclient,FXWindow::ID_DELETE);
  new FXMenuTitle(menubar,"&Window",windowmenu);
  
  // Help menu
  FXMenuPane *helpmenu=new FXMenuPane(this);
  new FXMenuCommand(helpmenu,"&About FOX...",this,ID_ABOUT,0);
  new FXMenuTitle(menubar,"&Help",helpmenu,LAYOUT_RIGHT);
  }
  

// About
long FXMDIApp::onCmdAbout(FXObject*,FXSelector,void*){
  showModalInformationBox(MBOX_OK,"About MDI Test","Test of the FOX MDI Widgets\nWritten by Jeroen van der Zijp");
  return 1;
  }


// New
long FXMDIApp::onCmdNew(FXObject*,FXSelector,void*){
  FXMDIChild *mdichild=new FXMDIChild(mdiclient,"Child",mdiicon,mdimenu,LAYOUT_FIX_X|LAYOUT_FIX_Y,20,20,300,200);
  new FXScrollWindow(mdichild,HSCROLLER_ALWAYS|VSCROLLER_ALWAYS);
  mdichild->create();
  return 1;
  }


// Start
void FXMDIApp::create(){
  FXApp::create();
  mainwindow->show();
  }


/*******************************************************************************/

//extern char foxclasses[];

// Start the whole thing
int main(int argc,char *argv[]){

//  fprintf(stderr,"foxclasses = %lx\n",foxclasses);
  
  // Make application
  FXMDIApp* application=new FXMDIApp;
  
  // Open display
  application->init(argc,argv);
  
  // Create app
  application->create();
  
  // Run
  application->run();
  }


