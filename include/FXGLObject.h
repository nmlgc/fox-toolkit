/********************************************************************************
*                                                                               *
*                           O p e n G L   O b j e c t                           *
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
* $Id: FXGLObject.h,v 1.18 1998/10/26 23:31:13 jvz Exp $                     *
********************************************************************************/
#ifndef FXGLOBJECT_H
#define FXGLOBJECT_H


class FXGLViewer;

class FXGLObject;


// List of objects
typedef FXObjectListOf<FXGLObject> FXGLObjectList;


// Basic OpenGL object
class FXGLObject : public FXObject {
  FXDECLARE(FXGLObject)
protected:
  FXGLObject(const FXGLObject&){}
public:
  enum{
    ID_LAST=FXGLViewer::ID_LAST
    };
public:
  FXGLObject();
  virtual void bounds(FXRange& box);
  virtual void draw(FXGLViewer* viewer);
  virtual void hit(FXGLViewer* viewer);
  virtual FXGLObject* identify(FXuint* path);
  virtual FXbool canDrag() const;
  virtual FXbool drag(FXGLViewer* viewer,FXint fx,FXint fy,FXint tx,FXint ty);
  virtual FXbool select(FXGLViewer* viewer);
  virtual FXbool deselect(FXGLViewer* viewer);
  virtual FXuint stamp(FXGLViewer* viewer) const;
  virtual ~FXGLObject();
  };


  
// Group object
class FXGLGroup : public FXGLObject {
  FXDECLARE(FXGLGroup)
protected:
  FXGLObjectList  list;       // List of all objects
  FXint           current;    // Current object (-1 if no current object)
protected:
  FXGLGroup(const FXGLGroup&){}
public:
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
public:
  FXGLGroup();
  virtual void bounds(FXRange& box);
  virtual void draw(FXGLViewer* viewer);
  virtual void hit(FXGLViewer* viewer);
  virtual FXGLObject* identify(FXuint* path);
  virtual FXbool canDrag() const;
  virtual FXbool drag(FXGLViewer* viewer,FXint fx,FXint fy,FXint tx,FXint ty);
  FXuint no() const { return list.no(); }
  FXGLObject* const& childAt(FXuint i) const { return list[i]; }
  FXGLObject*& childAt(FXuint i){ return list[i]; }
  void insert(FXuint pos,FXGLObject* obj){ list.insert(pos,obj); }
  void append(FXGLObject* obj){ list.append(obj); }
  void remove(FXGLObject* obj){ list.remove(obj); }
  void remove(FXuint pos){ list.remove(pos); }
  void clear(){ list.clear(); }
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  virtual ~FXGLGroup();
  };

  
// Test
class FXGLPoint : public FXGLObject {
  FXDECLARE(FXGLPoint)
public:
  FXVec  pos;
protected:
  FXGLPoint(const FXGLPoint&){}
public:
  FXGLPoint();
  FXGLPoint(FXfloat x,FXfloat y,FXfloat z);
  virtual void bounds(FXRange& box);
  virtual void draw(FXGLViewer* viewer);
  virtual void hit(FXGLViewer* viewer);
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  };

  
class FXGLLine : public FXGLObject {
  FXDECLARE(FXGLLine)
public:
  FXGLPoint fm,to;
protected:
  FXGLLine(const FXGLLine&){}
public:
  FXGLLine();
  FXGLLine(FXfloat fx,FXfloat fy,FXfloat fz,FXfloat tx,FXfloat ty,FXfloat tz);
  virtual void bounds(FXRange& box);
  virtual void draw(FXGLViewer* viewer);
  virtual void hit(FXGLViewer* viewer);
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  };

  
class FXGLCube : public FXGLObject {
  FXDECLARE(FXGLCube)
public:
  FXfloat xmin,xmax,ymin,ymax,zmin,zmax;
  FXVec   color;
  FXuint  shading;
protected:
  FXGLCube(const FXGLCube&){}
public:
  enum {
    ID_SHADEOFF=FXGLObject::ID_LAST,
    ID_SHADEON,
    ID_SHADESMOOTH,
    ID_LAST
    };
public:
  long onDNDDrop(FXObject*,FXSelector,void*);
  long onDNDMotion(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
  long onCmdShadeOff(FXObject*,FXSelector,void*);
  long onCmdShadeOn(FXObject*,FXSelector,void*);
  long onCmdShadeSmooth(FXObject*,FXSelector,void*);
  long onUpdShadeOff(FXObject*,FXSelector,void*);
  long onUpdShadeOn(FXObject*,FXSelector,void*);
  long onUpdShadeSmooth(FXObject*,FXSelector,void*);
public:
  FXGLCube();
  FXGLCube(FXfloat xl,FXfloat xh,FXfloat yl,FXfloat yh,FXfloat zl,FXfloat zh);
  virtual void bounds(FXRange& box);
  virtual void draw(FXGLViewer* viewer);
  virtual void hit(FXGLViewer* viewer);
  virtual FXbool canDrag() const;
  virtual FXbool drag(FXGLViewer* viewer,FXint fx,FXint fy,FXint tx,FXint ty);
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  };

#endif

