/********************************************************************************
*                                                                               *
*                           S t r i n g   O b j e c t                           *
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
* $Id: FXString.cpp,v 1.20 1998/10/02 14:40:53 jvz Exp $                      *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"

// No string will ever get less than this size; this should reduce memory
// fragmentation during frequent string operations.  Note that this size is
// probably large enough to fit 99% of all strings in a typical GUI...
#define MINSIZE 32



/*
  Notes:
  - Use string to return data.
  - Add member functions for formatting.
  - It should be safe to compare NULL strings.
  - When comparing NULL is smallest possible string value.
  - Always FXRESIZE() in size() might be safer.
*/


/*******************************************************************************/

// Simple construct
FXString::FXString(){
  str=NULL;
  }


// Copy construct
FXString::FXString(const FXString& s){
  str=NULL;
  if(s.str){
    register FXuint sz=s.size();
    FXMALLOC(&str,FXchar,FXMAX(sz,MINSIZE));
    memcpy(str,s.str,sz);
    }
  }


// Construct and init
FXString::FXString(const FXchar* s){
  str=NULL;
  if(s){
    register FXuint sz=strlen(s)+1;
    FXMALLOC(&str,FXchar,FXMAX(sz,MINSIZE));
    memcpy(str,s,sz);
    }
  }


// Construct and init with substring
FXString::FXString(const FXchar* s,FXuint n){
  str=NULL;
  if(s){
    register FXuint t;
    if((t=strlen(s))<n) n=t;
    FXMALLOC(&str,FXchar,FXMAX((n+1),MINSIZE));
    memcpy(str,s,n);
    str[n]=0;
    }
  }


// Construct to certain size
FXString::FXString(FXuint sz){
  str=NULL;
  if(sz){
    FXMALLOC(&str,FXchar,FXMAX(sz,MINSIZE));
    str[0]=0;
    }
  }


// Construct string from two parts
FXString::FXString(const FXchar *s1,const FXchar* s2){
  register FXuint sz1,sz2,sz;
  str=NULL;
  if(s1 || s2){
    sz1=s1?strlen(s1):0;
    sz2=s2?strlen(s2):0;
    sz=sz1+sz2;
    FXMALLOC(&str,FXchar,FXMAX((sz+1),MINSIZE));
    memcpy(str,s1,sz1);
    memcpy(&str[sz1],s2,sz2);
    str[sz]=0;
    }
  }
  

// Construct and init from the nth part of delimiter-separated string
// This rather complicated constructor makes life much easier for initializing
// Buttons etc. with labels, tips, and help strings...
FXString::FXString(const FXchar* s,FXchar delim,FXuint n){
  FXuint sz;
  str=NULL;
  if(s){
    while(s[0] && n){n-=(s[0]==delim);s++;}
    if(s[0]){
      sz=0;
      while(s[sz] && s[sz]!=delim) sz++;
      if(sz){
        FXMALLOC(&str,FXchar,FXMAX((sz+1),MINSIZE));
        memcpy(str,s,sz);
        str[sz]=0;
        }
      }
    }
  }


// Construct and init from the nth part of delimiter-separated string,
// while also eliminating escape codes for hot keys.
FXString::FXString(const FXchar* s,FXchar delim,FXchar esc,FXuint n){
  FXuint i,j,sz;
  str=NULL;
  if(s){
    while(s[0] && n){n-=(s[0]==delim);s++;}
    if(s[0]){
      sz=0;
      while(s[sz] && s[sz]!=delim) sz++; 
      if(sz){
        FXMALLOC(&str,FXchar,FXMAX((sz+1),MINSIZE));
        for(i=j=0; j<sz; j++){
          if(s[j]==esc){ if(s[j+1]!=esc) continue; j++; }
          str[i++]=s[j];
          }
        str[i]=0;
        }
      }
    }
  }


// Size to capacity 
void FXString::size(FXuint sz){
  if(sz==0){                        // Sized to zero length means ditch the buffer
    FXFREE(&str);
    }
  else if(!str){                    // Allocate a buffer as we didn't have one
    FXMALLOC(&str,FXchar,FXMAX(sz,MINSIZE));
    }
  else if(sz>MINSIZE){              // No string will ever be less than MINSIZE; so this is pretty efficient!
    FXRESIZE(&str,FXchar,sz);
    }
  }

  
// Assignment
FXString& FXString::operator=(const FXString& s){
  if(this!=&s){
    if(s.str){
      register FXuint sz=s.size();
      size(sz);
      memcpy(str,s.str,sz);
      }
    else{
      FXFREE(&str);
      }
    }
  return *this;
  }


// Assign a string
FXString& FXString::operator=(const FXchar* s){
  if(s){
    register FXuint sz=strlen(s)+1;
    size(sz);
    memcpy(str,s,sz);
    }
  else{
    FXFREE(&str);
    }
  return *this;
  }


// Size of text array
FXuint FXString::size() const {
  return str ? strlen(str)+1 : 0;
  }


