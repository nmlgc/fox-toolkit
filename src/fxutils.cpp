/********************************************************************************
*                                                                               *
*                          U t i l i t y   F u n c t i o n s                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: fxutils.cpp,v 1.71 2002/02/27 02:27:36 fox Exp $                         *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"


/*
  To do:
  - Those functions manipulating strings should perhaps become FXString type
    functions?
  - Need to pass buffer-size argument to all those fxdirpart() etc. functions
    to protect against memory overruns (make it a default argument last one
    so as to not impact anyone).
  - Revive my old malloc() replacement library to detect memory block overwrites.
*/

/*******************************************************************************/


// Global flag which controls tracing level
unsigned int fxTraceLevel=0;


// Version number that the library has been compiled with
const FXuchar fxversion[3]={FOX_MAJOR,FOX_MINOR,FOX_LEVEL};


// Allows GNU autoconfigure to find FOX
extern "C" FXAPI void fxfindfox(void){ }


/*******************************************************************************/


// Thread-safe, linear congruential random number generator from Knuth & Lewis.
FXuint fxrandom(FXuint& seed){
  seed=1664525UL*seed+1013904223UL;
  return seed;
  }


// Allocate memory
FXint fxmalloc(void** ptr,unsigned long size){
  FXASSERT(ptr);
  *ptr=malloc(size);
  return *ptr!=NULL;
  }


// Allocate cleaned memory
FXint fxcalloc(void** ptr,unsigned long size){
  FXASSERT(ptr);
  *ptr=calloc(size,1);
  return *ptr!=NULL;
  }


// Resize memory
FXint fxresize(void** ptr,unsigned long size){
  FXASSERT(ptr);
  void *p=realloc(*ptr,size);
  if(!size || p){ *ptr=p; return TRUE; }
  return FALSE;
  }


// Allocate and initialize memory
FXint fxmemdup(void** ptr,unsigned long size,const void* src){
  FXASSERT(ptr);
  *ptr=NULL;
  if(src){
    *ptr=malloc(size);
    if(*ptr){ memcpy(*ptr,src,size); return TRUE; }
    }
  return FALSE;
  }


// Free memory, resets ptr to NULL afterward
void fxfree(void** ptr){
  FXASSERT(ptr);
  if(*ptr) free(*ptr);
  *ptr=NULL;
  }


// String duplicate
FXchar *fxstrdup(const FXchar* str){
  FXchar *copy;
  FXMALLOC(&copy,FXchar,strlen(str)+1);
  strcpy(copy,str);
  return copy;
  }

#ifdef WIN32

// Missing in CYGWIN
#ifndef IMAGE_SUBSYSTEM_NATIVE_WINDOWS
#define IMAGE_SUBSYSTEM_NATIVE_WINDOWS 8
#endif

#ifndef IMAGE_SUBSYSTEM_WINDOWS_CE_GUI
#define IMAGE_SUBSYSTEM_WINDOWS_CE_GUI 9
#endif

// getmodulefilename

