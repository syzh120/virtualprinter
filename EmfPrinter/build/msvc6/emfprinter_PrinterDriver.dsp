# Microsoft Developer Studio Project File - Name="emfprinter_PrinterDriver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PrinterDriver - Win32 Default
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "emfprinter_PrinterDriver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "emfprinter_PrinterDriver.mak" CFG="PrinterDriver - Win32 Default"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PrinterDriver - Win32 Default" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PrinterDriver - Win32 Default"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release\PrinterDriver"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\PrinterDriver"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MD /W1 /GR /EHsc /I "$(WXPBASE)\inc\api" /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /c
# ADD CPP /nologo /FD /MD /W1 /GR /EHsc /I "$(WXPBASE)\inc\api" /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /c
# ADD BASE MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i $(WXPBASE)\inc\api
# ADD RSC /l 0x409 /i $(WXPBASE)\inc\api
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /dll /machine:i386 /out:"Release\emfprinter_drv.dll" /libpath:"$(WXPBASE)\lib\wxp\i386"
# ADD LINK32 /nologo /dll /machine:i386 /out:"Release\emfprinter_drv.dll" /libpath:"$(WXPBASE)\lib\wxp\i386"

!ENDIF

# Begin Target

# Name "PrinterDriver - Win32 Default"
# Begin Group "sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\driver\driver.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\driver\waitdlg.cpp
# End Source File
# End Group
# Begin Group "headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\driver\driver.h
# End Source File
# Begin Source File

SOURCE=..\..\src\driver\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\src\driver\waitdlg.h
# End Source File
# End Group
# Begin Group "resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\driver\driver.rc
# End Source File
# End Group
# Begin Group "misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\driver\driver.def
# End Source File
# Begin Source File

SOURCE=..\..\src\driver\sources
# End Source File
# End Group
# End Target
# End Project

