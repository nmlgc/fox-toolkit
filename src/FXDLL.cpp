/********************************************************************************
*                                                                               *
*             D y n a m i c   L i n k   L i b r a r y   S u p p o r t           *
*                                                                               *
*********************************************************************************
* Copyright (C) 2002,2005 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXDLL.cpp,v 1.15 2005/01/16 16:06:06 fox Exp $                           *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXDLL.h"
#ifndef WIN32
#include <dlfcn.h>
#endif

/*
  Notes:
  - Make sure it works on other unices.
*/


#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0           // Does not exist on DEC
#endif


using namespace FX;

/*******************************************************************************/

namespace FX {


// Open DLL and return dllhandle to it
void* fxdllOpen(const FXchar *dllname){
  if(dllname){
#ifndef WIN32
#ifdef DL_LAZY		// OpenBSD
    return dlopen(dllname,DL_LAZY);
#else			// The rest
    return dlopen(dllname,RTLD_NOW|RTLD_GLOBAL);
#endif
#else
    // Order of loading with LoadLibrary (or LoadLibraryEx with no
    // LOAD_WITH_ALTERED_SEARCH_PATH flag):
    //
    // 1. Directory from which the application was loaded.
    // 2. Current directory.
    // 3. System directory, as determined by GetSystemDirectory().
    // 4. 16-bit system directory.
    // 5. Windows directory, as determined by GetWindowsDirectory().
    // 6. Directories in the $PATH.
    //
    // With flag LOAD_WITH_ALTERED_SEARCH_PATH:
    //
    // 1. Directory specified by the filename path.
    // 2. Current directory.
    // 3. System directory.
    // 4. 16-bit system directory.
    // 5. Windows directory.
    // 6. Directories in the $PATH.
    //
    // We switched to the latter so sub-modules needed by a DLL are
    // plucked from the same place as dllname (thanks to Rafael de
    // Pelegrini Soares" <Rafael@enq.ufrgs.br>).
    //return LoadLibrary(dllname);
    return LoadLibraryEx(dllname,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
#endif
    }
  return NULL;
  }


// Close DLL of given dllhandle
void fxdllClose(void* dllhandle){
  if(dllhandle){
#ifndef WIN32
    dlclose(dllhandle);
#else
    FreeLibrary((HMODULE)dllhandle);
#endif
    }
  }


// Return address of the given symbol in library dllhandle
void* fxdllSymbol(void* dllhandle,const FXchar* dllsymbol){
  if(dllhandle && dllsymbol){
#ifndef WIN32
    return dlsym(dllhandle,dllsymbol);
#else
    return (void*)GetProcAddress((HMODULE)dllhandle,dllsymbol);
#endif
    }
  return NULL;
  }


// Return the string error message when loading dll's.
// Suggested by Rafael de Pelegrini Soares <rafael@enq.ufrgs.br>
FXString fxdllError(){
#ifdef WIN32
  DWORD dw=GetLastError();
  FXchar buffer[512];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,dw,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR)&buffer,sizeof(buffer),NULL);
  return buffer;
#else
  return dlerror();
#endif
  }


}

