/*
 * installer.cpp
 *
 * Copyright (C) 2006-2007 Michael H. Overlin, Francesco Montorsi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Author contact info:
 * frm@users.sourceforge.net (Francesco Montorsi)
 * poster_printer@yahoo.com (Michael H. Overlin)
 */


// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include "stdheaderwrapper.h"
#include "installerdlg.h"
#include "debug.h"
#include "utils.h"

//#include <shlobj.h>
#include <commctrl.h>
#include <shellapi.h>


// -----------------------------------------------------------------------------
// WINMAIN
// -----------------------------------------------------------------------------

int APIENTRY wWinMain(
                      HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow
                      )
{
    DEBUGMESSAGE(("emfprinter_inst.exe started built %s with '%ws' cmdline", 
                 __TIME__, lpCmdLine));

    // check win version
    DWORD dwVersion = GetVersion();
    DWORD dwWindowsMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));

    // this driver is for Win NT 4.0, Win2000, WinXP, Vista(TOTEST!)
    BOOL ok = FALSE;
    if (dwWindowsMajorVersion == 4 ||    /* Win NT 4 */
        dwWindowsMajorVersion == 5 ||    /* Win2000 or WinXP */ 
        dwWindowsMajorVersion == 6)      /* WinVista */
        ok = TRUE;

    if ( !ok )
    {
        ErrorMessage(NULL, TEXT("emfprinter_inst.exe [Driver installer]"), 
                     TEXT("Sorry, this driver is for Windows NT 4.0, Windows 2000, Windows XP and Windows Vista only.\nCannot proceed."));
        return -1;
    }

    // init COM and other std libs
    //CoInitialize(0);
    InitCommonControls();

    // we need two info from command line: the printer name and DLL source dir
    int nArgs = 0;
    LPWSTR *args = CommandLineToArgvW(GetCommandLine() /* better than lpCmdLine */,
                                      &nArgs);

    // parse them (first is always this exe's name)
    InstallerMode mode;
    LPTSTR sourceDir = NULL;
    LPTSTR printerName = NULL;

    if (nArgs < 2)
    {
        ErrorMessage(NULL, TEXT("emfprinter_inst.exe [Driver installer]"), 
            TEXT("Invalid argument count; usage:\nemfprinter_inst.exe mode [sourceDir] [printerName]"));
        GlobalFree(args);
        return -1;
    }

    // detect the mode
    if (lstrcmpi(args[1], TEXT("install")) == 0)
        mode = IM_INSTALL;
    else if (lstrcmpi(args[1], TEXT("check")) == 0)
        mode = IM_CHECK;
    else if (lstrcmpi(args[1], TEXT("uninstall")) == 0)
        mode = IM_UNINSTALL;
    else
    {
        ErrorMessage(NULL, TEXT("emfprinter_inst.exe [Driver installer]"), 
            TEXT("Invalid mode argument; it should be 'install', 'check' or 'uninstall'"));
        GlobalFree(args);
        return -1;
    }
    
    // do we have other args to parse?
    if (mode == IM_INSTALL)
    {
        if (nArgs >= 3) 
            sourceDir = args[2];

        if (nArgs >= 4) 
            printerName = args[3];
    }

    // create the progress window
    InstallerDialogWindow *pidw = 
        new InstallerDialogWindow(hInstance, mode, sourceDir, printerName);
    pidw->DoModalDialog(NULL);
    pidw->ShowWindow(SW_NORMAL);
    
    // message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //CoUninitialize();

    GlobalFree(args);

    int exitCode = pidw->Failed() ? -1 : 0;

    DEBUGMESSAGE(("emfprinter_inst.exe exiting with code %d", exitCode));

    // retcode = 0 means everything was ok; NSIS installation can proceed!
    return exitCode;
}

