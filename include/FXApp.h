/********************************************************************************
*                                                                               *
*                     A p p l i c a t i o n   O b j e c t                       *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXApp.h,v 1.53 1998/10/14 05:41:01 jeroen Exp $                          *
********************************************************************************/
#ifndef FXAPP_H
#define FXAPP_H


// Forward declarations
class FXWindow;
class FXIcon;
class FXCursor;
class FXRootWindow;
class FXMainWindow;
class FXFont;

// Opaque FOX objects
struct FXTimer;
struct FXChore;
struct FXRepaint;


// Application Object
class FXApp : public FXObject {
  FXDECLARE(FXApp)
    
  // We've got many friends
  friend class FXId;
  friend class FXImage;
  friend class FXIcon;
  friend class FXCursor;
  friend class FXDrawable;
  friend class FXWindow;
  friend class FXRootWindow;
  friend class FXShell;
  friend class FXTopWindow;
  friend class FXMainWindow;
  friend class FXGLCanvas;
  friend class FXFont;
  
private:
  
  // Platform independent private data
  FXchar       *appname;                // Application name
  FXWindow     *focusWindow;            // Window which has the focus
  FXWindow     *cursorWindow;           // Window under the cursor
  FXWindow     *grabWindow;             // Window which has the grab
  FXWindow     *refresher;              // GUI refresher pointer
  FXRootWindow *root;                   // Root window
  FXTimer      *timers;                 // List of timers, sorted by time
  FXChore      *chores;                 // List of chores
  FXRepaint    *repaints;               // Unhandled repaint rectangles
  FXTimer      *timerrecs;              // List of recycled timer records
  FXChore      *chorerecs;              // List of recycled chore records
  FXRepaint    *repaintrecs;            // List of recycled repaint records
  FXInvocation *invocation;             // Modal loop invocation
  FXuint        done;                   // True if application is done
  FXint         exitcode;               // Exit code
  FXEvent       event;                  // Event
  
private:
  
  // Global data
  static FXApp *app;                    // Application pointer
  
private:

  // Platform dependent private data
  Display      *display;                // Display we're talking to
  int           connection;             // Connection fd of display
  FXContext     wcontext;               // Window hash context
  Visual       *visual;                 // Application visual
  FXuint        depth;                  // Visual depth
  FXID          colormap;               // Application colormap
  FXID          wmDeleteWindow;         // Catch delete window 
  FXID          wmQuitApp;              // Catch quit application
  FXID          wmProtocols;            // Window manager protocols
  FXID          wmMotifHints;           // Motif hints
  FXID          wmTakeFocus;            // Focus explicitly set by app
  FXID          xdndAware;              // XDND awareness atom
  FXID          xdndEnter;              // XDND enter window message
  FXID          xdndLeave;              // XDND leave window message
  FXID          xdndPosition;           // XDND position update message
  FXID          xdndStatus;             // XDND status feedback message
  FXID          xdndDrop;               // XDND drop message
  FXID          xdndFinished;           // XDND finished message
  FXID          xdndSelection;          // XDND selection atom
  FXID          xdndActionMove;         // XDND Move action
  FXID          xdndActionCopy;         // XDND Copy action
  FXID          xdndActionLink;         // XDND Link action
  FXID          xdndTypes;              // XDND types list atom
  FXID          xdndSource;             // XDND drag source window
  FXID          xdndTarget;             // XDND drop target window
  FXID          xdndAction;             // XDND Drag and Drop action
  FXbool        xdndAccepts;            // XDND true if target accepts
  FXbool        xdndSendPosition;       // XDND send position update when status comes in
  FXbool        xdndStatusPending;      // XDND waiting for status feedback
  FXbool        xdndFinishPending;      // XDND waiting for drop-confirmation
  FXbool        xdndStatusReceived;     // XDND received at least one status
  FXbool        xdndWantUpdates;        // XDND target wants new positions while in rect
  FXuint        xdndVersion;            // XDND version for ongoing transaction
  FXID         *xdndTypeList;           // XDND type list
  FXuint        xdndNumTypes;           // XDND number of types
  FXRectangle   xdndRect;               // XDND rectangle bounding target
  FXint         xdndXPos;               // XDND Cached X position
  FXint         xdndYPos;               // XDND Cached Y position
  FXID          ddeAtom;                // DDE Exchange Atom
  FXID          ddeRequestor;           // DDE requestor
  FXID          ddeSelection;           // DDE selection
  FXID          ddeProperty;            // DDE on requestor's window to receive data
  FXID          ddeTarget;              // DDE requested target type
  FXuchar      *ddeData;                // DDE array
  FXuint        ddeSize;                // DDE array size
  FXID          graybitmap;             // Gray pattern
  FXID          graystipple;            // Gray stipple pattern
  FXID          blackstipple;           // Black stipple pattern
  GC            gc;                     // Main graphics context
  FXbool        shmi;                   // Use XSHM Image possible
  FXbool        shmp;                   // Use XSHM Pixmap possible
  FXbool        synchronize;            // Synchronized
  FXPalEntry   *palette;                // Palette of colors
  FXuint        ncolors;                // Number of colors in palette
  
public:
  
