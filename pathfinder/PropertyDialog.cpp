/********************************************************************************
*                                                                               *
*                  F i l e   P r o p e r t i e s   D i a l o g                  *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2005 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This program is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU General Public License as published by          *
* the Free Software Foundation; either version 2 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This program is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU General Public License for more details.                                  *
*                                                                               *
* You should have received a copy of the GNU General Public License             *
* along with this program; if not, write to the Free Software                   *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: PropertyDialog.cpp,v 1.20 2005/01/16 16:06:06 fox Exp $                  *
********************************************************************************/
#include "xincs.h"
#include "fx.h"
#include "PropertyDialog.h"
#include "PathFinder.h"
#include "icons.h"
#include <stdio.h>
#include <stdlib.h>

/*
  Notes:
  - Refer to RFC 2045, 2046, 2047, 2048, and 2077.
    The Internet media type registry is at:
    ftp://ftp.iana.org/in-notes/iana/assignments/media-types/
*/

/**********************************  Preferences  ******************************/

// Map
FXDEFMAP(PropertyDialog) PropertyDialogMap[]={
  FXMAPFUNC(SEL_COMMAND,PropertyDialog::ID_ADD_MIMETYPE,PropertyDialog::onCmdAddMimeType),
  FXMAPFUNC(SEL_COMMAND,PropertyDialog::ID_REMOVE_MIMETYPE,PropertyDialog::onCmdRemoveMimeType),
  FXMAPFUNC(SEL_CHANGED,PropertyDialog::ID_CHANGE_MIMETYPE,PropertyDialog::onCmdChangeMimeType),
  };


FXIMPLEMENT(PropertyDialog,FXDialogBox,PropertyDialogMap,ARRAYNUMBER(PropertyDialogMap))


