/********************************************************************************
*                                                                               *
*                   A c c e l e r a t o r   T a b l e   C l a s s               *
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
* $Id: FXAccelTable.cpp,v 1.11 1998/10/27 04:57:43 jeroen Exp $                  *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXAccelTable.h"

  
/*
  Notes:
  - Mostly complete
*/

#define DEF_HASH_SIZE      16           // Initial table size (MUST be power of 2)
#define MAX_LOAD           80           // Maximum hash table load factor (%)
#define UNUSEDSLOT         0xffffffff   // Unsused slot marker
#define EMPTYSLOT          0xfffffffe   // Previously used, now empty slot
#define HASH1(x,n) (((unsigned int)(x)*13)%(n))           // Number [0..n-1]
#define HASH2(x,n) (1|(((unsigned int)(x)*17)%((n)-1)))   // Number [1..n-1]

/*******************************************************************************/


// Accelerator entry
struct FXAccelKey {
  FXObject    *target;        // Target object of message
  FXSelector   messagedn;     // Message being sent
  FXSelector   messageup;     // Message being sent
  FXHotKey     code;          // Keysym and modifier mask to match
  };


/*******************************************************************************/

// Map
FXDEFMAP(FXAccelTable) FXAccelTableMap[]={
  FXMAPFUNC(SEL_KEYPRESS,0,FXAccelTable::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXAccelTable::onKeyRelease),
  };


// Object implementation
FXIMPLEMENT(FXAccelTable,FXObject,FXAccelTableMap,ARRAYNUMBER(FXAccelTableMap))


// Make empty accelerator table
FXAccelTable::FXAccelTable(){
  nkey=DEF_HASH_SIZE;
  FXMALLOC(&key,FXAccelKey,nkey);
  for(FXuint i=0; i<nkey; i++) key[i].code=UNUSEDSLOT;
  num=0;
//fprintf(stderr,"construct Table[%08x]\n",this);
  }


// Grow hash table, and rehash old stuff into it
void FXAccelTable::grow(){
  FXuint i,n,p,x,c;
  FXAccelKey *k;
  n=nkey<<1;
  FXMALLOC(&k,FXAccelKey,n);
  for(i=0; i<n; i++) k[i].code=UNUSEDSLOT;
  for(i=0; i<nkey; i++){
    c=key[i].code;
    if(c==UNUSEDSLOT || c==EMPTYSLOT) continue;
    p=HASH1(c,n);
    FXASSERT(p<n);
    x=HASH2(c,n);
    FXASSERT(1<=x && x<n);
    while(k[p].code!=UNUSEDSLOT) p=(p+x)%n;
    k[p]=key[i];
    }
  FXFREE(&key);
  key=k;
  nkey=n;
  }
        

// Add (or replace) accelerator
void FXAccelTable::addAccel(FXHotKey hotkey,FXObject* target,FXSelector seldn,FXSelector selup){
  if(hotkey){
    register FXuint p,i,x,c;
//fprintf(stderr,"addAccel[%08x]: code=%04x state=%04x\n",this,(FXushort)hotkey,(FXushort)(hotkey>>16));
    FXASSERT(hotkey!=UNUSEDSLOT);
    FXASSERT(hotkey!=EMPTYSLOT);
    p=HASH1(hotkey,nkey);
    FXASSERT(p<nkey);
    x=HASH2(hotkey,nkey);
    FXASSERT(1<=x && x<nkey);
    i=UNUSEDSLOT;
    while((c=key[p].code)!=UNUSEDSLOT){
      if((i==UNUSEDSLOT)&&(c==EMPTYSLOT)) i=p;
      if(c==hotkey){
        key[p].target=target;
        key[p].messagedn=seldn;
        key[p].messageup=selup;
        return;
        }
      p=(p+x)%nkey;
      }
    FXASSERT(num<nkey);
    if(i==UNUSEDSLOT) i=p;
    key[i].code=hotkey;
    key[i].target=target;
    key[i].messagedn=seldn;
    key[i].messageup=selup;
    num++;
    if((100*num)>=(MAX_LOAD*nkey)) grow();
    }
  }