  // Public platform independent data
  FXFont       *normalFont;             // Normal font
  FXCursor     *arrowCursor;            // Arrow cursor
  FXCursor     *rarrowCursor;           // Reverse arrow cursor
  FXCursor     *textCursor;             // Text cursor
  FXCursor     *hsplitCursor;           // Horizontal split cursor
  FXCursor     *vsplitCursor;           // Vertical split cursor
  FXCursor     *resizeCursor;           // Resize grip cursor
  FXCursor     *swatchCursor;           // Color swatch drag cursor
  FXCursor     *dontdropCursor;         // Cursor indicating no drop
  FXCursor     *moveCursor;             // Move cursor
  FXCursor     *dragHCursor;            // Resize horizontal edge
  FXCursor     *dragVCursor;            // Resize vertical edge
  FXCursor     *dragTLCursor;           // Resize upper-left or bottom-right corner
  FXCursor     *dragTRCursor;           // Resize upper-left or bottom-right corner
  FXCursor     *dndCopyCursor;          // Drag and drop copy
  FXCursor     *dndMoveCursor;          // Drag and drop move
  FXCursor     *dndLinkCursor;          // Drag and drop link
  FXuint        clickSpeed;             // Double click speed
  FXuint        scrollSpeed;            // Scroll speed
  FXuint        blinkSpeed;             // Cursor blink speed
  FXuint        menuPause;              // Menu popup delay
  FXuint        tooltipPause;           // Tooltip popup delay
  FXuint        tooltipTime;            // Tooltip display time
  FXint         scrollbarWidth;         // Scrollbar width
  FXint         dragDelta;              // Minimum distance considered a move
  FXColor       backColor;              // Background color
  FXColor       foreColor;              // Foreground color
  FXColor       hiliteColor;            // Hightlight color
  FXColor       shadowColor;            // Shadow color
  FXColor       borderColor;            // Border color
  FXColor       selforeColor;           // Select foreground color
  FXColor       selbackColor;           // Select background color
  
private:
  
  // Private member functions
  FXApp(const FXApp&){}
  void addRepaint(FXID win,FXint x,FXint y,FXint w,FXint h,FXbool synth=0);
  
  // Event handling
  void getNextEvent(XEvent& ev);
  void dispatchEvent(XEvent& ev);
  
  // Color handling
  void makePalette();
  void getPalette();
  void freePalette();
  FXPixel matchColor(FXuint r,FXuint g,FXuint b);
  

public:
  
  // Message handlers
  long onQuit(FXObject*,FXSelector,void*);

public:
  
  // Messages the application understands
  enum{
    ID_QUIT=0,
    ID_LAST
    };

public:

  FXApp();

  // Open display  
  int openDisplay(const char* dpyname);
  
  // Close display
  void closeDisplay();

  // Get root Window
  FXRootWindow* getRoot() const { return root; }

  // Get the window under the cursor, if any  
  FXWindow *getCursorWindow() const { return cursorWindow; }

  // Get the window which has the focus, if any  
  FXWindow *getFocusWindow() const { return focusWindow; }

  // Add timeout
  FXTimer* addTimeout(FXint ms,FXObject* tgt,FXSelector sel);

  // Remove timeout, returns NULL
  FXTimer* removeTimeout(FXTimer *t);
  
  // Add a chore
  FXChore* addChore(FXObject* tgt,FXSelector sel);
  
  // Remove chore
  FXChore* removeChore(FXChore * c);
  
  // Create application's windows
  void create();

  // Destroy application's windows
  void destroy();

  // Peek for event
  FXbool peekEvent();
  
  // Main application event loop
  FXint run();
  
  // Run till some flag becomes non-zero
  void runUntil(FXuint& condition);
  
  // Run modal for window
  void runModalFor(FXWindow* window);

  // Force GUI refresh
  void forceRefresh();
  
  // Schedule a refresh
  void refresh();

  // Flush pending repaints
  void flush();

  // Initialize application
  void init(int& argc,char** argv);
  
  // Exit application
  void exit(FXint code);

  // Register new DND type
  FXDragType registerDragType(const FXString& name) const;
  
  // Get drag type name
  FXString getDragTypeName(FXDragType type) const;
  
  // Show a modal error message
  FXuint showModalErrorBox(FXuint opts,const char* title,const char* message,...);

  // Show a modal warning message
  FXuint showModalWarningBox(FXuint opts,const char* title,const char* message,...);

  // Show a modal question dialog
  FXuint showModalQuestionBox(FXuint opts,const char* title,const char* message,...);

  // Show a modal information dialog
  FXuint showModalInformationBox(FXuint opts,const char* title,const char* message,...);

  // Beep
  void beep();

  // Return application instance
  static inline FXApp* instance(){ return app; }

  // Save
  virtual void save(FXStream& store) const;
  
  // Load
  virtual void load(FXStream& store);
  
  // Virtual destructor
  virtual ~FXApp();
  };


#endif