// Create properties dialog
PropertyDialog::PropertyDialog(FXWindow *owner):
  FXDialogBox(owner,"Properties",DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE,0,0,0,0, 4,4,4,4, 4,4){

  // Close button
  FXHorizontalFrame *closebox=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
  new FXButton(closebox,"&OK",NULL,this,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20,5,5);
  new FXButton(closebox,"&Cancel",NULL,this,FXDialogBox::ID_CANCEL,BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20,5,5);

  // Separator
  new FXHorizontalSeparator(this,SEPARATOR_GROOVE|LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);

  FXTabBook* tabbook=new FXTabBook(this,NULL,0,LAYOUT_SIDE_TOP|TABBOOK_TOPTABS|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  // ===== General Info =====
  new FXTabItem(tabbook,"&General\tGeneral Information\tGeneral information about the item.",NULL,TAB_TOP|ICON_BEFORE_TEXT);
  FXVerticalFrame *generalFrame=new FXVerticalFrame(tabbook,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED|FRAME_THICK);

  FXGroupBox *namegroup=new FXGroupBox(generalFrame,"Description",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  filename=new FXLabel(namegroup,NULL,NULL,LAYOUT_CENTER_Y|LAYOUT_SIDE_LEFT|ICON_BEFORE_TEXT|LAYOUT_FIX_HEIGHT,0,0,0,40);
  filename->setTarget(owner);
  filename->setSelector(PathFinderMain::ID_FILE_DESC);

  FXGroupBox *attrgroup=new FXGroupBox(generalFrame,"Attributes",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  FXMatrix *attrmatrix=new FXMatrix(attrgroup,2,MATRIX_BY_COLUMNS|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(attrmatrix,"Type:",NULL,LAYOUT_RIGHT);
  filetype=new FXLabel(attrmatrix,NULL,NULL,LAYOUT_LEFT|LAYOUT_FILL_COLUMN);
  filetype->setTarget(owner);
  filetype->setSelector(PathFinderMain::ID_FILE_TYPE);
  new FXLabel(attrmatrix,"Location:",NULL,LAYOUT_RIGHT);
  directory=new FXLabel(attrmatrix,NULL,NULL,LAYOUT_LEFT|LAYOUT_FILL_COLUMN);
  directory->setTarget(owner);
  directory->setSelector(PathFinderMain::ID_FILE_LOCATION);
  new FXLabel(attrmatrix,"Size:",NULL,LAYOUT_RIGHT);
  filesize=new FXLabel(attrmatrix,NULL,NULL,LAYOUT_LEFT|LAYOUT_FILL_COLUMN);
  filesize->setTarget(owner);
  filesize->setSelector(PathFinderMain::ID_FILE_SIZE);

  FXGroupBox *timegroup=new FXGroupBox(generalFrame,"File Time",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  FXMatrix *timematrix=new FXMatrix(timegroup,2,MATRIX_BY_COLUMNS|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(timematrix,"Created:",NULL,LAYOUT_RIGHT);
  createtime=new FXLabel(timematrix,NULL,NULL,LAYOUT_LEFT|LAYOUT_FILL_COLUMN);
  createtime->setTarget(owner);
  createtime->setSelector(PathFinderMain::ID_FILE_CREATED);
  new FXLabel(timematrix,"Modified:",NULL,LAYOUT_RIGHT);
  modifytime=new FXLabel(timematrix,NULL,NULL,LAYOUT_LEFT|LAYOUT_FILL_COLUMN);
  modifytime->setTarget(owner);
  modifytime->setSelector(PathFinderMain::ID_FILE_MODIFIED);
  new FXLabel(timematrix,"Accessed:",NULL,LAYOUT_RIGHT);
  accesstime=new FXLabel(timematrix,NULL,NULL,LAYOUT_LEFT|LAYOUT_FILL_COLUMN);
  accesstime->setTarget(owner);
  accesstime->setSelector(PathFinderMain::ID_FILE_ACCESSED);

  // ===== Permissions =====
  new FXTabItem(tabbook,"&Permissions\tAccess Permissions\tAccess permissions for this item.",NULL,TAB_TOP|ICON_BEFORE_TEXT);
  FXVerticalFrame *permFrame=new FXVerticalFrame(tabbook,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED|FRAME_THICK);
  FXGroupBox *accessgroup=new FXGroupBox(permFrame,"Access Permissions",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  FXMatrix *accessmatrix=new FXMatrix(accessgroup,6,MATRIX_BY_COLUMNS|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXFrame(accessmatrix,LAYOUT_FILL_COLUMN);
  new FXLabel(accessmatrix,"Read",NULL,LAYOUT_CENTER_X);
  new FXLabel(accessmatrix,"Write",NULL,LAYOUT_CENTER_X);
  new FXLabel(accessmatrix,"Exec",NULL,LAYOUT_CENTER_X);
  new FXFrame(accessmatrix,LAYOUT_FILL_COLUMN);
  new FXLabel(accessmatrix,"Special",NULL,LAYOUT_CENTER_X);

  new FXLabel(accessmatrix,"&User",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_RUSR,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_WUSR,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_XUSR,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  new FXLabel(accessmatrix,"Set UID",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_SUID,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);

  new FXLabel(accessmatrix,"&Group",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_RGRP,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_WGRP,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_XGRP,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  new FXLabel(accessmatrix,"Set GID",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_SGID,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);

  new FXLabel(accessmatrix,"&Other",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_ROTH,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_WOTH,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_XOTH,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  new FXLabel(accessmatrix,"Sticky",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  new FXCheckButton(accessmatrix,NULL,owner,PathFinderMain::ID_SVTX,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);

  FXGroupBox *ownergroup=new FXGroupBox(permFrame,"Ownership",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  FXMatrix *ownermatrix=new FXMatrix(ownergroup,2,MATRIX_BY_COLUMNS|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(ownermatrix,"O&wner",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  fileowner=new FXTextField(ownermatrix,20,owner,PathFinderMain::ID_OWNER,LAYOUT_FILL_X|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN|FRAME_SUNKEN|FRAME_THICK);
  new FXLabel(ownermatrix,"Grou&p",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  filegroup=new FXTextField(ownermatrix,20,owner,PathFinderMain::ID_GROUP,LAYOUT_FILL_X|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN|FRAME_SUNKEN|FRAME_THICK);

  // ===== Bindings =====
  new FXTabItem(tabbook,"&Bindings\tBindings\tBindings associated with the item.",NULL,TAB_TOP|ICON_BEFORE_TEXT);
  FXVerticalFrame *bindingFrame=new FXVerticalFrame(tabbook,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED|FRAME_THICK);

  FXHorizontalFrame *iconsandext=new FXHorizontalFrame(bindingFrame,LAYOUT_FILL_X,0,0,0,0, 0,0,0,0);

  // List of possible extensions of this file type
  FXGroupBox *extensiongroup=new FXGroupBox(iconsandext,"File Extensions",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXVerticalFrame *extensionFrame=new FXVerticalFrame(extensiongroup,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
  extensions=new FXList(extensionFrame,NULL,0,LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y|HSCROLLER_NEVER);
  extensions->appendItem("fox.tar.gz");
  extensions->appendItem("tar.gz");
  extensions->appendItem("gz");
  extensions->appendItem(FXFileDict::defaultFileBinding);

  // Various icons for this extension
  FXGroupBox *iconsgroup=new FXGroupBox(iconsandext,"Icons",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_Y);
  FXMatrix *iconsmatrix=new FXMatrix(iconsgroup,4,MATRIX_BY_COLUMNS|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
  new FXLabel(iconsmatrix,"Big Open:",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  bigopen=new FXButton(iconsmatrix,"\tChange icon",NULL,NULL,0,FRAME_RAISED|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_COLUMN,0,0,36,36, 1,1,1,1);
  new FXLabel(iconsmatrix,"Closed:",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  bigclosed=new FXButton(iconsmatrix,"\tChange icon",NULL,NULL,0,FRAME_RAISED|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_COLUMN,0,0,36,36, 1,1,1,1);
  new FXLabel(iconsmatrix,"Small Open:",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  smallopen=new FXButton(iconsmatrix,"\tChange icon",NULL,NULL,0,FRAME_RAISED|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_COLUMN,0,0,36,36, 1,1,1,1);
  new FXLabel(iconsmatrix,"Closed:",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN);
  smallclosed=new FXButton(iconsmatrix,"\tChange icon",NULL,NULL,0,FRAME_RAISED|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_COLUMN,0,0,36,36, 1,1,1,1);

  // Description of extension
  FXGroupBox *descgroup=new FXGroupBox(bindingFrame,"Description of extension",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X);
  description=new FXTextField(descgroup,20,NULL,0,LAYOUT_FILL_X|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN|FRAME_SUNKEN|FRAME_THICK);

  FXHorizontalFrame *commandandmime=new FXHorizontalFrame(bindingFrame,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

  // Mime type
  FXGroupBox *mimegroup=new FXGroupBox(commandandmime,"Mime Types",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  FXHorizontalFrame *mimebuttons=new FXHorizontalFrame(mimegroup,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0, 0,0,0,0);
  new FXButton(mimebuttons,"Add...",NULL,this,ID_ADD_MIMETYPE,FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_CENTER_X);
  new FXButton(mimebuttons,"Remove",NULL,this,ID_REMOVE_MIMETYPE,FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_CENTER_X);

  FXVerticalFrame *mimeFrame=new FXVerticalFrame(mimegroup,LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
  mimetypes=new FXList(mimeFrame,this,ID_CHANGE_MIMETYPE,LIST_BROWSESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y|HSCROLLER_NEVER);

  // Add a few if none exist yet
  mimetypes->appendItem("image/gif");
  mimetypes->appendItem("audio/mp3");

  // Command
  FXGroupBox *commandgroup=new FXGroupBox(commandandmime,"Command",GROUPBOX_TITLE_LEFT|FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  runinterminal=new FXCheckButton(commandgroup,"Run in terminal",NULL,0,ICON_BEFORE_TEXT|LAYOUT_LEFT|LAYOUT_SIDE_BOTTOM);
  changedirectory=new FXCheckButton(commandgroup,"Change directory",NULL,0,ICON_BEFORE_TEXT|LAYOUT_LEFT|LAYOUT_SIDE_BOTTOM);
  new FXButton(commandgroup,"Set...",NULL,NULL,0,LAYOUT_SIDE_RIGHT|LAYOUT_CENTER_Y|FRAME_RAISED|FRAME_THICK);
  command=new FXTextField(commandgroup,2,NULL,0,LAYOUT_SIDE_LEFT|LAYOUT_FILL_X|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK);
  }


// Add mime type
long PropertyDialog::onCmdAddMimeType(FXObject*,FXSelector,void*){
  FXString string;
  if(FXInputDialog::getString(string,this,"Add New Mime Type","Add a new mime type to the list:")){
    mimetypes->appendItem(string);
    }
  return 1;
  }


// Remove mime type
long PropertyDialog::onCmdRemoveMimeType(FXObject*,FXSelector,void*){
  if(0<=mimetypes->getCurrentItem()){
    mimetypes->removeItem(mimetypes->getCurrentItem(),TRUE);
    }
  return 1;
  }


// Change mime type
long PropertyDialog::onCmdChangeMimeType(FXObject*,FXSelector,void*){
  FXint index=mimetypes->getCurrentItem();
  if(0<=index){
    FXTRACE((190,"Switching to mime type: %s\n",mimetypes->getItemText(index).text()));
    }
  return 1;
  }


// Change big open icon
void PropertyDialog::setBigIconOpen(FXIcon* icon){
  bigopen->setIcon(icon);
  }


// Change big closed icon
void PropertyDialog::setBigIconClosed(FXIcon* icon){
  filename->setIcon(icon);
  bigclosed->setIcon(icon);
  }


// Get big open icon
FXIcon* PropertyDialog::getBigIconOpen() const {
  return bigopen->getIcon();
  }


// Get big closed icon
FXIcon* PropertyDialog::getBigIconClosed() const {
  return bigclosed->getIcon();
  }


// Change small open icon
void PropertyDialog::setSmallIconOpen(FXIcon* icon){
  smallopen->setIcon(icon);
  }


// Change small closed icon
void PropertyDialog::setSmallIconClosed(FXIcon* icon){
  smallclosed->setIcon(icon);
  }


// Get small open icon
FXIcon* PropertyDialog::getSmallIconOpen() const {
  return smallopen->getIcon();
  }


// Get small closed icon
FXIcon* PropertyDialog::getSmallIconClosed() const {
  return smallclosed->getIcon();
  }


// Change command
void PropertyDialog::setCommand(const FXString& cmd){
  command->setText(cmd);
  }


// Get command
FXString PropertyDialog::getCommand() const{
  return command->getText();
  }


// Destroy
PropertyDialog::~PropertyDialog(){
  }
