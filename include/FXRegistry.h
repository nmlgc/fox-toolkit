/********************************************************************************
*                                                                               *
*                           R e g i s t r y   C l a s s                         *
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
* $Id: FXRegistry.h,v 1.2 1998/03/03 06:04:38 jeroen Exp $                      *
********************************************************************************/
#ifndef FXREGISTRY_H
#define FXREGISTRY_H


class FXRegistry {

public:
  FXRegistry();
  
  // Read a registry entry
  const FXchar *readEntry(const char *key,const char *def=NULL) const;
  const FXint readEntry(const char *key,FXint def=0) const;
  const FXdouble readEntry(const char *key,FXdouble def=0.0) const;
  
  // Write a registry entry
  FXbool writeEntry(const char *key,const FXchar *val);
  FXbool writeEntry(const char *key,FXint val);
  FXbool writeEntry(const char *key,FXdouble val);
  
  // Delete a registry entry
  FXbool deleteEntry(const char *key);
  
 ~FXRegistry();
  };
  
  
#endif
