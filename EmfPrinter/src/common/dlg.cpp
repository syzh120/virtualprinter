/*
 * dlg.cpp
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
#include "dlg.h"

#define DIALOGWINDOW_PROPERTY        TEXT("hwndDialogWindow")


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

DialogWindow::DialogWindow(IN HINSTANCE hInst, IN DWORD templateId)
{
	m_hInst = hInst;
    m_wTemplateID = templateId;
	m_hDlg = NULL;
	m_bAutoDelete = TRUE;
}

DialogWindow::~DialogWindow()
{
    if (m_hDlg)
        DestroyWindow();
}

INT_PTR DialogWindow::DoModalDialog(HWND hwndParent)
{
	INT_PTR retValue = 
        ::DialogBoxParamW(
		                  m_hInst,
		                  MAKEINTRESOURCE(m_wTemplateID),
		                  hwndParent,
		                  (DLGPROC) DialogWindow::StaticDlgProc,
		                  (LPARAM) this
		                 );
	
	return retValue;
}

BOOL DialogWindow::DoModelessDialog(HWND hwndParent)
{
    m_hDlg = CreateDialog(GetInstance(), MAKEINTRESOURCE(m_wTemplateID),
                          hwndParent, (DLGPROC)DialogWindow::StaticDlgProc);

    if (!m_hDlg)
        return FALSE;
    
    return TRUE;
}

BOOL DialogWindow::DoCreateDialog(HWND hwndParent)
{
	m_hDlg = ::CreateDialogParam(
		                        m_hInst,
		                        MAKEINTRESOURCE(m_wTemplateID),
		                        hwndParent,
		                        (DLGPROC) DialogWindow::StaticDlgProc,
		                        (LPARAM) this
		                        );

	return m_hDlg != NULL;
}

void DialogWindow::CenterWindowInScreen()
{
	HWND hwndParent = ::GetParent(m_hDlg);
	RECT rect, r;

    ::SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID) &rect, 0);
	::GetWindowRect(m_hDlg, &r);
	
    r.left = ((rect.right - rect.left) - (r.right - r.left))/2;
    r.top = ((rect.bottom - rect.top) - (r.bottom - r.top))/2;

	::SetWindowPos(m_hDlg, NULL, r.left, r.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
/*
void DialogWindow::SetWindowStyleAndRedraw(DWORD dwStyle)
{
	::SetWindowLong(m_hDlg, GWL_STYLE, dwStyle);
	::SetWindowPos(m_hDlg, NULL, 0,0,0,0, 
		SWP_FRAMECHANGED |  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );
}*/

void DialogWindow::DestroyWindow()
{
    ::DestroyWindow(m_hDlg);
    m_hDlg = NULL;
}

void DialogWindow::ShowWindow(int nCmdShow)
{
    ::ShowWindow(m_hDlg, nCmdShow);
}

void DialogWindow::SetDlgItemText(int nIDDlgItem, LPCTSTR lptstr)
{ 
    ::SetDlgItemText(m_hDlg, nIDDlgItem, lptstr);
}

void DialogWindow::SetDlgItemFmtText(int nIDDlgItem, LPCTSTR lpstrFormat, ...)
{ 
    TCHAR buf[256];
    va_list arglist;

    va_start( arglist, lpstrFormat );
    int iLen = StringCchVPrintf( buf, 255, lpstrFormat, arglist );
    va_end( arglist );

    ::SetDlgItemText(m_hDlg, nIDDlgItem, buf);
}

