/********************************************************************************
*                                                                               *
*                    D i r e c t o r y   B o x   W i d g e t                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXDirBox.h,v 1.13 2002/01/18 22:42:52 jeroen Exp $                       *
********************************************************************************/
#ifndef FXDIRBOX_H
#define FXDIRBOX_H

#ifndef FXTREELISTBOX_H
#include "FXTreeListBox.h"
#endif



class FXIcon;


/// Directory Box
class FXAPI FXDirBox : public FXTreeListBox {
  FXDECLARE(FXDirBox)
protected:
  FXString    directory;              // Current directory
  FXIcon     *foldericon;             // Folder icons
  FXIcon     *cdromicon;
  FXIcon     *desktopicon;
  FXIcon     *harddiskicon;
  FXIcon     *networkicon;
  FXIcon     *floppyicon;
  FXIcon     *computericon;
  FXIcon     *nethoodicon;
  FXIcon     *zipdiskicon;
protected:
  FXDirBox(){}
  FXString itempath(FXTreeItem *item);
private:
  FXDirBox(const FXDirBox&);
  FXDirBox &operator=(const FXDirBox&);
public:
  long onChanged(FXObject*,FXSelector,void*);
  long onCommand(FXObject*,FXSelector,void*);
  long onCmdSetValue(FXObject*,FXSelector,void*);
  long onCmdSetStringValue(FXObject*,FXSelector,void*);
  long onCmdGetStringValue(FXObject*,FXSelector,void*);
public:

  /// Constructor
  FXDirBox(FXComposite *p,FXint nvis,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=FRAME_SUNKEN|FRAME_THICK|TREELISTBOX_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);

  /// Create server-side resources
  virtual void create();

  /// Detach server-side resources
  virtual void detach();

  /// Destroy server-side resources
  virtual void destroy();

  /// Save to stream
  virtual void save(FXStream& store) const;

  /// Load from stream
  virtual void load(FXStream& store);

  /// Set current directory
  void setDirectory(const FXString& pathname);

  /// Return current directory
  FXString getDirectory() const { return directory; }

  /// Destructor
  virtual ~FXDirBox();
  };


#endif
