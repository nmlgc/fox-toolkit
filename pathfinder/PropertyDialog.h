/********************************************************************************
*                                                                               *
*                  F i l e   P r o p e r t i e s   D i a l o g                  *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2005 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This program is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU General Public License as published by          *
* the Free Software Foundation; either version 2 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This program is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU General Public License for more details.                                  *
*                                                                               *
* You should have received a copy of the GNU General Public License             *
* along with this program; if not, write to the Free Software                   *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: PropertyDialog.h,v 1.13 2005/01/16 16:06:06 fox Exp $                    *
********************************************************************************/
#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H


// Property dialog
class PropertyDialog : public FXDialogBox {
  FXDECLARE(PropertyDialog)
protected:
  FXLabel       *filename;
  FXLabel       *filetype;
  FXLabel       *filesize;
  FXLabel       *directory;
  FXLabel       *createtime;
  FXLabel       *modifytime;
  FXLabel       *accesstime;
  FXTextField   *fileowner;
  FXTextField   *filegroup;
  FXList        *extensions;
  FXList        *mimetypes;
  FXTextField   *description;
  FXTextField   *command;
  FXCheckButton *runinterminal;
  FXCheckButton *changedirectory;
  FXButton      *bigopen;
  FXButton      *bigclosed;
  FXButton      *smallopen;
  FXButton      *smallclosed;
private:
  PropertyDialog(){}
  PropertyDialog(const PropertyDialog&);
public:
  enum{
    ID_ADD_MIMETYPE=FXDialogBox::ID_LAST,
    ID_REMOVE_MIMETYPE,
    ID_CHANGE_MIMETYPE
    };
public:
  long onCmdAddMimeType(FXObject*,FXSelector,void*);
  long onCmdRemoveMimeType(FXObject*,FXSelector,void*);
  long onCmdChangeMimeType(FXObject*,FXSelector,void*);
public:
  PropertyDialog(FXWindow *owner);
  void setBigIconOpen(FXIcon* icon);
  void setBigIconClosed(FXIcon* icon);
  FXIcon* getBigIconOpen() const;
  FXIcon* getBigIconClosed() const;
  void setSmallIconOpen(FXIcon* icon);
  void setSmallIconClosed(FXIcon* icon);
  FXIcon* getSmallIconOpen() const;
  FXIcon* getSmallIconClosed() const;
  void setCommand(const FXString& cmd);
  FXString getCommand() const;
  virtual ~PropertyDialog();
  };

#endif
