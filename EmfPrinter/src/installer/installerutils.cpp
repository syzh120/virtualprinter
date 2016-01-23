/*
 * installerutils.cpp
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

#include "installer.h"
#include "installerutils.h"
#include "printerutils.h"
#include "utils.h"
#include <strsafe.h>


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

VOID GenerateAlternativePortNames(OUT emfFILENAME_ALTERNATES& alts, 
                                  LPCTSTR lptstrFileName)
{
    for (unsigned int k = 0; k < ARRCOUNT(alts); ++k)
    {
        if (k == 0)
            StringCchCopy(alts[k], ARRCOUNT(alts[k]), lptstrFileName);
        else
            StringCchPrintf(alts[k], ARRCOUNT(alts[k]), TEXT("%ws_%d"), lptstrFileName, k);
    }
}


// -----------------------------------------------------------------------------
// ALREADY-INSTALLED CHECKERS
// -----------------------------------------------------------------------------

BOOL CheckPrinterInstalled(OUT BOOL& bInstalled, IN LPCTSTR lptstrName)
{
    bInstalled = FALSE;

    if (lptstrName != NULL) {
        HANDLE hPrinter = NULL;
        OpenPrinter((LPTSTR) lptstrName, &hPrinter, NULL);
        if (hPrinter != NULL) {
            ClosePrinter(hPrinter);
            bInstalled = TRUE;
        }
    }

    // we actually never fail
    return TRUE;
}

BOOL CheckPortInstalled(OUT BOOL& bInstalled, IN LPCTSTR lptstrPortName)
{
    BOOL bRetValue = FALSE;
    DWORD dwcPorts;
    PPORT_INFO_1 pinfoPorts = GetInstalledPorts(dwcPorts);
    if (pinfoPorts != NULL)
    {
        BOOL bPortInstalled = FALSE;
        for(uint kPort = 0; kPort < dwcPorts; ++kPort) {
            if (lstrcmpi(pinfoPorts[kPort].pName, lptstrPortName) == 0) {
                bPortInstalled = TRUE;
                break;
            }
        }

        bInstalled = bPortInstalled;
        bRetValue = TRUE;
        free(pinfoPorts);
    } // else GETINSTALLEDPORTS ERROR

    return bRetValue;
}

BOOL CheckPrinterDriverInstalled(
                                 OUT PDRIVER_INFO_3& out_pinfoDrivers, 
                                 OUT uint& out_kDriver, 
                                 IN LPCTSTR lptstrDriverName
                                 )
{
    DWORD dwcPrinterDrivers;
    PDRIVER_INFO_3 pinfoDrivers = 
        GetInstalledPrinterDrivers(dwcPrinterDrivers);

    if (pinfoDrivers == NULL)
        return FALSE;   // can't check

    // search in installed printer drivers for given driver name
    BOOL bFound = FALSE;
    uint kDriver;
    for(kDriver = 0; kDriver < dwcPrinterDrivers; ++kDriver) {
        if (lstrcmpi(pinfoDrivers[kDriver].pName, lptstrDriverName) == 0) {
            bFound = TRUE;
            break;
        }
    }

    if (bFound) {
        out_kDriver = kDriver;
        out_pinfoDrivers = pinfoDrivers;
    } else {
        free(pinfoDrivers);
        out_pinfoDrivers = NULL;
    }

    return TRUE;        // we could check successful
}


// -----------------------------------------------------------------------------
// SPOOLER PROCESS MANAGEMENT UTILITIES
// -----------------------------------------------------------------------------

// NOTE: this code was taken and readapted from http://sourceforge.net/projects/ceps
//       which is licensed under GPL too

BOOL IsSpoolerRunning()
{
    SC_HANDLE       scManager   = NULL;
    SC_HANDLE       scSpooler   = NULL;
    bool            fRetValue   = false;

    if ( (scManager = OpenSCManager( NULL, NULL, GENERIC_READ )) != NULL )
    {
        if ( (scSpooler = OpenService( scManager, TEXT("Spooler"), 
                                       SERVICE_QUERY_STATUS )) != NULL )
        {
            SERVICE_STATUS  ssStatus;

            QueryServiceStatus( scSpooler, &ssStatus );
            fRetValue   = (ssStatus.dwCurrentState == SERVICE_RUNNING);
            CloseServiceHandle( scSpooler );
        }
        CloseServiceHandle( scManager );
    }
    return fRetValue;
}

BOOL StartSpooler()
{
    SC_HANDLE       scManager   = NULL;
    SC_HANDLE       scSpooler   = NULL;
    bool            fRetValue   = false;
    DWORD           dwRetValue  = 0;

    if ( (scManager = OpenSCManager( NULL, NULL, GENERIC_EXECUTE )) != NULL )
    {
        if ( (scSpooler = OpenService( scManager, TEXT("Spooler"), 
                                       SERVICE_START | SERVICE_QUERY_STATUS )) != NULL )
        {
            // this start spooler may trigger a download and restart of the spooler
            // it must be the last meaningful thing that the installer does.
            int             nRetryCount = 2;
            int             nLoopCount;
            HANDLE          hServer     = NULL;
            SERVICE_STATUS  ssStatus;

            while ( nRetryCount-- > 0 && hServer == NULL )
            {
                if ( StartService( scSpooler, 0, NULL ) == 0
                        && GetLastError() != ERROR_SERVICE_ALREADY_RUNNING )
                {
                    SER(TEXT("Failed to start spooler"));
                    return FALSE;
                }
                else
                {
                    for ( nLoopCount = 0
                            ; QueryServiceStatus( scSpooler, &ssStatus )
                                && ssStatus.dwCurrentState != SERVICE_RUNNING
                                && nLoopCount < 10
                            ; nLoopCount++ )
                    {
                        Sleep( 2000 );
                    }

                    if ( ssStatus.dwCurrentState == SERVICE_RUNNING )
                    {
                        fRetValue   = true;

                        // Force Spooler to initialise by opening and 
                        // closing lcoal print server
                        if ( OpenPrinter( NULL, &hServer, NULL ) )
                        {
                            ClosePrinter( hServer );
                        }
                    }
                    else
                    {
                        SER(TEXT("Spooler not started"));
                        return FALSE;
                    }
                }
            }
            CloseServiceHandle( scSpooler );
        }
        CloseServiceHandle( scManager );
    }
    else
    {
        SER(TEXT("Service Manager could not be opened"));
    }

    return fRetValue;
}

BOOL StopSpooler()
{
    SC_HANDLE       scManager   = NULL;
    SC_HANDLE       scSpooler   = NULL;
    bool            fRetValue   = false;
    DWORD           dwRetValue  = 0;

    if ( (scManager = OpenSCManager( NULL, NULL, GENERIC_EXECUTE )) != NULL )
    {
        if ( (scSpooler = OpenService( scManager, TEXT("Spooler"), 
                SERVICE_STOP | SERVICE_START | SERVICE_QUERY_STATUS )) != NULL )
        {
            SERVICE_STATUS  ssStatus;

            if ( ControlService( scSpooler, SERVICE_CONTROL_STOP, &ssStatus ) == 0
                    && GetLastError() != ERROR_SERVICE_NOT_ACTIVE )
            {
                SER(TEXT("Failed to stop spooler"));
                return FALSE;
            }
            else
            {
                // VERY IMPORTANT: keep the do-while in this form so to always
                // sleep 500ms otherwise sometimes even if QueryServiceStatus()
                // says the service is stopped trying to immediately delete
                // files which were held by the spooler can fail...

                do
                {
                    Sleep( 500 );
                } while ( QueryServiceStatus( scSpooler, &ssStatus )
                          && ssStatus.dwCurrentState != SERVICE_STOPPED );

                fRetValue   = true;
            }
            CloseServiceHandle( scSpooler );
        }
        else
        {
            SER(TEXT("Failed to open Spooler service"));
            return FALSE;
        }
        CloseServiceHandle( scManager );
    }
    else
    {
        SER(TEXT("Service Manager could not be opened"));
    }

    return fRetValue;
}