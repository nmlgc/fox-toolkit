/********************************************************************************
*                                                                               *
*     R e g u l a r   E x p r e s s i o n   M a t c h i n g   E n g i n e       *
*                                                                               *
*********************************************************************************
*  Copyright (c) 1986 by University of Toronto.                                 *
*  Written by Henry Spencer.  Not derived from licensed software.               *
*  This has been shamelessly hacked by Jeroen van der Zijp; if broken, its      *
*  probably my fault.                                                           *
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
* $Id: fxregex.h,v 1.3 1998/03/08 06:54:12 jeroen Exp $                         *
********************************************************************************/
#ifndef FXREGEXPR_H
#define FXREGEXPR_H

#define NSUBEXP  10


// Renamed fxregexp so as to not bite anybody else's matchers.
struct fxregexp {
  const char *startp[NSUBEXP];
  const char *endp[NSUBEXP];
  char  regstart;           /* Internal use only. */
  char  reganch;            /* Internal use only. */
  char *regmust;            /* Internal use only. */
  int   regmlen;            /* Internal use only. */
  char  program[1];         /* Unwarranted chumminess with compiler. */
  };



// Compile regular expression
extern fxregexp *fxregcomp(const char *exp);

// Free regular expression program
extern void fxregfree(fxregexp* prog);

// Execute regular expression
extern int fxregexec(fxregexp *prog,const char *string);

// Perform regular expression substitution
extern void fxregsubst(fxregexp *prog,const char *source,char *dest);


#endif
