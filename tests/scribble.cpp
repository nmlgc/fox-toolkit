/********************************************************************************
*                                                                               *
*                         Scribble  Application coding sample                   *
*                                                                               *
********************************************************************************/
#include "fx.h"



// Event Handler Object
class ScribbleApp : public FXApp {

  // Macro for class hierarchy declarations
  FXDECLARE(ScribbleApp)

private:

  FXMainWindow      *main;                    // Main window
  FXHorizontalFrame *contents;                // Content frame
  FXVerticalFrame   *canvasFrame;             // Canvas frame
  FXVerticalFrame   *buttonFrame;             // Button frame
  FXCanvas          *canvas;                  // Canvas to draw into
  int                mdflag;                  // Mouse button down?
  int                dirty;                   // Canvas has been painted?
  FXPixel            drawColor;               // Color for the line
  
public:

  // Message handlers
  long onMouseDown(FXObject*,FXSelector,void*);
  long onMouseUp(FXObject*,FXSelector,void*);
  long onMouseMove(FXObject*,FXSelector,void*);
  long onCmdClear(FXObject*,FXSelector,void*);
  long onUpdClear(FXObject*,FXSelector,void*);
  
public:
  
  // Messages for our class
  enum{
    ID_MOUSE=FXApp::ID_LAST,
    ID_CLEAR,
    ID_LAST
    };
    
public:

  // ScribbleApp constructor
  ScribbleApp();
  
  // Initialize
  virtual void create();
  };



// Message Map for the Scribble App class
FXDEFMAP(ScribbleApp) ScribbleAppMap[]={

  //________Message_Type_____________________ID____________Message_Handler_______
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,   ScribbleApp::ID_MOUSE,  ScribbleApp::onMouseDown),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE, ScribbleApp::ID_MOUSE,  ScribbleApp::onMouseUp),
  FXMAPFUNC(SEL_MOTION,            ScribbleApp::ID_MOUSE,  ScribbleApp::onMouseMove),
  FXMAPFUNC(SEL_COMMAND,           ScribbleApp::ID_CLEAR,  ScribbleApp::onCmdClear),
  FXMAPFUNC(SEL_UPDATE,            ScribbleApp::ID_CLEAR,  ScribbleApp::onUpdClear),
  
  };



// Macro for the ScribbleApp class hierarchy implementation
FXIMPLEMENT(ScribbleApp,FXApp,ScribbleAppMap,ARRAYNUMBER(ScribbleAppMap))



// Construct a ScribbleApp
ScribbleApp::ScribbleApp(){

    // Make my own main window
  main=new FXMainWindow(this,"Scribble Application",DECOR_ALL,0,0,800,600);
  
  contents=new FXHorizontalFrame(main,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
  
  // LEFT pane to contain the canvas
  canvasFrame=new FXVerticalFrame(contents,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,10,10,10,10);
  
    // Label above the canvas               
    new FXLabel(canvasFrame,"Canvas Frame",NULL,JUSTIFY_CENTER_X|LAYOUT_FILL_X);
  
    // Horizontal divider line
    new FXHorizontalSeparator(canvasFrame,SEPARATOR_GROOVE|LAYOUT_FILL_X);

    // Drawing canvas
    canvas=new FXCanvas(canvasFrame,this,ID_MOUSE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT);


  // RIGHT pane for the buttons
  buttonFrame=new FXVerticalFrame(contents,FRAME_SUNKEN|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,10,10,10,10);

    // Label above the buttons  
    new FXLabel(buttonFrame,"Button Frame",NULL,JUSTIFY_CENTER_X|LAYOUT_FILL_X);
    
    // Horizontal divider line
    new FXHorizontalSeparator(buttonFrame,SEPARATOR_RIDGE|LAYOUT_FILL_X);

    // Button to clear
    new FXButton(buttonFrame,"&Clear",NULL,this,ID_CLEAR,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,10,10,5,5);

    // Exit button
    new FXButton(buttonFrame,"&Exit",NULL,this,ID_QUIT,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,10,10,5,5);

  // Initialize private variables
  drawColor=0;
  mdflag=0;
  dirty=0;    
  }
    
 

// Create and initialize 
void ScribbleApp::create(){

  // Create the windows
  FXApp::create();
  
  // Get color
  drawColor=main->acquireColor(FXRGB(255,0,0));
  
  // Make the main window appear
  main->show();
  }



// Mouse button was pressed somewhere
long ScribbleApp::onMouseDown(FXObject*,FXSelector,void*){

  // While the mouse is down, we'll draw lines
  mdflag=1;
  
  return 1;
  }



// The mouse has moved, draw a line
long ScribbleApp::onMouseMove(FXObject*, FXSelector, void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  if(mdflag){
    
    canvas->setForeground(drawColor);
    canvas->drawLine(ev->last_x, ev->last_y, ev->win_x, ev->win_y);
    
    // We have drawn something, so now the canvas is dirty
    dirty=1;
    }
  return 1;
  }


// The mouse button was released again
long ScribbleApp::onMouseUp(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*) ptr;
  if(mdflag){
    
    canvas->setForeground(drawColor);
    canvas->drawLine(ev->last_x, ev->last_y, ev->win_x, ev->win_y);
    
    // We have drawn something, so now the canvas is dirty
    dirty=1;
    
    // Mouse no longer down
    mdflag=0;
    }
  return 1;
  }


// Handle the clear message
long ScribbleApp::onCmdClear(FXObject*,FXSelector,void*){

  // Erase the canvas
  canvas->clearWindow();
  
  dirty=0;
  
  return 1;
  }



// Update the clear button:- each gui element (widget) in FOX
// receives a message during idle processing asking it to be updated.
// For example, buttons can be sensitized or desensitized when the
// state of the application changes.
// In this case, we desensitize the sender (the clear button) when
// the canvas has already been cleared, and sensitize it when it has
// been painted (as indicated by the dirty flag).
long ScribbleApp::onUpdClear(FXObject* sender,FXSelector,void*){
  FXButton* button=(FXButton*)sender;
  
  // Button is available when canvas is dirty only
  dirty ? button->enable() : button->disable(); 

  return 1;
  }


// Here we begin
int main(int argc,char *argv[]){

  // Make application
  ScribbleApp* application=new ScribbleApp;
  
  // Start app
  application->init(argc,argv);

  // Create the application's windows
  application->create();
  
  // Run the application
  application->run();
  
  return 0;
  }




