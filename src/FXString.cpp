/********************************************************************************
*                                                                               *
*                           S t r i n g   O b j e c t                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXString.cpp,v 1.64 2002/02/27 20:48:36 fox Exp $                        *
********************************************************************************/
#ifdef HAVE_VSSCANF
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"


/*
  Notes:
  - The special pointer-value null represents an empty "" string.
  - A call to strlen(str) is optimized to i386 scansb instruction; we call strlen() internally,
    rather than using length().
  - Strings are never NULL:- this speeds things up a lot.
  - Whenever an empty string "" would result, we try to substitute that with null,
    rather than keep a buffer.
*/



// The string buffer is always rounded to a multiple of ROUNDVAL
// which must be 2^n.  Thus, small size changes will not result in any
// actual resizing of the buffer except when ROUNDVAL is exceeded.
#define ROUNDVAL    16

// Round up to nearest ROUNDVAL
#define ROUNDUP(n)  (((n)+ROUNDVAL-1)&-ROUNDVAL)

// Special NULL string
const FXchar FXString::null[4]={0,0,0,0};

// Numbers for hexadecimal
const FXchar FXString::hex[17]="0123456789abcdef";
const FXchar FXString::HEX[17]="0123456789ABCDEF";


/*******************************************************************************/


// Size to capacity
void FXString::size(FXint sz){
  if(sz==0){
    if(str!=null){ free(str); str=(FXchar*)null; }
    }
  else if(str==null){
    str=(FXchar*)malloc(ROUNDUP(sz)); str[0]=0;
    }
  else{
    str=(FXchar*)realloc(str,ROUNDUP(sz));
    }
  }


// Simple construct
FXString::FXString(){
  str=(FXchar*)null;
  }


// Copy construct
FXString::FXString(const FXString& s){
  str=(FXchar*)null;
  if(s.str[0]){
    register FXint len=strlen(s.str);
    size(len+1);
    memcpy(str,s.str,len+1);
    }
  }


// Construct and init
FXString::FXString(const FXchar* s){
  str=(FXchar*)null;
  if(s && s[0]){
    register FXint len=strlen(s);
    size(len+1);
    memcpy(str,s,len+1);
    }
  }


// Construct and init with substring
FXString::FXString(const FXchar* s,FXint n){
  str=(FXchar*)null;
  if(0<n){
    size(n+1);
    memcpy(str,s,n);
    str[n]=0;
    }
  }


// Construct and fill with constant
FXString::FXString(FXchar c,FXint n){
  str=(FXchar*)null;
  if(0<n){
    size(n+1);
    memset(str,c,n);
    str[n]=0;
    }
  }


// Construct string from two parts
FXString::FXString(const FXchar *s1,const FXchar* s2){
  register FXint len1=0,len2=0,len;
  if(s1 && s1[0]){ len1=strlen(s1); }
  if(s2 && s2[0]){ len2=strlen(s2); }
  len=len1+len2;
  str=(FXchar*)null;
  if(len){
    size(len+1);
    memcpy(str,s1,len1);
    memcpy(&str[len1],s2,len2);
    str[len]=0;
    }
  }


// Extract partition of delimiter separated string
FXString FXString::extract(FXint part,FXchar delim) const {
  FXString res;
  register const FXchar *s=str;
  while(s[0] && part){part-=(s[0]==delim);s++;}
  if(s[0]){
    register FXint len=0;
    while(s[len] && s[len]!=delim) len++;
    if(0<len){
      res.size(len+1);
      memcpy(res.str,s,len);
      res.str[len]=0;
      }
    }
  return res;
  }


// Extract partition of string, interpreting escapes
FXString FXString::extract(FXint part,FXchar delim,FXchar esc) const {
  FXString res;
  register const FXchar *s=str;
  while(s[0] && part){part-=(s[0]==delim);s++;}
  if(s[0]){
    register FXint len=0;
    while(s[len] && s[len]!=delim) len++;
    if(0<len){
      register FXint i,j;
      res.size(len+1);
      for(i=j=0; j<len; j++){
        if(s[j]==esc){ if(s[j+1]!=esc) continue; j++; }
        res.str[i++]=s[j];
        }
      res.str[i]=0;
      }
    }
  return res;
  }