// Length of text only
FXuint FXString::length() const { 
  return str ? strlen(str) : 0; 
  }


// Equality 
FXbool operator==(const FXString &s1,const FXString &s2){ 
  return (s1.str==s2.str) || (s1.str && s2.str && strcmp(s1.str,s2.str)==0); 
  }

FXbool operator==(const FXString &s1,const FXchar *s2){ 
  return (s1.str==s2) || (s1.str && s2 && strcmp(s1.str,s2)==0); 
  }

FXbool operator==(const FXchar *s1,const FXString &s2){ 
  return (s1==s2.str) || (s1 && s2.str && strcmp(s1,s2.str)==0); 
  }


// Inequality 
FXbool operator!=(const FXString &s1,const FXString &s2){ 
  return !((s1.str==s2.str) || (s1.str && s2.str && strcmp(s1.str,s2.str)==0));
  }

FXbool operator!=(const FXString &s1,const FXchar *s2){ 
  return !((s1.str==s2) || (s1.str && s2 && strcmp(s1.str,s2)==0));
  }

FXbool operator!=(const FXchar *s1,const FXString &s2){ 
  return !((s1==s2.str) || (s1 && s2.str && strcmp(s1,s2.str)==0));
  }


// Less than
FXbool operator<(const FXString &s1,const FXString &s2){ 
  return (!s1.str && s2.str) || (s1.str && s2.str && (strcmp(s1.str,s2.str)<0)); 
  }

FXbool operator<(const FXString &s1,const FXchar *s2){ 
  return (!s1.str && s2) || (s1.str && s2 && (strcmp(s1.str,s2)<0)); 
  }

FXbool operator<(const FXchar *s1,const FXString &s2){ 
  return (!s1 && s2.str) || (s1 && s2.str && (strcmp(s1,s2.str)<0)); 
  }


// Less than or equal
FXbool operator<=(const FXString &s1,const FXString &s2){ 
  return (!s1.str) || (s1.str && s2.str && (strcmp(s1.str,s2.str)<=0)); 
  }

FXbool operator<=(const FXString &s1,const FXchar *s2){ 
  return (!s1.str) || (s1.str && s2 && (strcmp(s1.str,s2)<=0)); 
  }

FXbool operator<=(const FXchar *s1,const FXString &s2){ 
  return (!s1) || (s1 && s2.str && (strcmp(s1,s2.str)<=0)); 
  }


// Greater than 
FXbool operator>(const FXString &s1,const FXString &s2){ 
  return (s1.str && !s2.str) || (s1.str && s2.str && (strcmp(s1.str,s2.str)>0)); 
  }

FXbool operator>(const FXString &s1,const FXchar *s2){ 
  return (s1.str && !s2) || (s1.str && s2 && (strcmp(s1.str,s2)>0)); 
  }

FXbool operator>(const FXchar *s1,const FXString &s2){ 
  return (s1 && !s2.str) || (s1 && s2.str && (strcmp(s1,s2.str)>0)); 
  }


// Greater than or equal
FXbool operator>=(const FXString &s1,const FXString &s2){ 
  return (!s2.str) || (s1.str && s2.str && (strcmp(s1.str,s2.str)>=0)); 
  }

FXbool operator>=(const FXString &s1,const FXchar *s2){ 
  return (!s2) || (s1.str && s2 && (strcmp(s1.str,s2)>=0)); 
  }

FXbool operator>=(const FXchar *s1,const FXString &s2){ 
  return (!s2.str) || (s1 && s2.str && (strcmp(s1,s2.str)>=0)); 
  }


// Append FXString
FXString& FXString::operator+=(const FXString& s){
  register FXuint len,n;
  if(s.str){
    n=strlen(s.str);
    if(n){
      len=length();
      size(len+n+1);
      memcpy(&str[len],s.str,n);
      str[len+n]=0;
      }
    }
  return *this;
  }


// Append string
FXString& FXString::operator+=(const FXchar* s){
  register FXuint len,n;
  if(s){
    n=strlen(s);
    if(n){
      len=length();
      size(len+n+1);
      memcpy(&str[len],s,n);
      str[len+n]=0;
      }
    }
  return *this;
  }


// Concatenate two FXStrings
FXString operator+(const FXString& s1,const FXString& s2){
  return FXString(s1.str,s2.str);
  }


// Concatenate FXString and string
FXString operator+(const FXString& s1,const FXchar* s2){
  return FXString(s1.str,s2);
  }


// Concatenate string and FXString
FXString operator+(const FXchar* s1,const FXString& s2){
  return FXString(s1,s2.str);
  }


// Fill with a constant
FXString& FXString::fill(FXchar c,FXuint len){
  size(len+1);
  memset(str,c,len);
  str[len]=0;
  return *this;
  }


// Fill up to current length
FXString& FXString::fill(FXchar c){
  if(str){memset(str,c,strlen(str));}
  return *this;
  }


