/*
 * uidevpropsheets.cpp
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
#include "uidevpropsheets.h"
#include "uidriver.h"
#include "resource.h"

#include "debug.h"
#include "utils.h"

#include <windows.h>
#include <winddi.h>
#include <winddiui.h>
#include <winspool.h>

#include <malloc.h>


// -----------------------------------------------------------------------------
// DATA
// -----------------------------------------------------------------------------

// PRIVATE TYPEDEFS 

typedef struct _DIALOGDATA
{
    PFNCOMPROPSHEET pfnComPropSheet;
    HANDLE hPropSheetAdded;
    HANDLE hComPropSheet;
} DIALOGDATA;

typedef DIALOGDATA *PDIALOGDATA;

// PRIVATE PROTOTYPES

static BOOL CALLBACK DevPropDlgProc(
                                    HWND hDlg,
                                    UINT message,
                                    WPARAM wParam,
                                    LPARAM lParam
                                   );


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

// PUBLIC ROUTINES

//  DrvDevicePropertySheets function is responsible for creating property sheet 
// pages that describe a printer's properties.
LONG DrvDevicePropertySheets(
                            PPROPSHEETUI_INFO  ppsuii,
                            LPARAM             lParam
                            )
{
    //DEBUGMESSAGE(("DrvDevicePropertySheets"));

    PDEVICEPROPERTYHEADER pdph; 

    if (ppsuii == NULL ||
        (pdph = (PDEVICEPROPERTYHEADER) ppsuii->lParamInit) == NULL)
    {
        SetLastError(ERROR_INVALID_DATA);
        return ERR_CPSUI_GETLASTERROR;
    }

    switch (ppsuii->Reason)
    {
    case PROPSHEETUI_REASON_INIT:
        {
            DEBUGMESSAGE(("DrvDevicePropertySheets - initializing the sheet"));

            LPPROPSHEETPAGE page = NULL;
            PDIALOGDATA data = NULL;

            LONG_PTR lptr = (LONG_PTR) NULL;

            // allocate a new dialog
            data = (PDIALOGDATA) malloc(sizeof(*data));
            data->pfnComPropSheet = ppsuii->pfnComPropSheet;
            data->hComPropSheet = ppsuii->hComPropSheet;
            // data->hPropSheetAdded INITIALIZED BELOW

            // allocate a new propsheet page
            page = (LPPROPSHEETPAGE) malloc(sizeof(PROPSHEETPAGE));
            ZeroMemory(page, sizeof(*page));
            page->dwSize = sizeof(*page);
            page->hInstance = g_hInstDLL;
            page->pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_DEVICEPROPERTIES);
            page->lParam = (LPARAM) data;
            page->pfnDlgProc = (DLGPROC) DevPropDlgProc;

            ppsuii->UserData = (ULONG_PTR) page;
            ppsuii->Result   = CPSUI_CANCEL;

            data->hPropSheetAdded = 
                (HANDLE) ppsuii->pfnComPropSheet(
                                                ppsuii->hComPropSheet,
                                                CPSFUNC_ADD_PROPSHEETPAGE,
                                                (LPARAM)page, 
                                                0
                                                );

            if (data->hPropSheetAdded == NULL) {
                DEBUGMESSAGE(("DrvDevicePropertySheets - ERROR - could not create the property sheet!"));
                free(data);
                free(page);
                return -1; // means failure
            }

            return 1;   // success
        }
        break;
       
    // msplot handles this case (?)
    //case PROPSHEETUI_REASON_GET_INFO_HEADER:

    case PROPSHEETUI_REASON_SET_RESULT:
        DEBUGMESSAGE(("DrvDevicePropertySheets - saving the result"));

        //
        // Save the result and also set the result to the caller.
        //
        /*
        if (pPI->hCPSUI == ((PSETRESULT_INFO)lParam)->hSetResult) {

            pPSUIInfo->Result = ((PSETRESULT_INFO)lParam)->Result;
            Result = 1;
        }
        */
        return 1;   // success

    case PROPSHEETUI_REASON_DESTROY:
        {
            DEBUGMESSAGE(("DrvDevicePropertySheets - destroying the sheet"));

            LPPROPSHEETPAGE page = (LPPROPSHEETPAGE) ppsuii->UserData;
            if (page != NULL)
            {
                PDIALOGDATA data = (PDIALOGDATA) page->lParam;
                if (data != NULL)
                    free(data);
                free(page);
                ppsuii->UserData = 0;
            }

            return 1;      // success
        }
        break;

    default:
        break;
    }

    DEBUGMESSAGE(("DrvDevicePropertySheets - Unhandled reason %d", ppsuii->Reason));

    return -1;      // failure
}

// *******************************************************************************
// **  PRIVATE ROUTINES  *********************************************************
// *******************************************************************************

static BOOL CALLBACK DevPropDlgProc(
                                    HWND hDlg,
                                    UINT message,
                                    WPARAM wParam,
                                    LPARAM lParam
                                    )
{
    //DEBUGMESSAGE(("DevPropDlgProc"));

    BOOL bRetValue = TRUE;
    switch(message)
    {
    case WM_INITDIALOG:
        {
            DEBUGMESSAGE(("DevPropDlgProc - WM_INITDIALOG"));

            LPPROPSHEETPAGE page = (LPPROPSHEETPAGE) lParam;
            PDIALOGDATA data = (PDIALOGDATA) page->lParam;

            SetWindowLongPtr(hDlg, DWL_USER, (LONG) data);
        }
        break;

    case WM_NOTIFY:
        {
            //DEBUGMESSAGE(("DevPropDlgProc - WM_NOTIFY"));

            LPNMHDR lpNMHdr = (LPNMHDR) lParam;
            UINT uiCode = lpNMHdr->code;

            switch(uiCode) {
            case PSN_APPLY:
                {
                    PDIALOGDATA data = (PDIALOGDATA) GetWindowLongPtr(hDlg, DWL_USER);
                    PFNCOMPROPSHEET pfnComPropSheet = data->pfnComPropSheet;

                    LONG lTemp = pfnComPropSheet(
                                                 data->hComPropSheet, 
                                                 CPSFUNC_SET_RESULT,
                                                 (LPARAM) data->hPropSheetAdded,
                                                 CPSUI_OK
                                                 );
                } break;

            case PSN_RESET:
                break;

            case PSN_SETACTIVE:
                break;

            default:
                bRetValue = FALSE;
                break;

            } // switch(uiCde)
        }
        break; // case WM_NOTIFY

    default:
        //DEBUGMESSAGE(("DevPropDlgProc - unhandled message %d", message));
        bRetValue = FALSE;
        break;
    }

    return bRetValue;
}
