/********************************************************************************
*                                                                               *
*                             Regular Expression Test                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2001 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* $Id: rex.cpp,v 1.14 2001/12/27 07:52:33 jeroen Exp $                          *
********************************************************************************/
#include "fx.h"
#include "FXRex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*


*/

#define NCAP 10    // Must be less that or equal to 10

/*******************************************************************************/


// Start the whole thing
int main(int argc,char** argv){
  FXRexError err;
  FXRex rex;
  FXbool ok;
  FXint i;
  FXint beg[NCAP];
  FXint end[NCAP];
  fxTraceLevel=101;
  if(argc==1){
    fprintf(stderr,"no arguments\n");
    exit(1);
    }
  if(2<=argc){
    err=rex.parse(argv[1],REX_NORMAL);
    fprintf(stderr,"parse(\"%s\") = %s\n",argv[1],FXRex::getError(err));
    }
  if(3<=argc){
    ok=rex.match(argv[2],strlen(argv[2]),beg,end,REX_FORWARD,NCAP);
    if(ok){
      fprintf(stderr,"match at %d:%d\n",beg[0],end[0]);
      for(i=1; i<NCAP; i++){
        fprintf(stderr,"capture at %d:%d\n",beg[i],end[i]);
        }
      for(i=beg[0]; i<end[0]; i++){
        fprintf(stderr,"%c",argv[2][i]);
        }
      fprintf(stderr,"\n");
      }
    else{
      fprintf(stderr,"no match\n");
      }
    }
  return 1;
  }




// int lineno;   // Error location
//
//
//
// void complain(char *s1,char * s2){
//   fprintf(stderr,"attempt: %d: ",lineno);
//   fprintf(stderr,s1,s2);
//   }
//
//
//
// void attempt(char **fields) {
//   char buf[BUFSIZ];
//   regexp *r;
//
//   //printf("line: %d\n",lineno);
//
//   r=regcomp(fields[0]);
//
//   if(r==NULL){
//     if(*fields[2]!='c') complain("regcomp failure in \"%s\"",fields[0]);
//     return;
//     }
//
//   if(*fields[2]=='c'){
//     complain("unexpected regcomp success in \"%s\"",fields[0]);
//     free(r);
//     return;
//     }
//
//   if(!regexec(r,fields[1])){
//     if(*fields[2]!='n') complain("regexec failure in \"%s\"",fields[0]);
//     free(r);
//     return;
//     }
//
//   if(*fields[2]=='n') {
//     complain("unexpected regexec success in \"%s\"",fields[0]);
//     free(r);
//     return;
//     }
//
//   regsub(r,fields[3],buf);
//
//   if(strcmp(buf,fields[4])!=0){
//     complain("regsub result \"%s\" wrong",buf);
//     free(r);
//     return;
//     }
//
//   free(r);
//   }
//
//
// void multiple(){
//   char rbuf[BUFSIZ];
//   char *field[5];
//   char *scan;
//   int i;
//
//   lineno=1;
//
//   // Perform test battery
//   while(fgets(rbuf,sizeof(rbuf),stdin)!=NULL){
//
//     // Strip return
//     rbuf[strlen(rbuf)-1]='\0';
//
//     // Split into fields
//     scan=rbuf;
//     for(i=0; i<5; i++){
//       field[i]=scan;
//       if(field[i]==NULL){
//         printf("Bad testfile format\n");
//         exit(1);
//         }
//       scan=strchr(scan,'\t');
//       if(scan!=NULL) *scan++='\0';
//       }
//
//     // Perform test
//     attempt(field);
//     lineno++;
//     }
//   }
//
//
//
// // Run test battery
// int main(int argc,char *argv[]){
//   char buf[BUFSIZ];
//   regexp *r;
//   const char *p;
//   int i,m;
//
//   if(argc==1){
//     multiple();
//     exit(0);
//     }
//
//   r=regcomp(argv[1]);
//   printf("compile: \"%s\" %s\n",argv[1],r?"OK":"FAIL");
//
//   if(argc>2){
//     m=regexec(r,argv[2]);
//     printf("match: \"%s\" %s ",argv[2],m?"MATCH":"FAIL");
//     for(i=0; i<NSUBEXP && r->startp[i]!=NULL && r->endp[i]!=NULL; i++){
//       printf("\"");
//       for(p=r->startp[i]; p!=r->endp[i]; p++){
//         printf("%c",*p);
//         }
//       printf("\" ");
//       }
//     printf("\n");
//     }
//
//   if(argc>3){
//     regsub(r,argv[3],buf);
//     printf("subst: \"%s\" is \"%s\"\n",argv[3],buf);
//     }
//
//   exit(0);
//   }
