/********************************************************************************
*                                                                               *
*         M u l t i p l e   D o c u m e n t   C l i e n t   W i n d o w         *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXMDIClient.h,v 1.18 2002/01/18 22:42:53 jeroen Exp $                    *
********************************************************************************/
#ifndef FXMDICLIENT_H
#define FXMDICLIENT_H

#ifndef FXSCROLLAREA_H
#include "FXScrollArea.h"
#endif



class FXMDIChild;


/**
* The MDI client window manages a number of MDI child windows in
* a multiple-document interface (MDI) application.
* MDI child windows usually receive messages from the GUI through
* delegation via the MDI client, i.e. the MDI client window is set as
* the target for most GUI commands; the MDI client filters out a few messages
* and forwards all other messages to the active MDI child.
* MDI client can arrange the MDI child windows in various ways:-
* it may maximize one of the MDI child windows, arrange them side-by-side,
* cascade them, or iconify them.
* MDI child windows are notified about changes in the active MDI child
* window by the MDI client.
*/
class FXAPI FXMDIClient : public FXScrollArea {
  FXDECLARE(FXMDIClient)
  friend class FXMDIChild;
protected:
  FXint       cascadex;           // Cascade offset X
  FXint       cascadey;           // Cascade offset Y
  FXint       xmin;               // Space taken up by all children
  FXint       xmax;
  FXint       ymin;
  FXint       ymax;
private:
  FXMDIChild *mdifirst;
  FXMDIChild *mdilast;
  FXMDIChild *active;
protected:
  FXMDIClient();
  void recompute();
  virtual void layout();
private:
  FXMDIClient(const FXMDIClient&);
  FXMDIClient &operator=(const FXMDIClient&);
public:
  long onChanged(FXObject*,FXSelector,void*);
  long onCmdActivateNext(FXObject*,FXSelector,void*);
  long onCmdActivatePrev(FXObject*,FXSelector,void*);
  long onCmdTileHorizontal(FXObject*,FXSelector,void*);
  long onCmdTileVertical(FXObject*,FXSelector,void*);
  long onCmdCascade(FXObject*,FXSelector,void*);
  long onUpdActivateNext(FXObject*,FXSelector,void*);
  long onUpdActivatePrev(FXObject*,FXSelector,void*);
  long onUpdTileVertical(FXObject*,FXSelector,void*);
  long onUpdTileHorizontal(FXObject*,FXSelector,void*);
  long onUpdCascade(FXObject*,FXSelector,void*);
  long onUpdClose(FXObject*,FXSelector,void*);
  long onUpdMenuClose(FXObject*,FXSelector,void*);
  long onUpdRestore(FXObject*,FXSelector,void*);
  long onUpdMenuRestore(FXObject*,FXSelector,void*);
  long onUpdMinimize(FXObject*,FXSelector,void*);
  long onUpdMenuMinimize(FXObject*,FXSelector,void*);
  long onUpdMaximize(FXObject*,FXSelector,void*);
  long onUpdMenuWindow(FXObject*,FXSelector,void*);
  long onCmdCloseDocument(FXObject*,FXSelector,void*);
  long onCmdCloseAllDocuments(FXObject*,FXSelector,void*);
  long onUpdCloseDocument(FXObject*,FXSelector,void*);
  long onUpdCloseAllDocuments(FXObject*,FXSelector,void*);
  long onCmdWindowSelect(FXObject*,FXSelector,void*);
  long onUpdWindowSelect(FXObject*,FXSelector,void*);
  long onUpdAnyWindows(FXObject*,FXSelector,void*);
  virtual long onDefault(FXObject*,FXSelector,void*);
public:
  enum {
    ID_MDI_ANY=65400,
    ID_MDI_1,
    ID_MDI_2,
    ID_MDI_3,
    ID_MDI_4,
    ID_MDI_5,
    ID_MDI_6,
    ID_MDI_7,
    ID_MDI_8,
    ID_MDI_9,
    ID_MDI_10,
    ID_LAST
    };
public:

  /// Construct MDI Client window
  FXMDIClient(FXComposite* p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);

  /// Recalculate interior for scrollbars
  virtual void recalc();

  /// Move MDI Children around
  virtual void moveContents(FXint x,FXint y);

  /// Get first MDI Child
  FXMDIChild* getMDIChildFirst() const { return mdifirst; }

  /// Get last MDI Child
  FXMDIChild* getMDIChildLast() const { return mdilast; }

  /// Pass message to all MDI Child windows
  long forallWindows(FXObject* sender,FXSelector sel,void* ptr);

  /// Pass message to all MDI Child windows whose target is document
  long forallDocWindows(FXObject* document,FXObject* sender,FXSelector sel,void* ptr);

  /// Recompute content width
  virtual FXint getContentWidth();

  /// Recompute content height
  virtual FXint getContentHeight();

  /// Get current active child; may be NULL!
  FXMDIChild* getActiveChild() const { return active; }

  /// Set active MDI Child
  FXbool setActiveChild(FXMDIChild* child=NULL,FXbool notify=TRUE);

  /// Change cascade offset X
  void setCascadeX(FXint off){ cascadex=off; }

  /// Change cascade offset Y
  void setCascadeY(FXint off){ cascadey=off; }

  /// Get cascade offset X
  FXint getCascadeX() const { return cascadex; }

  /// Get cascade offset Y
  FXint getCascadeY() const { return cascadey; }

  /// Save object to a stream
  virtual void save(FXStream& store) const;

  /// Load object from a stream
  virtual void load(FXStream& store);

  /// Destructor
  virtual ~FXMDIClient();
  };


#endif
