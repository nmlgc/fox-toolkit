/********************************************************************************
*                                                                               *
*                            W i n d o w   O b j e c t                          *
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
* $Id: FXWindow.h,v 1.67 1998/10/30 04:49:05 jeroen Exp $                       *
********************************************************************************/
#ifndef FXWINDOW_H
#define FXWINDOW_H


// Layout hints
enum FXLayoutHints {
  LAYOUT_NORMAL       = 0,              // Default layout mode
  LAYOUT_SIDE_TOP     = 0,              // Pack on top side (default)
  LAYOUT_SIDE_BOTTOM  = 0x00000001,     // Pack on bottom side
  LAYOUT_SIDE_LEFT    = 0x00000002,     // Pack on left side
  LAYOUT_SIDE_RIGHT   = 0x00000003,     // Pack on right side
  LAYOUT_FILL_COLUMN  = 0x00000001,     // Matrix column is stretchable
  LAYOUT_FILL_ROW     = 0x00000002,     // Matrix row is stretchable
  LAYOUT_LEFT         = 0,              // Stick on left (default)
  LAYOUT_RIGHT        = 0x00000004,     // Stick on right
  LAYOUT_CENTER_X     = 0x00000008,     // Center horizontally
  LAYOUT_TOP          = 0,              // Stick on top (default)
  LAYOUT_BOTTOM       = 0x00000010,     // Stick on bottom
  LAYOUT_CENTER_Y     = 0x00000020,     // Center vertically
  LAYOUT_FIX_X        = 0x00000040,     // X fixed
  LAYOUT_FIX_Y        = 0x00000080,     // Y fixed
  LAYOUT_FIX_WIDTH    = 0x00000100,     // Width fixed
  LAYOUT_FIX_HEIGHT   = 0x00000200,     // height fixed
  LAYOUT_MIN_WIDTH    = 0,              // Minimum width is the default
  LAYOUT_MIN_HEIGHT   = 0,              // Minimum height is the default
  LAYOUT_FILL_X       = 0x00000400,     // Stretch horizontally
  LAYOUT_FILL_Y       = 0x00000800,     // Stretch vertically
  LAYOUT_SIDE_MASK    = LAYOUT_SIDE_LEFT|LAYOUT_SIDE_RIGHT|LAYOUT_SIDE_TOP|LAYOUT_SIDE_BOTTOM,
  LAYOUT_MASK         = LAYOUT_SIDE_MASK|LAYOUT_RIGHT|LAYOUT_CENTER_X|LAYOUT_BOTTOM|LAYOUT_CENTER_Y|LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y
  };


// Border appearance options (for subclasses)
enum FXBorderStyle {
  FRAME_NONE       = 0,                 // Default is no frame
  FRAME_SUNKEN     = 0x00001000,
  FRAME_RAISED     = 0x00002000,
  FRAME_THICK      = 0x00004000,
  FRAME_GROOVE     = FRAME_THICK,
  FRAME_RIDGE      = FRAME_THICK|FRAME_RAISED|FRAME_SUNKEN,
  FRAME_LINE       = FRAME_RAISED|FRAME_SUNKEN,
  FRAME_NORMAL     = FRAME_SUNKEN|FRAME_THICK,
  FRAME_MASK       = FRAME_SUNKEN|FRAME_RAISED|FRAME_THICK
  };


// Packing style (for packers)
enum FXPackingStyle {
  PACK_NORMAL         = 0,              // Default is each its own size
  PACK_UNIFORM_HEIGHT = 0x00008000,     // Uniform height
  PACK_UNIFORM_WIDTH  = 0x00010000      // Uniform width
  };
  
  

class FXComposite;


class FXWindow : public FXDrawable {
  FXDECLARE(FXWindow)
    
private:
  FXWindow     *parent;                 // Parent Window
  FXWindow     *shell;                  // Shell Window
  FXWindow     *first;                  // First Child
  FXWindow     *last;                   // Last Child
  FXWindow     *next;                   // Next Sibling
  FXWindow     *prev;                   // Previous Sibling
  FXWindow     *focus;                  // Focus Child
  
protected:
  FXCursor     *defaultCursor;          // Normal Cursor
  FXCursor     *dragCursor;             // Cursor during drag
  FXAccelTable *accelTable;             // Accelerator table
  FXObject     *target;                 // Target object
  FXSelector    message;                // Message ID
  FXint         xpos,ypos;              // Window X,Y Position
  FXPixel       backColor;              // Window background color
  FXuint        flags;                  // Window state flags
  FXuint        options;                // Window options
  
protected:
  
  // Constructors for specific subclasses
  FXWindow();
  FXWindow(const FXWindow&){}
  FXWindow(FXApp* a);
  FXWindow(FXApp* a,FXuint opts,FXint x,FXint y,FXint w,FXint h);
  
protected:
  
