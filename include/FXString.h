/********************************************************************************
*                                                                               *
*                           S t r i n g   O b j e c t                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2005 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXString.h,v 1.74 2005/01/16 16:06:06 fox Exp $                          *
********************************************************************************/
#ifndef FXSTRING_H
#define FXSTRING_H

namespace FX {


/**
* FXString provides essential string manipulation capabilities.
*/
class FXAPI FXString {
private:
  FXchar* str;
public:
  static const FXchar null[];
  static const FXchar hex[17];
  static const FXchar HEX[17];
public:

  /// Create empty string
  FXString();

  /// Copy construct
  FXString(const FXString& s);

  /// Construct and init
  FXString(const FXchar* s);

  /// Construct and init with substring
  FXString(const FXchar* s,FXint n);

  /// Construct and fill with constant
  FXString(FXchar c,FXint n);

  /// Construct string from two parts
  FXString(const FXchar* s1,const FXchar* s2);

  /// Change the length of the string to len
  void length(FXint len);

  /// Length of text
  FXint length() const { return *(((FXint*)str)-1); }

  /// Get text contents
  const FXchar* text() const { return (const FXchar*)str; }

  /// See if string is empty
  FXbool empty() const { return (((FXint*)str)[-1]==0); }

  /// Return a non-const reference to the ith character
  FXchar& operator[](FXint i){ return str[i]; }

  /// Return a const reference to the ith character
  const FXchar& operator[](FXint i) const { return str[i]; }

  /// Assign another string to this
  FXString& operator=(const FXString& s);

  /// Assign a C-style string to this
  FXString& operator=(const FXchar* s);

  /// Fill with a constant
  FXString& fill(FXchar c,FXint n);

  /// Fill up to current length
  FXString& fill(FXchar c);

  /// Convert to lower case
  FXString& lower();

  /// Convert to upper case
  FXString& upper();

  /// Return num partition(s) beginning at start from a string separated by delimiters delim.
  FXString section(FXchar delim,FXint start,FXint num=1) const;

  /// Return num partition(s) beginning at start from a string separated by set of delimiters from delim of size n
  FXString section(const FXchar* delim,FXint n,FXint start,FXint num) const;

  /// Return num partition(s) beginning at start from a string separated by set of delimiters from delim.
  FXString section(const FXchar* delim,FXint start,FXint num=1) const;

  /// Return num partition(s) beginning at start from a string separated by set of delimiters from delim.
  FXString section(const FXString& delim,FXint start,FXint num=1) const;

  /// Assign character c to this string
  FXString& assign(FXchar c);

  /// Assign n characters c to this string
  FXString& assign(FXchar c,FXint n);

  /// Assign first n characters of string s to this string
  FXString& assign(const FXchar *s,FXint n);

  /// Assign string s to this string
  FXString& assign(const FXString& s);

  /// Assign string s to this string
  FXString& assign(const FXchar* s);

  /// Insert character at specified position
  FXString& insert(FXint pos,FXchar c);

  /// Insert n characters c at specified position
  FXString& insert(FXint pos,FXchar c,FXint n);

  /// Insert first n characters of string at specified position
  FXString& insert(FXint pos,const FXchar* s,FXint n);

  /// Insert string at specified position
  FXString& insert(FXint pos,const FXString& s);

  /// Insert string at specified position
  FXString& insert(FXint pos,const FXchar* s);

  /// Prepend string with input character
  FXString& prepend(FXchar c);

  /// Prepend string with n characters c
  FXString& prepend(FXchar c,FXint n);

  /// Prepend string with first n characters of input string
  FXString& prepend(const FXchar* s,FXint n);

  /// Prepend string with input string
  FXString& prepend(const FXString& s);

  /// Prepend string with input string
  FXString& prepend(const FXchar* s);

  /// Append input character to this string
  FXString& append(FXchar c);

  /// Append input n characters c to this string
  FXString& append(FXchar c,FXint n);

  /// Append first n characters of input string to this string
  FXString& append(const FXchar* s,FXint n);

  /// Append input string to this string
  FXString& append(const FXString& s);

  /// Append input string to this string
  FXString& append(const FXchar* s);

  /// Replace a single character
  FXString& replace(FXint pos,FXchar c);

  /// Replace the m characters at pos with n characters c
  FXString& replace(FXint pos,FXint m,FXchar c,FXint n);

