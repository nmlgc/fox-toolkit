/********************************************************************************
*                                                                               *
*                 T o p - L e v e l   W i n d o w   W i d g e t                 *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: FXTopWindow.h,v 1.36 2002/01/18 22:42:55 jeroen Exp $                    *
********************************************************************************/
#ifndef FXTOPWINDOW_H
#define FXTOPWINDOW_H

#ifndef FXSHELL_H
#include "FXShell.h"
#endif



/// Title and border decorations
enum {
  DECOR_NONE     = 0,                 /// Borderless window
  DECOR_TITLE    = 0x00020000,        /// Window title
  DECOR_MINIMIZE = 0x00040000,        /// Minimize button
  DECOR_MAXIMIZE = 0x00080000,        /// Maximize button
  DECOR_CLOSE    = 0x00100000,        /// Close button
  DECOR_BORDER   = 0x00200000,        /// Border
  DECOR_RESIZE   = 0x00400000,        /// Resize handles
  DECOR_MENU     = 0x00800000,        /// Window menu
  DECOR_ALL      = (DECOR_TITLE|DECOR_MINIMIZE|DECOR_MAXIMIZE|DECOR_CLOSE|DECOR_BORDER|DECOR_RESIZE|DECOR_MENU)
  };


/// Initial window placement
enum {
  PLACEMENT_DEFAULT,                  /// Place it at the default size and location
  PLACEMENT_VISIBLE,                  /// Place window to be fully visible
  PLACEMENT_CURSOR,                   /// Place it under the cursor position
  PLACEMENT_OWNER,                    /// Place it centered on its owner
  PLACEMENT_SCREEN,                   /// Place it centered on the screen
  PLACEMENT_MAXIMIZED                 /// Place it maximized to the screen size
  };


class FXToolbar;



/// Abstract base class for all top-level windows
class FXAPI FXTopWindow : public FXShell {
  FXDECLARE_ABSTRACT(FXTopWindow)
protected:
  FXString  title;                    // Window title
  FXIcon   *icon;                     // Window icon (big)
  FXIcon   *miniIcon;                 // Window icon (small)
  FXint     padtop;                   // Top margin
  FXint     padbottom;                // Bottom margin
  FXint     padleft;                  // Left margin
  FXint     padright;                 // Right margin
  FXint     hspacing;                 // Horizontal child spacing
  FXint     vspacing;                 // Vertical child spacing
  FXint     offx;
  FXint     offy;
protected:
  FXTopWindow(){}
  void settitle();
  void seticons();
  void setdecorations();
  virtual void layout();
  FXTopWindow(FXApp* a,const FXString& name,FXIcon *ic,FXIcon *mi,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs);
  FXTopWindow(FXWindow* owner,const FXString& name,FXIcon *ic,FXIcon *mi,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs);
private:
  FXTopWindow(const FXTopWindow&);
  FXTopWindow& operator=(const FXTopWindow&);
#ifdef WIN32
  virtual const char* GetClass() const;
  static void* makeicon(FXIcon* icon);
#endif
public:
  long onClose(FXObject*,FXSelector,void*);
  long onCmdSetStringValue(FXObject*,FXSelector,void*);
  long onCmdIconify(FXObject*,FXSelector,void*);
  long onCmdDeiconify(FXObject*,FXSelector,void*);
  long onFocusUp(FXObject*,FXSelector,void*);
  long onFocusDown(FXObject*,FXSelector,void*);
  long onFocusLeft(FXObject*,FXSelector,void*);
  long onFocusRight(FXObject*,FXSelector,void*);
public:
  enum {
    ID_ICONIFY=FXShell::ID_LAST,    /// Iconify the window
    ID_DEICONIFY,                   /// Deiconify the window
    ID_QUERY_DOCK,                  /// Toolbar asks to dock
    ID_LAST
    };
public:

  /// Create server-side resources
  virtual void create();

  /// Detach the server-side resources for this window
  virtual void detach();

  /// Move the focus to this window
  virtual void setFocus();

  /// Remove the focus from this window
  virtual void killFocus();

  /// Show this window
  virtual void show();

  /// Hide this window
  virtual void hide();

  /// Show this window with given placement
  virtual void show(FXuint placement);

  /// Position the window based on placement
  void place(FXuint placement);

  /// Return the default width of this window
  virtual FXint getDefaultWidth();

  /// Return the default height of this window
  virtual FXint getDefaultHeight();

  /// Move this window to the specified position in the parent's coordinates
  virtual void move(FXint x,FXint y);

  /// Resize this window to the specified width and height
  virtual void resize(FXint w,FXint h);

  /// Move and resize this window in the parent's coordinates
  virtual void position(FXint x,FXint y,FXint w,FXint h);

  /// Iconify window
  virtual void iconify();

  /// Deiconify window
  virtual void deiconify();

  /// Return TRUE if window has been iconified
  FXbool isIconified() const;

  /// Change window title
  void setTitle(const FXString& name);

  /// Return window title
  FXString getTitle() const { return title; }

  /// Change top padding
  void setPadTop(FXint pt);

  /// Get top interior padding
  FXint getPadTop() const { return padtop; }

  /// Change bottom padding
  void setPadBottom(FXint pb);

  /// Get bottom interior padding
  FXint getPadBottom() const { return padbottom; }

  /// Change left padding
  void setPadLeft(FXint pl);

  /// Get left interior padding
  FXint getPadLeft() const { return padleft; }

  /// Change right padding
  void setPadRight(FXint pr);

  /// Get right interior padding
  FXint getPadRight() const { return padright; }

  /// Return horizontal spacing between children
  FXint getHSpacing() const { return hspacing; }

  /// Return vertical spacing between children
  FXint getVSpacing() const { return vspacing; }

  /// Change horizontal spacing between children
  void setHSpacing(FXint hs);

  /// Change vertical spacing between children
  void setVSpacing(FXint vs);

  /// Change packing hints for children
  void setPackingHints(FXuint ph);

  /// Return packing hints for children
  FXuint getPackingHints() const;

  /// Change title and border decorations
  void setDecorations(FXuint decorations);

  /// Return current title and border decorations
  FXuint getDecorations() const;

  /// Return window icon
  FXIcon* getIcon() const { return icon; }

  /// Change window icon
  void setIcon(FXIcon* ic);

  /// Return window mini (title) icon
  FXIcon* getMiniIcon() const { return miniIcon; }

  /// Change window mini (title) icon
  void setMiniIcon(FXIcon *ic);

  /// Save to stream
  virtual void save(FXStream& store) const;

  /// Load from stream
  virtual void load(FXStream& store);

  /// Destructor
  virtual ~FXTopWindow();
  };


#endif
