/********************************************************************************
*                                                                               *
*                       C l a s s   T a b le   U t i l i t y                    *
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
* $Id: clstab.cpp,v 1.7 1998/07/25 03:10:20 jeroen Exp $                         *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "fxdefs.h"
//#include "FXStream.h"
//#include "FXObject.h"

/*

  Notes:
  
  - We have to use extern "C" for the metaClass names, as in some compilers,
    e.g. MS C++, mangled C++ names are no longer legal identifiers in C or C++.
  
  - Should be able to read from stdin.
// nm lists the symbols of the executable after link;
// then munch builds a hash table of class id's to constructors
// and links it together with the executable
// Result: an optimal hash table from classid to ctor!!!

// # the executable
// EXECMD  :  $(OBJS) 
// 	$(LNK) $(LNK_OPTS) -o  $(LNKOUT)   $(OBJS)  $(LIBS)
// 	$(NM)  $(LNKOUT)  > __sym.tmp
// 	type __sym.tmp | $(MUNCH)> __ctordtor.c
// 	$(CC) $(INCS) $(CC_OPTS) __ctordtor.c
// 	$(LNK) $(LNK_OPTS) -o  $(EXE)  __ctordtor.o  $(LNKOUT) 
*/


#define INITIALSIZE 128
#define MAXLOAD     80

const char version[]="1.0.0";

char **symbols=NULL;
int    maxsymbols=0;
int    numsymbols=0;


  
/* Print some help */
void printusage(){
  fprintf(stderr,"Usage: clstab [options] [-o outfile] symbolfiles...\n");
  fprintf(stderr,"  options:\n");
  fprintf(stderr,"  -h       Print help\n");
  fprintf(stderr,"  -d       Generate dummy table\n");
  fprintf(stderr,"  -n name  Specify class table name\n");
  fprintf(stderr,"  -v       Print version number\n");
  }


/* Compute hash string */
unsigned int hashstring(const char *str){
 register unsigned int h=0;
 register unsigned int g;
 while(*str) {
   h=(h<<4)+*str++;
   g=h&0xF0000000UL;
   if(g) h^=g>>24;
   h&=~g;
   }
 return h;
 }


/* Add symbol */
void addsymbol(char *sym){
  if(numsymbols>=maxsymbols){
    maxsymbols=maxsymbols*2;
    symbols=(char**)realloc(symbols,sizeof(char*)*maxsymbols);
    }
  symbols[numsymbols]=strdup(sym);
  numsymbols++;
  }
      
      
/* Process one input file */
void processfile(FILE* symfile){
  char buffer[1000],*pbeg,*pend;
  while(fgets(buffer,sizeof(buffer),symfile)){
    buffer[strlen(buffer)-1]=0;
    pbeg=strstr(buffer,"__FOX__");
    if(pbeg){
      pend=strstr(pbeg,"__metaClass");
      if(pend){
        *pend=0;
        addsymbol(pbeg+7);
        }
      }
    }
  }


/* Main */
int main(int argc,char **argv){
  FILE *outfile;
  FILE *symfile;
  char *filename;
  char *classtablename="FOX_CLASSES";
  int hadfiles=0;
  int commandlinefiles=1;
  int i;
  
  /* Initialize */
  outfile=stdout;
  symfile=stdin;
  
  /* Make some room */
  maxsymbols=INITIALSIZE;
  numsymbols=0;
  symbols=(char**)calloc(sizeof(char*),maxsymbols);
  
  
  /* Process options */
  for(i=1; i<argc; i++){
    
    /* Option */
    if(argv[i][0]=='-'){
      
      /* Change output file */
      if(argv[i][1]=='o'){
        i++;
        if(i>=argc){
          fprintf(stderr,"clstab: missing argument for -o option\n");
          exit(1);
          }
        if(outfile!=stdout) fclose(outfile);
        outfile=fopen(argv[i],"w");
        if(!outfile){
          fprintf(stderr,"clstab: unable to open output file %s\n",argv[i]);
          exit(1);
          }
        }
      
      /* Print help */
      else if(argv[i][1]=='h'){
        printusage();
        exit(0);
        }
      
      /* Print help */
      else if(argv[i][1]=='n'){
        i++;
        if(i>=argc){
          fprintf(stderr,"clstab: missing argument for -n option\n");
          exit(1);
          }
        classtablename=argv[i];
        }
      
      /* Print version */
      else if(argv[i][1]=='v'){
        fprintf(stderr,"clstab version %s\n",version);
        exit(0);
        }
      
      /* Generate dummy table */
      else if(argv[i][1]=='d'){
        addsymbol("FXObject");
        hadfiles=1;
        commandlinefiles=0;
        }
      }
    
    /* File name on command line */
    else if(commandlinefiles){
      
      /* Get file name */
      if((filename=strrchr(argv[i],PATHSEP))!=0) 
        filename=filename+1; 
      else 
        filename=argv[i];
      
      /* Open object file */
      symfile=fopen(argv[i],"r");
      if(!symfile){
        fprintf(stderr,"clstab: unable to open input file %s\n",argv[i]);
        exit(1);
        }
      
      /* Read the file */
      processfile(symfile);
      
      /* Close it */
      fclose(symfile);
      hadfiles=1;
      }
    }
  
  /* If had no files yet, read from stdin */
  if(!hadfiles){
    processfile(stdin);
    }
  
  /* Print header */
  fprintf(outfile,"/* Generated by clstab */\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"class FXMetaClass;\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"struct FXClassTable {\n");
  fprintf(outfile,"  const FXMetaClass *const *classes;\n");
  fprintf(outfile,"  unsigned int              nclasses;\n");
  fprintf(outfile,"  };\n");
  fprintf(outfile,"\n");
  
  /* Spit out external defines */
  for(i=0; i<numsymbols; i++){
    fprintf(outfile,"extern \"C\" const FXMetaClass __FOX__%s__metaClass[1];\n",symbols[i]);
    }
  fprintf(outfile,"\n\n");
  
  
  /* Spit out the table itself */
  fprintf(outfile,"const FXMetaClass *const __class_table__[] = {\n");
  for(i=0; i<numsymbols; i++){
    fprintf(outfile,"  __FOX__%s__metaClass,\n",symbols[i]);
    }
  fprintf(outfile,"  };\n");
  fprintf(outfile,"\n");
  
  /* Define the structure */
  fprintf(outfile,"const FXClassTable %s={__class_table__,%d};\n",classtablename,numsymbols);
  
  /* Done */
  fclose(outfile);
  }


  
