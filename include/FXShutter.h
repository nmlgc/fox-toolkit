/********************************************************************************
*                                                                               *
*                 V e r t i c a l   C o n t a i n e r   O b j e c t             *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXShutter.h,v 1.6 1998/07/22 03:02:18 jeroen Exp $                       *
********************************************************************************/
#ifndef FXSHUTTER_H
#define FXSHUTTER_H


// Shutter Control
class FXShutterItem : public FXVerticalFrame {
  FXDECLARE(FXShutterItem)
  friend class FXShutter;  
protected:
  FXButton         *button;
  FXScrollWindow   *scrollWindow;
  FXVerticalFrame  *content;
protected:
  FXShutterItem(){}
  FXShutterItem(const FXShutterItem&){}
public:
  long onFocusUp(FXObject*,FXSelector,void*);
  long onFocusDown(FXObject*,FXSelector,void*);
  long onCmdButton(FXObject*,FXSelector,void*);
  long onHideShutterItem(FXObject* sender,FXSelector sel,void* ptr);
public:
  enum{
    ID_SHUTTERITEM_BUTTON=FXVerticalFrame::ID_LAST,
    ID_LAST
    };
public:
  FXShutterItem(FXComposite *p,const char* text=NULL,FXIcon* ic=NULL,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
  virtual void create();
  FXVerticalFrame* getContent() const { return content; }
  virtual ~FXShutterItem();
  };


// Shutter Control
class FXShutter : public FXVerticalFrame {
  FXDECLARE(FXShutter)
  friend class FXShutterItem;  
protected:
  FXTimer       *timer;                 // Timer for animation
  FXShutterItem *selectedItem;          // Item currently open
  FXShutterItem *closingItem;           // Item closing down
  FXint          heightIncrement;       // Height delta
  FXint          closingHeight;         // Closing items current height
  FXbool         closingHadScrollbar;   // Closing item had a scroll bar
protected:
  FXShutter(){}
  FXShutter(const FXShutter&){}
public:
  long onFocusUp(FXObject*,FXSelector,void*);
  long onFocusDown(FXObject*,FXSelector,void*);
  long onTimeout(FXObject* sender,FXSelector sel,void* ptr);
  long onOpenItem(FXObject* sender,FXSelector sel,void* ptr);
public:
  enum{
    ID_SHUTTER_TIMEOUT=FXVerticalFrame::ID_LAST,
    ID_OPEN_SHUTTERITEM,
    ID_LAST
    };
public:
  FXShutter(FXComposite *p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
  virtual void layout();
  virtual ~FXShutter();
  };


#endif
