/********************************************************************************
*                                                                               *
*                     M e n u   C o m m a n d   W i d g e t                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXMenuCommand.h,v 1.19 2002/01/18 22:42:53 jeroen Exp $                  *
********************************************************************************/
#ifndef FXMENUCOMMAND_H
#define FXMENUCOMMAND_H

#ifndef FXMENUCAPTION_H
#include "FXMenuCaption.h"
#endif



/// States the menu command can be in
enum {
  MENUSTATE_NORMAL   = 0,           /// Normal, unchecked state
  MENUSTATE_CHECKED  = 1,           /// Checked with a checkmark
  MENUSTATE_RCHECKED = 2            /// Checked with a bullet
  };


/**
* The menu command widget is used to invoke a command in the
* application from a menu.  Menu commands may reflect
* the state of the application by graying out, becoming hidden,
* or by a check mark or bullit.
*/
class FXAPI FXMenuCommand : public FXMenuCaption {
  FXDECLARE(FXMenuCommand)
protected:
  FXuchar      state;       // State of menu
  FXString     accel;       // Accelerator string
  FXHotKey     acckey;      // Accelerator key
protected:
  FXMenuCommand();
  void drawCheck(FXDCWindow& dc,FXint x,FXint y);
  void drawBullit(FXDCWindow& dc,FXint x,FXint y);
private:
  FXMenuCommand(const FXMenuCommand&);
  FXMenuCommand &operator=(const FXMenuCommand&);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onButtonPress(FXObject*,FXSelector,void*);
  long onButtonRelease(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onHotKeyPress(FXObject*,FXSelector,void*);
  long onHotKeyRelease(FXObject*,FXSelector,void*);
  long onCheck(FXObject*,FXSelector,void*);
  long onUncheck(FXObject*,FXSelector,void*);
  long onCmdSetValue(FXObject*,FXSelector,void*);
  long onCmdSetIntValue(FXObject*,FXSelector,void*);
  long onCmdGetIntValue(FXObject*,FXSelector,void*);
  long onCmdAccel(FXObject*,FXSelector,void*);
public:

  /// Construct a menu command
  FXMenuCommand(FXComposite* p,const FXString& text,FXIcon* ic=NULL,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0);

  /// Return default width
  virtual FXint getDefaultWidth();

  /// Return default height
  virtual FXint getDefaultHeight();

  /// Yes it can receive the focus
  virtual FXbool canFocus() const;

  /// Move the focus to this window
  virtual void setFocus();

  /// Remove the focus from this window
  virtual void killFocus();

  /// Place checkmark next to text
  void check();

  /// Uncheck the item
  void uncheck();

  /// Return TRUE if checked
  FXint isChecked() const;

  /// Place radio bullit next to text
  void checkRadio();

  /// Uncheck radio bullit
  void uncheckRadio();

  /// Return TRUE if radio-checked
  FXint isRadioChecked() const;

  /// Set accelerator text
  void setAccelText(const FXString& text);

  /// Return accelarator text
  FXString getAccelText() const { return accel; }

  /// Save menu to a stream
  virtual void save(FXStream& store) const;

  /// Load menu from a stream
  virtual void load(FXStream& store);

  /// Destructor
  virtual ~FXMenuCommand();
  };


#endif