BOOL DialogWindow::YesNoMessage(LPCTSTR lpstrFormat, ...)
{
    TCHAR buf[1024];
    va_list arglist;

    va_start( arglist, lpstrFormat );
    int iLen = StringCchVPrintf( buf, 1024, lpstrFormat, arglist );
    va_end( arglist );

    return MessageBox(GetDlgHwnd(), buf, TEXT("Confirm"), 
                      MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES; 
}

VOID DialogWindow::OkMessage(LPCTSTR lpstrFormat, ...)
{
    TCHAR buf[1024];
    va_list arglist;

    va_start( arglist, lpstrFormat );
    int iLen = StringCchVPrintf( buf, 1024, lpstrFormat, arglist );
    va_end( arglist );

    MessageBox(GetDlgHwnd(), buf, TEXT("Confirm"), 
               MB_OK | MB_ICONINFORMATION); 
}

BOOL DialogWindow::InitMsg(WPARAM wParam, LPARAM lParam) {
	//m_hDlg = hdlg;
	return TRUE;
}

BOOL DialogWindow::CommandMsg(WPARAM wParam, LPARAM lParam) {
	return FALSE;
}

BOOL DialogWindow::CommonControlColorNotifyMsg(UINT msg, WPARAM wParam, LPARAM lParam) {
	return FALSE;
}

BOOL DialogWindow::NotificationMsg(UINT msg, WPARAM wParam, LPARAM lParam) {
	return FALSE;
}

BOOL DialogWindow::NotifyMsg(WPARAM wParam, LPARAM lParam) {
	return FALSE;
}

BOOL DialogWindow::ScrollMsg(UINT msg, WPARAM wParam, LPARAM lParam) {
	return FALSE;
}

BOOL DialogWindow::TimerMsg(WPARAM wParam, LPARAM lParam) {
	return FALSE;
}

BOOL DialogWindow::UserMsg(UINT msg, WPARAM wParam, LPARAM lParam) {
	return FALSE;
}

BOOL DialogWindow::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL bRetValue = FALSE;

	switch(msg) {
	case WM_INITDIALOG: 
		bRetValue = this->InitMsg(wParam, lParam);
		break; 

	case WM_COMMAND: 
		bRetValue = this->CommandMsg(wParam, lParam);
		break;

	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
		bRetValue = this->CommonControlColorNotifyMsg(msg, wParam, lParam);
		break;

	case WM_ACTIVATEAPP:
	case WM_CANCELMODE:
	case WM_CHILDACTIVATE:
	case WM_CLOSE:
	case WM_COMPACTING:
	case WM_CREATE:
	case WM_DESTROY:
	case WM_ENABLE:
	case WM_ENTERSIZEMOVE:
	case WM_EXITSIZEMOVE:
	case WM_GETICON:
	case WM_GETMINMAXINFO:
	case WM_INPUTLANGCHANGE:
	case WM_INPUTLANGCHANGEREQUEST:
	case WM_MOVE:
	case WM_MOVING:
	case WM_NCACTIVATE:
	case WM_NCCALCSIZE:
	case WM_NCCREATE:
	case WM_NCDESTROY:
	case WM_NULL:
	case WM_PARENTNOTIFY:
	case WM_QUERYDRAGICON:
	case WM_QUERYOPEN:
	case WM_SHOWWINDOW:
	case WM_SIZE:
	case WM_SIZING:
	case WM_STYLECHANGED:
	case WM_STYLECHANGING:
	//case WM_THEMECHANGED:
	case WM_USERCHANGED:
	case WM_WINDOWPOSCHANGED:
	case WM_WINDOWPOSCHANGING:
		bRetValue = this->NotificationMsg(msg, wParam, lParam);
		break;

	case WM_NOTIFY:
		bRetValue = this->NotifyMsg(wParam, lParam);
		break;

	case WM_HSCROLL:
	case WM_VSCROLL: 
		bRetValue = this->ScrollMsg(msg, wParam, lParam);
		break;

	case WM_TIMER:
		bRetValue = this->TimerMsg(wParam, lParam);
		break;

    case WM_USER+1:
    case WM_USER+2:
    case WM_USER+3:
        bRetValue = this->UserMsg(msg, wParam, lParam);
        break;

	default:
		break;
	} // switch(msg)

	return bRetValue;
}

/* static */
BOOL CALLBACK DialogWindow::StaticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL bRetValue = FALSE;
	DialogWindow *pdw = (DialogWindow *) ::GetProp(hDlg, DIALOGWINDOW_PROPERTY);

	if (msg == WM_INITDIALOG)
    {
		pdw = (DialogWindow *) lParam;
		pdw->m_hDlg = hDlg;
		SetProp(hDlg, DIALOGWINDOW_PROPERTY, (HANDLE) pdw);
	}

	if (pdw != NULL)
		bRetValue = pdw->DlgProc(msg, wParam, lParam);

	return bRetValue;
}