FXbool fxisconsole(const FXchar *path){
  IMAGE_OPTIONAL_HEADER optional_header;
  IMAGE_FILE_HEADER     file_header;
  IMAGE_DOS_HEADER      dos_header;
  DWORD                 dwCoffHeaderOffset;
  DWORD                 dwNewOffset;
  DWORD                 dwMoreDosHeader[16];
  ULONG                 ulNTSignature;
  HANDLE                hImage;
  DWORD                 dwBytes;
  FXbool                flag=MAYBE;

  // Open the application file.
  hImage=CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if(hImage!=INVALID_HANDLE_VALUE){

    // Read MS-Dos image header.
    if(ReadFile(hImage,&dos_header,sizeof(IMAGE_DOS_HEADER),&dwBytes,NULL)==0) goto x;

    // Test bytes read
    if(dwBytes!=sizeof(IMAGE_DOS_HEADER)) goto x;

    // Test signature
    if(dos_header.e_magic!=IMAGE_DOS_SIGNATURE) goto x;

    // Read more MS-Dos header.
    if(ReadFile(hImage,dwMoreDosHeader,sizeof(dwMoreDosHeader),&dwBytes,NULL)==0) goto x;

    // Test bytes read
    if(dwBytes!=sizeof(dwMoreDosHeader)) goto x;

    // Move the file pointer to get the actual COFF header.
    dwNewOffset=SetFilePointer(hImage,dos_header.e_lfanew,NULL,FILE_BEGIN);
    dwCoffHeaderOffset=dwNewOffset+sizeof(ULONG);
    if(dwCoffHeaderOffset==0xFFFFFFFF) goto x;

    // Read NT signature of the file.
    if(ReadFile(hImage,&ulNTSignature,sizeof(ULONG),&dwBytes,NULL)==0) goto x;

    // Test bytes read
    if(dwBytes!=sizeof(ULONG)) goto x;

    // Test NT signature
    if(ulNTSignature!=IMAGE_NT_SIGNATURE) goto x;

    if(ReadFile(hImage,&file_header,IMAGE_SIZEOF_FILE_HEADER,&dwBytes,NULL)==0) goto x;

    // Test bytes read
    if(dwBytes!=IMAGE_SIZEOF_FILE_HEADER) goto x;

    // Read the optional header of file.
    if(ReadFile(hImage,&optional_header,IMAGE_SIZEOF_NT_OPTIONAL_HEADER,&dwBytes,NULL)==0) goto x;

    // Test bytes read
    if(dwBytes!=IMAGE_SIZEOF_NT_OPTIONAL_HEADER) goto x;

    // Switch on systems
    switch(optional_header.Subsystem){
      case IMAGE_SUBSYSTEM_WINDOWS_GUI:     // Windows GUI (2)
      case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:  // Windows CE GUI (9)
        flag=FALSE;
        break;
      case IMAGE_SUBSYSTEM_WINDOWS_CUI:     // Windows Console (3)
      case IMAGE_SUBSYSTEM_OS2_CUI:         // OS/2 Console (5)
      case IMAGE_SUBSYSTEM_POSIX_CUI:       // Posix Console (7)
        flag=TRUE;
        break;
      case IMAGE_SUBSYSTEM_NATIVE:          // Native (1)
      case IMAGE_SUBSYSTEM_NATIVE_WINDOWS:  // Native Win9x (8)
      case IMAGE_SUBSYSTEM_UNKNOWN:         // Unknown (0)
      default:
        break;
      }
x:  CloseHandle(hImage);
    }
  return flag;
  }

#endif


// Assert failed routine
void fxassert(const char* expression,const char* filename,unsigned int lineno){
#ifndef WIN32
  fprintf(stderr,"%s:%d: FXASSERT(%s) failed.\n",filename,lineno,expression);
#else
  char msg[256];
  sprintf(msg,"%s(%d): FXASSERT(%s) failed.\n",filename,lineno,expression);
#ifdef _WINDOWS
  OutputDebugString(msg);
  fprintf(stderr,"%s",msg); // if a console is available
#else
  fprintf(stderr,"%s",msg);
#endif
#endif
  }


// Log message to [typically] stderr
void fxmessage(const char* format,...){
#ifndef WIN32                         // Unix
  va_list arguments;
  va_start(arguments,format);
  vfprintf(stderr,format,arguments);
  va_end(arguments);
#else
#ifdef _WINDOWS
  char msg[512];
  va_list arguments;
  va_start(arguments,format);
  vsnprintf(msg,sizeof(msg),format,arguments);
  va_end(arguments);
  OutputDebugString(msg);
  fprintf(stderr,"%s",msg); // if a console is available
#else
  va_list arguments;
  va_start(arguments,format);
  vfprintf(stderr,format,arguments);
  va_end(arguments);
#endif
#endif
  }