  /// Replaces the m characters at pos with first n characters of input string
  FXString& replace(FXint pos,FXint m,const FXchar* s,FXint n);

  /// Replace the m characters at pos with input string
  FXString& replace(FXint pos,FXint m,const FXString& s);

  /// Replace the m characters at pos with input string
  FXString& replace(FXint pos,FXint m,const FXchar* s);

  /// Remove substring
  FXString& remove(FXint pos,FXint n=1);

  /// Return number of occurrences of ch in string
  FXint contains(FXchar ch);

  /// Return number of occurrences of string sub in string
  FXint contains(const FXchar* sub,FXint n);

  /// Return number of occurrences of string sub in string
  FXint contains(const FXchar* sub);

  /// Return number of occurrences of string sub in string
  FXint contains(const FXString& sub);

  /// Substitute one character by another
  FXString& substitute(FXchar org,FXchar sub,FXbool all=TRUE);

  /// Substitute one string by another
  FXString& substitute(const FXchar* org,FXint olen,const FXchar *rep,FXint rlen,FXbool all=TRUE);

  /// Substitute one string by another
  FXString& substitute(const FXchar* org,const FXchar *rep,FXbool all=TRUE);

  /// Substitute one string by another
  FXString& substitute(const FXString& org,const FXString& rep,FXbool all=TRUE);

  /// Simplify whitespace in string
  FXString& simplify();

  /// Remove leading and trailing whitespace
  FXString& trim();

  /// Remove leading whitespace
  FXString& trimBegin();

  /// Remove trailing whitespace
  FXString& trimEnd();

  /// Truncate string at pos
  FXString& trunc(FXint pos);

  /// Clear
  FXString& clear();

  /// Get left most part
  FXString left(FXint n) const;

  /// Get right most part
  FXString right(FXint n) const;

  /// Get some part in the middle
  FXString mid(FXint pos,FXint n) const;

  /**
  * Return all characters before the n-th occurrence of ch,
  * searching from the beginning of the string. If the character
  * is not found, return the entire string.  If n<=0, return
  * the empty string.
  */
  FXString before(FXchar ch,FXint n=1) const;

  /**
  * Return all characters before the n-th occurrence of ch,
  * searching from the end of the string. If the character
  * is not found, return the empty string. If n<=0, return
  * the entire string.
  */
  FXString rbefore(FXchar ch,FXint n=1) const;

  /**
  * Return all characters after the nth occurrence of ch,
  * searching from the beginning of the string. If the character
  * is not found, return the empty string.  If n<=0, return
  * the entire string.
  */
  FXString after(FXchar ch,FXint n=1) const;

  /**
  * Return all characters after the nth occurrence of ch,
  * searching from the end of the string. If the character
  * is not found, return the entire string. If n<=0, return
  * the empty string.
  */
  FXString rafter(FXchar ch,FXint n=1) const;

  /// Find a character, searching forward; return position or -1
  FXint find(FXchar c,FXint pos=0) const;

  /// Find a character, searching backward; return position or -1
  FXint rfind(FXchar c,FXint pos=2147483647) const;

  /// Find n-th occurrence of character, searching forward; return position or -1
  FXint find(FXchar c,FXint pos,FXint n) const;

  /// Find n-th occurrence of character, searching backward; return position or -1
  FXint rfind(FXchar c,FXint pos,FXint n) const;

  /// Find a substring of length n, searching forward; return position or -1
  FXint find(const FXchar* substr,FXint n,FXint pos) const;

  /// Find a substring of length n, searching backward; return position or -1
  FXint rfind(const FXchar* substr,FXint n,FXint pos) const;

  /// Find a substring, searching forward; return position or -1
  FXint find(const FXchar* substr,FXint pos=0) const;

  /// Find a substring, searching backward; return position or -1
  FXint rfind(const FXchar* substr,FXint pos=2147483647) const;

  /// Find a substring, searching forward; return position or -1
  FXint find(const FXString& substr,FXint pos=0) const;

  /// Find a substring, searching backward; return position or -1
  FXint rfind(const FXString& substr,FXint pos=2147483647) const;

  /// Find first character in the set of size n, starting from pos; return position or -1
  FXint find_first_of(const FXchar* set,FXint n,FXint pos) const;

