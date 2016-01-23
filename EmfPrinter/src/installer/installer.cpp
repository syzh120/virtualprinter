/*
 * installer.cpp
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
#include "installer.h"
#include "installerutils.h"
#include "printerutils.h"
#include "resource.h"
#include "utils.h"
#include "addport.h"

TCHAR g_strInstallerError[1024];
DWORD g_lastError;

// -----------------------------------------------------------------------------
// INSTALLER IMPLEMENTATION
// -----------------------------------------------------------------------------

void SER(LPCTSTR strFormat, ...)
{
    g_lastError = GetLastError(); // save it before it's overwritten

    va_list arglist;

    va_start( arglist, strFormat );
    int iLen = StringCchVPrintf( g_strInstallerError, 1024, strFormat, arglist );
    va_end( arglist );

    OutputDebugString(TEXT("\n"));
    OutputDebugString(g_strInstallerError);
    OutputDebugString(TEXT("\n"));
    OutputDebugString(GetErrorDesc(g_lastError));
    OutputDebugString(TEXT("\n\n"));
}

BOOL DoAllInstall(
                  LPCTSTR   lptstrPrinterName, 
                  LPCTSTR   pSourceDir
                  )
{
    // CORRECT INSTALL ORDER: the driver, the port, the printer

    if (!DoInstallDriver(pSourceDir))
        return FALSE;

    TCHAR portName[256];
    if (!DoInstallOrFindPort(portName, 256))
        return FALSE;

    if (!DoInstallPrinter(lptstrPrinterName, portName))
        return FALSE;

    // now check the printer driver version:

    PDEVMODE pdm = GetPrinterDevMode(lptstrPrinterName);
    if (pdm == NULL) {
        SER(TEXT("CANNOT GET THE PRINTER '%ws' DEVICE MODE"), lptstrPrinterName);
        return FALSE;
    }

    if (pdm->dmDriverVersion != emfDRIVER_REAL_VERSION) {
        SER(TEXT("THE PRINTER DRIVER VERSION IS %d WHILE SHOULD BE %d"), 
            pdm->dmDriverVersion, emfDRIVER_REAL_VERSION);
        return FALSE;
    }

    return TRUE;
}

BOOL DoInstallDriver(
                     LPCTSTR   pSourceDir
                     )
{
    PDRIVER_INFO_3 pinfoDriversTemp = NULL;
    unsigned int kTemp;
    if (!CheckPrinterDriverInstalled(pinfoDriversTemp, kTemp, emfDRIVER_NAME)) {
        SER(TEXT("UNABLE TO DETERMINE IF THE PRINTER DRIVER IS ALREADY INSTALLED"));
        return FALSE;
    }

    if (pinfoDriversTemp != NULL) {
        SER(TEXT("THE PRINTER DRIVER IS ALREADY INSTALLED"));
        free(pinfoDriversTemp); 
        return FALSE;
    }

    // our driver is not already installed, we can proceed...
    DEBUGMESSAGE(("PROCEDING WITH INSTALLATION OF THE DRIVER"));

    LPTSTR lptstrPrinterDriverDirectory = GetPrinterDriverDirectory();
    if (lptstrPrinterDriverDirectory == NULL) {
        SER(TEXT("ERROR GETTING PRINTER DRIVER DIRECTORY BUFFER"));
        return FALSE;
    }

    // when calling AddPrinterDriver() we need to pass all the .dll
    // files as full paths:

    static const LPCTSTR dllNames[] =
    {
        emfDRIVER_FILENAME,
        emfUIDLL_FILENAME,
        emfLICENSE_FILENAME
    };

    LPTSTR sourceFullNames[ARRCOUNT(dllNames)+1];
    LPTSTR destFullNames[ARRCOUNT(dllNames)+1];
    ZeroMemory(&sourceFullNames, sizeof(sourceFullNames));
    ZeroMemory(&destFullNames, sizeof(destFullNames));

    // prefix the dll names with the printer driver directory to get full paths:

    unsigned int k;
    for (k = 0; k < ARRCOUNT(destFullNames)-1; ++k) {
        sourceFullNames[k] = MakeFullFileName(pSourceDir, dllNames[k]);
        destFullNames[k] = MakeFullFileName(lptstrPrinterDriverDirectory, dllNames[k]);
        if (destFullNames[k] == NULL || sourceFullNames[k] == NULL) {
            SER(TEXT("OUT OF MEMORY"));
            strFree(lptstrPrinterDriverDirectory);
            return FALSE;
        }

        // AddPrinterDriver also requires all files have been copied before it's invoked
        if (!CopyFile(sourceFullNames[k], destFullNames[k], /*TRUE*/ FALSE)) {
            SER(TEXT("CANNOT INSTALL THE '%ws' DRIVER DLL IN THE SYSTEM FOLDER '%ws'..."),
                sourceFullNames[k], destFullNames[k]);
            strFree(lptstrPrinterDriverDirectory);
            return FALSE;
        }
    }

    DEBUGMESSAGE(("FINISHED COPYING DRIVER FILES"));

    // last one is not really a DLL name, rather a null-separed list of
    // dependent filenames....
    /*destFullNames[k] = MakePrinterDriverDependentFiles();
    if (destFullNames[k] == NULL) {
        SER(TEXT("OUT OF MEMORY"));
        strFree(lptstrPrinterDriverDirectory);
        return FALSE;
    }*/

    free(lptstrPrinterDriverDirectory);     // don't need it anymore

    DRIVER_INFO_3 di;
    ZeroMemory(&di, sizeof(di));
    
    // the order is important!! see dllNames[] definition above
    
    // NOTE: the pDriverPath, pConfigFile and pDataFile members
    //       are all _required_ (even the datafile!!)

    di.pDriverPath      = destFullNames[ 0 ];
    di.pConfigFile      = destFullNames[ 1 ];
    di.pDataFile        = destFullNames[ 2 ];
    di.pName            = emfDRIVER_NAME;
    di.cVersion         = emfDRIVER_VERSION;

    // DOUBLE CHECKED - IT FAILS ON MY SYSTEM IF IT'S "EMF" OR "EMF 1.008", 
    // WORKS IF "RAW" DEBUG / FIX
    di.pDefaultDataType = emfDRIVER_DEFAULT_DATATYPE;

    // NOTE: FIELDS "pEnvironment" AND "pMonitorName" ARE LEFT NULL

    BOOL bError = FALSE;
    if (!AddPrinterDriver(NULL, 3, (LPBYTE)&di))
    {
        // ERROR ADDING THE DRIVER
        // THIS WOULD MOST LIKELY BE BECAUSE:
        // 1.  THIS IS A WRONG DRIVER FOR THIS OS
        // 2.  WE WERE UNABLE TO OVERWRITE THE DLL'S IN THE OS-SPECIFIC DIRECTORY;
        //     PROBABLY THEY ARE ALREADY INSTALLED; THIS IS FIXED BY HAVING THE USER 
        //     RESTART WINDOWS, AND THEN IMMEDIATELY, BEFORE RUNNING ANY OTHER PROGRAMS,
        //     UNINSTALL OLD DLLs AND THEN TRY INSTALLING AGAIN
        //     A MORE ROBUST METHOD WOULD BE TO ITERATE PROCESSES THAT USE
        //     THE DLL'S THAT NEED TO BE OVERWRITTEN AND QUERY THE USER TO TERMINATE THEM
        //
        // NOTE: WE'VE CHECKED THIS DRIVER NAME ISN'T USED, 
        //       SO THAT IT CAN'T BE IN CONFLICT

        SER(TEXT("CANNOT INSTALL THE PRINTER DRIVER"));
        bError = TRUE;
    }
    else
        DEBUGMESSAGE(("DRIVER SUCCESSFULLY REGISTERED IN WINSPOOLER"));

    // free allocated strings
    for (k = 0; k < ARRCOUNT(destFullNames); ++k)
        strFree(destFullNames[k]);

    return !bError;
}


