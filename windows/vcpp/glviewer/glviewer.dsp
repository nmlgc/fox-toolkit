# Microsoft Developer Studio Project File - Name="glviewer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=glviewer - Win32 MesaDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "glviewer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "glviewer.mak" CFG="glviewer - Win32 MesaDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "glviewer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "glviewer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "glviewer - Win32 MesaRelease" (based on "Win32 (x86) Application")
!MESSAGE "glviewer - Win32 MesaDebug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "glviewer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "HAVE_OPENGL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winspool.lib comdlg32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib comctl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib mpr.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "glviewer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /MT /W3 /Gm /GX /ZI /Od /I "..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_OPENGL" /FD /c
# SUBTRACT CPP /nologo /X /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winspool.lib opengl32.lib glu32.lib comctl32.lib wsock32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib mpr.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "glviewer - Win32 MesaRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "glviewer___Win32_MesaRelease"
# PROP BASE Intermediate_Dir "glviewer___Win32_MesaRelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "MesaRelease"
# PROP Intermediate_Dir "MesaRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "FX_NATIVE_WIN32" /D "HAVE_OPENGL" /D "FOXDLL" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "D:\src\Mesa-3.0\include" /I "..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "HAVE_OPENGL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib comctl32.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /machine:I386
# ADD LINK32 winspool.lib comdlg32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib wsock32.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /machine:I386 /libpath:"D:\src\Mesa-3.0\lib"

!ELSEIF  "$(CFG)" == "glviewer - Win32 MesaDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "glviewer___Win32_MesaDebug"
# PROP BASE Intermediate_Dir "glviewer___Win32_MesaDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "MesaDebug"
# PROP Intermediate_Dir "MesaDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "FX_NATIVE_WIN32" /D "HAVE_OPENGL" /D "FOXDLL" /YX /FD /c
# SUBTRACT BASE CPP /X
# ADD CPP /nologo /MT /W3 /Gm /GX /ZI /Od /I "D:\src\Mesa-3.0\include" /I "..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_OPENGL" /FD /c
# SUBTRACT CPP /X /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib comctl32.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winspool.lib comdlg32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib wsock32.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"D:\src\Mesa-3.0\lib"

!ENDIF 

# Begin Target

# Name "glviewer - Win32 Release"
# Name "glviewer - Win32 Debug"
# Name "glviewer - Win32 MesaRelease"
# Name "glviewer - Win32 MesaDebug"
# Begin Source File

SOURCE=..\..\..\tests\glviewer.cpp
# End Source File
# End Target
# End Project