// Assignment
FXString& FXString::operator=(const FXString& s){
  if(str!=s.str){
    if(s.str[0]){
      register FXint len=strlen(s.str);
      size(len+1);
      memcpy(str,s.str,len+1);
      }
    else{
      size(0);
      }
    }
  return *this;
  }


// Assign a string
FXString& FXString::operator=(const FXchar* s){
  if(str!=s){
    if(s && s[0]){
      register FXint len=strlen(s);
      size(len+1);
      memcpy(str,s,len+1);
      }
    else{
      size(0);
      }
    }
  return *this;
  }


// Size of text array, including end of string marker
FXint FXString::size() const {
  return (str!=null) ? strlen(str)+1 : 0;
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


// Concatenate FXString and character
FXString operator+(const FXString& s,FXchar c){
  FXchar string[2];
  string[0]=c;
  string[1]=0;
  return FXString(s.str,string);
  }


// Concatenate character and FXString
FXString operator+(FXchar c,const FXString& s){
  FXchar string[2];
  string[0]=c;
  string[1]=0;
  return FXString(string,s.str);
  }


// Fill with a constant
FXString& FXString::fill(FXchar c,FXint n){
  if(0<n){
    size(n+1);
    memset(str,c,n);
    str[n]=0;
    }
  else{
    size(0);
    }
  return *this;
  }


// Fill up to current length
FXString& FXString::fill(FXchar c){
  if(str!=null){
    register FXchar *p=str;
    register FXchar cc=c;
    while(*p){ *p++=cc; }
    }
  return *this;
  }


// Insert character at position
FXString& FXString::insert(FXint pos,FXchar c){
  register FXint len=strlen(str);
  if(pos<=0){
    size(len+2);
    memmove(&str[1],str,len+1);
    str[0]=c;
    }
  else if(pos>=len){
    size(len+2);
    str[len]=c;
    str[len+1]=0;
    }
  else{
    size(len+2);
    memmove(&str[pos+1],&str[pos],len-pos+1);
    str[pos]=c;
    }
  return *this;
  }


// Insert n characters c at specified position
FXString& FXString::insert(FXint pos,FXchar c,FXint n){
  if(0<n){
    register FXint len=strlen(str);
    if(pos<=0){
      size(len+n+1);
      memmove(&str[n],str,len+1);
      memset(str,c,n);
      }
    else if(pos>=len){
      size(len+n+1);
      memset(&str[len],c,n);
      str[len+n]=0;
      }
    else{
      size(len+n+1);
      memmove(&str[pos+n],&str[pos],len-pos+1);
      memset(&str[pos],c,n);
      }
    }
  return *this;
  }



// Insert string at position
FXString& FXString::insert(FXint pos,const FXchar* s,FXint n){
  if(0<n){
    register FXint len=strlen(str);
    if(pos<=0){
      size(len+n+1);
      memmove(&str[n],str,len+1);
      memcpy(str,s,n);
      }
    else if(pos>=len){
      size(len+n+1);
      memcpy(&str[len],s,n);
      str[len+n]=0;
      }
    else{
      size(len+n+1);
      memmove(&str[pos+n],&str[pos],len-pos+1);
      memcpy(&str[pos],s,n);
      }
    }
  return *this;
  }


// Insert string at position
FXString& FXString::insert(FXint pos,const FXString& s){
  insert(pos,s.str,strlen(s.str));
  return *this;
  }


// Insert string at position
FXString& FXString::insert(FXint pos,const FXchar* s){
  insert(pos,s,strlen(s));
  return *this;
  }


// Add character to the end
FXString& FXString::append(FXchar c){
  register FXint len=strlen(str);
  size(len+2);
  str[len]=c;
  str[len+1]=0;
  return *this;
  }


// Append input n characters c to this string
FXString& FXString::append(FXchar c,FXint n){
  if(0<n){
    register FXint len=strlen(str);
    size(len+n+1);
    memset(&str[len],c,n);
    str[len+n]=0;
    }
  return *this;
  }


// Add string to the end
FXString& FXString::append(const FXchar *s,FXint n){
  if(0<n){
    register FXint len=strlen(str);
    size(len+n+1);
    memcpy(&str[len],s,n);
    str[len+n]=0;
    }
  return *this;
  }


// Add string to the end
FXString& FXString::append(const FXString& s){
  append(s.str,strlen(s.str));
  return *this;
  }


// Add string to the end
FXString& FXString::append(const FXchar *s){
  append(s,strlen(s));
  return *this;
  }


// Append FXString
FXString& FXString::operator+=(const FXString& s){
  append(s.str,strlen(s.str));
  return *this;
  }


// Append string
FXString& FXString::operator+=(const FXchar* s){
  append(s,strlen(s));
  return *this;
  }


// Append character
FXString& FXString::operator+=(FXchar c){
  append(c);
  return *this;
  }


// Prepend character
FXString& FXString::prepend(FXchar c){
  register FXint len=strlen(str);
  size(len+2);
  memmove(&str[1],str,len+1);
  str[0]=c;
  return *this;
  }


// Prepend string
FXString& FXString::prepend(const FXchar *s,FXint n){
  if(0<n){
    register FXint len=strlen(str);
    size(len+n+1);
    memmove(&str[n],str,len+1);
    memcpy(str,s,n);
    }
  return *this;
  }


// Prepend string with n characters c
FXString& FXString::prepend(FXchar c,FXint n){
  if(0<n){
    register FXint len=strlen(str);
    size(len+n+1);
    memmove(&str[n],str,len+1);
    memset(str,c,n);
    }
  return *this;
  }


// Prepend string
FXString& FXString::prepend(const FXString& s){
  prepend(s.str,strlen(s.str));
  return *this;
  }


// Prepend string
FXString& FXString::prepend(const FXchar *s){
  prepend(s,strlen(s));
  return *this;
  }


// Replace character in string
FXString& FXString::replace(FXint pos,FXchar c){
  register FXint len=strlen(str);
  if(pos<0){
    size(len+2);
    memmove(&str[1],str,len+1);
    str[0]=c;
    }
  else if(pos>=len){
    size(len+2);
    str[len]=c;
    str[len+1]=0;
    }
  else{
    str[pos]=c;
    }
  return *this;
  }


// Replace the m characters at pos with n characters c
FXString& FXString::replace(FXint pos,FXint m,FXchar c,FXint n){
  register FXint len=strlen(str);
  if(pos+m<=0){
    if(0<n){
      size(len+n+1);
      memmove(&str[pos+n],str,len+1);
      memset(str,c,n);
      }
    }
  else if(len<=pos){
    if(0<n){
      size(len+n+1);
      memset(&str[len],c,n);
      str[len+n]=0;
      }
    }
  else{
    if(pos<0){m+=pos;pos=0;}
    if(pos+m>len){m=len-pos;}
    if(m<n){
      size(len-m+n+1);
      memmove(&str[pos+n],&str[pos+m],len-pos-m+1);
      }
    else if(m>n){
      memmove(&str[pos+n],&str[pos+m],len-pos-m+1);
      }
    if(0<n){
      memset(&str[pos],c,n);
      }
    }
  return *this;
  }


// Replace part of string
FXString& FXString::replace(FXint pos,FXint m,const FXchar *s,FXint n){
  register FXint len=strlen(str);
  if(pos+m<=0){
    if(0<n){
      size(len+n+1);
      memmove(&str[pos+n],str,len+1);
      memcpy(str,s,n);
      }
    }
  else if(len<=pos){
    if(0<n){
      size(len+n+1);
      memcpy(&str[len],s,n);
      str[len+n]=0;
      }
    }
  else{
    if(pos<0){m+=pos;pos=0;}
    if(pos+m>len){m=len-pos;}
    if(m<n){
      size(len-m+n+1);
      memmove(&str[pos+n],&str[pos+m],len-pos-m+1);
      }
    else if(m>n){
      memmove(&str[pos+n],&str[pos+m],len-pos-m+1);
      }
    if(0<n){
      memcpy(&str[pos],s,n);
      }
    }
  return *this;
  }


// Replace part of string
FXString& FXString::replace(FXint pos,FXint m,const FXString& s){
  replace(pos,m,s.str,strlen(s.str));
  return *this;
  }


// Replace part of string
FXString& FXString::replace(FXint pos,FXint m,const FXchar *s){
  replace(pos,m,s,strlen(s));
  return *this;
  }


// Remove section from buffer
FXString& FXString::remove(FXint pos,FXint n){
  if(0<n){
    register FXint len=strlen(str);
    if(pos<len && pos+n>0){
      if(pos<0){n+=pos;pos=0;}
      if(pos+n>len){n=len-pos;}
      memmove(&str[pos],&str[pos+n],len-pos-n+1);
      }
    }
  return *this;
  }


// Substitute one character by another
FXString& FXString::substitute(FXchar orig,FXchar sub){
  if(str[0]){
    register FXchar *s=str;
    do{ if(*s==orig){ *s=sub; } }while(*++s);
    }
  return *this;
  }


// Remove leading and trailing whitespace
FXString& FXString::trim(){
  if(str!=null){
    register FXint s=0;
    register FXint e=strlen(str);
    while(0<e && isspace((FXuchar)str[e-1])) e--;
    while(s<e && isspace((FXuchar)str[s])) s++;
    if(s<e) memmove(str,&str[s],e-s);
    str[e-s]=0;
    }
  return *this;
  }


// Remove leading whitespace
FXString& FXString::trimBegin(){
  if(str!=null){
    register FXint s=0;
    register FXint x=0;
    while(isspace((FXuchar)str[s])) s++;
    while(str[s]) str[x++]=str[s++];
    str[x]=0;
    }
  return *this;
  }


// Remove trailing whitespace
FXString& FXString::trimEnd(){
  if(str!=null){
    register FXint e=strlen(str);
    while(0<e && isspace((FXuchar)str[e-1])) e--;
    str[e]=0;
    }
  return *this;
  }


// Truncate string
FXString& FXString::trunc(FXint pos){
  if(str!=null){
    if(pos<0) pos=0;
    if(pos<(FXint)strlen(str)) str[pos]=0;
    }
  return *this;
  }


// Clean string
FXString& FXString::clear(){
  if(str!=null) str[0]=0;
  return *this;
  }


// Get leftmost part
FXString FXString::left(FXint n) const {
  if(0<n){
    register FXint len=strlen(str);
    if(n>len) n=len;
    return FXString(str,n);
    }
  return FXString::null;
  }


// Get rightmost part
FXString FXString::right(FXint n) const {
  if(0<n){
    register FXint len=strlen(str);
    if(n>len) n=len;
    return FXString(str+len-n,n);
    }
  return FXString::null;
  }


// Get some part in the middle
FXString FXString::mid(FXint pos,FXint n) const {
  if(0<n){
    register FXint len=strlen(str);
    if(pos<len && pos+n>0){
      if(pos<0){n+=pos;pos=0;}
      if(pos+n>len){n=len-pos;}
      return FXString(str+pos,n);
      }
    }
  return FXString::null;
  }


// Return all characters before the nth occurrence of ch,
// counting from the beginning of the string if n>0, or
// from the end if n<0.
FXString FXString::before(FXchar c,FXint n) const {
  register FXchar *end;
  register FXchar cc=c;
  if(n){
    if(0<n){
      end=str;
      while(*end){
        if(*end==cc){
          if(--n==0) break;
          }
        end++;
        }
      }
    else{
      end=str+strlen(str);
      while(str<end){
        end--;
        if(*end==cc){
          if(++n==0) break;
          }
        }
      }
    return FXString(str,end-str);
    }
  return FXString::null;
  }


// Return all characters after the nth occurrence of ch,
// counting from the beginning of the string if n>0, or
// from the end if n<0.
FXString FXString::after(FXchar c,FXint n) const {
  register FXchar *beg,*end;
  register FXchar cc=c;
  if(n){
    end=str+strlen(str);
    if(0<n){
      beg=str;
      while(*beg){
        beg++;
        if(*(beg-1)==cc){
          if(--n==0) break;
          }
        }
      }
    else{
      beg=end;
      while(str<beg){
        if(*(beg-1)==cc){
          if(++n==0) break;
          }
        beg--;
        }
      }
    return FXString(beg,end-beg);
    }
  return FXString::null;
  }


// Convert to lower case
FXString& FXString::lower(){
  register FXuchar *p=(FXuchar*)str;
  while(*p){ *p=tolower(*p); p++; }
  return *this;
  }


// Convert to upper case
FXString& FXString::upper(){
  register FXuchar *p=(FXuchar*)str;
  while(*p){ *p=toupper(*p); p++; }
  return *this;
  }


// Compare strings
FXint compare(const FXchar *s1,const FXchar *s2){
  register const FXuchar *p1=(const FXuchar *)s1;
  register const FXuchar *p2=(const FXuchar *)s2;
  register FXint c1,c2;
  do{
    c1=*p1++;
    c2=*p2++;
    }
  while(c1 && (c1==c2));
  return c1-c2;
  }

FXint compare(const FXchar *s1,const FXString &s2){
  return compare(s1,s2.str);
  }

FXint compare(const FXString &s1,const FXchar *s2){
  return compare(s1.str,s2);
  }

FXint compare(const FXString &s1,const FXString &s2){
  return compare(s1.str,s2.str);
  }


// Compare strings up to n
FXint compare(const FXchar *s1,const FXchar *s2,FXint n){
  register const FXuchar *p1=(const FXuchar *)s1;
  register const FXuchar *p2=(const FXuchar *)s2;
  register FXint c1,c2;
  if(0<n){
    do{
      c1=*p1++;
      c2=*p2++;
      }
    while(--n && c1 && (c1==c2));
    return c1-c2;
    }
  return 0;
  }

FXint compare(const FXchar *s1,const FXString &s2,FXint n){
  return compare(s1,s2.str,n);
  }

FXint compare(const FXString &s1,const FXchar *s2,FXint n){
  return compare(s1.str,s2,n);
  }

FXint compare(const FXString &s1,const FXString &s2,FXint n){
  return compare(s1.str,s2.str,n);
  }


// Compare strings case insensitive
FXint comparecase(const FXchar *s1,const FXchar *s2){
  register const FXuchar *p1=(const FXuchar *)s1;
  register const FXuchar *p2=(const FXuchar *)s2;
  register FXint c1,c2;
  do{
    c1=tolower(*p1++);
    c2=tolower(*p2++);
    }
  while(c1 && (c1==c2));
  return c1-c2;
  }

FXint comparecase(const FXchar *s1,const FXString &s2){
  return comparecase(s1,s2.str);
  }

FXint comparecase(const FXString &s1,const FXchar *s2){
  return comparecase(s1.str,s2);
  }

FXint comparecase(const FXString &s1,const FXString &s2){
  return comparecase(s1.str,s2.str);
  }


// Compare strings case insensitive up to n
FXint comparecase(const FXchar *s1,const FXchar *s2,FXint n){
  register const FXuchar *p1=(const FXuchar *)s1;
  register const FXuchar *p2=(const FXuchar *)s2;
  register FXint c1,c2;
  if(0<n){
    do{
      c1=tolower(*p1++);
      c2=tolower(*p2++);
      }
    while(--n && c1 && (c1==c2));
    return c1-c2;
    }
  return 0;
  }

FXint comparecase(const FXchar *s1,const FXString &s2,FXint n){
  return comparecase(s1,s2.str,n);
  }

FXint comparecase(const FXString &s1,const FXchar *s2,FXint n){
  return comparecase(s1.str,s2,n);
  }

FXint comparecase(const FXString &s1,const FXString &s2,FXint n){
  return comparecase(s1.str,s2.str,n);
  }

/// Comparison operators
FXbool operator==(const FXString &s1,const FXString &s2){
  return compare(s1.str,s2.str)==0;
  }

FXbool operator==(const FXString &s1,const FXchar *s2){
  return compare(s1.str,s2)==0;
  }

FXbool operator==(const FXchar *s1,const FXString &s2){
  return compare(s1,s2.str)==0;
  }

FXbool operator!=(const FXString &s1,const FXString &s2){
  return compare(s1.str,s2.str)!=0;
  }

FXbool operator!=(const FXString &s1,const FXchar *s2){
  return compare(s1.str,s2)!=0;
  }

FXbool operator!=(const FXchar *s1,const FXString &s2){
  return compare(s1,s2.str)!=0;
  }

FXbool operator<(const FXString &s1,const FXString &s2){
  return compare(s1.str,s2.str)<0;
  }

FXbool operator<(const FXString &s1,const FXchar *s2){
  return compare(s1.str,s2)<0;
  }

FXbool operator<(const FXchar *s1,const FXString &s2){
  return compare(s1,s2.str)<0;
  }

FXbool operator<=(const FXString &s1,const FXString &s2){
  return compare(s1.str,s2.str)<=0;
  }

FXbool operator<=(const FXString &s1,const FXchar *s2){
  return compare(s1.str,s2)<=0;
  }

FXbool operator<=(const FXchar *s1,const FXString &s2){
  return compare(s1,s2.str)<=0;
  }

FXbool operator>(const FXString &s1,const FXString &s2){
  return compare(s1.str,s2.str)>0;
  }

FXbool operator>(const FXString &s1,const FXchar *s2){
  return compare(s1.str,s2)>0;
  }

FXbool operator>(const FXchar *s1,const FXString &s2){
  return compare(s1,s2.str)>0;
  }

FXbool operator>=(const FXString &s1,const FXString &s2){
  return compare(s1.str,s2.str)>=0;
  }

FXbool operator>=(const FXString &s1,const FXchar *s2){
  return compare(s1.str,s2)>=0;
  }

FXbool operator>=(const FXchar *s1,const FXString &s2){
  return compare(s1,s2.str)>=0;
  }


// Find a character, searching forward; return position or -1
FXint FXString::findf(FXchar c,FXint pos) const {
  if(0<=pos){
    register FXint len=strlen(str);
    register FXint p=pos;
    register FXchar cc=c;
    while(p<len){ if(str[p]==cc){ return p; } p++; }
    }
  return -1;
  }


// Find a character, searching backward; return position or -1
FXint FXString::findb(FXchar c,FXint pos) const {
  if(0<=pos){
    register FXint len=strlen(str);
    register FXint p=pos;
    register FXchar cc=c;
    if(p>=len) p=len-1;
    while(0<=p){ if(str[p]==cc){ return p; } p--; }
    }
  return -1;
  }


// Find a substring of length n, searching forward; return position or -1
FXint FXString::findf(const FXchar* substr,FXint n,FXint pos) const {
  register FXint len=strlen(str);
  if(0<=pos && 0<n && n<=len){
    register FXchar c=substr[0];
    len=len-n+1;
    while(pos<len){
      if(str[pos]==c){
        if(!compare(str+pos,substr,n)){
          return pos;
          }
        }
      pos++;
      }
    }
  return -1;
  }


// Find a substring of length n, searching backward; return position or -1
FXint FXString::findb(const FXchar* substr,FXint n,FXint pos) const {
  register FXint len=strlen(str);
  if(0<=pos && 0<n && n<=len){
    register FXchar c=substr[0];
    len-=n;
    if(pos>len) pos=len;
    while(0<=pos){
      if(str[pos]==c){
        if(!compare(str+pos,substr,n)){
          return pos;
          }
        }
      pos--;
      }
    }
  return -1;
  }


// Find a substring, searching forward; return position or -1
FXint FXString::findf(const FXchar* substr,FXint pos) const {
  return findf(substr,strlen(substr),pos);
  }

// Find a substring, searching backward; return position or -1
FXint FXString::findb(const FXchar* substr,FXint pos) const {
  return findb(substr,strlen(substr),pos);
  }

// Find a substring, searching forward; return position or -1
FXint FXString::findf(const FXString &substr,FXint pos) const {
  return findf(substr.text(),substr.length(),pos);
  }

// Find a substring, searching backward; return position or -1
FXint FXString::findb(const FXString &substr,FXint pos) const {
  return findb(substr.text(),substr.length(),pos);
  }


// Find number of occurances of character in string
FXint FXString::count(FXchar c) const {
  register FXchar *s=str;
  register FXint n=0;
  while(*s){ n+=(c==*s); s++; }
  return n;
  }


// Get hash value
FXint FXString::hash() const {
  register const FXchar *s=str;
  register FXint h=0;
  register FXint g;
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
  FXint sz=s.size();
  store << sz;
  if(sz){store.save(s.str,sz);}
  return store;
  }


// Load
FXStream& operator>>(FXStream& store,FXString& s){
  FXint sz;
  store >> sz;
  s.size(sz);
  if(sz){store.load(s.str,sz);}
  return store;
  }


// Print formatted string a-la vprintf
FXString& FXString::vformat(const char *fmt,va_list args){
  register FXint len=0;
  if(fmt && *fmt){
    register FXint s=strlen(fmt);
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1)
    s+=128;                    // Add a bit of slop
    while(1){
      size(s+1);
      len=vsnprintf(str,s+1,fmt,args)+1;
      if(len<=s) break;
      s=len;
      }
#elif (__GLIBC__ == 2) || defined(WIN32)
    s+=128;                    // Add a bit of slop
    do{
      size(s+1);
      len=vsnprintf(str,s+1,fmt,args)+1;
      s<<=1;                  // Double the buffer size if it didn't fit
      }
    while(len<=0);
#else
    s+=1024;                  // Add a lot of slop
    size(s+1);
    len=vsprintf(str,fmt,args)+1;
#endif
    }
  size(len);
  return *this;
  }


