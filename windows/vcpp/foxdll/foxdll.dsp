# Microsoft Developer Studio Project File - Name="foxdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=foxdll - Win32 MesaDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "foxdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "foxdll.mak" CFG="foxdll - Win32 MesaDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "foxdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "foxdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "foxdll - Win32 MesaDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "foxdll - Win32 MesaRelease" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "foxdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\lib"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FOXDLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FOXDLL" /D "FOXDLL_EXPORTS" /D "HAVE_OPENGL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 msvcrtd.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib opengl32.lib glu32.lib comctl32.lib winspool.lib wsock32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "foxdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\lib"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FOXDLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FOXDLL" /D "FOXDLL_EXPORTS" /D "HAVE_OPENGL" /FD /GZ /c
# SUBTRACT CPP /nologo /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msvcrtd.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib opengl32.lib glu32.lib comctl32.lib winspool.lib wsock32.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"MSVCRTD.LIB" /nodefaultlib:"LIBCMTD.LIB" /out:"..\..\..\lib\foxdlld.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "foxdll - Win32 MesaDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "foxdll___Win32_MesaDebug"
# PROP BASE Intermediate_Dir "foxdll___Win32_MesaDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\lib"
# PROP Intermediate_Dir "MesaDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FOXDLL" /D "FOXDLL_EXPORTS" /D "FX_NATIVE_WIN32" /D "HAVE_OPENGL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "D:\src\Mesa-3.0\include" /I "..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FOXDLL" /D "FOXDLL_EXPORTS" /D "HAVE_OPENGL" /D "HAVE_MESA" /D "__WIN32__" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib comctl32.lib /nologo /dll /debug /machine:I386 /out:"..\..\..\lib/foxdlld.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OpenGL32.lib GLU32.lib comctl32.lib winspool.lib /nologo /dll /debug /machine:I386 /out:"..\..\..\lib\foxmesadlld.dll" /pdbtype:sept /libpath:"D:\src\Mesa-3.0\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "foxdll - Win32 MesaRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "foxdll___Win32_MesaRelease"
# PROP BASE Intermediate_Dir "foxdll___Win32_MesaRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\lib"
# PROP Intermediate_Dir "MesaRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FOXDLL" /D "FOXDLL_EXPORTS" /D "FX_NATIVE_WIN32" /D "HAVE_OPENGL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "D:\src\Mesa-3.0\include" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FOXDLL" /D "FOXDLL_EXPORTS" /D "HAVE_OPENGL" /D "HAVE_MESA" /D "__WIN32__" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib comctl32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib OpenGL32.lib GLU32.lib comctl32.lib winspool.lib /nologo /dll /machine:I386 /out:"..\..\..\lib\foxmesadll.dll" /libpath:"D:\src\Mesa-3.0\lib"

!ENDIF 

# Begin Target

# Name "foxdll - Win32 Release"
# Name "foxdll - Win32 Debug"
# Name "foxdll - Win32 MesaDebug"
# Name "foxdll - Win32 MesaRelease"
# Begin Source File

