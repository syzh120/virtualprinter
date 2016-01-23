/*
 * printerutils.cpp
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

#include "printerutils.h"
#include "utils.h"
#include <strsafe.h>


// -----------------------------------------------------------------------------
// PRINTER-STUFF GETTERS
// -----------------------------------------------------------------------------

PDRIVER_INFO_3 GetInstalledPrinterDrivers(OUT DWORD& dwcPrinterDrivers)
{
	PDRIVER_INFO_3 pinfo = NULL;
	DWORD dwTemp;
	DWORD dwCbNeeded;

    // enumerate installed drivers
	EnumPrinterDrivers(NULL, NULL, 3, NULL, 0, &dwCbNeeded, &dwTemp);

	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		pinfo = (PDRIVER_INFO_3) malloc(dwCbNeeded);
		if (pinfo != NULL) {
			if (!EnumPrinterDrivers(NULL, NULL, 3, (LPBYTE) pinfo, 
                                    dwCbNeeded, &dwCbNeeded, &dwTemp)) {
				free(pinfo);
				pinfo = NULL;
			} else {
				dwcPrinterDrivers = dwTemp;
			}
		}
	}

	return pinfo;
}

PPRINTER_INFO_2 GetInstalledPrintersInfo2(OUT PDWORD pdwcCount)
{
	PPRINTER_INFO_2 pinfoRetValue = NULL;
	DWORD dwcBuff = 0;
	DWORD dwcEnum = 0;

	BOOL bTemp = EnumPrinters(
		PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, 
		NULL, 2, NULL, 0, &dwcBuff, &dwcEnum
		);

	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER || bTemp) {
		pinfoRetValue = (PPRINTER_INFO_2) malloc(dwcBuff);

		if (pinfoRetValue != NULL) {
			BOOL b = EnumPrinters(
				            PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, 
				            NULL, 2, (LPBYTE) pinfoRetValue, dwcBuff, &dwcBuff, &dwcEnum
				            );
			if (!b) {
				free(pinfoRetValue);
				pinfoRetValue = NULL;
			} else {
				*pdwcCount = dwcEnum;
			}
		}
	}

	return pinfoRetValue;
}

PDEVMODE GetPrinterDevMode(IN LPCTSTR lptstrPrinterName)
{
	PDEVMODE pdmRetValue = NULL;
	PPRINTER_INFO_2 pinfo = GetPrinterInfo2(lptstrPrinterName);
	if (pinfo != NULL) {
		PDEVMODE pdm = pinfo->pDevMode;
		if (pdm != NULL) {
			DWORD dwcNeeded = pdm->dmSize + pdm->dmDriverExtra;
			pdmRetValue = (PDEVMODE) malloc(dwcNeeded);
			CopyMemory(pdmRetValue, pdm, dwcNeeded);
		}
		free(pinfo);
	}
	return pdmRetValue;
}

PPRINTER_INFO_2 GetPrinterInfo2(IN LPCTSTR lptstrPrinterName)
{
	PPRINTER_INFO_2 pinfoRetValue = NULL;
	HANDLE hPrinter = NULL;
	OpenPrinter((LPTSTR) lptstrPrinterName, &hPrinter, NULL);
	if (hPrinter != NULL) {
		pinfoRetValue = GetPrinterInfo2(hPrinter);
		ClosePrinter(hPrinter);
	}
	return pinfoRetValue;
}

PDRIVER_INFO_3 GetPrinterDriverInfo3(IN HANDLE hPrinter)
{
	::PDRIVER_INFO_3 pinfo = NULL;
	DWORD dwCbNeeded;
	::GetPrinterDriver(hPrinter, NULL, 3, 0, 0, &dwCbNeeded);
	if ( ::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		pinfo = (::PDRIVER_INFO_3) malloc(dwCbNeeded);
		if (pinfo != NULL) {
			if ( ! ::GetPrinterDriver(hPrinter, NULL, 3, (LPBYTE) pinfo, 
                                      dwCbNeeded, &dwCbNeeded)) {
				free(pinfo);
				pinfo = NULL;
			}
		}
	}
	return pinfo;
}

PPRINTER_INFO_2 GetPrinterInfo2(IN HANDLE hPrinter)
{
	PPRINTER_INFO_2 pinfo = NULL;
	DWORD dwcNeeded = 0;
	GetPrinter(hPrinter, 2, NULL, 0, &dwcNeeded);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		BOOL b;
		pinfo = (PPRINTER_INFO_2) malloc(dwcNeeded);
		b = GetPrinter(hPrinter, 2, (LPBYTE) pinfo, dwcNeeded, &dwcNeeded);
		if (b) {
			return pinfo;
		}
	}

	if (pinfo != NULL) {
		free(pinfo);
	}
		
	return NULL;
}

LPTSTR GetPrinterName(IN HANDLE hPrinter)
{
	LPTSTR lptstrRetValue = NULL;
	PPRINTER_INFO_2 pinfo = GetPrinterInfo2(hPrinter);
	if (pinfo != NULL) {
		lptstrRetValue = strDuplicate(pinfo->pPrinterName);
		free(pinfo);
	}
	return lptstrRetValue;
}

LPTSTR GetPrinterDriverDirectory()
{
	LPTSTR lptstrRetValue = NULL;
	DWORD dwCbNeeded;
	
    // use win's function:
    ::GetPrinterDriverDirectory(NULL, NULL, 1, 0, 0, &dwCbNeeded);

	if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		lptstrRetValue = (LPTSTR) malloc(dwCbNeeded);
		if (lptstrRetValue != NULL) {
            if (!::GetPrinterDriverDirectory(NULL, NULL, 1, (LPBYTE) lptstrRetValue, 
                                             dwCbNeeded, &dwCbNeeded)) {
				free(lptstrRetValue);
				lptstrRetValue = NULL;
			}
		}
	}

	return lptstrRetValue;
}

PJOB_INFO_1 GetPrintersJobs(IN HANDLE hPrinter, IN DWORD dwcJobs, 
                            OUT DWORD& dwcJobsReturned)
{
	::PJOB_INFO_1 pinfoJobs = NULL;
	DWORD dwTemp;
	DWORD dwCbNeeded;
	::EnumJobs(hPrinter, 0, dwcJobs, 1, NULL, 0, &dwCbNeeded, &dwTemp);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		pinfoJobs = (::PJOB_INFO_1) malloc(dwCbNeeded);
		if (pinfoJobs != NULL) {
			if (::EnumJobs(hPrinter, 0, dwcJobs, 1, (LPBYTE) pinfoJobs, 
                           dwCbNeeded, &dwCbNeeded, &dwTemp)) {
				dwcJobsReturned = dwTemp;
			} else {
				free(pinfoJobs);
				pinfoJobs = NULL;
			}
		}
	}
	return pinfoJobs;
}

PJOB_INFO_2 GetJobInfo2(IN HANDLE hPrinter, IN DWORD dwJobID, OUT PDWORD pdwcBuff)
{
	PJOB_INFO_2 pjiRetValue = NULL;
	DWORD dwcBuff = 0;
	GetJob(hPrinter, dwJobID, 2, NULL, 0, &dwcBuff);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		pjiRetValue = (PJOB_INFO_2) malloc(dwcBuff);
		if (pjiRetValue != NULL) {
			BOOL b = GetJob(hPrinter, dwJobID, 2, (LPBYTE) pjiRetValue, 
                            dwcBuff, &dwcBuff);
			if (!b) {
				free(pjiRetValue);
				pjiRetValue = NULL;
			}
		}
	}
	if (pjiRetValue != NULL) {
		*pdwcBuff = dwcBuff;
	}
	return pjiRetValue;
}

DWORD GetPrinterJobDeletionCommand()
{
	DWORD dwCommand = 0;
	DWORD dwWinVer = ::GetVersion();
	
    // WIN 95,98, ME HAVE THE HIGH BIT SET, WHILE WIN 4.0 AND LATER DO NOT
	// WIN 3.51 ETC WASN'T DOCUMENTED
	signed char scMajorVer = (char) LOBYTE(dwWinVer);
	if (scMajorVer >= 0)
		dwCommand = JOB_CONTROL_DELETE;
	else
		dwCommand = JOB_CONTROL_CANCEL;
	
	return dwCommand;
}

PPORT_INFO_1 GetInstalledPorts(DWORD& dwcPorts)
{
	PPORT_INFO_1 pinfo = NULL;
	DWORD dwTemp;
	DWORD dwCbNeeded;

	EnumPorts(NULL, 1, NULL, 0, &dwCbNeeded, &dwTemp);

	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		pinfo = (PPORT_INFO_1) malloc(dwCbNeeded);
		if (pinfo != NULL) {
			if (EnumPorts(NULL, 1, (LPBYTE) pinfo, dwCbNeeded, &dwCbNeeded, &dwTemp)) {
				dwcPorts = dwTemp;
			} else {
				// CANT GET INSTALLED PORTS
				free(pinfo);
				pinfo = NULL;
			}
		} // ELSE OUT OF MEMORY
	} // ELSE CANT GET INSTALLED PORTS

	return pinfo;
}

