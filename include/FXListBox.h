/********************************************************************************
*                                                                               *
*               D r o p - D o w n   L i s t   B o x   O b j e c t               *
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
* $Id: FXListBox.h,v 1.7 1998/08/26 07:41:11 jeroen Exp $                       *
********************************************************************************/
#ifndef FXLISTBOX_H
#define FXLISTBOX_H





// Drop down list
class FXListBox : public FXPacker {
  FXDECLARE(FXListBox)
protected:
  FXTextField   *text;
  FXArrowButton *button;
  FXList        *list;
  FXPopup       *pane;
protected:
  FXListBox(){}
  FXListBox(const FXListBox&){}
  virtual void layout();
public:
  enum{
    ID_BUTTON=FXPacker::ID_LAST,
    ID_LIST,
    ID_TEXT,
    ID_LAST
    };
public:
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onCmdPost(FXObject*,FXSelector,void*);
  long onCmdUnpost(FXObject*,FXSelector,void*);
  long onCmdButton(FXObject*,FXSelector,void*);
  long onCmdList(FXObject*,FXSelector,void*);
  long onCmdText(FXObject*,FXSelector,void*);
public:
  FXListBox(FXComposite *p,FXint cols,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual void create();
  virtual void destroy();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual FXbool contains(FXint parentx,FXint parenty) const;
  FXList* getList() const { return list; }
  FXPopup* getPopup() const { return pane; }
  virtual ~FXListBox();
  };


#endif