// Print formatted string a-la printf
FXString& FXString::format(const char *fmt,...){
  va_list args;
  va_start(args,fmt);
  vformat(fmt,args);
  va_end(args);
  return *this;
  }


// Furnish our own version if we have to
#ifndef HAVE_VSSCANF
extern "C" int vsscanf(const char* str, const char* format, va_list arg_ptr);
#endif


// Scan
FXint FXString::vscan(const char *fmt,va_list args) const {
  return vsscanf((char*)str,fmt,args);          // Cast needed for HP-UX 11, which has wrong prototype for vsscanf
  }


FXint FXString::scan(const char *fmt,...) const {
  FXint result;
  va_list args;
  va_start(args,fmt);
  result=vscan(fmt,args);
  va_end(args);
  return result;
  }


// Format a string a-la vprintf
FXString FXStringVFormat(const FXchar *fmt,va_list args){
  FXString result;
  result.vformat(fmt,args);
  return result;
  }


// Format a string a-la printf
FXString FXStringFormat(const FXchar *fmt,...){
  FXString result;
  va_list args;
  va_start(args,fmt);
  result.vformat(fmt,args);
  va_end(args);
  return result;
  }


// Conversion of integer to string
FXString FXStringVal(FXint num,FXint base){
  FXchar buf[34];
  register FXchar *p=buf+33;
  register FXuint nn=(FXuint)num;
  if(base<2 || base>16){fxerror("FXStringVal: base out of range\n");}
  if(num<0){nn=(FXuint)(~num)+1;}
  *p='\0';
  do{
    *--p=FXString::HEX[nn%base];
    nn/=base;
    }
  while(nn);
  if(num<0) *--p='-';
  FXASSERT(buf<=p);
  return FXString(p,buf+33-p);
  }


