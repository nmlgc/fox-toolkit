/********************************************************************************
*                                                                               *
*                  F O X   D e s k t o p   C a l c u l a t o r                  *
*                                                                               *
*********************************************************************************
* Copyright (C) 2001 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: main.cpp,v 1.4 2001/08/23 04:54:01 jeroen Exp $                          *
********************************************************************************/
#include "fx.h"
#include "fxkeys.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <ctype.h>
#include "Calculator.h"


// Start the whole thing
int main(int argc,char *argv[]){

  // Make application
  FXApp application("Calculator",FXString::null);

  // Open display
  application.init(argc,argv);

  // Main window
  Calculator* calculator=new Calculator(&application);

  // Handle interrupt to save stuff nicely
  application.addSignal(SIGINT,calculator,Calculator::ID_QUIT);

  // Create app
  application.create();

  // Run
  return application.run();
  }


