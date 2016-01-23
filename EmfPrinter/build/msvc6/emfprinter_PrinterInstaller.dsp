# Microsoft Developer Studio Project File - Name="emfprinter_PrinterInstaller" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PrinterInstaller - Win32 Default
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "emfprinter_PrinterInstaller.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "emfprinter_PrinterInstaller.mak" CFG="PrinterInstaller - Win32 Default"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PrinterInstaller - Win32 Default" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PrinterInstaller - Win32 Default"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug\PrinterInstaller"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\PrinterInstaller"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MDd /W1 /GR /EHsc /I "$(WXPBASE)\inc\api" /I "$(WXPBASE)\inc\crt" /Zi /FdDebug\emfprinter_inst.pdb /D "WIN32" /D "BUILDING_INSTALLER" /D "_UNICODE" /D "UNICODE" /D "_WINDOWS" /D "_DEBUG" /c
# ADD CPP /nologo /FD /MDd /W1 /GR /EHsc /I "$(WXPBASE)\inc\api" /I "$(WXPBASE)\inc\crt" /Zi /FdDebug\emfprinter_inst.pdb /D "WIN32" /D "BUILDING_INSTALLER" /D "_UNICODE" /D "UNICODE" /D "_WINDOWS" /D "_DEBUG" /c
# ADD BASE MTL /nologo /D "WIN32" /D "BUILDING_INSTALLER" /D "_UNICODE" /D "UNICODE" /D "_WINDOWS" /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "BUILDING_INSTALLER" /D "_UNICODE" /D "UNICODE" /D "_WINDOWS" /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXPBASE)\inc\api" /i "$(WXPBASE)\inc\crt" /d "BUILDING_INSTALLER" /d "_UNICODE" /d "UNICODE" /d "_WINDOWS" /d _DEBUG
# ADD RSC /l 0x409 /i "$(WXPBASE)\inc\api" /i "$(WXPBASE)\inc\crt" /d "BUILDING_INSTALLER" /d "_UNICODE" /d "UNICODE" /d "_WINDOWS" /d _DEBUG
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib kernel32.lib Gdi32.lib winspool.lib shell32.lib win32k.lib comctl32.lib Winmm.lib ole32.lib Uuid.lib Lz32.lib Advapi32.lib /nologo /machine:i386 /out:"Debug\emfprinter_inst.exe" /libpath:"$(WXPBASE)\lib\crt\i386" /libpath:"$(WXPBASE)\lib\wxp\i386" /subsystem:windows /debug /pdb:"Debug\emfprinter_inst.pdb"
# ADD LINK32 user32.lib kernel32.lib Gdi32.lib winspool.lib shell32.lib win32k.lib comctl32.lib Winmm.lib ole32.lib Uuid.lib Lz32.lib Advapi32.lib /nologo /machine:i386 /out:"Debug\emfprinter_inst.exe" /libpath:"$(WXPBASE)\lib\crt\i386" /libpath:"$(WXPBASE)\lib\wxp\i386" /subsystem:windows /debug /pdb:"Debug\emfprinter_inst.pdb"

!ENDIF

# Begin Target

# Name "PrinterInstaller - Win32 Default"
# Begin Group "sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\installer\addport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\dlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\installer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\installerdlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\installerutils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\printerutils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\utils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\winmain.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\workerthread.cpp
# End Source File
# End Group
# Begin Group "headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\installer\addport.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\dlg.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\globals.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\installer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\installerdlg.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\installerutils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\printerutils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\stdheaderwrapper.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\thread.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\types.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\undocumented.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\utils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\installer\workerthread.h
# End Source File
# End Group
# Begin Group "resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\installer\resource.rc
# End Source File
# End Group
# Begin Group "misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\installer\sources
# End Source File
# End Group
# End Target
# End Project