BOOL DoInstallOrFindPort(
                         OUT LPTSTR pPortName,
                         IN size_t nBufLen
                         )
{
    BOOL bError = FALSE;
    DWORD dwcPrinters;

    PPRINTER_INFO_2 pinfoPrinters = GetInstalledPrintersInfo2(&dwcPrinters);
    if (pinfoPrinters == NULL) {
        SER(TEXT("OUT OF MEMORY"));
        return FALSE;
    }

    // try to find an unused port name generating a few from the
    // master port name emfPORT_NAME
    emfFILENAME_ALTERNATES altPortNames;
    GenerateAlternativePortNames(altPortNames, emfPORT_NAME);

    BOOL bPortInUse = TRUE;
    unsigned int kAltPortName;
    for (kAltPortName = 0; kAltPortName < ARRCOUNT(altPortNames); ++kAltPortName)
    {
        bPortInUse = FALSE;
        for (unsigned int kPrinter = 0; kPrinter < dwcPrinters; ++kPrinter)
        {
            if (lstrcmpi(pinfoPrinters[kPrinter].pPortName, 
                         altPortNames[kAltPortName]) == 0) {
                bPortInUse = TRUE;
                break;
            }
        }
        if (!bPortInUse)
            break;      // we've found a free port
    }

    free(pinfoPrinters);    // we don't need it anymore

    if (bPortInUse) {
        SER(TEXT("ALL PORT NAMES SIMILAR TO '%ws' ARE IN USE"), emfPORT_NAME);
        return FALSE;
    }

    // we've got the name of our port, save it
    size_t nPortNameLen = strBufferSize(altPortNames[kAltPortName]);
    if (nPortNameLen >= nBufLen) {
        SER(TEXT("PORT NAME BUFFER TOO SMALL FOR '%ws'"), altPortNames[kAltPortName]);
        return FALSE;
    }

    strCopy(pPortName, altPortNames[kAltPortName], nPortNameLen);

    BOOL bPortInstalled;
    if (!CheckPortInstalled(bPortInstalled, altPortNames[kAltPortName])) {
        SER(TEXT("CANNOT CHECK IF THE PORT WAS INSTALLED SUCCESSFULLY"));
        return FALSE;
    }

    if (!bPortInstalled)
    {
        DEBUGMESSAGE(("PROCEDING WITH INSTALLATION OF THE PRINTER PORT"));

        // the port is not installed... install it
		AddPort(altPortNames[kAltPortName]);
        bPortInstalled = TRUE;

        if (!CheckPortInstalled(bPortInstalled, altPortNames[kAltPortName])) {
            SER(TEXT("CANNOT CHECK IF THE PORT WAS INSTALLED SUCCESSFULLY"));
            return FALSE;
        }
    }

    // still not installed ?
    if (!bPortInstalled) {
        SER(TEXT("COULDN'T INSTALL THE '%ws' PORT"), emfPORT_NAME);
        return FALSE;
    }

    DEBUGMESSAGE(("PRINTER PORT IS INSTALLED"));

    return TRUE;
}