// Error routine
void fxerror(const char* format,...){
#ifndef WIN32
  va_list arguments;
  va_start(arguments,format);
  vfprintf(stderr,format,arguments);
  va_end(arguments);
  abort();
#else
#ifdef _WINDOWS
  char msg[512];
  va_list arguments;
  va_start(arguments,format);
  vsnprintf(msg,sizeof(msg),format,arguments);
  va_end(arguments);
  OutputDebugString(msg);
  fprintf(stderr,"%s",msg); // if a console is available
  MessageBox(NULL,msg,NULL,MB_OK|MB_ICONEXCLAMATION|MB_APPLMODAL);
  DebugBreak();
#else
  va_list arguments;
  va_start(arguments,format);
  vfprintf(stderr,format,arguments);
  va_end(arguments);
  abort();
#endif
#endif
  }


// Warning routine
void fxwarning(const char* format,...){
#ifndef WIN32
  va_list arguments;
  va_start(arguments,format);
  vfprintf(stderr,format,arguments);
  va_end(arguments);
#else
#ifdef _WINDOWS
  char msg[512];
  va_list arguments;
  va_start(arguments,format);
  vsnprintf(msg,sizeof(msg),format,arguments);
  va_end(arguments);
  OutputDebugString(msg);
  fprintf(stderr,"%s",msg); // if a console is available
  MessageBox(NULL,msg,NULL,MB_OK|MB_ICONINFORMATION|MB_APPLMODAL);
#else
  va_list arguments;
  va_start(arguments,format);
  vfprintf(stderr,format,arguments);
  va_end(arguments);
#endif
#endif
  }


// Trace printout routine
void fxtrace(unsigned int level,const char* format,...){
  if(fxTraceLevel>level){
#ifndef WIN32
    va_list arguments;
    va_start(arguments,format);
    vfprintf(stderr,format,arguments);
    va_end(arguments);
#else
#ifdef _WINDOWS
    char msg[512];
    va_list arguments;
    va_start(arguments,format);
    vsnprintf(msg,sizeof(msg),format,arguments);
    va_end(arguments);
    OutputDebugString(msg);
    fprintf(stderr,"%s",msg); // if a console is available
#else
    va_list arguments;
    va_start(arguments,format);
    vfprintf(stderr,format,arguments);
    va_end(arguments);
#endif
#endif
    }
  }


// Sleep n microseconds
void fxsleep(unsigned int n){
#ifdef WIN32
  unsigned int zzz=n/1000;
  if(zzz==0) zzz=1;
  Sleep(zzz);
#else
#ifdef __USE_POSIX199309
  struct timespec value;
  value.tv_nsec = 1000 * (n%1000000);
  value.tv_sec = n/1000000;
  nanosleep(&value,NULL);
#else
#ifndef BROKEN_SELECT
  struct timeval value;
  value.tv_usec = n % 1000000;
  value.tv_sec = n / 1000000;
  select(1,0,0,0,&value);
#else
  unsigned int zzz=n/1000000;
  if(zzz==0) zzz=1;
  if(zzz){
    while((zzz=sleep(zzz))>0) ;
    }
#endif
#endif
#endif
  }


#define ISSEP(ch) ((ch)=='+' || (ch)=='-' || (ch)==' ')


