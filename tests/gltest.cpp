/********************************************************************************
*                                                                               *
*                         OpenGL Application coding sample                      *
*                                                                               *
********************************************************************************/
#include "fx.h"
#include "fx3d.h"
#include "fxregex.h"


// Event Handler Object
class GLTestApp : public FXApp {
  FXDECLARE(GLTestApp)

private:

  FXMainWindow    *main;                      // Main window
  FXGLCanvas      *glcanvas;                  // GL Canvas to draw into
  FXTimer         *timer;                     // Timer for spinning box
  int              spinning;                  // Is box spinning
  double           angle;                     // Rotation angle of box
  
public:

  // We define additional ID's, starting from the last one used by the base class+1.
  // This way, we know the ID's are all unique for this particular target.
  enum{
    ID_CANVAS=FXApp::ID_LAST,
    ID_SPIN,
    ID_STOP,
    ID_TIMEOUT
    };
    
  // Message handlers
  long onMouseDown(FXObject*,FXSelector,void*);
  long onMouseUp(FXObject*,FXSelector,void*);
  long onMouseMove(FXObject*,FXSelector,void*);
  long onExpose(FXObject*,FXSelector,void*);
  long onConfigure(FXObject*,FXSelector,void*);
  long onCmdSpin(FXObject*,FXSelector,void*);
  long onUpdSpin(FXObject*,FXSelector,void*);
  long onCmdStop(FXObject*,FXSelector,void*);
  long onUpdStop(FXObject*,FXSelector,void*);
  long onTimeout(FXObject*,FXSelector,void*);
  
public:

  // GLTestApp constructor
  GLTestApp();
  
  // Initialize
  void create();
  
  // Draw scene
  void drawScene();
  
  // GLTestApp destructor
  virtual ~GLTestApp();
  };



// Message Map for the Scribble App class
FXDEFMAP(GLTestApp) GLTestAppMap[]={

  //________Message_Type_________ID_____________________Message_Handler_______
  FXMAPFUNC(SEL_PAINT,         GLTestApp::ID_CANVAS,  GLTestApp::onExpose),
  FXMAPFUNC(SEL_CONFIGURE,     GLTestApp::ID_CANVAS,  GLTestApp::onConfigure),
  
  FXMAPFUNC(SEL_COMMAND,       GLTestApp::ID_SPIN,    GLTestApp::onCmdSpin),
  FXMAPFUNC(SEL_UPDATE,        GLTestApp::ID_SPIN,    GLTestApp::onUpdSpin),
  
  FXMAPFUNC(SEL_COMMAND,       GLTestApp::ID_STOP,    GLTestApp::onCmdStop),
  FXMAPFUNC(SEL_UPDATE,        GLTestApp::ID_STOP,    GLTestApp::onUpdStop),

  FXMAPFUNC(SEL_TIMEOUT,       GLTestApp::ID_TIMEOUT, GLTestApp::onTimeout),
  
  };



// Macro for the GLTestApp class hierarchy implementation
FXIMPLEMENT(GLTestApp,FXApp,GLTestAppMap,ARRAYNUMBER(GLTestAppMap))



