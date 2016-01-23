/*
 * installerdlg.cpp
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
#include "resource.h"
#include "installer.h"
#include "installerutils.h"
#include "utils.h"
    

// -----------------------------------------------------------------------------
// IMPLEMENTATION - InstallerDialogWindow
// -----------------------------------------------------------------------------

InstallerDialogWindow::InstallerDialogWindow(HINSTANCE hinst,
                                             InstallerMode mode,
                                             LPCTSTR sourceDir, 
                                             LPCTSTR printerName) 
    : DialogWindow(hinst, IDD_PROGRESS_DIALOG)
{
    m_mode = mode;

    if (!printerName)
        printerName = emfPRINTER_NAME;
    if (!sourceDir)
        sourceDir = TEXT(".");

    // setup the installer thread
    m_thread.SetSourceDir(sourceDir);
    m_thread.SetPrinterName(printerName);
    m_thread.SetInstallerDialogWindow(this);
    m_thread.SetMode(m_mode);
}

InstallerDialogWindow::~InstallerDialogWindow()
{
    // automatic wait for thread to terminate
}

void InstallerDialogWindow::ErrorMessage(LPCTSTR msg)
{
    ::ErrorMessage(GetDlgHwnd(), TEXT("Driver installer"), msg);
}

void InstallerDialogWindow::UpdateStatus(LPCTSTR lpstrFormat, ...)
{ 
    TCHAR buf[256];
    va_list arglist;

    va_start( arglist, lpstrFormat );
    int iLen = StringCchVPrintf( buf, 255, lpstrFormat, arglist );
    va_end( arglist );

    SetDlgItemText(IDC_STATIC_TEXT, buf);
}

BOOL InstallerDialogWindow::InitMsg(WPARAM wParam, LPARAM lParam)
{
    DialogWindow::InitMsg(wParam, lParam);
    CenterWindowInScreen();

    // set the title of this dialog so that in the taskbar
    // this window won't appear with an empty title
    SetWindowText(GetDlgHwnd(), TEXT("EmfPrinter installer"));

    // FIXME: TODO - support the cancel button
    EnableWindow(GetDlgItem(IDC_CANCEL_BUTTON), FALSE);

    // launch worker thread for doing install
    if (!m_thread.StartNewDoWorkThread())
    {
        ErrorMessage(TEXT("Cannot start installer thread!"));
        return FALSE;
    }

    return TRUE;
}

BOOL InstallerDialogWindow::CommandMsg(WPARAM wParam, LPARAM lParam)
{
    WORD wID = LOWORD(wParam);
    switch(wID)
    {
    case IDC_CANCEL_BUTTON:
        //this->DestroyWindow();    -- FIXME TODO
        break;

    default:
        break;
    }

    return TRUE;
}

BOOL InstallerDialogWindow::NotificationMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) {
        case WM_CLOSE:
            this->DestroyWindow();
            break;
        case WM_NCDESTROY:
            PostQuitMessage(0);
            break;
        default:
            break;
    }

    return FALSE;
}

BOOL InstallerDialogWindow::UserMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg != WM_THREAD_COMPLETED)
        return TRUE;

    // our thread completed...
    this->DestroyWindow();

    return FALSE;
}

