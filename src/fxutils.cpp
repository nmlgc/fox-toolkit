/********************************************************************************
*                                                                               *
*                          U t i l i t y   F u n c t i o n s                    *
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
* $Id: fxutils.cpp,v 1.43 1998/10/27 04:57:44 jeroen Exp $                      *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "fxkeys.h"



/*
  To do:
  - Turn malloc macros into templates.
  - Add common functions such as fxmemcpy and fxstrcasecmp?
  - Those functions manipulating strings should perhaps become FXString type
    functions?
*/

/*******************************************************************************/

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
  if(p){ *ptr=p; return TRUE; }
  return FALSE;
  }


// Free memory, resets ptr to NULL afterward
void fxfree(void** ptr){
  FXASSERT(ptr);
  if(*ptr) free(*ptr);
  *ptr=NULL;
  }


// Assert failed routine
void fxassert(const char* expression,const char* filename,unsigned int lineno){
  fprintf(stderr,"FXASSERT(%s) failed in file: %s (%d)\nPlease mail a bug report to: jvz@cfdrc.com\n",expression,filename,lineno);
  }


// Error routine
void fxerror(const char* format,...){
  va_list arguments;
  va_start(arguments,format);
  vfprintf(stderr,format,arguments);
  va_end(arguments);
  exit(1);
  }


