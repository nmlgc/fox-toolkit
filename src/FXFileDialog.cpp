/********************************************************************************
*                                                                               *
*                    F i l e   S e l e c t i o n   D i a l o g                  *
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
* $Id: FXFileDialog.cpp,v 1.23 2002/01/18 22:42:59 jeroen Exp $                 *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXFile.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXRecentFiles.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXShell.h"
#include "FXTopWindow.h"
#include "FXDialogBox.h"
#include "FXFileSelector.h"
#include "FXFileDialog.h"



/*
  To do:
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXFileDialog,FXDialogBox,NULL,0)


// File Open Dialog
FXFileDialog::FXFileDialog(FXWindow* owner,const FXString& name,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXDialogBox(owner,name,opts|DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE,x,y,w,h,0,0,0,0,4,4){
  filebox=new FXFileSelector(this,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  filebox->acceptButton()->setTarget(this);
  filebox->acceptButton()->setSelector(FXDialogBox::ID_ACCEPT);
  filebox->cancelButton()->setTarget(this);
  filebox->cancelButton()->setSelector(FXDialogBox::ID_CANCEL);
  }


// Set file name
void FXFileDialog::setFilename(const FXString& path){
  filebox->setFilename(path);
  }


// Get filename, if any
FXString FXFileDialog::getFilename() const {
  return filebox->getFilename();
  }


// Return empty-string terminated list of selected file names,
FXString* FXFileDialog::getFilenames() const {
  return filebox->getFilenames();
  }


// Set pattern
void FXFileDialog::setPattern(const FXString& ptrn){
  filebox->setPattern(ptrn);
  }


// Get pattern
FXString FXFileDialog::getPattern() const {
  return filebox->getPattern();
  }


// Change patterns, each pattern separated by newline
void FXFileDialog::setPatternList(const FXString& patterns){
  filebox->setPatternList(patterns);
  }


// Return list of patterns
FXString FXFileDialog::getPatternList() const {
  return filebox->getPatternList();
  }


// Set directory
void FXFileDialog::setDirectory(const FXString& path){
  filebox->setDirectory(path);
  }


// Get directory
FXString FXFileDialog::getDirectory() const {
  return filebox->getDirectory();
  }


// Set current file pattern from the list
void FXFileDialog::setCurrentPattern(FXint n){
  filebox->setCurrentPattern(n);
  }


// Return current pattern
FXint FXFileDialog::getCurrentPattern() const {
  return filebox->getCurrentPattern();
  }

FXString FXFileDialog::getPatternText(FXint patno) const {
  return filebox->getPatternText(patno);
  }


void FXFileDialog::setPatternText(FXint patno,const FXString& text){
  filebox->setPatternText(patno,text);
  }


// Set list of patterns (DEPRECATED)
void FXFileDialog::setPatternList(const FXchar **ptrns){
  filebox->setPatternList(ptrns);
  }


// Change space for item
void FXFileDialog::setItemSpace(FXint s){
  filebox->setItemSpace(s);
  }


// Get space for item
FXint FXFileDialog::getItemSpace() const {
  return filebox->getItemSpace();
  }


// Change File List style
void FXFileDialog::setFileBoxStyle(FXuint style){
  filebox->setFileBoxStyle(style);
  }


// Return File List style
FXuint FXFileDialog::getFileBoxStyle() const {
  return filebox->getFileBoxStyle();
  }


// Change file selection mode
void FXFileDialog::setSelectMode(FXuint mode){
  filebox->setSelectMode(mode);
  }


// Return file selection mode
FXuint FXFileDialog::getSelectMode() const {
  return filebox->getSelectMode();
  }


// Show readonly button
void FXFileDialog::showReadOnly(FXbool show){
  filebox->showReadOnly(show);
  }


// Return TRUE if readonly is shown
FXbool FXFileDialog::shownReadOnly() const {
  return filebox->shownReadOnly();
  }



// Set initial state of readonly button
void FXFileDialog::setReadOnly(FXbool state){
  filebox->setReadOnly(state);
  }


// Get readonly state
FXbool FXFileDialog::getReadOnly() const {
  return filebox->getReadOnly();
  }


// Save data
void FXFileDialog::save(FXStream& store) const {
  FXDialogBox::save(store);
  store << filebox;
  }


// Load data
void FXFileDialog::load(FXStream& store){
  FXDialogBox::load(store);
  store >> filebox;
  }


// Cleanup
FXFileDialog::~FXFileDialog(){
  filebox=(FXFileSelector*)-1;
  }


// Open existing filename
FXString FXFileDialog::getOpenFilename(FXWindow* owner,const FXString& caption,const FXString& path,const FXString& patterns,FXint initial){
  FXFileDialog opendialog(owner,caption);
  FXString filename;
  opendialog.setSelectMode(SELECTFILE_EXISTING);
  opendialog.setPatternList(patterns);
  opendialog.setCurrentPattern(initial);
  opendialog.setFilename(path);
  if(opendialog.execute()){
    filename=opendialog.getFilename();
    if(FXFile::isFile(filename)) return filename;
    }
  return FXString::null;
  }


// Save to filename
FXString FXFileDialog::getSaveFilename(FXWindow* owner,const FXString& caption,const FXString& path,const FXString& patterns,FXint initial){
  FXFileDialog savedialog(owner,caption);
  savedialog.setSelectMode(SELECTFILE_ANY);
  savedialog.setPatternList(patterns);
  savedialog.setCurrentPattern(initial);
  savedialog.setFilename(path);
  if(savedialog.execute()){
    return savedialog.getFilename();
    }
  return FXString::null;
  }


// Open multiple existing files
FXString* FXFileDialog::getOpenFilenames(FXWindow* owner,const FXString& caption,const FXString& path,const FXString& patterns,FXint initial){
  FXFileDialog opendialog(owner,caption);
  opendialog.setSelectMode(SELECTFILE_MULTIPLE);
  opendialog.setPatternList(patterns);
  opendialog.setCurrentPattern(initial);
  opendialog.setDirectory(path);
  if(opendialog.execute()){
    return opendialog.getFilenames();
    }
  return NULL;
  }


// Open existing directory name
FXString FXFileDialog::getOpenDirectory(FXWindow* owner,const FXString& caption,const FXString& path){
  FXFileDialog dirdialog(owner,caption);
  FXString dirname;
  dirdialog.setSelectMode(SELECTFILE_DIRECTORY);
  dirdialog.setFilename(path);
  if(dirdialog.execute()){
    dirname=dirdialog.getFilename();
    if(FXFile::isDirectory(dirname)) return dirname;
    }
  return FXString::null;
  }