// Parse for accelerator key codes in a string
FXHotKey fxparseaccel(const FXchar* string){
  register const FXuchar *s=(const FXuchar *)string;
  register FXuint code=0,mods=0;
  if(s){
    while(s[0] && s[0]!='\t' && s[0]!='\n'){
      if(ISSEP(s[0])){
        s++;
        continue;
        }
      if((tolower(s[0])=='c')&&(tolower(s[1])=='t')&&(tolower(s[2])=='l')&&ISSEP(s[3])){
        mods|=CONTROLMASK;
        s+=4;
        continue;
        }
      if((tolower(s[0])=='c')&&(tolower(s[1])=='t')&&(tolower(s[2])=='r')&&(tolower(s[3])=='l')&&ISSEP(s[4])){
        mods|=CONTROLMASK;
        s+=5;
        continue;
        }
      if((tolower(s[0])=='a')&&(tolower(s[1])=='l')&&(tolower(s[2])=='t')&&ISSEP(s[3])){
        mods|=ALTMASK;
        s+=4;
        continue;
        }
      if((tolower(s[0])=='s')&&(tolower(s[1])=='h')&&(tolower(s[2])=='i')&&(tolower(s[3])=='f')&&(tolower(s[4])=='t')&&ISSEP(s[5])){
        mods|=SHIFTMASK;
        s+=6;
        continue;
        }

      // One-digit function key
      if(tolower(s[0])=='f' && isdigit(s[1]) && (s[2]=='\0' || s[2]=='\t' || s[2]=='\n')){
        code=KEY_F1+s[1]-'1';
        }

      // Two digit function key
      else if(tolower(s[0])=='f' && isdigit(s[1]) && isdigit(s[2]) && (s[3]=='\0' || s[3]=='\t' || s[3]=='\n')){
        code=KEY_F1+10*(s[1]-'0')+(s[2]-'0')-1;
        }

      // One final character
      else if(s[0] && (s[1]=='\0' || s[1]=='\t' || s[1]=='\n')){
        if(mods&SHIFTMASK)
          code=toupper(s[0])+KEY_space-' ';
        else
          code=tolower(s[0])+KEY_space-' ';
        }
      FXTRACE((150,"fxparseaccel(%s): %08x code = %04x mods=%04x\n",string,MKUINT(code,mods),code,mods));
      return MKUINT(code,mods);
      }
    }
  return 0;
  }


// Parse for hot key in a string
FXHotKey fxparsehotkey(const FXchar* string){
  register const FXuchar *s=(const FXuchar*)string;
  FXuint code,mods;
  if(s){
    while(s[0] && s[0]!='\t'){
      if(s[0]=='&'){
        if(s[1]!='&'){
          if(isalnum(s[1])){
            mods=ALTMASK;
            code=tolower(s[1])+KEY_space-' ';
            FXTRACE((150,"fxparsehotkey(%s): %08x code = %04x mods=%04x\n",string,MKUINT(code,mods),code,mods));
            return MKUINT(code,mods);
            }
          break;
          }
        s++;
        }
      s++;
      }
    }
  return 0;
  }


// Locate hot key underline offset from begin of string
FXint fxfindhotkeyoffset(const FXchar* s){
  register FXint pos=0;
  if(s){
    while(s[pos] && s[pos]!='\t'){
      if(s[pos]=='&'){
        if(s[pos+1]!='&') return pos;
        pos++;
        }
      pos++;
      }
    }
  return -1;
  }



// Get highlight color
FXColor makeHiliteColor(FXColor clr){
  FXuint r,g,b;
  r=FXREDVAL(clr);
  g=FXGREENVAL(clr);
  b=FXBLUEVAL(clr);
  r=FXMAX(31,r);
  g=FXMAX(31,g);
  b=FXMAX(31,b);
  r=(133*r)/100;
  g=(133*g)/100;
  b=(133*b)/100;
  r=FXMIN(255,r);
  g=FXMIN(255,g);
  b=FXMIN(255,b);
  return FXRGB(r,g,b);
  }


// Get shadow color
FXColor makeShadowColor(FXColor clr){
  FXuint r,g,b;
  r=FXREDVAL(clr);
  g=FXGREENVAL(clr);
  b=FXBLUEVAL(clr);
  r=(66*r)/100;
  g=(66*g)/100;
  b=(66*b)/100;
  return FXRGB(r,g,b);
  }


// Get user name from uid
FXchar* fxgetusername(FXchar* result,FXuint uid){
  if(!result){fxerror("fxgetusername: NULL result argument.\n");}
#ifndef WIN32
  struct passwd *pwd;
  if((pwd=getpwuid(uid))!=NULL)
    strcpy(result,pwd->pw_name);
  else
    sprintf(result,"%d",uid);
#else
//  sprintf(result,"%d",uid);
  strcpy(result,"user");
#endif
  return result;
  }


// Get group name from gid
FXchar* fxgetgroupname(FXchar* result,FXuint gid){
  if(!result){fxerror("fxgetgroupname: NULL result argument.\n");}
#ifndef WIN32
  struct group *grp;
  if((grp=getgrgid(gid))!=NULL)
    strcpy(result,grp->gr_name);
  else
    sprintf(result,"%d",gid);
#else
  //sprintf(result,"%d",gid);
  strcpy(result,"group");
#endif
  return result;
  }


