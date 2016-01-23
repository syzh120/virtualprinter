/*
 * thread.cpp
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

#include "thread.h"
#include "utils.h"
#include "installerdlg.h"
#include "installer.h"
#include "installerutils.h"
#include "resource.h"
#include <process.h>


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

InstallWorkerThread::~InstallWorkerThread()
{
    strFree(m_strPrinterName);
    strFree(m_strSourceDir);

	this->PeekMessagesUntilTerminate();
}

void InstallWorkerThread::DoWork()
{
	//::CoInitialize(0);
    m_bFailed = FALSE;

    switch (m_mode)
    {
    case IM_INSTALL:
        m_bFailed = !Install();
        break;
    case IM_CHECK:
        m_bFailed = !Check();
        break;
    case IM_UNINSTALL:
        m_bFailed = !Uninstall();
        break;
    }

    PostMessage(m_pWnd->GetDlgHwnd(), WM_THREAD_COMPLETED, 0, 0);
//	::CoUninitialize();
}

BOOL InstallWorkerThread::Install()
{
    // some preliminar checks before the real installation
    // ---------------------------------------------------

    m_pWnd->UpdateStatus(
        TEXT("Checking if it's possible to install the printer and the drivers"));

    BOOL bInstalled;
    if (!CheckPrinterInstalled(bInstalled, m_strPrinterName))
    {
        m_pWnd->ErrorMessage(
            TEXT("Cannot check if a printer with the same name already exist!"));
        return FALSE;
    }

    if (bInstalled)
    {
        m_pWnd->ErrorMessage(TEXT("Printer name is already in use!"));
        return FALSE;
    }

    uint kDriver;
    PDRIVER_INFO_3 pinfoDrivers = NULL;
    if (!CheckPrinterDriverInstalled(pinfoDrivers, kDriver, emfDRIVER_NAME))
    {
        m_pWnd->ErrorMessage(
            TEXT("Cannot check if a driver with the same name already exist!"));
        return FALSE;
    }

    if (pinfoDrivers != NULL)
    {
        free(pinfoDrivers); // we're not really intetested to them

        BOOL bYes = m_pWnd->YesNoMessage(
                TEXT("'%ws' drivers are already installed on your system.\n")
                TEXT("Press \"No\" to keep your current drivers.")
                TEXT("\n\nTo upgrade, close all applications using '%ws'")
                TEXT(" drivers and components, and then press \"Yes\"."),
                emfDRIVER_NAME, emfPRINTER_NAME);
        if (!bYes)
            return FALSE;
    }

    // we're ready to start the real installation!
    // -------------------------------------------

    m_pWnd->UpdateStatus(TEXT("Installing EMF virtual printer as '%ws'..."),
                         m_strPrinterName);

    if (!DoAllUninstall() ||
        !DoAllInstall(m_strPrinterName, m_strSourceDir))
    {
        LPTSTR pDesc = GetErrorDesc(g_lastError);

        TCHAR buf[1024];
        StringCchPrintf( buf, 1024, 
            TEXT("%ws\nInstallation aborted.\n\nError code: %d\nError description: %ws"),
            g_strInstallerError, g_lastError, pDesc);
        
        // show the message to the user
        m_pWnd->ErrorMessage(buf);

        //strFree(g_strInstallerError);
        //LocalFree(pDesc);
        return FALSE;
    }

    return TRUE;
}


BOOL InstallWorkerThread::Uninstall()
{
    m_pWnd->UpdateStatus(TEXT("Uninstalling EMF virtual printer and drivers..."));

    return DoAllUninstall();
}


BOOL InstallWorkerThread::Check()
{
    m_pWnd->UpdateStatus(TEXT("Checking for EMF virtual printer drivers..."));

    uint kDriver;
    PDRIVER_INFO_3 pinfoDrivers = NULL;
    if (!CheckPrinterDriverInstalled(pinfoDrivers, kDriver, emfDRIVER_NAME))
    {
        m_pWnd->ErrorMessage(
            TEXT("Cannot check if a driver with the same name already exist!"));
        return FALSE;
    }

    if (pinfoDrivers == NULL)
    {
        m_pWnd->OkMessage(TEXT("'%ws' drivers are NOT correctly installed on your system."),
                          emfDRIVER_NAME);

        return FALSE;       // drivers not installed!
    }

    free(pinfoDrivers); // we're not really interested to them

    m_pWnd->OkMessage(TEXT("'%ws' drivers are correctly installed on your system."),
                      emfDRIVER_NAME);

    // drivers are installed!
    // TODO: check there is at least a printer using them!
    return TRUE;
}
