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
* $Id: Hilite.cpp,v 1.17 2002/02/08 23:05:06 fox Exp $                          *
********************************************************************************/
#include "fx.h"
#include "FXRex.h"
#include "Hilite.h"


/*
  Notes:
  - Basically works
*/

/*******************************************************************************/


struct HLNode {
  FXRex    pattern;       // Compiled expression
  FXint    priority;      // Priority
  FXint    context;       // Lines of context
  FXint    style;         // Style
  HLNode  *parent;        // Parent node
  HLNode  *alt;           // Alternative
  HLNode  *sub;           // Sub node
  
  // Construct node
  HLNode(const FXchar* rx,FXint st,FXint pr,FXint cx,HLNode* pa);

  // Delete node
 ~HLNode();
  };

  
// Construct node
HLNode::HLNode(const FXchar* rx,FXint st,FXint pr,FXint cx,HLNode* pa):pattern(rx,REX_NEWLINE|REX_CAPTURE),priority(pr),context(cx),style(st),parent(pa){
  alt=NULL;
  sub=NULL;
  }


// Delete node
HLNode::~HLNode(){
  register HLNode *n;
  while(sub){ 
    n=sub; 
    sub=sub->alt; 
    delete n; 
    }
  }


/*******************************************************************************/


// Stylize recursively
void Hilite::stylize(const FXchar* text,FXchar *style,FXint fm,FXint to,const HLNode* start) const {
  register const HLNode *node;
  register FXint pos;
  FXint beg[10];
  FXint end[10];
  pos=fm;
  while(pos<to){
    for(node=start; node; node=node->alt){
      if(node->pattern.match(text,to,beg,end,REX_NOT_EMPTY|REX_FORWARD,10,pos,pos)){
        memset(&style[beg[0]],node->style,end[0]-beg[0]);
        if(node->sub){
          stylize(text,style,beg[0],end[0],node->sub);  // FIXME need to be able to take advantage of captured text...
          }
        pos=end[0];
        goto nxt;
        }
      }
    pos++;
nxt:continue;
    }
  }
  

// Stylize text
void Hilite::stylize(const FXchar* text,FXchar *style,FXint fm,FXint to) const {
  memset(&style[fm],0,to-fm);
  if(root){
    stylize(text,style,fm,to,root);
    }
  }



// Append highlight pattern
HLNode *Hilite::append(const FXchar* pattern,FXint style,FXint priority,FXint context,HLNode *parent){
  register HLNode *node=new HLNode(pattern,style,priority,context,parent);
  register HLNode **nn=parent?&parent->sub:&root;
  register HLNode *n=*nn;
  while(n && (node->priority > n->priority)){
    nn=&n->alt;
    n=*nn;
    }
  node->alt=n;
  *nn=node;
  return node;
  }


// Remove highlight pattern
void Hilite::remove(HLNode* node){
  register HLNode **nn=node->parent?&node->parent->sub:&root;
  register HLNode *n=*nn;
  while(n){
    if(n==node){
      *nn=node->alt;
      delete node;
      break;
      }
    nn=&n->alt;
    n=*nn;
    }
  }


// Find pattern node by name
HLNode *Hilite::find(FXint style) const {
  register HLNode *node=root;
  while(node){
    if(node->style==style){break;}
    if(node->sub){node=node->sub; continue; }
    while(!node->alt && node->parent){ node=node->parent; }
    node=node->alt;
    }
  return node;
  }


// Clear all syntax rules
void Hilite::clear(){
  register HLNode *n;
  while(root){
    n=root; 
    root=root->alt; 
    delete n;
    }
  }


// Return style
FXint Hilite::style(HLNode *node) const {
  return node->style;
  }


// Return context
FXint Hilite::context(HLNode *node) const {
  return node->context;
  }


// Get parent of node
HLNode *Hilite::parent(HLNode *node) const {
  return node->parent;
  }


// Get first alternative of node
HLNode *Hilite::alternative(HLNode *node) const {
  return node->alt;
  }


// Get sub node of node
HLNode *Hilite::sub(HLNode *node) const {
  return node->sub;
  }


// Clean up
Hilite::~Hilite(){
  clear();
  }
