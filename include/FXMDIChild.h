/********************************************************************************
*                                                                               *
*          M u l t i p l e   D o c u m e n t   C h i l d   W i n d o w          *
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
* $Id: FXMDIChild.h,v 1.22 2002/01/18 22:42:53 jeroen Exp $                     *
********************************************************************************/
#ifndef FXMDICHILD_H
#define FXMDICHILD_H

#ifndef FXCOMPOSITE_H
#include "FXComposite.h"
#endif



struct FXTimer;
class FXMDIClient;
class FXMenuButton;
class FXButton;
class FXFont;


/// MDI Child Window styles
enum {
  MDI_NORMAL    = 0,                /// Normal display mode
  MDI_MAXIMIZED = 0x00001000,       /// Window appears maximized
  MDI_MINIMIZED = 0x00002000        /// Window is iconified or minimized
  };



/**
* The MDI child window contains the application
* work area in a Multiple Document Interface application.
*/
class FXAPI FXMDIChild : public FXComposite {
  FXDECLARE(FXMDIChild)
protected:
  FXString      title;
  FXMenuButton *windowbtn;
  FXButton     *minimizebtn;
  FXButton     *restorebtn;
  FXButton     *maximizebtn;
  FXButton     *deletebtn;
  FXFont       *font;
  FXColor       baseColor;
  FXColor       hiliteColor;
  FXColor       shadowColor;
  FXColor       borderColor;
  FXColor       titleColor;
  FXColor       titleBackColor;
  FXint         iconPosX;
  FXint         iconPosY;
  FXint         iconWidth;
  FXint         iconHeight;
  FXint         normalPosX;
  FXint         normalPosY;
  FXint         normalWidth;
  FXint         normalHeight;
  FXint         xoff;
  FXint         yoff;
  FXint         oldx;
  FXint         oldy;
  FXint         oldw;
  FXint         oldh;
  FXuchar       action;
private:
  FXMDIChild   *mdinext;
  FXMDIChild   *mdiprev;
protected:
  FXMDIChild();
  void drawRubberBox(FXint x,FXint y,FXint w,FXint h);
  void animateRectangles(FXint ox,FXint oy,FXint ow,FXint oh,FXint nx,FXint ny,FXint nw,FXint nh);
  FXuchar where(FXint x,FXint y);
  void changeCursor(FXint x,FXint y);
  void revertCursor();
  virtual void layout();
private:
  FXMDIChild(const FXMDIChild&);
  FXMDIChild &operator=(const FXMDIChild&);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onFocusSelf(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onRightBtnPress(FXObject*,FXSelector,void*);
  long onRightBtnRelease(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMiddleBtnPress(FXObject*,FXSelector,void*);
  long onMiddleBtnRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onDelete(FXObject*,FXSelector,void*);
  long onClose(FXObject*,FXSelector,void*);
  long onCloseAll(FXObject*,FXSelector,void*);
  long onSelected(FXObject*,FXSelector,void*);
  long onDeselected(FXObject*,FXSelector,void*);
  long onMinimize(FXObject*,FXSelector,void*);
  long onRestore(FXObject*,FXSelector,void*);
  long onMaximize(FXObject*,FXSelector,void*);
  long onCmdRestore(FXObject*,FXSelector,void*);
  long onCmdMaximize(FXObject*,FXSelector,void*);
  long onCmdMinimize(FXObject*,FXSelector,void*);
  long onCmdClose(FXObject*,FXSelector,void*);
  long onCmdDelete(FXObject*,FXSelector,void*);
  long onUpdRestore(FXObject*,FXSelector,void*);
  long onUpdMaximize(FXObject*,FXSelector,void*);
  long onUpdMinimize(FXObject*,FXSelector,void*);
  long onUpdWindow(FXObject*,FXSelector,void*);
  long onUpdMenuRestore(FXObject*,FXSelector,void*);
  long onUpdMenuMinimize(FXObject*,FXSelector,void*);
  long onUpdMenuClose(FXObject*,FXSelector,void*);
  long onUpdMenuWindow(FXObject*,FXSelector,void*);
  long onUpdClose(FXObject*,FXSelector,void*);
  long onCmdSetStringValue(FXObject*,FXSelector,void*);
  virtual long onDefault(FXObject*,FXSelector,void*);
public:

  /// Construct MDI Child window with given name and icon
  FXMDIChild(FXMDIClient* p,const FXString& name,FXIcon* ic=NULL,FXMenuPane* mn=NULL,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);

  /// Create window
  virtual void create();

  /// Detach window
  virtual void detach();

  /// Get next MDI Child
  FXMDIChild* getMDINext() const { return mdinext; }

  /// Get previous MDI Child
  FXMDIChild* getMDIPrev() const { return mdiprev; }

  /// Compute default size
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();

  /// MDI Child can receive focus
  virtual FXbool canFocus() const;

  /// Move this window to the specified position in the parent's coordinates
  virtual void move(FXint x,FXint y);

  /// Resize this window to the specified width and height
  virtual void resize(FXint w,FXint h);

  /// Move and resize this window in the parent's coordinates
  virtual void position(FXint x,FXint y,FXint w,FXint h);

  /// Change normal (restored) position
  void setNormalX(FXint x){ normalPosX=x; }
  void setNormalY(FXint y){ normalPosY=y; }
  void setNormalWidth(FXint w){ normalWidth=w; }
  void setNormalHeight(FXint h){ normalHeight=h; }

  /// Return normal (restored) position
  FXint getNormalX() const { return normalPosX; }
  FXint getNormalY() const { return normalPosY; }
  FXint getNormalWidth() const { return normalWidth; }
  FXint getNormalHeight() const { return normalHeight; }

  /// Change iconified position
  void setIconX(FXint x){ iconPosX=x; }
  void setIconY(FXint y){ iconPosY=y; }
  void setIconWidth(FXint w){ iconWidth=w; }
  void setIconHeight(FXint h){ iconHeight=h; }

  /// Return iconified position
  FXint getIconX() const { return iconPosX; }
  FXint getIconY() const { return iconPosY; }
  FXint getIconWidth() const { return iconWidth; }
  FXint getIconHeight() const { return iconHeight; }

  /// Return content window
  FXWindow *contentWindow() const;

  /// Change MDI Child's title
  void setTitle(const FXString& name);

  /// Get current title
  FXString getTitle() const { return title; }

  /// Get colors
  FXColor getHiliteColor() const { return hiliteColor; }
  FXColor getShadowColor() const { return shadowColor; }
  FXColor getBaseColor() const { return baseColor; }
  FXColor getBorderColor() const { return borderColor; }
  FXColor getTitleColor () const { return titleColor; }
  FXColor getTitleBackColor() const { return titleBackColor; }

  /// Change colors
  void setHiliteColor(FXColor clr);
  void setShadowColor(FXColor clr);
  void setBaseColor(FXColor clr);
  void setBorderColor(FXColor clr);
  void setTitleColor(FXColor clr);
  void setTitleBackColor(FXColor clr);

  /// Return TRUE if maximized
  FXbool isMaximized() const;

  /// Return TRUE if minimized
  FXbool isMinimized() const;

  /// Maximize MDI Child
  virtual void maximize(FXbool notify=FALSE);

  /// Minimize/iconify MDI Child
  virtual void minimize(FXbool notify=FALSE);

  /// Restore MDI Child to normal
  virtual void restore(FXbool notify=FALSE);

  /// Get window icon
  FXIcon *getWindowIcon() const;

  /// Set window icon
  void setWindowIcon(FXIcon* icon);

  /// Get window menu
  FXPopup* getWindowMenu() const;

  /// Set window menu
  void setWindowMenu(FXPopup* menu);

  /// Set title font
  void setFont(FXFont *fnt);

  /// Get title font
  FXFont* getFont() const { return font; }

  /// Save to stream
  virtual void save(FXStream& store) const;

  /// Load from stream
  virtual void load(FXStream& store);

  /// Destroy
  virtual ~FXMDIChild();
  };


#endif