// Get permissions string from mode
FXchar* fxgetpermissions(FXchar* result,FXuint mode){
  if(!result){fxerror("fxgetpermissions: NULL result argument.\n");}
#ifndef WIN32
  result[0]=S_ISLNK(mode) ? 'l' : S_ISREG(mode) ? '-' : S_ISDIR(mode) ? 'd' : S_ISCHR(mode) ? 'c' : S_ISBLK(mode) ? 'b' : S_ISFIFO(mode) ? 'p' : S_ISSOCK(mode) ? 's' : '?';
  result[1]=(mode&S_IRUSR) ? 'r' : '-';
  result[2]=(mode&S_IWUSR) ? 'w' : '-';
  result[3]=(mode&S_ISUID) ? 's' : (mode&S_IXUSR) ? 'x' : '-';
  result[4]=(mode&S_IRGRP) ? 'r' : '-';
  result[5]=(mode&S_IWGRP) ? 'w' : '-';
  result[6]=(mode&S_ISGID) ? 's' : (mode&S_IXGRP) ? 'x' : '-';
  result[7]=(mode&S_IROTH) ? 'r' : '-';
  result[8]=(mode&S_IWOTH) ? 'w' : '-';
  result[9]=(mode&S_ISVTX) ? 't' : (mode&S_IXOTH) ? 'x' : '-';
  result[10]=0;
#else
  result[0]='-';
#ifdef _S_IFDIR
  if(mode&_S_IFDIR) result[0]='d';
#endif
#ifdef _S_IFCHR
  if(mode&_S_IFCHR) result[0]='c';
#endif
#ifdef _S_IFIFO
  if(mode&_S_IFIFO) result[0]='p';
#endif
  result[1]='r';
  result[2]='w';
  result[3]='x';
  result[4]='r';
  result[5]='w';
  result[6]='x';
  result[7]='r';
  result[8]='w';
  result[9]='x';
  result[10]=0;
#endif
  return result;
  }


// Get process id
FXint fxgetpid(){
#ifndef WIN32
  return getpid();
#else
  return (int)GetCurrentProcessId();
#endif
  }


// Convert RGB to HSV
void fxrgb_to_hsv(FXfloat& h,FXfloat& s,FXfloat& v,FXfloat r,FXfloat g,FXfloat b){
  FXfloat t,delta;
  v=FXMAX3(r,g,b);
  t=FXMIN3(r,g,b);
  delta=v-t;
  if(v!=0.0)
    s=delta/v;
  else
    s=0.0;
  if(s==0.0){
    h=0.0;
    }
  else{
    if(r==v)
      h=(g-b)/delta;
    else if(g==v)
      h=2.0f+(b-r)/delta;
    else if(b==v)
      h=4.0f+(r-g)/delta;
    h=h*60.0f;
    if(h<0.0) h=h+360;
    }
  }


// Convert to RGB
void fxhsv_to_rgb(FXfloat& r,FXfloat& g,FXfloat& b,FXfloat h,FXfloat s,FXfloat v){
  FXfloat f,w,q,t;
  FXint i;
  if(s==0.0){
    r=v;
    g=v;
    b=v;
    }
  else{
    if(h==360.0) h=0.0;
    h=h/60.0f;
    i=(FXint)h;
    f=h-i;
    w=v*(1.0f-s);
    q=v*(1.0f-(s*f));
    t=v*(1.0f-(s*(1.0f-f)));
    switch(i){
      case 0: r=v; g=t; b=w; break;
      case 1: r=q; g=v; b=w; break;
      case 2: r=w; g=v; b=t; break;
      case 3: r=w; g=q; b=v; break;
      case 4: r=t; g=w; b=v; break;
      case 5: r=v; g=w; b=q; break;
      }
    }
  }


