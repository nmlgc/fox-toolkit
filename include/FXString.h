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
* $Id: FXString.h,v 1.9 1998/10/01 05:48:03 jeroen Exp $                        *
********************************************************************************/
#ifndef FXSTRING_H
#define FXSTRING_H



class FXString {
private:
  FXchar* str;      // String data
public:
  
  // Create empty string
  FXString();
  
  // Copy construct
  FXString(const FXString& s);

  // Construct and init
  FXString(const FXchar* s);

  // Construct and init with substring
  FXString(const FXchar* s,FXuint n);

  // Construct to certain size
  FXString(FXuint sz);

  // Construct string from two parts
  FXString(const FXchar *s1,const FXchar* s2);
  
  // Construct and init from the nth part of delimiter-separated string
  FXString(const FXchar* s,FXchar delim,FXuint n);
  
  // Construct and init from the nth part of delimiter-separated string,
  // while also eliminating escape codes for hot keys.
  FXString(const FXchar* s,FXchar delim,FXchar esc,FXuint n);
  
  // Size to some desired capacity
  void size(FXuint sz);

  // Size of text data
  FXuint size() const;

  // Get text contents
  const FXchar* text() const { return (const FXchar*)str; }

  // Use whereever you use char*
  operator const FXchar*() const { return (const FXchar*)str; }
  operator FXchar*(){ return (FXchar*)str; }

  // Easy indexing
  FXchar& operator[](const FXuint i){ return str[i]; }
  const FXchar& operator[](const FXuint i) const { return str[i]; }

  // Assignment
  FXString& operator=(const FXString& s);
  FXString& operator=(const FXchar* s);

  // Length of text only
  FXuint length() const;

  // Fill with a constant
  FXString& fill(FXchar c,FXuint len);
  
  // Fill up to current length
  FXString& fill(FXchar c);
  
  // Convert to lower case
  FXString& lower();
  
  // Convert to upper case
  FXString& upper();
  
  // Insert string
  FXString& insert(FXuint pos,const FXString& s);
  FXString& insert(FXuint pos,const FXchar* s);
  FXString& insert(FXuint pos,const FXchar* s,FXuint n);
  
  // Prepend string
  FXString& prepend(const FXString& s);
  FXString& prepend(const FXchar *s);
  FXString& prepend(const FXchar *s,FXuint n);
  
  // Append string
  FXString& append(const FXString& s);
  FXString& append(const FXchar *s);
  FXString& append(const FXchar *s,FXuint n);
  
  // Remove substring
  FXString& remove(FXuint pos,FXuint n=1);
  
  // Clear
  FXString& clear();

  // Get leftmost part
  FXString left(FXuint n) const;
  
  // Get rightmost part
  FXString right(FXuint n) const;
  
  // Get some part in the middle
  FXString mid(FXuint pos,FXuint n) const;
  
  // Find a character, searching forward; return position or -1
  FXint findf(FXchar c,FXint pos=0) const;
  
  // Find a character, searching backward; return position or -1
  FXint findb(FXchar c,FXint pos=0) const;
  
  // Find number of occurances of character in string
  FXuint count(FXchar c) const;
  
  // Format a string a-la printf
  FXString& format(const char *fmt,...);
  
  // Get hash value
  FXuint hash() const;
  
  // Equality
  friend FXbool operator==(const FXString &s1,const FXString &s2);
  friend FXbool operator==(const FXString &s1,const char *s2);
  friend FXbool operator==(const FXchar *s1,const FXString &s2);
  
  // Inequality
  friend FXbool operator!=(const FXString &s1,const FXString &s2);
  friend FXbool operator!=(const FXString &s1,const FXchar *s2);
  friend FXbool operator!=(const FXchar *s1,const FXString &s2);
  
  // Less than
  friend FXbool operator<(const FXString &s1,const FXString &s2);
  friend FXbool operator<(const FXString &s1,const FXchar *s2);
  friend FXbool operator<(const FXchar *s1,const FXString &s2);
  
  // Less than or equal
  friend FXbool operator<=(const FXString &s1,const FXString &s2);
  friend FXbool operator<=(const FXString &s1,const FXchar *s2);
  friend FXbool operator<=(const FXchar *s1,const FXString &s2); 
  
  // Greater than
  friend FXbool operator>(const FXString &s1,const FXString &s2);
  friend FXbool operator>(const FXString &s1,const FXchar *s2);
  friend FXbool operator>(const FXchar *s1,const FXString &s2);
  
  // Greater than or equal
  friend FXbool operator>=(const FXString &s1,const FXString &s2);
  friend FXbool operator>=(const FXString &s1,const FXchar *s2);
  friend FXbool operator>=(const FXchar *s1,const FXString &s2);

  // Append
  FXString& operator+=(const FXString& s);
  FXString& operator+=(const FXchar* s);

  // Concatenation operators
  friend FXString operator+(const FXString& s1,const FXString& s2);
  friend FXString operator+(const FXString& s1,const FXchar* s2);
  friend FXString operator+(const FXchar* s1,const FXString& s2);

  // Saving and loading
  friend FXStream& operator<<(FXStream& store,const FXString& s);
  friend FXStream& operator>>(FXStream& store,FXString& s);

  // Delete
 ~FXString();
  };


#endif
