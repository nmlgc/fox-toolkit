/********************************************************************************
*                                                                               *
*                             R a n g e    C l a s s                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 1994,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXRange.h,v 1.6 2002/01/18 22:42:54 jeroen Exp $                         *
********************************************************************************/
#ifndef FXRANGE_H
#define FXRANGE_H


/// Range
class FXAPI FXRange {
protected:
  FXfloat d[3][2];
public:

  /// Default constructor
  FXRange(){}

  /// Initialize
  FXRange(FXfloat xlo,FXfloat xhi,FXfloat ylo,FXfloat yhi,FXfloat zlo,FXfloat zhi){
    d[0][0]=xlo; d[0][1]=xhi;
    d[1][0]=ylo; d[1][1]=yhi;
    d[2][0]=zlo; d[2][1]=zhi;
    }

  /// Length of side i
  FXfloat side(FXint i) const { return d[i][1]-d[i][0]; }

  /// Width of box
  FXfloat width() const { return d[0][1]-d[0][0]; }

  /// Height of box
  FXfloat height() const { return d[1][1]-d[1][0]; }

  /// Depth of box
  FXfloat depth() const { return d[2][1]-d[2][0]; }

  /// Longest side
  FXfloat longest() const;

  /// shortest side
  FXfloat shortest() const;

  /// Test if empty
  FXbool empty() const;

  /// Test if overlap
  FXbool overlap(const FXRange& box) const;

  /// Test if box contains point x,y,z
  FXbool contains(FXfloat x,FXfloat y,FXfloat z) const;

  /// Indexing
  FXfloat* operator[](FXint i){ return d[i]; }

  /// Indexing
  const FXfloat* operator[](FXint i) const { return d[i]; }

  /// Include given range into box
  FXRange& include(const FXRange& box);

  /// Include point
  FXRange& include(FXfloat x,FXfloat y,FXfloat z);

  /// Include point
  FXRange& include(const FXVec& v);

  /// Clip domain against another
  FXRange& clipTo(const FXRange& box);

  /// Get corners of box
  friend FXAPI void boxCorners(FXVec* points,const FXRange& box);

  /// Ray intersection test
  friend FXAPI FXbool boxIntersect(const FXRange& box,const FXVec& u,const FXVec& v);

  /// Get center of box
  friend FXAPI FXVec boxCenter(const FXRange& box);

  /// Compute diagonal
  friend FXAPI FXfloat boxDiagonal(const FXRange& box);

  /// Save object to a stream
  friend FXAPI FXStream& operator<<(FXStream& store,const FXRange& box);

  /// Load object from a stream
  friend FXAPI FXStream& operator>>(FXStream& store,FXRange& box);
  };


#endif