  /// Find first character in the set, starting from pos; return position or -1
  FXint find_first_of(const FXchar* set,FXint pos=0) const;

  /// Find first character in the set, starting from pos; return position or -1
  FXint find_first_of(const FXString& set,FXint pos=0) const;

  /// Find first character, starting from pos; return position or -1
  FXint find_first_of(FXchar c,FXint pos=0) const;

  /// Find last character in the set of size n, starting from pos; return position or -1
  FXint find_last_of(const FXchar* set,FXint n,FXint pos) const;

  /// Find last character in the set, starting from pos; return position or -1
  FXint find_last_of(const FXchar* set,FXint pos=2147483647) const;

  /// Find last character in the set, starting from pos; return position or -1
  FXint find_last_of(const FXString& set,FXint pos=2147483647) const;

  /// Find last character, starting from pos; return position or -1
  FXint find_last_of(FXchar c,FXint pos=0) const;

  /// Find first character NOT in the set of size n, starting from pos; return position or -1
  FXint find_first_not_of(const FXchar* set,FXint n,FXint pos) const;

  /// Find first character NOT in the set, starting from pos; return position or -1
  FXint find_first_not_of(const FXchar* set,FXint pos=0) const;

  /// Find first character NOT in the set, starting from pos; return position or -1
  FXint find_first_not_of(const FXString& set,FXint pos=0) const;

  /// Find first character NOT equal to c, starting from pos; return position or -1
  FXint find_first_not_of(FXchar c,FXint pos=0) const;

  /// Find last character NOT in the set of size n, starting from pos; return position or -1
  FXint find_last_not_of(const FXchar* set,FXint n,FXint pos) const;

  /// Find last character NOT in the set, starting from pos; return position or -1
  FXint find_last_not_of(const FXchar* set,FXint pos=2147483647) const;

  /// Find last character NOT in the set, starting from pos; return position or -1
  FXint find_last_not_of(const FXString& set,FXint pos=2147483647) const;

  /// Find last character NOT equal to c, starting from pos; return position or -1
  FXint find_last_not_of(FXchar c,FXint pos=0) const;

  /// Find number of occurrences of character in string
  FXint count(FXchar c) const;

  /// Format a string a-la printf
  FXString& format(const char* fmt,...) FX_PRINTF(2,3) ;
  FXString& vformat(const char* fmt,va_list args);

  /// Scan a string a-la scanf
  FXint scan(const char* fmt,...) const FX_SCANF(2,3) ;
  FXint vscan(const char* fmt,va_list args) const;

  /// Get hash value
  FXuint hash() const;

  /// Compare
  friend FXAPI FXint compare(const FXchar* s1,const FXchar* s2);
  friend FXAPI FXint compare(const FXchar* s1,const FXString& s2);
  friend FXAPI FXint compare(const FXString& s1,const FXchar* s2);
  friend FXAPI FXint compare(const FXString& s1,const FXString& s2);

  /// Compare up to n
  friend FXAPI FXint compare(const FXchar* s1,const FXchar* s2,FXint n);
  friend FXAPI FXint compare(const FXchar* s1,const FXString& s2,FXint n);
  friend FXAPI FXint compare(const FXString& s1,const FXchar* s2,FXint n);
  friend FXAPI FXint compare(const FXString& s1,const FXString& s2,FXint n);

  /// Compare case insensitive
  friend FXAPI FXint comparecase(const FXchar* s1,const FXchar* s2);
  friend FXAPI FXint comparecase(const FXchar* s1,const FXString& s2);
  friend FXAPI FXint comparecase(const FXString& s1,const FXchar* s2);
  friend FXAPI FXint comparecase(const FXString& s1,const FXString& s2);

  /// Compare case insensitive up to n
  friend FXAPI FXint comparecase(const FXchar* s1,const FXchar* s2,FXint n);
  friend FXAPI FXint comparecase(const FXchar* s1,const FXString& s2,FXint n);
  friend FXAPI FXint comparecase(const FXString& s1,const FXchar* s2,FXint n);
  friend FXAPI FXint comparecase(const FXString& s1,const FXString& s2,FXint n);

  /// Comparison operators
  friend FXAPI FXbool operator==(const FXString& s1,const FXString& s2);
  friend FXAPI FXbool operator==(const FXString& s1,const FXchar* s2);
  friend FXAPI FXbool operator==(const FXchar* s1,const FXString& s2);