SOURCE=..\..\..\include\fx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\fx3d.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FX4Splitter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FX4Splitter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXAccelTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXAccelTable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXApp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXApp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXArrowButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXArrowButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXBitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXBitmap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXBMPIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXBMPIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXBMPImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXBMPImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxbmpio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXCanvas.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXCanvas.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXCharset.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXCharset.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXCheckButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXCheckButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXColorBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXColorBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXColorDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXColorDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXColorNames.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXColorSelector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXColorSelector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXColorWell.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXColorWell.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXColorWheel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXColorWheel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXComboBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXComboBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXComposite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXComposite.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXCURCursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXCURCursor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxcurio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXCursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXCursor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDataTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDataTarget.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDC.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDC.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDCPrint.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDCPrint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDCWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDCWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDebugTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDebugTarget.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\fxdefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDelegator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDelegator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDHMat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDHMat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDHVec.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDHVec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDial.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDial.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDialogBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDialogBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDict.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDict.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDirBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDirBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDirDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDirDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDirList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDirList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDirSelector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDirSelector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDocument.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDocument.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDQuat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDQuat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDragCorner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDragCorner.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDrawable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDrawable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDriveBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDriveBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXDVec.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXDVec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXElement.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXException.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXException.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXFile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXFileDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXFileDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXFileDict.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXFileDict.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXFileList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXFileList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxfilematch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXFileSelector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXFileSelector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXFont.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXFont.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXFontDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXFontDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXFontSelector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXFontSelector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXFrame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGIFCursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGIFCursor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGIFIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGIFIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGIFImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGIFImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxgifio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLCanvas.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLCanvas.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLCone.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLCone.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLContext.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLContext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLCube.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLCube.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLCylinder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLCylinder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLObject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLObject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLShape.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLShape.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLSphere.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLSphere.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLTriangleMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLTriangleMesh.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLViewer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLViewer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGLVisual.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGLVisual.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXGroupBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXGroupBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXHMat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXHMat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXHorizontalFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXHorizontalFrame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXHVec.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXHVec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXICOIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXICOIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXICOImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXICOImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxicoio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXIconList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXIconList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXId.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXId.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXImageView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXImageView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXInputDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXInputDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxjpegio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXJPGIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXJPGIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXJPGImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXJPGImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\fxkeys.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXLabel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXLabel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXListBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMainWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMainWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMatrix.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMDIButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMDIButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMDIChild.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMDIChild.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMDIClient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMDIClient.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMenubar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMenubar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMenuButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMenuButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMenuCaption.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMenuCaption.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMenuCascade.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMenuCascade.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMenuCommand.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMenuCommand.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMenuPane.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMenuPane.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMenuSeparator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMenuSeparator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMenuTitle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMenuTitle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXMessageBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXMessageBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXObject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXObject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXObjectList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXObjectList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXOptionMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXOptionMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXPacker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXPacker.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXPCXIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXPCXIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXPCXImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXPCXImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxpcxio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXPicker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXPicker.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXPNGIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXPNGIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXPNGImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXPNGImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxpngio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXPoint.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXPoint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXPopup.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXPopup.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXPrintDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXPrintDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxpriv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxpriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXProgressBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXProgressBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXProgressDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXProgressDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxquantize.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxquantize.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXQuat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXQuat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXRadioButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXRadioButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXRange.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXRange.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXRecentFiles.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXRecentFiles.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXRectangle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXRectangle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXRegion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXRegion.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXRegistry.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXRegistry.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXReplaceDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXReplaceDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXRex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXRex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXRGBIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXRGBIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXRGBImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXRGBImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxrgbio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXRootWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXRootWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXScrollArea.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXScrollArea.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXScrollbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXScrollbar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXScrollWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXScrollWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXSearchDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXSearchDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXSeparator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXSeparator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXSettings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXSettings.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXShell.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXShell.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXShutter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXShutter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXSize.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXSize.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXSlider.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXSlider.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXSpinner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXSpinner.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXSplitter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXSplitter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXStatusbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXStatusbar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXStatusline.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXStatusline.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXString.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXString.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXStringDict.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXStringDict.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXSwitcher.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXSwitcher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTabBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTabBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTabBook.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTabBook.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTabItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTabItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTableItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxtargaio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXText.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXText.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTextField.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTextField.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTGAIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTGAIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTGAImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTGAImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTIFIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTIFIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTIFImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTIFImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxtifio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXToggleButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXToggleButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXToolbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXToolbar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXToolbarGrip.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXToolbarGrip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXToolbarShell.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXToolbarShell.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXToolbarTab.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXToolbarTab.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTooltip.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTooltip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTopWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTopWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTreeList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTreeList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXTreeListBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXTreeListBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXUndoList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXUndoList.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXURL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXURL.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxutils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXVec.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXVec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\fxver.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXVerticalFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXVerticalFrame.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXVisual.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXVisual.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxwinkbd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXXPMIcon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXXPMIcon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\FXXPMImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\FXXPMImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\fxxpmio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\jitter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\version.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\src\vsscanf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\include\xincs.h
# End Source File
# End Target
# End Project
