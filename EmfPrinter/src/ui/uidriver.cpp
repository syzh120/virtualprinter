/*
 * uidriver.cpp
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

#define UIDRIVERENTRYPOINTS_C

#include "uidriver.h"
#include "debug.h"
#include "extdevmode.h"
#include "spoolerdata.h"
#include "utils.h"
#include "printerutils.h"


// -----------------------------------------------------------------------------
// DATA
// -----------------------------------------------------------------------------

HMODULE g_hInstDLL = NULL;


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

BOOL WINAPI DllMain(
                    HINSTANCE hInstance,
                    DWORD     ulReason,
                    LPVOID    lpvReserved
                    )
{
    // DllMain is continuosly called; thus this log message will appear
    // very often in DebugView: we put here info about the driver build
    // so that it's easy to check if the DLL version is OK
    //DEBUGMESSAGE(("DriverUI DllMain [%s]", __TIME__));

    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInstDLL = hInstance;
        //DisableThreadLibraryCalls( (HMODULE)hInstance );
        break;

    case DLL_PROCESS_DETACH:
        g_hInstDLL = NULL;
        break;
    }
	
	return TRUE;
}

BOOL DevQueryPrintEx(
	                PDEVQUERYPRINT_INFO  pDQPInfo
                    )
{
    DEBUGMESSAGE(("DevQueryPrintEx"));

	// DEBUG/FIX -- supposed to check that print is possible
    //              with the given settings

	return TRUE;
}

BOOL DrvPrinterEvent(
	                LPWSTR  lpwstrPrinterName,
	                INT     iEvent,
	                DWORD   dwFlags,
	                LPARAM  lParam
	                )
{
    DEBUGMESSAGE(("DrvPrinterEvent"));
	BOOL bRetValue = TRUE;

    switch(iEvent) {
	case PRINTER_EVENT_INITIALIZE:
        break;

	default:
        bRetValue = FALSE;
		break;
	}

	return bRetValue;
}


// The print spooler calls a printer interface DLL's DrvDriverEvent 
// function when the spooler processes driver-specific events that 
// might require action by the printer driver.
BOOL DrvDriverEvent(
                    DWORD   dwDriverEvent,
                    DWORD   dwLevel,
                    LPBYTE  pDriverInfo,
                    LPARAM  lParam
	                )
{
    DEBUGMESSAGE(("DrvDriverEvent"));

	BOOL bRetValue = TRUE;
	switch(dwDriverEvent)
    {
	case DRIVER_EVENT_DELETE:
		bRetValue = TRUE;
		break;
	case DRIVER_EVENT_INITIALIZE:
		bRetValue = TRUE;
		break;
	default:
		break;
	}

	return bRetValue;
}


// DrvDocumentEvent function can handle certain events associated 
// with printing a document.
//
// NOTE: printmirror uses this to grap the emf file
//
INT DrvDocumentEvent(
	                HANDLE  hPrinter,
	                HDC     hdc,
	                int     iEsc,
	                ULONG   ulcIn,
	                PULONG  pulIn,
	                ULONG   ulcOut,
	                PULONG  pulOut
	                )
{
    //DEBUGMESSAGE(("DrvDocumentEvent"));

	INT retValue = DOCUMENTEVENT_SUCCESS;

	switch(iEsc)
    {
	case DOCUMENTEVENT_RESETDCPRE:
        DEBUGMESSAGE(("DrvDocumentEvent - RESETDCPRE"));
        break;
	case DOCUMENTEVENT_RESETDCPOST:
        DEBUGMESSAGE(("DrvDocumentEvent - RESETDCPOST"));
        break;

	case DOCUMENTEVENT_CREATEDCPRE:
        DEBUGMESSAGE(("DrvDocumentEvent - CREATEDCPRE"));
        break;
    case DOCUMENTEVENT_CREATEDCPOST: 
        DEBUGMESSAGE(("DrvDocumentEvent - CREATEDCPOST"));
        break;
    case DOCUMENTEVENT_DELETEDC:
        DEBUGMESSAGE(("DrvDocumentEvent - DELETEDC"));
        break;

    case DOCUMENTEVENT_STARTPAGE:
        DEBUGMESSAGE(("DrvDocumentEvent - STARTPAGE"));
        break;
	case DOCUMENTEVENT_ENDPAGE: 
        DEBUGMESSAGE(("DrvDocumentEvent - ENDPAGE"));
        break;

    case DOCUMENTEVENT_ESCAPE:
        DEBUGMESSAGE(("DrvDocumentEvent - ESCAPE"));
        break;
    case DOCUMENTEVENT_QUERYFILTER:
        DEBUGMESSAGE(("DrvDocumentEvent - QUERYFILTER"));
        break;

	case DOCUMENTEVENT_STARTDOC:        // aka DOCUMENTEVENT_STARTDOCPRE
        DEBUGMESSAGE(("DrvDocumentEvent - STARTDOC"));
        break;
    case DOCUMENTEVENT_ABORTDOC:
        DEBUGMESSAGE(("DrvDocumentEvent - ABORTDOC"));
        break;

	case DOCUMENTEVENT_STARTDOCPOST:
        DEBUGMESSAGE(("DrvDocumentEvent - STARTDOCPOST"));
        break;

	case DOCUMENTEVENT_ENDDOC:          // aka DOCUMENTEVENT_ENDDOCPRE
        DEBUGMESSAGE(("DrvDocumentEvent - ENDDOC"));
        break;
    case DOCUMENTEVENT_ENDDOCPOST:
        DEBUGMESSAGE(("DrvDocumentEvent - ENDDOCPOST"));
        break;

    default:
        DEBUGMESSAGE(("DrvDocumentEvent - unknown escape code %d", iEsc));
    } 

	return retValue;
}
