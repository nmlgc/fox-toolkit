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
* $Id: FXDebugTarget.cpp,v 1.8 1998/10/30 15:49:38 jvz Exp $                 *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXDebugTarget.h"

/*

  Notes:
  - The point of this object is simply to accept all messages,
    and print out where they came from, which type they were, etc.
  - So purely for debugging purposes.
*/


/*******************************************************************************/


// Table of message type names
const char *const messageTypeName[]={
  "SEL_NONE",
  "SEL_KEYPRESS",           
  "SEL_KEYRELEASE",         
  "SEL_LEFTBUTTONPRESS",    
  "SEL_LEFTBUTTONRELEASE",  
  "SEL_MIDDLEBUTTONPRESS",  
  "SEL_MIDDLEBUTTONRELEASE",
  "SEL_RIGHTBUTTONPRESS",   
  "SEL_RIGHTBUTTONRELEASE", 
  "SEL_MOTION",             
  "SEL_ENTER",
  "SEL_LEAVE",
  "SEL_FOCUSIN",
  "SEL_FOCUSOUT",
  "SEL_KEYMAP",
  "SEL_PAINT",              
  "SEL_CREATE",
  "SEL_DESTROY",
  "SEL_UNMAP",
  "SEL_MAP",
  "SEL_CONFIGURE",          
  "SEL_SELECTION_LOST",     
  "SEL_SELECTION_GAINED",   
  "SEL_SELECTION_REQUEST",  
  "SEL_RAISED",
  "SEL_LOWERED",
  "SEL_VISIBILITY",         
  "SEL_CLOSE",              
  "SEL_UPDATE",             
  "SEL_COMMAND",            
  "SEL_CLICKED",            
  "SEL_DOUBLECLICKED",      
  "SEL_TRIPLECLICKED",      
  "SEL_CHANGED",            
  "SEL_DESELECTED",         
  "SEL_SELECTED",           
  "SEL_INSERTED",           
  "SEL_DELETED",           
  "SEL_OPENED",             
  "SEL_CLOSED",             
  "SEL_EXPANDED",           
  "SEL_COLLAPSED",          
  "SEL_DRAGGED",            
  "SEL_TIMEOUT",            
  "SEL_CHORE",              
  "SEL_FOCUS_RIGHT",        
  "SEL_FOCUS_LEFT",
  "SEL_FOCUS_DOWN",
  "SEL_FOCUS_UP",
  "SEL_FOCUS_HOME",
  "SEL_FOCUS_END",
  "SEL_FOCUS_NEXT",
  "SEL_FOCUS_PREV",
  "SEL_DND_ENTER",          
  "SEL_DND_LEAVE",          
  "SEL_DND_DROP",           
  "SEL_DND_MOTION",         
  "SEL_ACTIVATE",           
  "SEL_DEACTIVATE",         
  "SEL_UNCHECK_OTHER",      
  "SEL_UNCHECK_RADIO",      
  };
  

// Map
FXDEFMAP(FXDebugTarget) FXDebugTargetMap[]={
  FXMAPTYPES(SEL_KEYPRESS,SEL_LAST,FXDebugTarget::onMessage),
  };


// Object implementation
FXIMPLEMENT(FXDebugTarget,FXObject,FXDebugTargetMap,ARRAYNUMBER(FXDebugTargetMap))


// Init
FXDebugTarget::FXDebugTarget(){
  lastsender=NULL;
  lastsel=0;
  count=0;
  }


// Got one
long FXDebugTarget::onMessage(FXObject* sender,FXSelector sel,void* ptr){
  FXuint type=SELTYPE(sel);
  FXuint msid=SELID(sel);
  FXASSERT(ARRAYNUMBER(messageTypeName)==SEL_LAST);
  if(sender!=lastsender || sel!=lastsel){
    fprintf(stderr,"\nTYPE:%-23s ID:%-5d SENDER: %-15s PTR: 0x%08lx #%-4d",type<SEL_LAST?messageTypeName[type]:"ILLEGAL",msid,sender?sender->getClassName():"NULL",(unsigned long)ptr,1);
    lastsender=sender;
    lastsel=sel;
    count=1;
    }
  else{
    count++;
    fprintf(stderr,"\b\b\b\b%-4d",count);
    }
  return 0;
  }