// ASSUMES DRIVER AND PORT ARE ALREADY INSTALLED
// IT SELECTS THE PRINTPROCESSOR FOR THE NEW PRINTER AND SET THE DATATYPE
BOOL DoInstallPrinter(
                      IN LPCTSTR lptstrPrinterName, 
                      IN LPCTSTR lptstrPortName
                      )
{
    HANDLE hPrinter;
    if (OpenPrinter( const_cast<LPTSTR>(lptstrPrinterName), &hPrinter, NULL))
    {
        SER(TEXT("THIS PRINTER NAME IS ALREADY IN USE"));
        if (hPrinter != NULL)
            ClosePrinter(hPrinter);
        return FALSE;
    }

    DEBUGMESSAGE(("PROCEDING WITH INSTALLATION OF THE PRINTER"));

    PRINTER_INFO_2 pi;
    ZeroMemory(&pi, sizeof(pi));
    pi.pDatatype = emfPRINTER_DEFAULT_DATATYPE;
    pi.pDriverName = emfDRIVER_NAME;
    pi.pPortName = const_cast<LPTSTR>(lptstrPortName);
    pi.pPrinterName = const_cast<LPTSTR>(lptstrPrinterName);
    pi.pPrintProcessor = TEXT("WinPrint");      // we use the default win's print proc
    pi.Attributes = PRINTER_ATTRIBUTE_QUEUED | PRINTER_ATTRIBUTE_LOCAL;

    hPrinter = AddPrinter(NULL, 2, (LPBYTE) &pi);
    if (hPrinter == NULL)
    {
        // ERROR ADDING PRINTER - PRESUMABLY BECAUSE THERE IS AN ERROR 
        // IN THE DRIVER OR BECAUSE SOMETHING ISN'T INSTALLED OR IS IN USE 
        // (PORT, DRIVER)
        SER(TEXT("CANNOT ADD THE '%ws' PRINTER"), lptstrPrinterName);
        return FALSE;
    }

    DEBUGMESSAGE(("PRINTER SUCCESSFULLY REGISTERED IN WINSPOOLER"));


    // make sure the Win XP/2003 and later' "file pooling" feature
    // is disabled otherwise we won't be able to catch the spooler file name
    //
    // See http://www.undocprint.org/winspool/spool_files#force_jobid_in_spoolfile_names
    // for more info

    const TCHAR strRegKey[] = 
        TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Print\\Printers\\");

    LPTSTR buff = strCat(strRegKey, lptstrPrinterName, NULL);

    HKEY hkey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, buff, 
                     0, KEY_SET_VALUE, &hkey) != ERROR_SUCCESS) {
        SER(TEXT("Could not open the '%ws' registry key"), buff);
        return FALSE;
    }

    strFree(buff);

    // get spooler default directory
    DWORD cbNeeded;  
    DWORD dwType = REG_SZ;                   // data type
    if (GetPrinterData(
            hPrinter,                         // handle to printer or print server
            SPLREG_DEFAULT_SPOOL_DIRECTORY,
            &dwType,                         // data type
            NULL,                            // configuration data buffer
            0,                               // size of configuration data buffer
            &cbNeeded                        // bytes received or required
            ) != ERROR_MORE_DATA) {
        SER(TEXT("Cannot get default spool directory..."));
        return FALSE;
    }

    LPBYTE pSpoolDirectory = (LPBYTE)malloc(cbNeeded+1);
    if (GetPrinterData(
            hPrinter,                        // handle to printer or print server
            SPLREG_DEFAULT_SPOOL_DIRECTORY,
            &dwType,                        // data type
            pSpoolDirectory,                // configuration data buffer
            cbNeeded,                       // size of configuration data buffer
            &cbNeeded                       // bytes received or required
            ) != ERROR_SUCCESS) {
        SER(TEXT("Cannot get default spool directory..."));
        return FALSE;
    }

    if (RegSetValueEx(hkey, TEXT("SpoolDirectory"), 0, REG_SZ, 
                      (LPBYTE) pSpoolDirectory, cbNeeded)) {
        SER(TEXT("Cannot set the spooling directory in '%s' key"), buff);
        RegCloseKey(hkey);
        return FALSE;
    }

    free(pSpoolDirectory);
    RegCloseKey(hkey);
    ClosePrinter(hPrinter);

    // we need to stop the spooler
    if (IsSpoolerRunning())
        if (!StopSpooler())
            return FALSE;

    // ...and restart it in order to make the file pooling setting take place
    if (!StartSpooler())
        return FALSE;

    return TRUE;
}



