/********************************************************************************
*                                                                               *
*                           S o c k e t   C l a s s                             *
*                                                                               *
*********************************************************************************
* Copyright (C) 2005,2013 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxascii.h"
#include "FXArray.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXPath.h"
#include "FXIO.h"
#include "FXIODevice.h"
#include "FXSocket.h"



/*
  Notes:

  - Obviously this will get fleshed out some more...
*/


#ifdef WIN32
#define BadHandle INVALID_HANDLE_VALUE
#else
#define BadHandle -1
#endif


using namespace FX;

/*******************************************************************************/

namespace FX {



// Construct file and attach existing handle h
FXSocket::FXSocket(FXInputHandle h,FXuint m){
  open(h,m);
  }


// Open device with access mode and handle
FXbool FXSocket::open(FXInputHandle h,FXuint m){
  return FXIODevice::open(h,m);
  }


// Read block
FXival FXSocket::readBlock(void* data,FXival count){
  FXival nread=-1;
  if(isOpen()){
#ifdef WIN32
    DWORD nr;
    if(::ReadFile(device,data,(DWORD)count,&nr,NULL)!=0){
      nread=(FXival)nr;
      }
#else
    do{
      nread=::read(device,data,count);
      }
    while(nread<0 && errno==EINTR);
#endif
    }
  return nread;
  }


// Write block
FXival FXSocket::writeBlock(const void* data,FXival count){
  FXival nwritten=-1;
  if(isOpen()){
#ifdef WIN32
    DWORD nw;
    if(::WriteFile(device,data,(DWORD)count,&nw,NULL)!=0){
      nwritten=(FXival)nw;
      }
#else
    do{
      nwritten=::write(device,data,count);
      }
    while(nwritten<0 && errno==EINTR);
#endif
    }
  return nwritten;
  }


// Close socket
FXbool FXSocket::close(){
  if(isOpen()){
    if(access&OwnHandle){
#ifdef WIN32
      if(::CloseHandle(device)!=0){
        device=BadHandle;
        access=NoAccess;
        return true;
        }
#else
      if(::close(device)==0){
        device=BadHandle;
        access=NoAccess;
        return true;
        }
#endif
      }
    device=BadHandle;
    access=NoAccess;
    }
  return false;
  }


// Destroy
FXSocket::~FXSocket(){
  close();
  }


}
