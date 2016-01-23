/*
 * installerdlg.h
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

#ifndef INSTALLERDLG_H
#define INSTALLERDLG_H

#include "dlg.h"
#include "thread.h"

class InstallerDialogWindow : public DialogWindow
{
public:
    InstallerDialogWindow(HINSTANCE hinst, InstallerMode mode, 
                          LPCTSTR sourceDir, LPCTSTR printerName = NULL);
	~InstallerDialogWindow();

	BOOL Failed() const { return m_thread.Failed(); }

    void ErrorMessage(LPCTSTR msg);
    void UpdateStatus(LPCTSTR lpstrFormat, ...);

protected:
	virtual BOOL InitMsg(WPARAM wParam, LPARAM lParam);
	virtual BOOL CommandMsg(WPARAM wParam, LPARAM lParam);
	virtual BOOL NotificationMsg(UINT msg, WPARAM wParam, LPARAM lParam);
    virtual BOOL UserMsg(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    InstallerMode m_mode;
    InstallWorkerThread m_thread;
};

#endif      // INSTALLERDLG_H