// Calculate a hash value from a string; algorithm same as in perl
FXint fxstrhash(const FXchar* str){
  register FXint h=0;
  register FXint g;
  while(*str) {
    h=(h<<4)+*str++;
    g=h&0xF0000000;
    if(g) h^=g>>24;
    h&=0x0fffffff;
    }
  FXASSERT(h<=0x0fffffff);
  return h;
  }


// Classify IEEE 754 floating point number
FXint fxieeefloatclass(FXfloat number){
  FXfloat num=number;
  FXASSERT(sizeof(FXfloat)==sizeof(FXuint));
  FXuint s=(*((FXuint*)&num)&0x80000000);        // Sign
  FXuint e=(*((FXuint*)&num)&0x7f800000);        // Exponent
  FXuint m=(*((FXuint*)&num)&0x007fffff);        // Mantissa
  FXint result=0;
  if(e==0x7f800000){
    if(m==0)
      result=1;     // Inf
    else
      result=2;     // NaN
    if(s)
      result=-result;
    }
  return result;
  }


// Classify IEEE 754 floating point number
FXint fxieeedoubleclass(FXdouble number){
  FXdouble num=number;
  FXASSERT(sizeof(FXdouble)==2*sizeof(FXuint));
#if FOX_BIGENDIAN
  FXuint s=(((FXuint*)&num)[0]&0x80000000);     // Sign
  FXuint e=(((FXuint*)&num)[0]&0x7ff00000);     // Exponent
  FXuint h=(((FXuint*)&num)[0]&0x000fffff);     // Mantissa high
  FXuint l=(((FXuint*)&num)[1]);                // Mantissa low
#else
  FXuint s=(((FXuint*)&num)[1]&0x80000000);     // Sign
  FXuint e=(((FXuint*)&num)[1]&0x7ff00000);     // Exponent
  FXuint h=(((FXuint*)&num)[1]&0x000fffff);     // Mantissa high
  FXuint l=(((FXuint*)&num)[0]);                // Mantissa low
#endif
  FXint result=0;
  if(e==0x7ff00000){
    if(h==0 && l==0)
      result=1;     // Inf
    else
      result=2;     // NaN
    if(s)
      result=-result;
    }
  return result;
  }


#ifdef WIN32

// Convert font size (in decipoints) to device logical units
int fxpointsize_to_height(HDC hdc,unsigned size){
  // The calls to SetGraphicsMode() and ModifyWorldTransform() have no effect on Win95/98
  // but that's OK.
  SetGraphicsMode(hdc,GM_ADVANCED);
  XFORM xform;
  ModifyWorldTransform(hdc,&xform,MWT_IDENTITY);
  SetViewportOrgEx(hdc,0,0,NULL);
  SetWindowOrgEx(hdc,0,0,NULL);
  FLOAT cyDpi=(FLOAT)(25.4*GetDeviceCaps(hdc,VERTRES)/GetDeviceCaps(hdc,VERTSIZE));
  POINT pt;
  pt.x=0;
  pt.y=(int)(size*cyDpi/72);
  DPtoLP(hdc,&pt,1);
  return -(int)(fabs(pt.y)/10.0+0.5);
  }


// Convert logical units to decipoints
unsigned fxheight_to_pointsize(HDC hdc,int height){
  // The calls to SetGraphicsMode() and ModifyWorldTransform() have no effect on Win95/98
  // but that's OK.
  SetGraphicsMode(hdc,GM_ADVANCED);
  XFORM xform;
  xform.eM11=0.0f;
  xform.eM12=0.0f;
  xform.eM21=0.0f;
  xform.eM22=0.0f;
  xform.eDx=0.0f;
  xform.eDy=0.0f;
  ModifyWorldTransform(hdc,&xform,MWT_IDENTITY);
  SetViewportOrgEx(hdc,0,0,NULL);
  SetWindowOrgEx(hdc,0,0,NULL);
  FLOAT cyDpi=(FLOAT)(25.4*GetDeviceCaps(hdc,VERTRES)/GetDeviceCaps(hdc,VERTSIZE));
  POINT pt;
  pt.x=0;
  pt.y=10*height;
  LPtoDP(hdc,&pt,1);
  return (FXuint)(72*pt.y/cyDpi);
  }

#endif


