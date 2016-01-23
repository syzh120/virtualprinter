# Microsoft Developer Studio Project File - Name="emfprinter_PrinterCommon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=PrinterCommon - Win32 Default
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "emfprinter_PrinterCommon.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "emfprinter_PrinterCommon.mak" CFG="PrinterCommon - Win32 Default"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PrinterCommon - Win32 Default" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PrinterCommon - Win32 Default"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release\PrinterCommon"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\PrinterCommon"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MD /W1 /GR /EHsc /I "$(WXPBASE)\inc\api" /D "WIN32" /D "_LIB" /c
# ADD CPP /nologo /FD /MD /W1 /GR /EHsc /I "$(WXPBASE)\inc\api" /D "WIN32" /D "_LIB" /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Release\common.lib"
# ADD LIB32 /nologo /out:"Release\common.lib"

!ENDIF

# Begin Target

# Name "PrinterCommon - Win32 Default"
# Begin Group "sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\common\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\devmode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\devmodeV351.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\devmodeV400.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\devmodeV401.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\dlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\extdevmode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\printerutils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\spoolerdata.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\common\utils.cpp
# End Source File
# End Group
# Begin Group "headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\common\buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\devmode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\devmodeV351.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\devmodeV400.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\devmodeV401.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\dlg.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\extdevmode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\globals.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\printerutils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\spoolerdata.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\stdheaderwrapper.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\types.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\undocumented.h
# End Source File
# Begin Source File

SOURCE=..\..\src\common\utils.h
# End Source File
# End Group
# Begin Group "misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\common\sources
# End Source File
# End Group
# End Target
# End Project

