/********************************************************************************
*                                                                               *
*      F i l e   I n f o r m a t i o n   a n d   M a n i p u l a t i o n        *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2005 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXFile.h,v 1.69 2005/01/16 16:06:06 fox Exp $                            *
********************************************************************************/
#ifndef FXFILE_H
#define FXFILE_H


/// Declared as "C" so as to not clash tag-names
extern "C" { struct stat; }

namespace FX {


/// Options for listing files
enum {
  LIST_MATCH_ALL      = 0,              /// Matching files and directories
  LIST_NO_FILES       = 1,              /// Don't list any files
  LIST_NO_DIRS        = 2,              /// Don't list any directories
  LIST_ALL_FILES      = 4,              /// List all files
  LIST_ALL_DIRS       = 8,              /// List all directories
  LIST_HIDDEN_FILES   = 16,             /// List hidden files also
  LIST_HIDDEN_DIRS    = 32,             /// List hidden directories also
  LIST_NO_PARENT      = 64,             /// Don't include '..' in the listing
  LIST_CASEFOLD       = 128             /// Matching is case-insensitive
  };


namespace FXFile {


/// Get current user name
FXString FXAPI getCurrentUserName();

/// Return value of environment variable name
FXString FXAPI getEnvironment(const FXString& name);

/// Return the home directory for the current user.
FXString FXAPI getHomeDirectory();

/// Return the home directory for a given user.
FXString FXAPI getUserDirectory(const FXString& user);

/// Return temporary directory.
FXString FXAPI getTempDirectory();

/// Set the current working directory
FXbool FXAPI setCurrentDirectory(const FXString& path);

/// Get the current working directory
FXString FXAPI getCurrentDirectory();

/// Set the current drive (for Win32 systems)
FXbool FXAPI setCurrentDrive(const FXString& prefix);

/// Return the current drive (for Win32 systems)
FXString FXAPI getCurrentDrive();

/// Get executable path
FXString FXAPI getExecPath();

/**
* Return the directory part of the path name.
* Note that directory("/bla/bla/") is "/bla/bla" and NOT "/bla".
* However, directory("/bla/bla") is "/bla" as we expect!
*/
FXString FXAPI directory(const FXString& file);

/**
* Return name and extension part of the path name.
* Note that name("/bla/bla/") is "" and NOT "bla".
* However, name("/bla/bla") is "bla" as we expect!
*/
FXString FXAPI name(const FXString& file);

/// Return file title, i.e. document name only
FXString FXAPI title(const FXString& file);

/// Return extension part of the file name
FXString FXAPI extension(const FXString& file);

/// Return file name less the extension
FXString FXAPI stripExtension(const FXString& file);

/// Return the drive letter prefixing this file name (if any).
FXString FXAPI drive(const FXString& file);

/// Perform tilde or environment variable expansion
FXString FXAPI expand(const FXString& file);

/**
* Simplify a file path; the path will remain relative if it was relative,
* or absolute if it was absolute.  Also, a trailing "/" will be preserved
* as this is important in other functions.
* For example, simplify("..//aaa/./bbb//../c/") becomes "../aaa/c/".
*/
FXString FXAPI simplify(const FXString& file);

/// Return absolute path from current directory and file name
FXString FXAPI absolute(const FXString& file);

/// Return absolute path from base directory and file name
FXString FXAPI absolute(const FXString& base,const FXString& file);

/// Return relative path of file to the current directory
FXString FXAPI relative(const FXString& file);

/// Return relative path of file to given base directory
FXString FXAPI relative(const FXString& base,const FXString& file);

/**
* Return root of absolute path; on Unix, this is just "/". On
* Windows, this is "\\" or "C:\".  Returns the empty string
* if the given path is not absolute.
*/
FXString FXAPI root(const FXString& file);

/// Enquote filename to make safe for shell
FXString FXAPI enquote(const FXString& file,FXbool forcequotes=FALSE);

/// Dequote filename to get original again
FXString FXAPI dequote(const FXString& file);

/**
* Generate unique filename of the form pathnameXXX.ext, where
* pathname.ext is the original input file, and XXX is a number,
* possibly empty, that makes the file unique.
*/
FXString FXAPI unique(const FXString& file);

/// Search path list for this file, return full path name for first occurrence
FXString FXAPI search(const FXString& pathlist,const FXString& file);

/// Return path to directory above input directory name
FXString FXAPI upLevel(const FXString& file);

/// Return true if file name is absolute
FXbool FXAPI isAbsolute(const FXString& file);

/// Return true if input directory is a top-level directory
FXbool FXAPI isTopDirectory(const FXString& file);

/// Return true if input path is a file name
FXbool FXAPI isFile(const FXString& file);

/// Return true if input path is a link
FXbool FXAPI isLink(const FXString& file);

/// Return true if input path is a directory
FXbool FXAPI isDirectory(const FXString& file);

/// Return true if input path is a file share
FXbool FXAPI isShare(const FXString& file);

/// Return true if file is readable
FXbool FXAPI isReadable(const FXString& file);

/// Return true if file is writable
FXbool FXAPI isWritable(const FXString& file);

/// Return true if file is executable
FXbool FXAPI isExecutable(const FXString& file);

/// Return true if owner has read-write-execute permissions
FXbool FXAPI isOwnerReadWriteExecute(const FXString& file);

/// Return true if owner has read permissions
FXbool FXAPI isOwnerReadable(const FXString& file);

/// Return true if owner has write permissions
FXbool FXAPI isOwnerWritable(const FXString& file);

/// Return true if owner has execute permissions
FXbool FXAPI isOwnerExecutable(const FXString& file);

/// Return true if group has read-write-execute permissions
FXbool FXAPI isGroupReadWriteExecute(const FXString& file);

/// Return true if group has read permissions
FXbool FXAPI isGroupReadable(const FXString& file);

/// Return true if group has write permissions
FXbool FXAPI isGroupWritable(const FXString& file);

/// Return true if group has execute permissions
FXbool FXAPI isGroupExecutable(const FXString& file);

/// Return true if others have read-write-execute permissions
FXbool FXAPI isOtherReadWriteExecute(const FXString& file);

/// Return true if others have read permissions
FXbool FXAPI isOtherReadable(const FXString& file);

/// Return true if others have write permissions
FXbool FXAPI isOtherWritable(const FXString& file);

/// Return true if others have execute permissions
FXbool FXAPI isOtherExecutable(const FXString& file);

/// Return true if the file sets the user id on execution
FXbool FXAPI isSetUid(const FXString& file);

/// Return true if the file sets the group id on execution
FXbool FXAPI isSetGid(const FXString& file);

/// Return true if the file has the sticky bit set
FXbool FXAPI isSetSticky(const FXString& file);

/// Return owner name from uid if available
FXString FXAPI owner(FXuint uid);

/// Return owner name of file if available
FXString FXAPI owner(const FXString& file);

/// Return group name from gid if available
FXString FXAPI group(FXuint gid);

/// Return group name of file if available
FXString FXAPI group(const FXString& file);

/// Return permissions string
FXString FXAPI permissions(FXuint mode);

/// Return file size in bytes
FXlong FXAPI size(const FXString& file);

/**
* Return last modified time for this file, on filesystems
* where this is supported.  This is the time when any data
* in the file was last modified.
*/
FXTime FXAPI modified(const FXString& file);

/**
* Return last accessed time for this file, on filesystems
* where this is supported.
*/
FXTime FXAPI accessed(const FXString& file);

/**
* Return created time for this file, on filesystems
* where this is supported.  This is also the time when
* ownership, permissions, links, and other meta-data may
* have changed.
*/
FXTime FXAPI created(const FXString& file);

/**
* Return touched time for this file, on filesystems
* where this is supported.  This is the time when anything
* at all, either contents or meta-data, about the file was
* changed.
*/
FXTime FXAPI touched(const FXString& file);

/// Match filenames using *, ?, [^a-z], and so on
FXbool FXAPI match(const FXString& pattern,const FXString& file,FXuint flags=(FILEMATCH_NOESCAPE|FILEMATCH_FILE_NAME));

/**
* List files in a given directory.
* Returns the number of files in the string-array list which matched the
* pattern or satisfied the flag conditions.
*/
FXint FXAPI listFiles(FXString*& filelist,const FXString& path,const FXString& pattern="*",FXuint flags=LIST_MATCH_ALL);

/// Return current time
FXTime FXAPI now();

/// Convert file time to date-string
FXString FXAPI time(FXTime filetime);

/**
* Convert file time to date-string as per strftime.
* Format characters supported by most systems are:
*
*  %a %A %b %B %c %d %H %I %j %m %M %p %S %U %w %W %x %X %y %Y %Z %%
*
* Some systems support additional conversions.
*/
FXString FXAPI time(const FXchar *format,FXTime filetime);

/// Return file info as reported by system stat() function
FXbool FXAPI info(const FXString& file,struct stat& inf);

/// Return file info as reported by system lstat() function
FXbool FXAPI linkinfo(const FXString& file,struct stat& inf);

/// Return true if file exists
FXbool FXAPI exists(const FXString& file);

/// Return true if files are identical
FXbool FXAPI identical(const FXString& file1,const FXString& file2);

/// Return the mode flags for this file
FXuint FXAPI mode(const FXString& file);

/// Change the mode flags for this file
FXbool FXAPI mode(const FXString& file,FXuint mode);

/// Create new directory
FXbool FXAPI createDirectory(const FXString& path,FXuint mode);

/// Create new (empty) file
FXbool FXAPI createFile(const FXString& file,FXuint mode);

/**
* Concatenate srcfile1 and srcfile2 to a dstfile.
* If overwrite is true, then the operation fails if dstfile already exists.
* srcfile1 and srcfile2 should not be the same as dstfile.
*/
FXbool FXAPI concatenate(const FXString& srcfile1,const FXString& srcfile2,const FXString& dstfile,FXbool overwrite=FALSE);

/// Remove file or directory, recursively.
FXbool FXAPI remove(const FXString& file);

/// Copy file or directory, recursively
FXbool FXAPI copy(const FXString& srcfile,const FXString& dstfile,FXbool overwrite=FALSE);

/// Rename or move file or directory
FXbool FXAPI move(const FXString& srcfile,const FXString& dstfile,FXbool overwrite=FALSE);

/// Link file
FXbool FXAPI link(const FXString& srcfile,const FXString& dstfile,FXbool overwrite=FALSE);

/// Symbolic link file
FXbool FXAPI symlink(const FXString& srcfile,const FXString& dstfile,FXbool overwrite=FALSE);

/// Read symbolic link
FXString FXAPI symlink(const FXString& file);

}

}

#endif