// Conversion of unsigned integer to string
FXString FXStringVal(FXuint num,FXint base){
  FXchar buf[34];
  register FXchar *p=buf+33;
  register FXuint nn=num;
  if(base<2 || base>16){fxerror("FXStringVal: base out of range\n");}
  *p='\0';
  do{
    *--p=FXString::HEX[nn%base];
    nn/=base;
    }
  while(nn);
  FXASSERT(buf<=p);
  return FXString(p,buf+33-p);
  }


// Formatting for reals
static const char *const expo[]={"%.*f","%.*E","%.*G"};


// Conversion of float to string
FXString FXStringVal(FXfloat num,FXint prec,FXbool exp){
  return FXStringFormat(expo[exp],prec,num);
  }


// Conversion of double to string
FXString FXStringVal(FXdouble num,FXint prec,FXbool exp){
  return FXStringFormat(expo[exp],prec,num);
  }


// Value of digit 0-9,A-F
static inline FXint digitvalue(FXchar c){
  return (c<='9') ? (FXint)(c-'0') : (FXint)(toupper(c)-'A'+10);
  }


// Conversion of string to integer
FXint FXIntVal(const FXString& s,FXint base){
  return (FXint)strtol(s.str,NULL,base);
  }


// Conversion of string to unsigned integer
FXuint FXUIntVal(const FXString& s,FXint base){
  return (FXuint)strtoul(s.str,NULL,base);
  }


// Conversion of string to float
FXfloat FXFloatVal(const FXString& s){
  return (FXfloat)strtod(s.str,NULL);
  }


// Conversion of string to double
FXdouble FXDoubleVal(const FXString& s){
  return strtod(s.str,NULL);
  }


// Delete
FXString::~FXString(){
  size(0);
  }