  // Other member functions
  virtual FXint getDefaultDepth();
  virtual Visual* getDefaultVisual();
  virtual FXID getDefaultColormap();
  virtual FXbool doesOverrideRedirect() const;
  virtual FXbool doesSaveUnder() const;
  void addColormapWindows();
  void removeColormapWindows();
  FXID getWindowAt(FXint x,FXint y) const;
  FXuint mayDropOn(FXID window) const;
  virtual void layout();
  
protected:
  
  // Window state flags
  enum {
    FLAG_SHOWN       = 0x00000001,      // Window is shown
    FLAG_FOCUSED     = 0x00000002,      // Window has focus
    FLAG_DEFAULT     = 0x00000004,
    FLAG_UPDATE      = 0x00000008,      // Window is subject to GUI update
    FLAG_PRESSED     = 0x00000010,      // Button has been pressed
    FLAG_OBSCURED    = 0x00000020,      // Window is partially obscured
    FLAG_INVISIBLE   = 0x00000040,      // Window is invisible
    FLAG_SELECTION   = 0x00000080,      // Window owns selection
    FLAG_ACTIVE      = 0x00000100,
    FLAG_ENABLED     = 0x00000200,      // Window able to receive input
    FLAG_DOWN        = 0x00000400,
    FLAG_CHECKED     = 0x00000800,
    FLAG_ENGAGED     = 0x00001000,
    FLAG_RCHECKED    = 0x00002000,
    FLAG_DIRTY       = 0x00004000,      // Window needs layout
    FLAG_RECALC      = 0x00008000,      // Window needs recalculation
    FLAG_MAPPED      = 0x00010000,      
    FLAG_DRAGGING    = 0x00020000,      // DND Dragging from this window
    FLAG_DROPTARGET  = 0x00040000,      // Window is drop target
    FLAG_CARET       = 0x00080000,      // Caret is on
    FLAG_CHANGED     = 0x00100000,      // Window data changed
    FLAG_CURSOR      = 0x00200000,      // Cursor window
    FLAG_TIP         = 0x00400000,      // Show tip
    FLAG_HELP        = 0x00800000,      // Show help
    FLAG_KEY         = 0x01000000,      // Keyboard key pressed
    FLAG_FIRED       = 0x02000000       // Fired a message
    };

public:
  // Message handlers
  long onPaint(FXObject*,FXSelector,void*);
  long onUpdate(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMiddleBtnPress(FXObject*,FXSelector,void*);
  long onMiddleBtnRelease(FXObject*,FXSelector,void*);
  long onRightBtnPress(FXObject*,FXSelector,void*);
  long onRightBtnRelease(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onHotKeyPress(FXObject*,FXSelector,void*);
  long onHotKeyRelease(FXObject*,FXSelector,void*);
  long onDestroy(FXObject*,FXSelector,void*);
  long onVisibility(FXObject*,FXSelector,void*);
  long onConfigure(FXObject*,FXSelector,void*);
  long onMap(FXObject*,FXSelector,void*);
  long onUnmap(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onSelectionLost(FXObject*,FXSelector,void*);
  long onSelectionGained(FXObject*,FXSelector,void*);
  long onSelectionRequest(FXObject*,FXSelector,void*);
  long onDNDEnter(FXObject*,FXSelector,void*);
  long onDNDLeave(FXObject*,FXSelector,void*);
  long onDNDMotion(FXObject*,FXSelector,void*);
  long onDNDDrop(FXObject*,FXSelector,void*);
  long onCmdShow(FXObject*,FXSelector,void*);
  long onCmdHide(FXObject*,FXSelector,void*);
  long onUpdToggleShown(FXObject*,FXSelector,void*);
  long onCmdToggleShown(FXObject*,FXSelector,void*);
  long onCmdRaise(FXObject*,FXSelector,void*);
  long onCmdLower(FXObject*,FXSelector,void*);
  long onCmdEnable(FXObject*,FXSelector,void*);
  long onCmdDisable(FXObject*,FXSelector,void*);
  long onUpdYes(FXObject*,FXSelector,void*);
  
public:
  
  // Message ID's common to most Windows
  enum {
    ID_SHOW=1,
    ID_HIDE,
    ID_TOGGLESHOWN,
    ID_RAISE,
    ID_LOWER,
    ID_ENABLE,
    ID_DISABLE,
    ID_CHECK,
    ID_UNCHECK,
    ID_UNKNOWN,
    ID_AUTOSCROLL,
    ID_HSCROLLED,
    ID_VSCROLLED,
    ID_SETVALUE,
    ID_SETINTVALUE,
    ID_SETREALVALUE,
    ID_SETSTRINGVALUE,
    ID_GETINTVALUE,
    ID_GETREALVALUE,
    ID_GETSTRINGVALUE,
    ID_QUERY_TIP,
    ID_QUERY_HELP,
    ID_QUERY_MENU,
    ID_HOTKEY,
    ID_ACCEL,
    ID_POST,
    ID_UNPOST,
    ID_TILE_HORIZONTAL,
    ID_TILE_VERTICAL,
    ID_CASCADE,
    ID_MAXIMIZE,
    ID_MINIMIZE,
    ID_RESTORE,
    ID_DELETE,
    ID_ACTIVATE_NEXT,
    ID_ACTIVATE_PREV,
    ID_LAST
    };
    
public:
  
  // Public Member functions
  FXWindow(FXComposite* p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  FXWindow* getParent() const { return parent; }
  FXWindow* getShell() const { return shell; }
  FXWindow* getRoot() const { return shell->parent; }
  FXWindow* getNext() const { return next; }
  FXWindow* getPrev() const { return prev; }
  FXWindow* getFirst() const { return first; }
  FXWindow* getLast() const { return last; }
  FXWindow* getFocus() const { return focus; }
  void setTarget(FXObject *t){ target=t; }
  FXObject* getTarget() const { return target; }
  void setSelector(FXSelector sel){ message=sel; }
  FXSelector getSelector() const { return message; }
  FXint getX() const { return xpos; }
  FXint getY() const { return ypos; }
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void setX(FXint x);
  void setY(FXint y);
  void setWidth(FXint w);
  void setHeight(FXint h);
  virtual void setLayoutHints(FXuint lout);
  FXuint getLayoutHints() const;
  FXAccelTable* getAccelTable() const { return accelTable; }
  void setAccelTable(FXAccelTable* acceltable){ accelTable=acceltable; }
  void addHotKey(FXHotKey code);
  void remHotKey(FXHotKey code);
  FXbool isChildOf(const FXWindow* window) const;
  FXbool containsChild(const FXWindow* child) const;
  FXWindow* getChildAt(FXint x,FXint y) const;
  FXint numChildren() const;
  void setDefaultCursor(FXCursor* cur);
  void setDragCursor(FXCursor* cur);
  FXCursor* getDefaultCursor() const { return defaultCursor; }
  FXCursor* getDragCursor() const { return dragCursor; }
  FXint getCursorPosition(FXint& x,FXint& y,FXuint& buttons) const;
  void setTransientFor(FXWindow* main);
  FXPixel acquireColor(FXColor clr);
  void releaseColor(FXPixel pix);
  FXColor pixelColor(FXPixel pix);
  FXbool passModalEvents() const;
  virtual FXbool canFocus() const;
  FXbool isEnabled() const;
  FXbool isDefault() const;
  FXbool isActive() const;
  FXbool hasFocus() const;
  virtual void setFocus();
  virtual void killFocus();
  void acquireSelection();
  void releaseSelection();
  FXbool hasSelection() const;
  virtual void enable();
  virtual void disable();
  virtual void create();
  virtual void destroy();
  virtual void recalc();
  virtual void raise();
  virtual void lower();
  virtual void move(FXint x,FXint y);
  virtual void resize(FXint w,FXint h);
  virtual void position(FXint x,FXint y,FXint w,FXint h);
  virtual void reparent(FXComposite* newparent);
  virtual void update(FXint x,FXint y,FXint w,FXint h);
  virtual void grab();
  virtual void ungrab();
  virtual void show();
  virtual void hide();
  FXbool shown() const;
  FXbool grabbed() const;
  virtual FXbool isComposite() const;
  void clearArea(FXint x,FXint y,FXint w,FXint h);
  void clearWindow();
  FXbool underCursor() const;
  virtual void dropEnable();
  virtual void dropDisable();
  FXbool isDropEnabled() const;
  FXbool inquireDNDTypes(const FXDragType*& types,FXuint& numtypes) const;
  FXbool offeredDNDType(FXDragType type) const;
  FXDragAction inquireDNDAction() const;
  FXbool beginDrag(const FXDragType *types,FXuint numtypes);
  FXbool handleDrag(FXint x,FXint y,FXDragAction action=DRAG_COPY);
  FXbool endDrag(FXbool drop=TRUE);
  FXbool isDragging() const;
  FXbool isDropTarget() const;
  void setDragRectangle(FXint x,FXint y,FXint w,FXint h,FXbool wantupdates=TRUE);
  void clearDragRectangle();
  void acceptDrop(FXDragAction action=DRAG_ACCEPT);
  FXDragAction didAccept() const;
  FXDragType getDNDType() const;
  FXbool getDNDData(FXDragType type,FXuchar*& data,FXuint& size);
  FXbool setDNDData(FXDragType type,FXuchar*  data,FXuint  size);
  virtual FXbool contains(FXint parentx,FXint parenty) const;
  void translateCoordinatesFrom(FXint& tox,FXint& toy,const FXWindow* fromwindow,FXint fromx,FXint fromy) const;
  void translateCoordinatesTo(FXint& tox,FXint& toy,const FXWindow* towindow,FXint fromx,FXint fromy) const;
  FXPixel getBackColor() const { return backColor; }
  void setBackColor(FXPixel clr);
  void clipByChildren(FXuint yes=1);
  void linkBefore(FXWindow* sibling);
  void linkAfter(FXWindow* sibling);
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  virtual ~FXWindow();
  };
  

#endif
