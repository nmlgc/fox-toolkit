/********************************************************************************
*                                                                               *
*                                 Test MDI Widgets                              *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* $Id: mditest.cpp,v 1.11 2001/02/21 21:05:40 jeroen Exp $                      *
********************************************************************************/
#include "fx.h"
#include <stdio.h>
#include <stdlib.h>



/*******************************************************************************/


// Mini application object
class MDITestWindow : public FXMainWindow {
  FXDECLARE(MDITestWindow)
    
protected:
  FXMenubar         *menubar;
  FXMDIClient       *mdiclient;               // MDI Client area
  FXMDIMenu         *mdimenu;                 // MDI Window Menu
  FXGIFIcon         *mdiicon;                 // MDI Icon
  FXMenuPane        *filemenu;
  FXMenuPane        *windowmenu;
  FXMenuPane        *helpmenu;
  FXFont            *font;
protected:
  MDITestWindow(){}
  
public:
  
  // We define additional ID's, starting from the last one used by the base class+1.
  // This way, we know the ID's are all unique for this particular target.
  enum {
    ID_ABOUT=FXMainWindow::ID_LAST,
    ID_NEW
    };
    
  // Message handlers
  long onCmdAbout(FXObject*,FXSelector,void*);
  long onCmdNew(FXObject*,FXSelector,void*);
  
public:
  MDITestWindow(FXApp* a);
  virtual void create();
  virtual ~MDITestWindow();
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


static const FXchar tyger[]=
  "The Tyger\n\n"
  "Tyger! Tyger! burning bright\n"
  "In the forests of the night\n"
  "What immortal hand or eye\n"
  "Could frame thy fearful symmetry?\n\n"
  "In what distant deeps or skies\n"
  "Burnt the fire of thine eyes?\n"
  "On what wings dare he aspire?\n"
  "What the hand dare seize the fire?\n\n"
  "And what shoulder, and what art,\n"
  "Could twist the sinews of thy heart,\n"
  "And when thy heart began to beat,\n"
  "What dread hand? and what dread feet?\n\n"
  "What the hammer? what the chain?\n"
  "In what furnace was thy brain?\n"
  "What the anvil? what dread grasp\n"
  "Dare its deadly terrors clasp?\n\n"
  "When the stars threw down their spears,\n"
  "And water'd heaven with their tears,\n"
  "Did he smile his work to see?\n"
  "Did he who made the Lamb make thee?\n\n"
  "Tyger! Tyger! burning bright\n"
  "In the forests of the night,\n"
  "What immortal hand or eye,\n"
  "Dare frame thy fearful symmetry?\n\n\n\n"
  "               - William Blake\n\n";


// Map
FXDEFMAP(MDITestWindow) MDITestWindowMap[]={
  //________Message_Type____________ID___________________Message_Handler________
  FXMAPFUNC(SEL_COMMAND,  MDITestWindow::ID_ABOUT,    MDITestWindow::onCmdAbout),
  FXMAPFUNC(SEL_COMMAND,  MDITestWindow::ID_NEW,      MDITestWindow::onCmdNew),
  };


// Object implementation
FXIMPLEMENT(MDITestWindow,FXMainWindow,MDITestWindowMap,ARRAYNUMBER(MDITestWindowMap))


// Make some windows
MDITestWindow::MDITestWindow(FXApp* a):FXMainWindow(a,"MDI Widget Test",NULL,NULL,DECOR_ALL,0,0,800,600){
  FXMDIChild *mdichild;
  FXScrollWindow *scrollwindow;
  FXButton *btn;
    
  font=new FXFont(getApp(),"courier",15,FONTWEIGHT_BOLD);
  
  // Menubar
  menubar=new FXMenubar(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  
  // Status bar
  new FXStatusbar(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER);
  
  // MDI Client
  mdiclient=new FXMDIClient(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  
  // Icon for MDI Child
  mdiicon=new FXGIFIcon(getApp(),penguin);

  // Make MDI Menu
  mdimenu=new FXMDIMenu(this,mdiclient);
  
  // MDI buttons in menu:- note the message ID's!!!!!
  // Normally, MDI commands are simply sensitized or desensitized;
  // Under the menubar, however, they're hidden if the MDI Client is
  // not maximized.  To do this, they must have different ID's.
  new FXMDIWindowButton(menubar,mdiclient,FXMDIClient::ID_MDI_MENUWINDOW,LAYOUT_LEFT);
  new FXMDIDeleteButton(menubar,mdiclient,FXMDIClient::ID_MDI_MENUCLOSE,FRAME_RAISED|LAYOUT_RIGHT);
  new FXMDIRestoreButton(menubar,mdiclient,FXMDIClient::ID_MDI_MENURESTORE,FRAME_RAISED|LAYOUT_RIGHT);
  new FXMDIMinimizeButton(menubar,mdiclient,FXMDIClient::ID_MDI_MENUMINIMIZE,FRAME_RAISED|LAYOUT_RIGHT);
  
  // Test window #1
  mdichild=new FXMDIChild(mdiclient,"Child",mdiicon,mdimenu,0,10,10,400,300);
  scrollwindow=new FXScrollWindow(mdichild,0);
  btn=new FXButton(scrollwindow,tyger,NULL,NULL,0,LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,0,0,600,1000);
  btn->setBackColor(FXRGB(255,255,255));
  btn->setFont(font);
  mdiclient->setActiveChild(mdichild);

  // Test window #2
  mdichild=new FXMDIChild(mdiclient,"Child",mdiicon,mdimenu,0,20,20,400,300);
  scrollwindow=new FXScrollWindow(mdichild,0);
  btn=new FXButton(scrollwindow,tyger,NULL,NULL,0,LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,0,0,600,1000);
  btn->setFont(font);
  btn->setBackColor(FXRGB(255,255,255));

  // Test window #3
  mdichild=new FXMDIChild(mdiclient,"Child",mdiicon,mdimenu,0,30,30,400,300);
  scrollwindow=new FXScrollWindow(mdichild,0);
  btn=new FXButton(scrollwindow,tyger,NULL,NULL,0,LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,0,0,600,1000);
  btn->setFont(font);
  btn->setBackColor(FXRGB(255,255,255));

  // File menu
  filemenu=new FXMenuPane(this);
  new FXMenuCommand(filemenu,"&New\tCtl-N\tCreate new document.",NULL,this,ID_NEW);
  new FXMenuCommand(filemenu,"&Open\tCtl-O\tOpen document.");
  new FXMenuCommand(filemenu,"&Quit\tCtl-Q\tQuit application.",NULL,getApp(),FXApp::ID_QUIT,0);
  new FXMenuTitle(menubar,"&File",NULL,filemenu);
    
  // Window menu
  windowmenu=new FXMenuPane(this);
  new FXMenuCommand(windowmenu,"Tile &Horizontally",NULL,mdiclient,FXMDIClient::ID_MDI_TILEHORIZONTAL);
  new FXMenuCommand(windowmenu,"Tile &Vertically",NULL,mdiclient,FXMDIClient::ID_MDI_TILEVERTICAL);
  new FXMenuCommand(windowmenu,"C&ascade",NULL,mdiclient,FXMDIClient::ID_MDI_CASCADE);
  new FXMenuCommand(windowmenu,"&Close",NULL,mdiclient,FXMDIClient::ID_MDI_CLOSE);
  new FXMenuCommand(windowmenu,"Close &All",NULL,mdiclient,FXMDIClient::ID_CLOSE_ALL_DOCUMENTS);
  FXMenuSeparator* sep1=new FXMenuSeparator(windowmenu);
  sep1->setTarget(mdiclient);
  sep1->setSelector(FXMDIClient::ID_MDI_ANY);
  new FXMenuCommand(windowmenu,NULL,NULL,mdiclient,FXMDIClient::ID_MDI_1);
  new FXMenuCommand(windowmenu,NULL,NULL,mdiclient,FXMDIClient::ID_MDI_2);
  new FXMenuCommand(windowmenu,NULL,NULL,mdiclient,FXMDIClient::ID_MDI_3);
  new FXMenuCommand(windowmenu,NULL,NULL,mdiclient,FXMDIClient::ID_MDI_4);
  new FXMenuTitle(menubar,"&Window",NULL,windowmenu);
  
  // Help menu
  helpmenu=new FXMenuPane(this);
  new FXMenuCommand(helpmenu,"&About FOX...",NULL,this,ID_ABOUT,0);
  new FXMenuTitle(menubar,"&Help",NULL,helpmenu,LAYOUT_RIGHT);
  
  }


// Clean up  
MDITestWindow::~MDITestWindow(){
  delete filemenu;
  delete windowmenu;
  delete helpmenu;
  delete font;
  }


// About
long MDITestWindow::onCmdAbout(FXObject*,FXSelector,void*){
  FXMessageBox::information(this,MBOX_OK,"About MDI Test","Test of the FOX MDI Widgets\nWritten by Jeroen van der Zijp");
  return 1;
  }


// New
long MDITestWindow::onCmdNew(FXObject*,FXSelector,void*){
  FXMDIChild *mdichild=new FXMDIChild(mdiclient,"Child",mdiicon,mdimenu,0,20,20,300,200);
  FXScrollWindow *scrollwindow=new FXScrollWindow(mdichild,0);
  FXButton* btn=new FXButton(scrollwindow,tyger,NULL,NULL,0,LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,0,0,600,1000);
  btn->setBackColor(FXRGB(255,255,255));
  mdichild->create();
  return 1;
  }


// Start
void MDITestWindow::create(){
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){
  
  // Make application
  FXApp application("MDIApp","FoxTest");
  
  // Open display
  application.init(argc,argv);
  
  // Make window
  new MDITestWindow(&application);
  
  // Create app
  application.create();
  
  // Run
  return application.run();
  }


