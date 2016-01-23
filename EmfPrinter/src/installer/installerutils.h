/*
 * installerutils.h
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

#ifndef INSTALLERUTILS_H
#define INSTALLERUTILS_H

#include "stdheaderwrapper.h"
#include "types.h"
#include "installer.h"
#include "globals.h"


// -----------------------------------------------------------------------------
// ALREADY-INSTALLED CHECKERS
// -----------------------------------------------------------------------------

BOOL CheckPrinterInstalled(OUT BOOL& bInstalled, IN LPCTSTR lptstrName);

BOOL CheckPrinterDriverInstalled(OUT PDRIVER_INFO_3& out_pinfoDrivers, 
                                 OUT uint& out_kDriver, 
                                 IN LPCTSTR lptstrDriverName);

BOOL CheckPrinterProcessorInstalled(OUT BOOL& bInstalled, IN LPCTSTR lptstrProcName);

BOOL CheckPortInstalled(OUT BOOL& bInstalled, IN LPCTSTR lptstrPortName);


// -----------------------------------------------------------------------------
// SPOOLER PROCESS MANAGEMENT UTILITIES
// -----------------------------------------------------------------------------

BOOL IsSpoolerRunning();
BOOL StartSpooler();
BOOL StopSpooler();


// -----------------------------------------------------------------------------
// MISC
// -----------------------------------------------------------------------------

VOID GenerateAlternativePortNames(OUT emfFILENAME_ALTERNATES& alts, 
                                  LPCTSTR lptstrFileName);

LPTSTR MakePrinterDriverDependentFiles();
    // CALLER RESPONSIBLE FOR FREEING BUFFER ALLOCATED WITH malloc

#endif  // INSTALLERUTILS_H
