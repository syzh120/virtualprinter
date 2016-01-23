/*
 * thread.h
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

#ifndef THREAD_H
#define THREAD_H

#include "workerthread.h"
#include "utils.h"

#define WM_THREAD_COMPLETED         WM_USER+1
class InstallerDialogWindow;

enum InstallerMode
{
    IM_INSTALL,
    IM_CHECK,
    IM_UNINSTALL
};

class InstallWorkerThread : public WorkerThread
{
public:
    InstallWorkerThread() 
    { 
        m_pWnd=NULL; 
        m_strSourceDir=m_strPrinterName=NULL; 
        m_bFailed=FALSE;
    }
	virtual ~InstallWorkerThread();

	BOOL Failed() const { return m_bFailed; }

    void SetSourceDir(LPCTSTR sourceDir)
        { m_strSourceDir = strDuplicate(sourceDir); }
    void SetPrinterName(LPCTSTR pName)
        { m_strPrinterName = strDuplicate(pName); }
    void SetInstallerDialogWindow(InstallerDialogWindow *pWnd)
        { m_pWnd = pWnd; }
    void SetMode(InstallerMode m)
        { m_mode = m; }

protected:
	virtual void DoWork();
    BOOL Install();
    BOOL Uninstall();
    BOOL Check();

private:    // all these vars are only read by DoWork()
	BOOL m_bFailed;
    InstallerMode m_mode;
	InstallerDialogWindow *m_pWnd;
    LPTSTR m_strSourceDir;
    LPTSTR m_strPrinterName;
};

#endif
