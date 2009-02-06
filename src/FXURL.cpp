/********************************************************************************
*                                                                               *
*                       U R L   M a n i p u l a t i o n                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2009 by Jeroen van der Zijp.   All Rights Reserved.        *
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
*********************************************************************************
* $Id: FXURL.cpp,v 1.63 2009/01/06 13:24:44 fox Exp $                           *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxpriv.h"
#include "fxascii.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXPath.h"
#include "FXSystem.h"
#include "FXURL.h"


/*
  Notes:

  - Functions contributed by Sean Hubbell and Sander Jansen.

  - About drive letters in URL's, Daniel Gehriger has some some
    empirical tests, and determined the following:

     NS = works on Netscape
     IE = works on IE
     O  = works on Opera

     - file:///C|/TEMP/                    NS, IE, O
     - file:///C:/TEMP/                    NS, IE, O

     - file://localhost/C:/TEMP/           NS, IE, O
     - file://localhost/C|/TEMP/           NS, IE, O

     - file://C:/TEMP/                     NS, IE, --
     - file:///C/TEMP/                     --, --, --

    The conclusion seems to be we should probably try to handle all
    of these possibilities, although keeping the `:' seems favorable.

  - Syntax (as per rfc3986):

      URI           =  scheme ":" hier-part [ "?" query ] [ "#" fragment ]

      hier-part     =  "//" authority path-abempty
                    /  path-absolute
                    /  path-rootless
                    /  path-empty

      URI-reference =  URI / relative-ref

      absolute-URI  =  scheme ":" hier-part [ "?" query ]

      relative-ref  =  relative-part [ "?" query ] [ "#" fragment ]

      relative-part =  "//" authority path-abempty
                    /  path-absolute
                    /  path-noscheme
                    /  path-empty

      scheme        =  ALPHA  *( ALPHA / DIGIT / "+" / "-" / "." )

      authority     =  [ userinfo "@" ] host [ ":" port ]

      userinfo      =  *( unreserved / pct-encoded / sub-delims / ":" )

      host          =  IP-literal / IPv4address / reg-name

      IP-literal    =  "[" ( IPv6address / IPvFuture  ) "]"

      IPvFuture     =  "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )

      IPv6address   =                             6( h16 ":" ) ls32
                    /                        "::" 5( h16 ":" ) ls32
                    /  [               h16 ] "::" 4( h16 ":" ) ls32
                    /  [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
                    /  [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
                    /  [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
                    /  [ *4( h16 ":" ) h16 ] "::"              ls32
                    /  [ *5( h16 ":" ) h16 ] "::"              h16
                    /  [ *6( h16 ":" ) h16 ] "::"

      ls32          =  ( h16 ":" h16 ) / IPv4address                   ; least-significant 32 bits of address

      h16           = 1*4HEXDIG                                        ; 16 bits of address represented in hexadecimal


      IPv4address   =  dec-octet "." dec-octet "." dec-octet "." dec-octet

      dec-octet     =  DIGIT                                            ; 0-9
                    /  %x31-39 DIGIT                                    ; 10-99
                    /  "1" 2DIGIT                                       ; 100-199
                    /  "2" %x30-34 DIGIT                                ; 200-249
                    /  "25" %x30-35                                     ; 250-255

      reg-name      = *( unreserved / pct-encoded / sub-delims )

      port          =  *DIGIT

      path          =  path-abempty                                     ; begins with "/" or is empty
                    /  path-absolute                                    ; begins with "/" but not "//"
                    /  path-noscheme                                    ; begins with a non-colon segment
                    /  path-rootless                                    ; begins with a segment
                    /  path-empty                                       ; zero characters

      path-abempty  =  *( "/" segment )

      path-absolute =  "/" [ segment-nz *( "/" segment ) ]

      path-noscheme =  segment-nz-nc *( "/" segment )

      path-rootless =  segment-nz *( "/" segment )

      path-empty    =  0<pchar>

      segment       =  *pchar

      segment-nz    =  1*pchar

      segment-nz-nc =  1*( unreserved / pct-encoded / sub-delims / "@" ) ; non-zero-length segment without any colon ":"

      pchar         =  unreserved / pct-encoded / sub-delims / ":" / "@"

      query         =  *( pchar / "/" / "?" )

      fragment      =  *( pchar / "/" / "?" )

      pct-encoded   =  "%" HEXDIG HEXDIG

      unreserved    =  ALPHA / DIGIT / "-" / "." / "_" / "~"

      reserved      =  gen-delims / sub-delims

      gen-delims    =  ":" / "/" / "?" / "#" / "[" / "]" / "@"

      sub-delims    =  "!" / "$" / "&" / "'" / "(" / ")"
                    /  "*" / "+" / "," / ";" / "="

*/

