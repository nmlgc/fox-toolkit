/********************************************************************************
*                                                                               *
*                                  X - O b j e c t                              *
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
* $Id: FXId.h,v 1.4 1998/09/18 05:54:02 jeroen Exp $                            *
********************************************************************************/
#ifndef FXID_H
#define FXID_H



class FXId : public FXObject {
  FXDECLARE(FXId)
private:
  FXApp *app;
protected:
  FXID   xid;
protected:
  FXId();
  FXId(const FXId&){}
public:

  // Make ID in an app
  FXId(FXApp* a,FXID i=0);
  
  // Get application
  FXApp* getApp() const { return app; }
  
  // Get display
  Display* getDisplay() const { return app->display; }

  // Get XID handle
  FXID id() const { return xid; }

  // Save object to stream
  virtual void save(FXStream& store) const;
  
  // Load object from stream
  virtual void load(FXStream& store);
  
  // Cleanup
  virtual ~FXId();
  };
  
    
#endif