// -----------------------------------------------------------------------------
// UNINSTALLER IMPLEMENTATION
// -----------------------------------------------------------------------------

BOOL DoAllUninstall()
{
    if (!DoUninstallPrinterAndDriver())
        return FALSE;

    return TRUE;
}

BOOL DoUninstallPrinterAndDriver()
{
	BOOL bError = FALSE;

	// NOTE WE SHOULD LOOP THROUGH THE VARIOUS OS TYPES, NOT JUST THE LOCAL MACHINE
	// IN CASE THE DRIVERS WERE INSTALLED FOR ADDITIONAL OS'S
	// FOR THOSE OTHER OS'S WE SHOULDN'T NEED TO CHECK FOR PRINTERS THAT USE THEM ... 
    // UNLESS THERE ARE SERVER COMPLICATIONS

	DWORD dwcPrinterDrivers;
	PDRIVER_INFO_3 pinfoDrivers = GetInstalledPrinterDrivers(dwcPrinterDrivers);
	if (pinfoDrivers == NULL) {
        SER(TEXT("CANNOT ENUMERATE INSTALLED PRINTER DRIVERS"));
        return FALSE;
    }

	for (unsigned int kDriver = 0; kDriver < dwcPrinterDrivers; ++kDriver)
    {
		// is this our driver?
		if (lstrcmp( pinfoDrivers[kDriver].pName, emfDRIVER_NAME) != 0 )
            continue;   // no, it's not

        DEBUGMESSAGE(("FOUND AN INSTANCE OF '%ws' DRIVER", emfDRIVER_NAME));

        // yes it is:
        if (!DoUninstallPrintersUsingDriver(&pinfoDrivers[kDriver])) {
            free(pinfoDrivers);
            return FALSE;
        }
	
		// ALL PRINTERS THAT USE THIS DRIVER SHOULD NOW BE DELETED
		if (!DeletePrinterDriver(NULL, NULL, pinfoDrivers[kDriver].pName)) {
			SER(TEXT("CANNOT DELETE '%ws' PRINTER DRIVER"), pinfoDrivers[kDriver].pName);
            free(pinfoDrivers);
            return FALSE;
		}

        // we need to stop the spooler before trying to delete the driver DLLs
        if (IsSpoolerRunning())
            if (!StopSpooler())
                return FALSE;

        DEBUGMESSAGE(("STOPPED SPOOLER... GOING TO DELETE DRIVER FILES"));

        #define MAX_RETRIES     10

        BOOL bSuccess = FALSE;
        for (int i=0; i<MAX_RETRIES && !bSuccess; i++)
        {
		    if (SafeDeleteFile(pinfoDrivers[kDriver].pConfigFile) &&
                SafeDeleteFile(pinfoDrivers[kDriver].pDataFile) &&
                SafeDeleteFile(pinfoDrivers[kDriver].pDriverPath) &&
                SafeDeleteFile(pinfoDrivers[kDriver].pHelpFile))
                bSuccess = TRUE;
            else
            {
                DEBUGMESSAGE(("SLEEPING IN ORDER TO ALLOW THE SPOOLER TO RELEASE ITS HANDLES"));
                Sleep(250);  // in ms
            }
        }

        if (!bSuccess)
        {
			SER(TEXT("CANNOT DELETE '%ws' PRINTER DRIVER FILES"), 
                pinfoDrivers[kDriver].pName);
            free(pinfoDrivers);
            StartSpooler();     // don't leave the spooler stopped
            return FALSE;
        }

		// DELETE ALL THE DRIVERS DEPENDENT FILES
		// ACCORDING TO DOCS THIS SHOULD INCLUDE THE ABOVE, BUT EXAMINATION OF
		// ACTUAL DATA SETS SHOWS THEY ARE DISTINCT
		LPTSTR lptstrDependentFile = pinfoDrivers[kDriver].pDependentFiles;
		if (lptstrDependentFile != NULL) {
			DWORD dwcLen = 0;
			while ((dwcLen = lstrlen(lptstrDependentFile)) != 0) {
				if (!SafeDeleteFile(lptstrDependentFile)) {
					SER(TEXT("CANNOT DELETE '%ws' FILE"), lptstrDependentFile);
                    free(pinfoDrivers);
                    StartSpooler();     // don't leave the spooler stopped
                    return FALSE;
				}
				lptstrDependentFile += dwcLen + 1;
			}
		}
        
        // spooler can be restarted
        if (!StartSpooler())
            return FALSE;

        DEBUGMESSAGE(("SPOOLER RESTARTED"));
    }

    return TRUE;
}

