#include "fx.h"	

/*

  This is a classical programming example.  Doing it graphically
  using FOX is, as you see, not a whole lot more involved than
  its command-line equivalent!
  
  Note the following things:
  
    - Each FOX application needs one (and only one) application
      object (FXApp).  You may subclass from FXApp if you need to.
      
    - Before doing anything, the application object needs to be
      initialized.  You need to pass argc and argv so that certain
      command line arguments may be filtered out by FOX (e.g. -display).
      
    - You need to create at least one toplevel window; in this case,
      that is FXMainWindow.
      
    - FOX widgets are nested simply by creating them in the right order.
      Here, we create FXButton as a child of "main."

    - A single call to FXApp::create() will create X windows for each
      widget.  Until calling create(), a widget exists only at the client,
      and has no associated X window yet.
      
    - Finally, FXApp::run() will start the main event loop.  This will
      only return when the application is done.      
  
*/
  
  
int main(int argc,char **argv){
  FXApp *app=new FXApp;
  app->init(argc,argv);
  FXMainWindow *main=new FXMainWindow(app,"Hello",DECOR_ALL);
  new FXButton(main,"&Hello, World!",NULL,app,FXApp::ID_QUIT);
  app->create();
  main->show();
  app->run();
  return 0;
  }
