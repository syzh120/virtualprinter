/*
 * dlg.h
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

#ifndef DIALOGWINDOW_H
#define DIALOGWINDOW_H

#include "stdheaderwrapper.h"


// -----------------------------------------------------------------------------
// DialogWindow
// -----------------------------------------------------------------------------

class DialogWindow 
{
public:
	DialogWindow(HINSTANCE hInst, DWORD wTemplateID);
	virtual ~DialogWindow();

    // creation/destruction
	BOOL DoCreateDialog(HWND hwndParent);
	INT_PTR DoModalDialog(HWND hwndParent);
    BOOL DoModelessDialog(HWND hwndParent);

    void DestroyWindow();
    void ShowWindow(int nCmdShow);

    // misc
	void CenterWindowInScreen();
	void SetDlgItemText(int nIDDlgItem, LPCTSTR lptstr);
    void SetDlgItemFmtText(int nIDDlgItem, LPCTSTR lpstrFormat, ...);
    BOOL YesNoMessage(IN LPCTSTR lptstrMessage, ...);
    VOID OkMessage(IN LPCTSTR lptstrMessage, ...);

	inline HINSTANCE GetInstance() const    { return m_hInst; }
	inline HWND GetDlgHwnd() const          { return m_hDlg; }
	inline HWND GetParentHwnd() const       { return GetParent(m_hDlg); }
    inline HWND GetDlgItem(int nID) const   { return ::GetDlgItem(GetDlgHwnd(), nID); }

protected:
	virtual BOOL InitMsg(WPARAM wParam, LPARAM lParam);
	virtual BOOL CommandMsg(WPARAM wParam, LPARAM lParam);
	virtual BOOL CommonControlColorNotifyMsg(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL NotificationMsg(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL NotifyMsg(WPARAM wParam, LPARAM lParam);
	virtual BOOL ScrollMsg(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL TimerMsg(WPARAM wParam, LPARAM lParam);
	virtual BOOL UserMsg(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual BOOL DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE m_hInst;
    DWORD m_wTemplateID;
	HWND m_hDlg;
	BOOL m_bAutoDelete;

    // the real window procedure for handling messages
	static BOOL CALLBACK StaticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
};


#endif
