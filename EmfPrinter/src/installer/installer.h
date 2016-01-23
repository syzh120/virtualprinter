/*
 * installer.h
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

#ifndef INSTALLER_H
#define INSTALLER_H

#include "stdheaderwrapper.h"
#include "types.h"

// -----------------------------------------------------------------------------
// GLOBAL ERROR STRING FOR INSTALL APIs
// -----------------------------------------------------------------------------

extern TCHAR g_strInstallerError[1024];
extern DWORD g_lastError;

// SetErrorString => SER
void SER(LPCTSTR strFormat, ...);


// -----------------------------------------------------------------------------
// (UN)INSTALL APIs
// -----------------------------------------------------------------------------

BOOL DoAllInstall(LPCTSTR lptstrPrinterName, LPCTSTR pSourceDir);

BOOL DoInstallDriver(LPCTSTR pSourceDir);

BOOL DoInstallPrinter(IN LPCTSTR lptstrPrinterName, IN LPCTSTR lptstrPortName);

BOOL DoInstallOrFindPort(OUT LPTSTR pPortName, IN size_t nBufLen);

BOOL DoAllUninstall();

BOOL DoUninstallPrinterAndDriver();

BOOL DoUninstallPrintersUsingDriver(PDRIVER_INFO_3 pinfoDriver);

#endif  // INSTALLER_H
