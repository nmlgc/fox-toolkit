/********************************************************************************
*                                                                               *
*                     H i g h l i g h t   E n g i n e                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 2002 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: Hilite.h,v 1.11 2002/01/25 14:23:10 jeroen Exp $                         *
********************************************************************************/
#ifndef HILITE_H
#define HILITE_H



// Opaque highlight node
struct HLNode;


// Highlight engine
class FXAPI Hilite {
private:
  HLNode  *root;  // Root of all syntax nodes
private:
  Hilite(const Hilite&);
  Hilite &operator=(const Hilite&);
public:

  // Create syntax coloring engine
  Hilite():root(NULL){}

  // Stylize text
  void stylize(const FXchar* text,FXchar *style,FXint fm,FXint to) const;
  
  // Stylize text from given node
  void stylize(const FXchar* text,FXchar *style,FXint fm,FXint to,const HLNode* start) const;

  // Append highlight pattern
  HLNode *append(const FXchar* pattern,FXint style,FXint priority=0,FXint context=0,HLNode *parent=NULL);
  
  // Remove highlight pattern
  void remove(HLNode* node);

  // Clear all pattern nodes
  void clear();

  // Find pattern node by style
  HLNode *find(FXint style) const;
  
  // Get top node
  HLNode *top() const { return root; }

  // Style of the pattern node
  FXint style(HLNode *node) const;
  
  // Context of the pattern node
  FXint context(HLNode *node) const;
  
  // Get parent of pattern node
  HLNode *parent(HLNode *node) const;
  
  // Get first alternative of pattern node
  HLNode *alternative(HLNode *node) const;
  
  // Get subnode of pattern node
  HLNode *sub(HLNode *node) const;
  
  // Clean up
  ~Hilite();
  };


#endif

