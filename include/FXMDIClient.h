/********************************************************************************
*                                                                               *
*          M u l t i p l e  D o c u m e n t   C l i e n t   W i n d o w         *
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
* $Id: FXMDIClient.h,v 1.21 1998/09/21 18:45:44 jvz Exp $                    *
********************************************************************************/
#ifndef FXMDICLIENT_H
#define FXMDICLIENT_H


class FXMDIChild;
class FXMenuBar;

enum FXMDIClientStyle {
  MDICLIENT_NORMAL    = 0,              // MDI Client is normal
  MDICLIENT_MAXIMIZED = 0x00100000,     // MDI Client is maximized
  MDICLIENT_MASK      = MDICLIENT_MAXIMIZED
  };
  

// Base class for scrolled stuff
class FXMDIClient : public FXScrollArea {
  FXDECLARE(FXMDIClient)
  friend class FXMDIChild;
protected:
  FXMDIChild *active;
  FXMenuBar  *menubar;
  FXint       xmin,xmax;
  FXint       ymin,ymax;
protected:
  FXMDIClient();
  FXMDIClient(const FXMDIClient&){}
  virtual void layout();
  void recompute();
public:
  long onCmdRestore(FXObject*,FXSelector,void*);
  long onUpdRestore(FXObject*,FXSelector,void*);
  long onCmdMaximize(FXObject*,FXSelector,void*);
  long onUpdMaximize(FXObject*,FXSelector,void*);
  long onCmdMinimize(FXObject*,FXSelector,void*);
  long onUpdMinimize(FXObject*,FXSelector,void*);
  long onCmdActivateNext(FXObject*,FXSelector,void*);
  long onUpdActivateNext(FXObject*,FXSelector,void*);
  long onCmdActivatePrev(FXObject*,FXSelector,void*);
  long onUpdActivatePrev(FXObject*,FXSelector,void*);
  long onCmdTileHorizontal(FXObject*,FXSelector,void*);
  long onUpdTileHorizontal(FXObject*,FXSelector,void*);
  long onCmdTileVertical(FXObject*,FXSelector,void*);
  long onUpdTileVertical(FXObject*,FXSelector,void*);
  long onCmdCascade(FXObject*,FXSelector,void*);
  long onUpdCascade(FXObject*,FXSelector,void*);
  long onCmdDelete(FXObject*,FXSelector,void*);
  long onUpdDelete(FXObject*,FXSelector,void*);
public:
  FXMDIClient(FXComposite* p,FXMenuBar* mb=NULL,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual void recalc();
  virtual FXint getContentWidth();
  virtual FXint getContentHeight();
  FXMDIChild* getMDIChildFirst() const;
  FXMDIChild* getMDIChildLast() const;
  virtual void moveContents(FXint x,FXint y);
  virtual long handle(FXObject* sender,FXSelector key,void* data);
  FXbool isMaximized() const;
  void setMenuBar(FXMenuBar* mb);
  FXMenuBar* getMenuBar() const;
  FXMDIChild* getActiveChild() const { return active; }
  FXbool setActiveChild(FXMDIChild* child=NULL);
  virtual ~FXMDIClient();
  };


#endif