// Insert string at position
FXString& FXString::insert(FXuint pos,const FXString& s){
  if(s.str){insert(pos,s.str,strlen(s.str));}
  return *this;
  }


// Insert string at position
FXString& FXString::insert(FXuint pos,const FXchar* s){
  if(s){insert(pos,s,strlen(s));}
  return *this;
  }


// Insert string at position
FXString& FXString::insert(FXuint pos,const FXchar* s,FXuint n){
  register FXuint len=length();
  if(len<pos){
    size(pos+n+1);
    memset(&str[len],' ',pos-len);    // Fill with spaces
    memcpy(&str[pos],s,n);
    str[len+pos+n]=0;
    }
  else{
    size(len+n+1);
    memmove(&str[pos+n],&str[pos],len-pos);
    memcpy(&str[pos],s,n);
    str[len+n]=0;
    }
  return *this;
  }


// Add string to the end
FXString& FXString::append(const FXString& s){
  if(s.str){append(s.str,strlen(s.str));}
  return *this;
  }


// Add string to the end
FXString& FXString::append(const FXchar *s){
  if(s){append(s,strlen(s));}
  return *this;
  }


// Add string to the end
FXString& FXString::append(const FXchar *s,FXuint n){
  register FXuint len=length();
  size(len+n+1);
  memcpy(&str[len],s,n);
  str[len+n]=0;
  return *this;
  }


// Prepend string
FXString& FXString::prepend(const FXString& s){
  if(s.str){prepend(s.str,strlen(s.str));}
  return *this;
  }


// Prepend string
FXString& FXString::prepend(const FXchar *s){
  if(s){prepend(s,strlen(s));}
  return *this;
  }


// Prepend string
FXString& FXString::prepend(const FXchar *s,FXuint n){
  register FXuint len=length();
  size(len+n+1);
  memmove(&str[n],str,len);
  memcpy(str,s,n);
  str[len+n]=0;
  return *this;
  }
  

// Remove section from buffer
FXString& FXString::remove(FXuint pos,FXuint n){
  register FXuint len=length();
  if(pos<len){
    if(pos+n<len){
      memmove(&str[pos],&str[pos+n],len-pos-n+1);
      }
    else{
      str[pos]=0;
      }
    }
  return *this;
  }


// Clean string
FXString& FXString::clear(){
  if(str) str[0]=0;
  return *this;
  }


// Get leftmost part
FXString FXString::left(FXuint n) const {
  return FXString(str,n);
  }


// Get rightmost part
FXString FXString::right(FXuint n) const {
  register FXuint len=length();
  if(n>len) n=len;
  return FXString(str+len-n,n);
  }


// Get some part in the middle
FXString FXString::mid(FXuint pos,FXuint n) const {
  register FXuint len=length();
  if(pos>len) pos=len;
  if(pos+n>len) n=len-pos;
  return FXString(str+pos,n);
  }


// Convert to lower case
FXString& FXString::lower(){
  if(str){for(FXchar* p=str; *p; p++){*p=tolower(*p);}}
  return *this;
  }


// Convert to upper case
FXString& FXString::upper(){
  if(str){for(FXchar* p=str; *p; p++){*p=toupper(*p);}}
  return *this;
  }


// Find a character, searching forward; return position or -1
FXint FXString::findf(FXchar c,FXint pos) const {
  register FXint len=length();
  if(pos<len){
    while(str[pos]){ if(str[pos]==c){ return pos; } pos++; }
    }
  return -1;
  }


// Find a character, searching backward; return position or -1
FXint FXString::findb(FXchar c,FXint pos) const {
  register FXint len=length();
  if(pos<len){
    while(0<=pos){ if(str[pos]==c){ return pos; } pos--; }
    }
  return -1;
  }


// Find number of occurances of character in string
FXuint FXString::count(FXchar c) const {
  FXuint n=0;
  if(str){for(FXchar *ptr=str; *ptr; ptr++){n+=(c==*ptr);}}
  return n;
  }


// Format
FXString& FXString::format(const char *fmt,...){
  va_list arguments;
  FXuint len;
  va_start(arguments,fmt);
  size(1024);
  len=vsprintf(str,fmt,arguments);
  FXASSERT(len<1024);
  size(len+1);
  va_end(arguments);
  return *this;
  }


// Get hash value
FXuint FXString::hash() const {
  register const FXchar *s=str;
  register FXuint h=0;
  register FXuint g;
  while(*s) {
    h=(h<<4)+*s++;
    g=h&0xF0000000UL;
    if(g) h^=g>>24;
    h&=~g;
    }
  return h;
  }
  

// Save
FXStream& operator<<(FXStream& store,const FXString& s){
  FXuint len=s.length();
  store << len;
  store.save(s.str,len+1);
  return store;
  }


// Load
FXStream& operator>>(FXStream& store,FXString& s){
  FXuint len;
  store >> len;
  s.size(len+1);
  store.load(s.str,len+1);
  return store;
  }


// Delete
FXString::~FXString(){
  FXFREE(&str);
  str=(FXchar*)-1;
  }