  friend FXAPI FXbool operator!=(const FXString& s1,const FXString& s2);
  friend FXAPI FXbool operator!=(const FXString& s1,const FXchar* s2);
  friend FXAPI FXbool operator!=(const FXchar* s1,const FXString& s2);

  friend FXAPI FXbool operator<(const FXString& s1,const FXString& s2);
  friend FXAPI FXbool operator<(const FXString& s1,const FXchar* s2);
  friend FXAPI FXbool operator<(const FXchar* s1,const FXString& s2);

  friend FXAPI FXbool operator<=(const FXString& s1,const FXString& s2);
  friend FXAPI FXbool operator<=(const FXString& s1,const FXchar* s2);
  friend FXAPI FXbool operator<=(const FXchar* s1,const FXString& s2);

  friend FXAPI FXbool operator>(const FXString& s1,const FXString& s2);
  friend FXAPI FXbool operator>(const FXString& s1,const FXchar* s2);
  friend FXAPI FXbool operator>(const FXchar* s1,const FXString& s2);

  friend FXAPI FXbool operator>=(const FXString& s1,const FXString& s2);
  friend FXAPI FXbool operator>=(const FXString& s1,const FXchar* s2);
  friend FXAPI FXbool operator>=(const FXchar* s1,const FXString& s2);

  /// Append operators
  FXString& operator+=(const FXString& s);
  FXString& operator+=(const FXchar* s);
  FXString& operator+=(FXchar c);

  /// Concatenate two strings
  friend FXAPI FXString operator+(const FXString& s1,const FXString& s2);
  friend FXAPI FXString operator+(const FXString& s1,const FXchar* s2);
  friend FXAPI FXString operator+(const FXchar* s1,const FXString& s2);

  /// Concatenate with single character
  friend FXAPI FXString operator+(const FXString& s,FXchar c);
  friend FXAPI FXString operator+(FXchar c,const FXString& s);

  /// Saving to a stream
  friend FXAPI FXStream& operator<<(FXStream& store,const FXString& s);

  /// Load from a stream
  friend FXAPI FXStream& operator>>(FXStream& store,FXString& s);

  /// Format a string a-la printf
  friend FXAPI FXString FXStringFormat(const FXchar* fmt,...) FX_PRINTF(1,2) ;
  friend FXAPI FXString FXStringVFormat(const FXchar* fmt,va_list args);

  /**
  * Convert integer number to a string, using the given number
  * base, which must be between 2 and 16.
  */
  friend FXAPI FXString FXStringVal(FXint num,FXint base=10);
  friend FXAPI FXString FXStringVal(FXuint num,FXint base=10);

  /**
  * Convert long integer number to a string, using the given number
  * base, which must be between 2 and 16.
  */
  friend FXAPI FXString FXStringVal(FXlong num,FXint base=10);
  friend FXAPI FXString FXStringVal(FXulong num,FXint base=10);

  /**
  * Convert real number to a string, using the given procision and
  * exponential notation mode, which may be FALSE (never), TRUE (always), or
  * MAYBE (when needed).
  */
  friend FXAPI FXString FXStringVal(FXfloat num,FXint prec=6,FXbool exp=MAYBE);
  friend FXAPI FXString FXStringVal(FXdouble num,FXint prec=6,FXbool exp=MAYBE);

  /// Convert string to a integer number, assuming given number base
  friend FXAPI FXint FXIntVal(const FXString& s,FXint base=10);
  friend FXAPI FXuint FXUIntVal(const FXString& s,FXint base=10);

  /// Convert string to long integer number, assuming given number base
  friend FXAPI FXlong FXLongVal(const FXString& s,FXint base=10);
  friend FXAPI FXulong FXULongVal(const FXString& s,FXint base=10);

  /// Convert string into real number
  friend FXAPI FXfloat FXFloatVal(const FXString& s);
  friend FXAPI FXdouble FXDoubleVal(const FXString& s);

  /// Escape special characters in a string
  friend FXAPI FXString escape(const FXString& s);

  /// Unescape special characters in a string
  friend FXAPI FXString unescape(const FXString& s);

  /// Swap two strings
  friend FXAPI void swap(FXString& a,FXString& b){ FXchar *t=a.str; a.str=b.str; b.str=t; }

  /// Delete
 ~FXString();
  };


}

#endif