BOOL DoUninstallPrintersUsingDriver(PDRIVER_INFO_3 pinfoDriver)
{
	DWORD dwcPrinter;

    PPRINTER_INFO_2 pinfoPrinters = 
        GetInstalledPrintersInfo2(&dwcPrinter);
    if (pinfoPrinters == NULL) {
        SER(TEXT("CANNOT RETRIEVE INFO ABOUT INSTALLED PRINTERS"));
        return FALSE;
    }

    // FIND ALL PRINTERS THAT USE THIS DRIVER
	for (unsigned int kPrinter = 0; kPrinter < dwcPrinter; ++kPrinter)
    {
		if (lstrcmp(pinfoPrinters[kPrinter].pDriverName, 
                    pinfoDriver->pName) != 0)
            continue;       // this printer doesn't use our kDriver-th driver...

        DEBUGMESSAGE(("FOUND A PRINTER WHICH USES '%ws' DRIVER", emfDRIVER_NAME));

		// DELETE THIS PRINTER
		HANDLE hPrinter = NULL;

		// IF WE'RE INSIDE DRIVER CODE, MAYBE WE SHOULD USE REVERTTOSELF
		PRINTER_DEFAULTS pd = { NULL, NULL, PRINTER_ALL_ACCESS };
		OpenPrinter(pinfoPrinters[kPrinter].pPrinterName, &hPrinter, &pd);
		if (hPrinter == NULL) {
            SER(TEXT("CANNOT UNINSTALL PRINTER '%ws'"), 
                pinfoPrinters[kPrinter].pPrinterName);
            free(pinfoPrinters);
            return FALSE;
        }

        DEBUGMESSAGE(("DELETING JOBS FOR '%ws' PRINTER", 
                     pinfoPrinters[kPrinter].pPrinterName));

        // DELETE FIRST ANY JOBS IT HAS PENDING
		if (pinfoPrinters[kPrinter].cJobs > 0)
        {
			DWORD dwcJobs;
			PJOB_INFO_1 pinfoJobs = 
                GetPrintersJobs(hPrinter, pinfoPrinters[kPrinter].cJobs, dwcJobs);

			if (pinfoJobs == NULL) {
                SER(TEXT("CANNOT QUERY THE PENDING JOBS FOR PRINTER '%ws'"), 
                    pinfoPrinters[kPrinter].pPrinterName);
                free(pinfoPrinters);
                return FALSE;
            }

			for (unsigned int kJob = 0; kJob < dwcJobs; ++kJob) {
				DWORD dwDeleteCommand = GetPrinterJobDeletionCommand();

				if (!SetJob(hPrinter, pinfoJobs[kJob].JobId, 1, 
                            (LPBYTE) & pinfoJobs[kJob], dwDeleteCommand))
                {
                    SER(TEXT("CANNOT DELETE '%ws' JOB FOR PRINTER '%ws'"), 
                        pinfoJobs[kJob].JobId, pinfoPrinters[kPrinter].pPrinterName);
                    free(pinfoPrinters);
                    return FALSE;
				}
			}
        }

        // NOW WE SHOULD BE ABLE TO DELETE THE PRINTER
        if (!DeletePrinter(hPrinter)) {
            SER(TEXT("CANNOT UNINSTALL PRINTER '%ws'"), 
                pinfoPrinters[kPrinter].pPrinterName);
            free(pinfoPrinters);
            return FALSE;
		}

        DEBUGMESSAGE(("DELETED THE '%ws' PRINTER", 
                     pinfoPrinters[kPrinter].pPrinterName));

        ClosePrinter(hPrinter);
    }

    return TRUE;
}

