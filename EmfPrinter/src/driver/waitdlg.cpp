/*
 * waitdlg.cpp
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
#include "waitdlg.h"
#include "resource.h"

#include <commctrl.h>

VOID ProgressBarSetIndeterminateMode(HWND hWnd);
VOID ProgressBarSetDeterminateMode(HWND hWnd);
VOID ProgressBarPulse(HWND hWnd);

#ifndef PBS_MARQUEE
    #define PBS_MARQUEE             0x08
#endif
#ifndef PBM_SETMARQUEE
    #define PBM_SETMARQUEE          (WM_USER+10)
#endif


// -----------------------------------------------------------------------------
// IMPLEMENTATION - WaitDialogWindow
// -----------------------------------------------------------------------------

WaitDialogWindow::WaitDialogWindow(HINSTANCE hinst) 
    : DialogWindow(hinst, IDD_WAITDLG)
{
    m_hProgressBar = NULL;

    DoCreateDialog(GetForegroundWindow());
    ShowWindow(SW_NORMAL);
}

WaitDialogWindow::~WaitDialogWindow()
{
}

BOOL WaitDialogWindow::InitMsg(WPARAM wParam, LPARAM lParam)
{
    DialogWindow::InitMsg(wParam, lParam);
    CenterWindowInScreen();

    // setup our progress bar
    m_hProgressBar = GetDlgItem(IDC_PROGRESS_BAR);

    // we cannot know how much time the Save() operation will require
    // (specially in case ImageMagick is called!)
    ProgressBarSetIndeterminateMode(m_hProgressBar);

    // force redrawing
    UpdateWindow(GetDlgHwnd());

    return TRUE;
}

BOOL WaitDialogWindow::CommandMsg(WPARAM wParam, LPARAM lParam)
{
    WORD wID = LOWORD(wParam);
    switch(wID)
    {
    case IDCANCEL:
        //this->DestroyWindow();    -- FIXME TODO
        break;

    default:
        break;
    }

    return TRUE;
}

BOOL WaitDialogWindow::NotificationMsg(UINT msg, WPARAM wParam, LPARAM lParam)
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

BOOL WaitDialogWindow::UserMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // our thread completed...
    this->DestroyWindow();

    return FALSE;
}

VOID WaitDialogWindow::Pulse()
{
    ProgressBarPulse(m_hProgressBar);

    MSG msg;
    while (PeekMessage( &msg, GetDlgHwnd(), 0, 0, PM_NOREMOVE) != 0)
    {
        if (GetMessage( &msg, GetDlgHwnd(), 0, 0 ) != -1)
        { 
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }

    UpdateWindow(GetDlgHwnd());
}

VOID WaitDialogWindow::UpdateStatus(LPCTSTR lpstrFormat, ...)
{ 
    TCHAR buf[256];
    va_list arglist;

    va_start( arglist, lpstrFormat );
    int iLen = StringCchVPrintf( buf, 255, lpstrFormat, arglist );
    va_end( arglist );

    SetDlgItemText(IDC_WAIT_MSG_TEXT, buf);
}

VOID WaitDialogWindow::UpdateStatus(DWORD page, LPCTSTR printerName)
{
    UpdateStatus(TEXT("Please wait while printing on %s the page #%d..."), 
                 printerName, page);
}


// -----------------------------------------------------------------------------
// PROGRESS BAR UTILITIES
// -----------------------------------------------------------------------------

VOID ProgressBarSetIndeterminateMode(HWND hWnd)
{
    // add the PBS_MARQUEE style to the progress bar
    LONG style = ::GetWindowLong(hWnd, GWL_STYLE);
    if ((style & PBS_MARQUEE) == 0)
        ::SetWindowLong(hWnd, GWL_STYLE, style|PBS_MARQUEE);

    // now the control can only run in indeterminate mode
}

VOID ProgressBarSetDeterminateMode(HWND hWnd)
{
    // remove the PBS_MARQUEE style to the progress bar
    LONG style = ::GetWindowLong(hWnd, GWL_STYLE);
    if ((style & PBS_MARQUEE) != 0)
        ::SetWindowLong(hWnd, GWL_STYLE, style & ~PBS_MARQUEE);

    // now the control can only run in determinate mode
}

VOID ProgressBarPulse(HWND hWnd)
{
    // NOTE: when in indeterminate mode, the PBM_SETPOS message will just make
    //       the bar's blocks move a bit and the WPARAM value is just ignored
    //       so that we can safely use zero
    SendMessage(hWnd, (UINT) PBM_SETPOS, (WPARAM)0, (LPARAM)0);
}
