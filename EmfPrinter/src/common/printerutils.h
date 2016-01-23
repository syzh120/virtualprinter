/*
 * printerutils.h
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

#ifndef PRINTERUTILS_H
#define PRINTERUTILS_H

#include "stdheaderwrapper.h"
#include "types.h"


// -----------------------------------------------------------------------------
// PRINTER-STUFF GETTERS
// -----------------------------------------------------------------------------

// CALLER RESPONSIBLE FOR FREEING THE RETURNED MALLOC'D BUFFER:

PDRIVER_INFO_3 GetInstalledPrinterDrivers(OUT DWORD& dwcPrinterDrivers);
PPRINTER_INFO_2 GetInstalledPrintersInfo2(OUT PDWORD pdwcCount);

LPTSTR GetPrinterDriverDirectory();
LPTSTR GetPrinterName(IN HANDLE hPrinter);

PPORT_INFO_1 GetInstalledPorts(DWORD& dwcPorts);
PJOB_INFO_1 GetPrintersJobs(IN HANDLE hPrinter, IN DWORD dwcJobs, 
                            OUT DWORD& dwcJobsReturned);
PJOB_INFO_2 GetJobInfo2(IN HANDLE hPrinter, IN DWORD dwJobID, OUT PDWORD pdwcBuff);
PDEVMODE GetPrinterDevMode(IN LPCTSTR lptstrPrinterName);
PDRIVER_INFO_3 GetPrinterDriverInfo3(IN HANDLE hPrinter);
PPRINTER_INFO_2 GetPrinterInfo2(IN LPCTSTR lptstrPrinterName);
PPRINTER_INFO_2 GetPrinterInfo2(IN HANDLE hPrinter);

DWORD GetPrinterJobDeletionCommand();

#endif  // PRINTERUTILS_H