// Construct a GLTestApp
GLTestApp::GLTestApp(){
  FXVerticalFrame *glcanvasFrame;
  FXVerticalFrame *buttonFrame;
  FXComposite *glpanel;
  FXHorizontalFrame *frame;

  // Make my own main window
  main=new FXMainWindow(this,"OpenGL Test Application",DECOR_ALL,0,0,800,600);

  frame=new FXHorizontalFrame(main,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
  
    // LEFT pane to contain the glcanvas
    glcanvasFrame=new FXVerticalFrame(frame,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,10,10,10,10);

      // Label above the glcanvas               
      new FXLabel(glcanvasFrame,"OpenGL Canvas Frame",NULL,JUSTIFY_CENTER_X|LAYOUT_FILL_X);

      // Horizontal divider line
      new FXHorizontalSeparator(glcanvasFrame,SEPARATOR_GROOVE|LAYOUT_FILL_X);

      // Drawing glcanvas
      glpanel=new FXVerticalFrame(glcanvasFrame,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0, 0,0,0,0);

        // Drawing glcanvas
        glcanvas=new FXGLCanvas(glpanel,this,ID_CANVAS,GL_INSTALL_COLORMAP|GL_DOUBLE_BUFFER|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT);

    // RIGHT pane for the buttons
    buttonFrame=new FXVerticalFrame(frame,LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,10,10,10,10);

      // Label above the buttons  
      new FXLabel(buttonFrame,"Button Frame",NULL,JUSTIFY_CENTER_X|LAYOUT_FILL_X);

      // Horizontal divider line
      new FXHorizontalSeparator(buttonFrame,SEPARATOR_RIDGE|LAYOUT_FILL_X);

      // Button to print
      new FXButton(buttonFrame,"Spin Boxes",NULL,this,ID_SPIN,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,10,10,5,5);

      // Button to print
      new FXButton(buttonFrame,"Stop Boxes",NULL,this,ID_STOP,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,10,10,5,5);

      // Exit button
      new FXButton(buttonFrame,"Exit",NULL,this,ID_QUIT,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,10,10,5,5);

  // Initialize private variables
  spinning=0;    
  timer = NULL;
  angle = 0.;
  }
    

// Destructor
GLTestApp::~GLTestApp(){
  if(timer) removeTimeout(timer);
  }

 

// Create and initialize 
void GLTestApp::create(){

  // Create the windows
  FXApp::create();
  
  // Make the main window appear
  main->show();
  }



// Widget has been resized
long GLTestApp::onConfigure(FXObject*,FXSelector,void* ptr){
  glcanvas->makeCurrent();
  glViewport(0,0,glcanvas->getWidth(),glcanvas->getHeight());
  return 1;
  }



// Widget needs repainting
long GLTestApp::onExpose(FXObject*,FXSelector,void* ptr){
  glcanvas->makeCurrent();
  drawScene();
  return 1;
  }



//  Rotate the boxes when a timer message is received
long GLTestApp::onTimeout(FXObject*,FXSelector,void*){
  angle += 2.;
  if(angle > 360.) angle -= 360.;
  drawScene();
  timer=addTimeout(100,this,ID_TIMEOUT);
  return 1;
  }



// Start the boxes spinning
long GLTestApp::onCmdSpin(FXObject*,FXSelector,void*){
  spinning=1;
  timer=addTimeout(100,this,ID_TIMEOUT);
  return 1;
  }



// Enable or disable the spin button
long GLTestApp::onUpdSpin(FXObject* sender,FXSelector,void*){
  FXButton* button=(FXButton*)sender;
  spinning ? button->disable() : button->enable(); 
  return 1;
  }



// If boxes are spinning, stop them
long GLTestApp::onCmdStop(FXObject*,FXSelector,void*){
  spinning=0;
  if(timer){
    removeTimeout(timer);
    timer=NULL;
    }
  return 1;
  }


// Enable or disable the stop button
long GLTestApp::onUpdStop(FXObject* sender,FXSelector,void*){
  FXButton* button=(FXButton*)sender;
  spinning ? button->enable() : button->disable(); 
  return 1;
  }



// Here we begin
int main(int argc,char *argv[]){

  // Make application
  GLTestApp* application=new GLTestApp;
  
  // Open the display
  application->init(argc,argv);

  // Create the application's windows
  application->create();
  
  // Run the application
  application->run();
  
  return 0;
  }


// Draws a simple box using the given corners
void drawBox(double xmin, double ymin, double zmin, double xmax, double ymax, double zmax) {
  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.,0.,-1.);
    glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymax, zmin);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(1.,0.,0.);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymax, zmax);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.,0.,1.);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymax, zmax);
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmin, ymax, zmax);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(-1.,0.,0.);
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmin, ymax, zmin);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.,1.,0.);
    glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmax, ymax, zmax);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.,-1.,0.);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmin, ymin, zmin);
  glEnd();
  }


// Draw the GL scene
void GLTestApp::drawScene(){
  GLdouble width = glcanvas->getWidth();
  GLdouble height = glcanvas->getHeight();
  GLdouble aspect = width / height;
  GLfloat lightPosition[]={15.,10.,5.,1.};
  GLfloat lightAmbient[]={.1,.1,.1,1.};
  GLfloat lightDiffuse[]={.9,.9,.9,1.};
  GLfloat redMaterial[]={1.,0.,0.,1.};
  GLfloat blueMaterial[]={0.,0.,1.,1.};

  glcanvas->makeCurrent();

  glClearColor(1.0,1.0,1.0,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.,aspect, 1., 100.);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(5.,10.,15.,0.,0.,0.,0.,1.,0.);

  glShadeModel(GL_SMOOTH);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  glMaterialfv(GL_FRONT, GL_AMBIENT, blueMaterial);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, blueMaterial);

  glPushMatrix();
  glRotated(angle, 0., 1., 0.);
  drawBox(-1, -1, -1, 1, 1, 1);

  glMaterialfv(GL_FRONT, GL_AMBIENT, redMaterial);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, redMaterial);

  glPushMatrix();
  glTranslated(0.,1.75,0.);
  glRotated(angle, 0., 1., 0.);
  drawBox(-.5,-.5,-.5,.5,.5,.5);
  glPopMatrix();

  glPushMatrix();
  glTranslated(0.,-1.75,0.);
  glRotated(angle, 0., 1., 0.);
  drawBox(-.5,-.5,-.5,.5,.5,.5);
  glPopMatrix();

  glPushMatrix();
  glRotated(90., 1., 0., 0.);
  glTranslated(0.,1.75,0.);
  glRotated(angle, 0., 1., 0.);
  drawBox(-.5,-.5,-.5,.5,.5,.5);
  glPopMatrix();

  glPushMatrix();
  glRotated(90., -1., 0., 0.);
  glTranslated(0.,1.75,0.);
  glRotated(angle, 0., 1., 0.);
  drawBox(-.5,-.5,-.5,.5,.5,.5);
  glPopMatrix();

  glPushMatrix();
  glRotated(90., 0., 0., 1.);
  glTranslated(0.,1.75,0.);
  glRotated(angle, 0., 1., 0.);
  drawBox(-.5,-.5,-.5,.5,.5,.5);
  glPopMatrix();

  glPushMatrix();
  glRotated(90., 0., 0., -1.);
  glTranslated(0.,1.75,0.);
  glRotated(angle, 0., 1., 0.);
  drawBox(-.5,-.5,-.5,.5,.5,.5);
  glPopMatrix();

  glPopMatrix();

  glcanvas->swapBuffers();
  }
