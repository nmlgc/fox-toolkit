/********************************************************************************
*                                                                               *
*                             R a n g e    C l a s s                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 1994 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXRange.h,v 1.3 1998/04/07 00:39:15 jeroen Exp $                         *
********************************************************************************/
#ifndef FXRANGE_H
#define FXRANGE_H


class FXRange {
protected:
  FXfloat d[3][2];
public:

  FXRange(){}

  // Initialize
  FXRange(const FXfloat xlo,const FXfloat xhi,
          const FXfloat ylo,const FXfloat yhi,
          const FXfloat zlo,const FXfloat zhi){
    d[0][0]=xlo; d[0][1]=xhi;
    d[1][0]=ylo; d[1][1]=yhi;
    d[2][0]=zlo; d[2][1]=zhi;
    }

  // Length of side i
  const FXfloat side(const int i) const { return d[i][1]-d[i][0]; }

  // Width of box
  const FXfloat width() const { return d[0][1]-d[0][0]; }

  // Height of box
  const FXfloat height() const { return d[1][1]-d[1][0]; }

  // Depth of box
  const FXfloat depth() const { return d[2][1]-d[2][0]; }

  // Longest side
  const FXfloat longest() const;

  // shortest side
  const FXfloat shortest() const;

  // Test if empty
  const FXbool empty() const;

  // Test if overlap
  const FXbool overlap(const FXRange& box) const;
   
  // Test if box contains point x,y,z
  const FXbool contains(const FXfloat x,const FXfloat y,const FXfloat z) const;

  // Indexing
  FXfloat* operator[](const int i){ return d[i]; }

  // Indexing
  const FXfloat* operator[](const int i) const { return d[i]; }

  // Include given range into box
  FXRange& include(const FXRange& box);

  // Include point 
  FXRange& include(const FXfloat x,const FXfloat y,const FXfloat z);

  // Clip domain against another 
  FXRange& clipTo(const FXRange& box);

  // Get corners of box
  friend void boxCorners(FXVec* points,const FXRange& box);
  
  // Ray intersection test
  friend FXbool boxIntersect(const FXRange& box,const FXVec& u,const FXVec& v);
  
  // Get center of box
  friend FXVec boxCenter(const FXRange& box);
  
  // Compute diagonal
  friend FXfloat boxDiagonal(const FXRange& box);
  
  // Saving and loading
  friend FXStream& operator<<(FXStream& store,const FXRange& box);
  friend FXStream& operator>>(FXStream& store,FXRange& box);
  };


#endif