// Remove accelerator
void FXAccelTable::removeAccel(FXHotKey hotkey){
  if(hotkey){
    register FXuint p,x,c;
//fprintf(stderr,"removeAccel[%08x]: code=%04x state=%04x\n",this,(FXushort)hotkey,(FXushort)(hotkey>>16));
    FXASSERT(hotkey!=UNUSEDSLOT);
    FXASSERT(hotkey!=EMPTYSLOT);
    p=HASH1(hotkey,nkey);
    FXASSERT(p<nkey);
    x=HASH2(hotkey,nkey);
    FXASSERT(1<=x && x<nkey);
    while((c=key[p].code)!=UNUSEDSLOT){
      if(c==hotkey){
        key[p].code=EMPTYSLOT;
        key[p].target=NULL;
        key[p].messagedn=0;
        key[p].messageup=0;
        num--;
        return;
        }
      p=(p+x)%nkey;
      }
    }
  }


// See if accelerator exists
FXbool FXAccelTable::hasAccel(FXHotKey hotkey) const {
  if(hotkey){
    register FXuint p,x,c;
    FXASSERT(hotkey!=UNUSEDSLOT);
    FXASSERT(hotkey!=EMPTYSLOT);
    p=HASH1(hotkey,nkey);
    FXASSERT(p<nkey);
    x=HASH2(hotkey,nkey);
    FXASSERT(1<=x && x<nkey);
    while((c=key[p].code)!=UNUSEDSLOT){
      if(c==hotkey) return TRUE;
      p=(p+x)%nkey;
      }
    }
  return FALSE;
  }


// Keyboard press; forward to focus child
long FXAccelTable::onKeyPress(FXObject* sender,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXuint p,x,code,c;
//fprintf(stderr,"%s::onKeyPress keysym=0x%04x\n",getClassName(),event->code);
  code=MKUINT(event->code,event->state);
  FXASSERT(code!=UNUSEDSLOT);
  FXASSERT(code!=EMPTYSLOT);
  p=HASH1(code,nkey);
  FXASSERT(p<nkey);
  x=HASH2(code,nkey);
  FXASSERT(1<=x && x<nkey);
  while((c=key[p].code)!=UNUSEDSLOT){
    if(c==code){ 
      if(key[p].target && key[p].messagedn){
        key[p].target->handle(sender,key[p].messagedn,ptr);
        }
      return 1;
      }
    p=(p+x)%nkey;
    }
  return 0;
  }


// Keyboard release; sent to focus widget
long FXAccelTable::onKeyRelease(FXObject* sender,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXuint p,x,code,c;
//fprintf(stderr,"%s::onKeyRelease keysym=0x%04x\n",getClassName(),event->code);
  code=MKUINT(event->code,event->state);
  FXASSERT(code!=UNUSEDSLOT);
  FXASSERT(code!=EMPTYSLOT);
  p=HASH1(code,nkey);
  FXASSERT(p<nkey);
  x=HASH2(code,nkey);
  FXASSERT(1<=x && x<nkey);
  while((c=key[p].code)!=UNUSEDSLOT){
    if(c==code){ 
      if(key[p].target && key[p].messageup){
        key[p].target->handle(sender,key[p].messageup,ptr);
        }
      return 1;
      }
    p=(p+x)%nkey;
    }
  return 0;
  }


// Save data
void FXAccelTable::save(FXStream& store) const {
  register FXint i;
  FXObject::save(store);
  store << nkey;
  store << num;
  for(i=0; i<nkey; i++){
    store << key[i].target; /////How to resolve?
    store << key[i].messagedn;
    store << key[i].messageup;
    store << key[i].code;
    }
  }


// Load data
void FXAccelTable::load(FXStream& store){
  register FXint i;
  FXObject::load(store);
  store >> nkey;
  store >> num;
  FXRESIZE(&key,FXAccelKey,nkey);
  for(i=0; i<nkey; i++){
    store >> key[i].target; /////How to resolve?
    store >> key[i].messagedn;
    store >> key[i].messageup;
    store >> key[i].code;
    }
  }


// Destroy table
FXAccelTable::~FXAccelTable(){
//fprintf(stderr,"destruct Table[%08x]\n",this);
  FXFREE(&key);
  key=(FXAccelKey*)-1;
  }