// Warning routine
void fxwarning(const char* format,...){
  va_list arguments;
  va_start(arguments,format);
  vfprintf(stderr,format,arguments);
  va_end(arguments);
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


// Sort items
void fxsort(FXItem*& f1,FXItem*& t1,FXItem*& f2,FXItem*& t2,FXItemSortFunc greater,int n){
  FXItem *ff1,*tt1,*ff2,*tt2,*q;
  FXint m;
  if(!greater) return;
  if(f2==NULL){ 
    f1=NULL; 
    t1=NULL; 
    return; 
    }
  if(n>1){
    m=n/2;
    n=n-m;
    fxsort(ff1,tt1,f2,t2,greater,n);  // 1 or more
    fxsort(ff2,tt2,f2,t2,greater,m);  // 0 or more
    FXASSERT(ff1);
    if(ff2 && greater(ff1,ff2)){
      f1=ff2;
      ff2->prev=NULL;
      ff2=ff2->next;
      }
    else{
      f1=ff1;
      ff1->prev=NULL;
      ff1=ff1->next;
      }
    t1=f1;
    t1->next=NULL;
    while(ff1 || ff2){
      if(ff1==NULL){ t1->next=ff2; ff2->prev=t1; t1=tt2; break; }
      if(ff2==NULL){ t1->next=ff1; ff1->prev=t1; t1=tt1; break; }
      if(greater(ff1,ff2)){
        t1->next=ff2;
        ff2->prev=t1;
        t1=ff2;
        ff2=ff2->next;
        }
      else{
        t1->next=ff1;
        ff1->prev=t1;
        t1=ff1;
        ff1=ff1->next;
        }
      t1->next=NULL;
      }
    return;
    }
  FXASSERT(f2);
  f1=f2;
  t1=f2;
  f2=f2->next;
  while(f2){
    f2->prev=NULL;
    if(greater(f2,t1)){
      t1->next=f2;
      f2->prev=t1;
      t1=f2;
      f2=f2->next;
      continue;
      }
    if(greater(f1,f2)){
      q=f2;
      f2=f2->next;
      q->next=f1;
      f1->prev=q;
      f1=q;
      continue;
      }
    break;
    }
  FXASSERT(f1);
  FXASSERT(t1);
  f1->prev=NULL;
  t1->next=NULL;
  }


// Character value of keysym+control state
FXchar fxkeyval(FXuint keysym,FXuint state){
  if((state&CONTROLMASK) && 0x60<=keysym && keysym<=0x7f) return keysym-0x60;
  if(KEY_space<=keysym && keysym<=KEY_asciitilde) return keysym;
  if(KEY_BackSpace<=keysym && keysym<=KEY_Return) return keysym&255;
  if(KEY_KP_0<=keysym && keysym<=KEY_KP_9) return '0'-KEY_KP_0+keysym;
  if(KEY_Escape==keysym) return '\033';
  if(KEY_KP_Space==keysym) return ' ';
  if(KEY_KP_Tab==keysym) return '\t';
  if(KEY_KP_Enter==keysym) return '\015';
  if(KEY_KP_Add==keysym) return '+';
  if(KEY_KP_Subtract==keysym) return '-';
  if(KEY_KP_Multiply==keysym) return '*';
  if(KEY_KP_Divide==keysym) return '/';
  if(KEY_KP_Equal==keysym) return '=';
  return 0;
  }

#define ISSEP(ch) ((ch)=='+' || (ch)=='-' || (ch)==' ')


// Parse for accelerator key codes in a string
FXHotKey fxparseaccel(const FXchar* s){
  FXuint code=0,mods=0;
  if(s){
    while(s[0] && s[0]!='\t' && s[0]!='\n'){
      if(ISSEP(s[0])){
        s++;
        continue;
        }
      if(s[0]=='C' && s[1]=='t' && s[2]=='l' && ISSEP(s[3])){
        mods|=CONTROLMASK;
        s+=4;
        continue;
        }
      if(s[0]=='A' && s[1]=='l' && s[2]=='t' && ISSEP(s[3])){
        mods|=ALTMASK;
        s+=4;
        continue;
        }
      if(s[0]=='S' && s[1]=='h' && s[2]=='i' && s[3]=='f' && s[4]=='t' && ISSEP(s[5])){
        mods|=SHIFTMASK;
        s+=6;
        continue;
        }
      if(isalnum(s[0]) && (s[1]=='\0' || s[1]=='\t' || s[1]=='\n')){
        code=tolower(s[0])+KEY_space-' ';
        }
//fprintf(stderr,"code = %04x mods=%04x\n",code,mods);
      return MKUINT(code,mods);
      }
    }
  return 0;
  }


// Parse for hot key in a string
FXHotKey fxparsehotkey(const FXchar* s){
  FXuint code;
  if(s){
    while(s[0] && s[0]!='\t'){
      if(s[0]=='&'){
        if(s[1]!='&'){
          if(isalnum(s[1])){
            code=tolower(s[1])+KEY_space-' ';
            return MKUINT(code,ALTMASK);
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


#define FOLD(c)	  ((flags&FILEMATCH_CASEFOLD)&&islower(c)?toupper(c):(c))


// Match a file name with a pattern; returning 1 if it matches, 0 if not.
FXint fxfilematch(const char *pattern,const char *string,FXuint flags){
  if(pattern && string){
    register const char *p=pattern,*n=string;
    register char c,c1,cstart,cend;
    register int not;
    
    // Loop over characters of pattern
    while((c=*p++)!='\0'){
      c=FOLD(c);
      switch(c){

        // Single character wild card
        case '?':
	  if(*n=='\0') return 0;
	  else if((flags&FILEMATCH_FILE_NAME) && *n==PATHSEP) return 0;
	  else if((flags&FILEMATCH_PERIOD) && *n=='.' && (n==string || ((flags&FILEMATCH_FILE_NAME) && n[-1]==PATHSEP))) return 0;
	  break;

        // Many character wild card
        case '*':
	  if((flags&FILEMATCH_PERIOD) && *n=='.' && (n==string || ((flags&FILEMATCH_FILE_NAME) && n[-1]==PATHSEP))) return 0;

	  for(c=*p++; c=='?' || c=='*'; c=*p++,n++){
	    if(((flags&FILEMATCH_FILE_NAME) && *n==PATHSEP) || (c=='?' && *n=='\0')) return 0;
            }

	  if(c=='\0') return 1;

	  c1=(!(flags&FILEMATCH_NOESCAPE) && c=='\\') ? *p : c;
	  c1=FOLD(c1);
          for(--p; *n!='\0'; n++){
	    if((c=='[' || FOLD(*n)==c1) && fxfilematch(p,n,flags&~FILEMATCH_PERIOD)) return 1;
            }
          return 0;

        // Character range wild card
        case '[':
	  if(*n=='\0') return 0;

	  if((flags&FILEMATCH_PERIOD) && *n=='.' && (n==string || ((flags&FILEMATCH_FILE_NAME) && n[-1]==PATHSEP))) return 0;

	  // Nonzero if the sense of the character class is inverted.
	  not=(*p=='!' || *p=='^');

	  if(not) ++p;

	  c=*p++;
	  for(;;){
	    cstart=c;
            cend=c;

	    if(!(flags&FILEMATCH_NOESCAPE) && c=='\\') cstart=cend=*p++;

	    cstart=cend=FOLD(cstart);

	    // [ (unterminated) loses.
	    if(c=='\0') return 0;

	    c=*p++;
	    c=FOLD(c);

	    // [/] can never match.
	    if((flags&FILEMATCH_FILE_NAME) && c==PATHSEP) return 0;

	    if(c=='-' && *p!=']'){
	      cend = *p++;
	      if(!(flags&FILEMATCH_NOESCAPE) && cend=='\\') cend=*p++;
	      if(cend=='\0') return 0;
	      cend=FOLD(cend);
	      c=*p++;
	      }

	    if(FOLD(*n)>=cstart && FOLD(*n)<=cend) goto matched;

	    if(c==']') break;
	    }
	  if(!not) return 0;
	  break;

matched:  // Skip the rest of the [...] that already matched.
	  while(c!=']'){

	    // [... (unterminated) loses.
	    if(c=='\0') return 0;

	    c=*p++;

	    // XXX 1003.2d11 is unclear if this is right.
	    if(!(flags&FILEMATCH_NOESCAPE) && c=='\\') ++p;
	    }
	  if(not) return 0;
	  break;

        // Escaped character
        case '\\':
	  if(!(flags&FILEMATCH_NOESCAPE)){
	    c=*p++;
	    c=FOLD(c);
	    }
          // Fall through...

        // Normal character
        default:
	  if(FOLD(*n)!=c) return 0;
	  break;
        }

      ++n;
      }

    if(*n=='\0') return 1;

    // The FILEMATCH_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".
    if((flags&FILEMATCH_LEADING_DIR) && *n==PATHSEP) return 1;
    }

  return 0;
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
  sprintf(result,"%d",uid);
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
  sprintf(result,"%d",gid);
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


// Return TRUE iff s is a prefix of t
FXbool fxprefix(const FXchar* s,const FXchar* t){
  const FXchar *p,*q;
  for(p=s,q=t; *p && *q && *p==*q; p++,q++);
  return !*p;
  }


// Return TRUE iff s is a suffix of t
FXbool fxsuffix(const FXchar* s, const FXchar* t){
  const FXchar *p,*q;
  for(p=s+strlen(s),q=t+strlen(t); p>s && q>t && *p==*q; p--,q--);
  return p==s && *p==*q;
  }


// Expand ~ in filenames; using HOME environment variable
// Note "~name" expands to "~name", "~/name" expands to "$HOME/name", 
// and "~" expands to "$HOME"
FXchar* fxexpand(FXchar* result,const FXchar* name){
  if(!result){fxerror("fxexpand: NULL result argument.\n");}
  *result='\0';
  if(name){
    if(name[0]=='~'){
      if(name[1]=='\0' || name[1]==PATHSEP){
        fxgethomedir(result);
        name++;
        }
      }
    strcat(result,name);
    }
  return result;
  }
        

// Construct a full pathname from the given directory and file name
FXchar* fxpathname(FXchar* result,const FXchar* dirname,const FXchar* filename){
  int l;
  if(!result){fxerror("fxpathname: NULL result argument.\n");}
  fxexpand(result,dirname);
  if((l=strlen(result)) && result[l-1]!=PATHSEP) result[l++]=PATHSEP;
  strcpy(result+l,filename);
  return result;
  }


// Return the directory part of pathname
FXchar* fxdirpart(FXchar* result,const FXchar* pathname){
  FXchar *p;
  if(!result){fxerror("fxdirpart: NULL result argument.\n");}
  fxexpand(result,pathname);
  if((p=strrchr(result,PATHSEP))){
    if(p==result) p++;
    *p='\0';
    } 
  else{
    fxgetcurrentdir(result);
    }
  return result;
  }


// Return the filename part of pathname
FXchar* fxfilepart(FXchar* result,const FXchar* pathname){
  FXchar *p;
  if(!result){fxerror("fxfilepart: NULL result argument.\n");}
  if((p=strrchr(pathname,PATHSEP)))
    return strcpy(result,p+1);
  else
    return strcpy(result,pathname);
  }


// Return the extension part of pathname
FXchar* fxfileext(FXchar* result,const FXchar* pathname){
  FXchar* p;
  if(!result){fxerror("fxfileext: NULL result argument.\n");}
  *result='\0';
  p=strrchr(pathname,PATHSEP);
  if(p) pathname=p+1;
  if((p=strrchr(pathname,'.'))) return strcpy(result,p+1);
  return result;
  }


// Check whether a file exists
FXbool fxexists(const FXchar *name){
  struct stat info;
  if(!name){fxerror("fxexists: NULL name argument.\n");}
  return stat(name,&info)==0;
  }


// Create a unique numbered backup file name for the given pathname 
FXchar* fxbakname(FXchar* result,const FXchar* pathname){
  FXchar* p;
  int n=0;
  if(!result){fxerror("fxbakname: NULL result argument.\n");}
  if(!pathname){fxerror("fxbakname: NULL pathname argument.\n");}
  p=result+strlen(pathname);
  strcpy(result,pathname);
  do{
    sprintf(p,".~%d~",++n);
    } 
  while(fxexists(result));
  return result;
  }



// Check whether two files are identical (refer to the same inode)
FXbool fxidentical(const FXchar *name1,const FXchar *name2){
  struct stat info1,info2;
  if(!name1){fxerror("fxidentical: NULL name1 argument.\n");}
  if(!name2){fxerror("fxidentical: NULL name2 argument.\n");}
  return !lstat(name1,&info1) && !lstat(name2,&info2) && info1.st_ino==info2.st_ino;
  }


// Split a string into substrings delimited by a given character
FXchar *fxsplit(FXchar*& s,FXchar c){
  FXchar *t=s;
  if(s && (s=strchr(s,c))) *s++='\0';
  return t;
  }


// Return the shortest path equivalent to pathname (remove . and ..)
FXchar *fxshortestpath(FXchar *result,const FXchar *pathname){
  FXchar path[MAXPATHLEN+1],*cur,*last,*part,*tmp;
  if(!result){fxerror("fxshortestpath: NULL result argument.\n");}
  if(!pathname){fxerror("fxshortestpath: NULL pathname argument.\n");}
  strcpy(path,pathname);
  cur=result;
  *cur='\0';
  tmp=path;
  last=NULL;
  if(*tmp==PATHSEP){ 
    *cur++=PATHSEP;
    *cur='\0';
    tmp++; 
    }
  while((part=fxsplit(tmp,PATHSEP))){
    if(strcmp(part,".")==0)
      ;
    else if(strcmp(part,"..")==0 && (last=strrchr(result,PATHSEP))){
      if(last>result)
        cur=last;
      else
        cur=last+1;
      *cur='\0';
      } 
    else{
      if(cur>result && *(cur-1)!=PATHSEP) *cur++=PATHSEP;
      strcpy(cur,part);
      cur+=strlen(part);
      }
    }
  
  // Remove trailing backslash(es)
  while(result<cur-1 && *(cur-1)==PATHSEP){
    *--cur='\0';
    }
  return result;
  }


// Return directory one level above given one
FXchar* fxupdir(FXchar* result,const FXchar *dirname){
  FXchar *ptr;
  
  // Test
  if(!result){fxerror("fxupdir: NULL result argument.\n");}

  // Expand tilde's
  fxexpand(result,dirname);
  
  // Find the last PATHSEP
  ptr=strrchr(result,PATHSEP);
  
  // If dirname had a trailing PATHSEP, lop it off and try again
  if(ptr && *(ptr+1)=='\0'){*ptr='\0';ptr=strrchr(result,PATHSEP);}

  // Try lop off the last directory
  if(ptr){
    
    // Is this the toplevel directory already?
    // The test is a bit complicated as we need to take a possible drive letter into account
    if((result[0]==PATHSEP && ptr==result) || (isalpha(result[0]) && result[1]==':' && result[2]==PATHSEP && ptr==result+2)){
      *(ptr+1)='\0';
      }
    else{
      *ptr='\0';
      }
    }
  return result;
  }


// Test if pathname is the toplevel directory
FXbool fxistopdir(const FXchar* path){
  return path==NULL || (path[0]==PATHSEP && path[1]=='\0') || (isalpha(path[0]) && path[1]==':' && path[2]==PATHSEP && path[3]=='\0');
  }


// Get current working directory
FXchar* fxgetcurrentdir(FXchar *result){
  if(!result){fxerror("fxgetcurrentdir: NULL result argument.\n");}
  if(getcwd(result,MAXPATHLEN)==NULL){fxerror("fxgetcurrentdir: failed.\n");}
  return result;
  }


// Get home directory
FXchar* fxgethomedir(FXchar *result){
  static FXchar* home=NULL;
  if(!result){fxerror("fxgethomedir: NULL result argument.\n");}
  if(!home){home=getenv("HOME");if(!home) home="";}
  strcpy(result,home);
  return result;
  }


// Translate filename to an absolute pathname; ~ in filename is expanded,
// and if the resulting pathname is still relative, basename is prepended
FXchar *fxabspath(FXchar *result,const FXchar *basename,const FXchar *filename){
  char s[MAXPATHLEN+1],t[MAXPATHLEN+1],cwd[MAXPATHLEN+1];
  
  // Test
  if(!result){fxerror("fxabspath: NULL result argument.\n");}

  // Expand tilde's
  fxexpand(s,filename);
  
  // Absolute path may be preceded by a drive letter as in C:\blabla\blabla
  // or just \blabla\blabla or on unix just /blabla/blabla
  if(!(s[0]==PATHSEP || (isalpha(s[0]) && s[1]==':' && s[2]==PATHSEP))){
    if(basename==NULL || *basename=='\0'){
      fxgetcurrentdir(cwd);
      basename=cwd;
      }
    fxpathname(t,basename,s);
    return fxshortestpath(result,t);
    }
  return fxshortestpath(result,s);
  }


// Search path for file name. A ~ in path is expanded. If name is absolute
// it is returned unchanged. Otherwise the absolute name is returned in
// result. If name is not found on path, NULL is returned. 
FXchar *fxsearchpath(FXchar *result,const FXchar *path,const FXchar *name){
  FXchar fullname[MAXPATHLEN+1];
  FXchar basedir[MAXPATHLEN+1];
  const FXchar *s,*t;
  if(!result){fxerror("fxsearchpath: NULL result argument.\n");}
  if(!path){fxerror("fxsearchpath: NULL path argument.\n");}
  if(!name){fxerror("fxsearchpath: NULL name argument.\n");}
  if(*name==PATHSEP){
    strcpy(result,name);
    if(fxexists(result)) return result;
    return NULL;
    }
  fxgetcurrentdir(basedir);
  for(s=path; *s; s=t){
    int l;
    if(!(t=strchr(s,PATHLISTSEP))) t=strchr(s,0);
    if(s!=t){
      if(s[0]=='.'){ 
        if(t==s+1) s=t; else if(s[1]==PATHSEP) s+=2; 
        }
      l=t-s;
      strncpy(fullname,s,l);
      if(l>0 && fullname[l-1]!=PATHSEP){ fullname[l]=PATHSEP; l++; }
      strcpy(fullname+l,name);
      fxabspath(result,basedir,fullname);
      if(fxexists(result)) return result;
      }
    if(*t) t++;
    }
  return NULL;
  }


// Convert RGB to HSV
void fxrgb_to_hsv(FXfloat& h,FXfloat& s,FXfloat& v,FXfloat r,FXfloat g,FXfloat b){
  FXfloat t,delta;
  v=r; if(g>v) v=g; if(b>v) v=b;
  t=r; if(g<t) t=g; if(b<t) t=b;
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
      h=2.0+(b-r)/delta;
    else if(b==v) 
      h=4.0+(r-g)/delta;
    h=h*60.0;
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
    h=h/60.0;
    i=(FXint)h;
    f=h-i;
    w=v*(1.0-s);
    q=v*(1.0-(s*f));
    t=v*(1.0-(s*(1.0-f)));
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


// Calculate a hash value from a string
FXuint fxstrhash(const FXchar* str){
  register FXuint h=0;
  register FXuint g;
  while(*str) {
    h=(h<<4)+*str++;
    g=h&0xF0000000UL;
    if(g) h^=g>>24;
    h&=~g;
    }
  return h;
  }
