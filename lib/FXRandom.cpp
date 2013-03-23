/********************************************************************************
*                                                                               *
*                  R a n d o m   N u m b e r   G e n e r a t o r                *
*                                                                               *
*********************************************************************************
* Copyright (C) 2007,2013 by Jeroen van der Zijp.   All Rights Reserved.        *
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
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXRandom.h"


/*
  Notes:
  - Algorithm based on Numerical Recipes, 3ed, pp. 351-352.
  - Original algorithm George Marsaglia, "Random number generators",
    Journal of Modern Applied Statistical Methods 2, No. 2, 2003.
  - Different shift-counts are certainly possible, and produce different
    sequences (of the same period), see G. Marsaglia, "Xorshift RNGs".
*/

#define SHIFTA    21
#define SHIFTB    35
#define SHIFTC     4

using namespace FX;


namespace FX {

/*******************************************************************************/

// Construct random generator with default seed
FXRandom::FXRandom():state(FXULONG(4101842887655102017)){
  }


// Construct random generator with given seed s
FXRandom::FXRandom(FXulong s):state(FXULONG(4101842887655102017)^s){
  }


// Initialize seed
void FXRandom::seed(FXulong s){
  state=s^FXULONG(4101842887655102017);
  }


// Generate next state
FXulong FXRandom::next(){
  state^=state>>SHIFTA;
  state^=state<<SHIFTB;
  state^=state>>SHIFTC;
  return state;
  }


// Get random long
FXulong FXRandom::randLong(){
  return next()*FXULONG(2685821657736338717);
  }


// Get random double
FXfloat FXRandom::randFloat(){
#if (_MSC_VER <= 1300)
  return ((FXuint)randLong())*2.3283064370808E-10f;
#else
  return randLong()*5.42101086242752217E-20f;
#endif
  }


// Get random double
FXdouble FXRandom::randDouble(){
#if (_MSC_VER <= 1300)
  return ((FXuint)randLong())*2.3283064370808E-10;
#else
  return randLong()*5.42101086242752217E-20;
#endif
  }

}