#define UNRESERVED   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~" // Unreserved characters
#define IPV6DIGITS   "abcdefABCDEF0123456789:."                                           // Stuff in IPv6 numbers
#define PCTENCODED   "%0123456789abcdefABCDEF"                                            // Percent encoded characters
#define GENDELIMS    ":/?#[]@"                                                            // General delimiters
#define SUBDELIMS    "!$&'()*+,;="                                                        // Sub-delimiters
#define RESERVED     ":/?#[]@!$&'()*+,;="                                                 // Reserved characters (GENDELIMS + SUBDELIMS)
#define UNSAFE       "<>#%{}|^~[]`\" "                                                    // Unsafe characters
#define ENCODE_THESE "<>#%{}|^~[]`\"?$&'*,;="                                             // Encode these for pathnames

using namespace FX;

/*******************************************************************************/

namespace FX {


// URL parts
class URL {
public:
  FXint prot[2];
  FXint user[2];
  FXint pass[2];
  FXint host[2];
  FXint port[2];
  FXint path[2];
  FXint quer[2];
  FXint frag[2];
public:
  URL(const FXString& string);
  };


// Parse string to url parts
URL::URL(const FXString& string){
  register FXint s=0;

  prot[0]=prot[1]=0;

  // Parse protocol
  if(Ascii::isLetter(string[0])){
    s++;

    // Scan till end of scheme name
    while(Ascii::isAlphaNumeric(string[s]) || string[s]=='+' || string[s]=='-' || string[s]=='.') s++;

    // Scheme end found
    if(string[s]==':' && s>1){
      prot[1]=s++;
      }
    else{
      s=prot[0];                                // Reset:- wasn't protocol after all since no ':' found
      }
    }

  user[0]=user[1]=s;
  pass[0]=pass[1]=s;
  host[0]=host[1]=s;
  port[0]=port[1]=s;

  // Parse hier part
  if(string[s]=='/' && string[s+1]=='/'){
    s+=2;

    // Parse user name
    user[0]=s;
    while(string[s] && strchr(UNRESERVED SUBDELIMS "%",string[s])){
      s++;
      }
    user[1]=s;

    // Check for password
    pass[0]=pass[1]=s;
    if(string[s]==':'){
      pass[0]=++s;
      while(string[s] && strchr(UNRESERVED SUBDELIMS "%",string[s])){
        s++;
        }
      pass[1]=s;
      }

    // Check for @ after user:pass
    if(string[s]=='@'){
      s++;
      }
    else{
      s=pass[0]=pass[1]=user[1]=user[0];        // Reset:- wasn't user:pass after all since no '@' found
      }

    // Parse hostname
    host[0]=s;
    while(string[s] && strchr(UNRESERVED SUBDELIMS "%",string[s])){
      s++;
      }
    host[1]=s;

    // Check for port number
    port[0]=port[1]=s;
    if(string[s]==':'){
      port[0]=++s;
      while(Ascii::isDigit(string[s])) s++;
      port[1]=s;
      }
    }

  // Eat slash preceding the drive letter (e.g. file:///c:/path)
  if(string[s]=='/' && Ascii::isLetter(string[s+1]) && (string[s+2]==':' || string[s+2]=='|')){
    s++;
    }

  // Parse path
  path[0]=s;
  while(string[s] && strchr(UNRESERVED SUBDELIMS "%:@/ ",string[s])){
    s++;
    }
  path[1]=s;

  // Parse query
  quer[0]=quer[1]=s;
  if(string[s]=='?'){
    quer[0]=++s;
    while(string[s] && strchr(UNRESERVED SUBDELIMS "%:@/?",string[s])){
      s++;
      }
    quer[1]=s;
    }

  // Parse fragment
  frag[0]=frag[1]=s;
  if(string[s]=='#'){
    frag[0]=++s;
    while(string[s] && strchr(UNRESERVED SUBDELIMS "%:@/?",string[s])){
      s++;
      }
    frag[1]=s;
    }
  }


// Encode control characters and characters from set using %-encoding
FXString FXURL::encode(const FXString& url,const FXchar* set){
  register FXint p=0;
  register FXint c;
  FXString result;
  while(p<url.length()){
    c=url[p++];
    if(c<0x20 || c>0x7F || c=='%' || (set && strchr(set,c))){
      result.append('%');
      result.append(FXString::value2Digit[(c>>4)&15]);
      result.append(FXString::value2Digit[c&15]);
      continue;
      }
    result.append(c);
    }
  return result;
  }


// Decode string containing %-encoded characters
FXString FXURL::decode(const FXString& url){
  register FXint p=0;
  register FXint c;
  FXString result;
  while(p<url.length()){
    c=url[p++];
    if(c=='%' && Ascii::isHexDigit(url[p]) && Ascii::isHexDigit(url[p+1])){
      c=Ascii::digitValue(url[p])*16+Ascii::digitValue(url[p+1]);
      p+=2;
      }
    result.append(c);
    }
  return result;
  }


// Return URL of filename
FXString FXURL::fileToURL(const FXString& file){
#ifdef WIN32
  if(ISPATHSEP(file[0]) && ISPATHSEP(file[1])){
    return "file:"+encode(FXPath::convert(file,'/','\\'),ENCODE_THESE);         // file://share/path-with-slashes
    }
  if(Ascii::isLetter(file[0]) && file[1]==':'){
    return "file:///"+encode(FXPath::convert(file,'/','\\'),ENCODE_THESE);      // file:///c:/path-with-slashes
    }
  return "file:"+encode(FXPath::convert(file,'/','\\'),ENCODE_THESE);           // file:path-with-slashes
#else
  return "file:"+encode(file,ENCODE_THESE);                                     // file:path
#endif
  }


// Return filename from URL, empty if url is not a local file
FXString FXURL::fileFromURL(const FXString& string){
  if(!string.empty()){
#ifdef WIN32
    URL url(string);
    if(url.host[0]<url.host[1]){
      return "\\\\"+string.mid(url.host[0],url.host[1]-url.host[0])+decode(FXPath::convert(string.mid(url.path[0],url.path[1]-url.path[0]),'\\','/'));
      }
    return decode(FXPath::convert(string.mid(url.path[0],url.path[1]-url.path[0]),'\\','/'));
#else
    URL url(string);
    return decode(string.mid(url.path[0],url.path[1]-url.path[0]));
#endif
    }
  return FXString::null;
  }


// Parse scheme from url
FXString FXURL::scheme(const FXString& string){
  URL url(string);
  return string.mid(url.prot[0],url.prot[1]-url.prot[0]);
  }


// Parse username from string containing url
FXString FXURL::username(const FXString& string){
  URL url(string);
  return string.mid(url.user[0],url.user[1]-url.user[0]);
  }


// Parse password from string containing url
FXString FXURL::password(const FXString& string){
  URL url(string);
  return string.mid(url.pass[0],url.pass[1]-url.pass[0]);
  }


// Parse hostname from string containing url
FXString FXURL::host(const FXString& string){
  URL url(string);
  return string.mid(url.host[0],url.host[1]-url.host[0]);
  }


// Parse port number from string containing url
FXint FXURL::port(const FXString& string){
  register FXint result=0;
  URL url(string);
  while(url.port[0]<url.port[1]){
    result=result*10+Ascii::digitValue(string[url.port[0]++]);
    }
  return result;
  }


// Parse path from string containing url
FXString FXURL::path(const FXString& string){
  URL url(string);
  return string.mid(url.path[0],url.path[1]-url.path[0]);
  }


// Parse query from string containing url
FXString FXURL::query(const FXString& string){
  URL url(string);
  return string.mid(url.quer[0],url.quer[1]-url.quer[0]);
  }


// Parse fragment from string containing url
FXString FXURL::fragment(const FXString& string){
  URL url(string);
  return string.mid(url.frag[0],url.frag[1]-url.frag[0]);
  }


}
