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
* $Id: FXRegistry.cpp,v 1.4 1998/06/03 02:08:55 jeroen Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXRegistry.h"

/*
  To do:
  - Put some meat in here!!
  - A Registry has to support:
    - File locking, to prevent corruption
    - Merging of app's (key/value) pairs back into (possibly changed)
      registry again
    - Support at least text strings
    - Should have global and per-user settings
    - Per-user settings shadow (or override) global settings
    
  Format for Registry file:
  
  <mainsectionname>
  <subsectionname>
  variable = string-with-no-spaces
  variable = "string\nwith a\nnewline in it\n"
  variable = "string with spaces"
  </subsectionname>
  </mainsectionname>
  
  Logic for determining variable value: search from current section
  outward, till you either find it or hit the root.
  Thus, per-app settings may override global settings, and per-vendor
  settings may override FOX global settings, and so on.
  Tags are case insensitive
  
  Certain predefined section names:
  
  <FOX>
  Global fox settings, such as colors, scrollbar size, and so on.
  For all FOX apps, from all vendors.
  <VENDORNAME>
  Vendor-wide settings, i.e. for all apps from vendor
  <APPNAME>
  Per-application settings, automatically determined from app name.
  ... other sections ...
  </APPNAME>
  <VENDORNAME>
  </FOX>
  Hey, this looks like HTML!
*/


/*******************************************************************************/

// Make registry object
FXRegistry::FXRegistry(){
  }


// Read a registry entry
const FXchar *FXRegistry::readEntry(const char *key,const char *def) const {
  return NULL;
  }


const FXint FXRegistry::readEntry(const char *key,FXint def) const {
  return 0;
  }


const FXdouble FXRegistry::readEntry(const char *key,FXdouble def) const{
  return 0.0;
  }


// Write a registry entry
FXbool FXRegistry::writeEntry(const char *key,const FXchar *val){
  return FALSE;
  }


FXbool FXRegistry::writeEntry(const char *key,FXint val){
  return FALSE;
  }


FXbool FXRegistry::writeEntry(const char *key,FXdouble val){
  return FALSE;
  }

// Delete a registry entry
FXbool FXRegistry::deleteEntry(const char *key){
  return FALSE;
  }

// Delete registry object
FXRegistry::~FXRegistry(){
  }
