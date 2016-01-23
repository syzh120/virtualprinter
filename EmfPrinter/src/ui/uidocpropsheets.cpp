/*
 * uidocpropsheets.cpp
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

#include <shlobj.h>     // needs to go before stdheaderwrapper.h

#include "stdheaderwrapper.h"
#include "uidocpropsheets.h"
#include "uidriver.h"
#include "resource.h"

#include "debug.h"
#include "devmode.h"
#include "extdevmode.h"
#include "utils.h"
#include "printerutils.h"
#include "hyperlink.h"

#include "globals.h"



// -----------------------------------------------------------------------------
// DATA STRUCTURES
// -----------------------------------------------------------------------------

// PRIVATE TYPEDEFS 

class DocumentPropDialogData        // holds data relative to this dialog
{
public:
    DocumentPropDialogData()
    { 
        m_pExtdmCurrent = extdmNew();
        m_bAllowUserUpdate = FALSE;
        m_hPropSheetAdded = m_hComPropSheet = NULL;
    }

    ~DocumentPropDialogData()
    { 
        extdmDelete(m_pExtdmCurrent);
    }

public:
    // this is the devmode which contains the current devmode
    // as edited by the user through the driver UI
    PEXTDEVMODE m_pExtdmCurrent;

    // proppage stuff:
    PFNCOMPROPSHEET m_pfnComPropSheet;
    HANDLE m_hPropSheetAdded;
    HANDLE m_hComPropSheet;

    // misc:
    BOOL m_bAllowUserUpdate;
};

// PRIVATE PROTOTYPES

static BOOL CALLBACK DocPropDlgProc(
                                    HWND hDlg,
                                    UINT message,
                                    WPARAM wParam,
                                    LPARAM lParam
                                    );

static BOOL CALLBACK AboutDlgProc(
                                    HWND hDlg,
                                    UINT message,
                                    WPARAM wParam,
                                    LPARAM lParam
                                    );

LONG DirectDrvDocumentPropertySheets(PDOCUMENTPROPERTYHEADER pdph);

LONG IndirectDrvDocumentPropertySheets(PPROPSHEETUI_INFO pPSUIInfo,
                                       PDOCUMENTPROPERTYHEADER pdph,
                                       LPARAM lParam);


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

// DrvDocumentPropertySheets function is responsible for creating property 
// sheet pages that describe a print document's properties.
LONG DrvDocumentPropertySheets(
                               PPROPSHEETUI_INFO pPSUIInfo,
                               LPARAM            lParam
                               )
{
    //DEBUGMESSAGE(("DrvDocumentPropertySheets"));

    LONG lRetValue = -1;

    if (pPSUIInfo == NULL)
        return DirectDrvDocumentPropertySheets((PDOCUMENTPROPERTYHEADER) lParam);
    else if (pPSUIInfo->lParamInit != 0)
        return IndirectDrvDocumentPropertySheets(pPSUIInfo,
                    (PDOCUMENTPROPERTYHEADER)pPSUIInfo->lParamInit,
                    lParam);
    else
        DEBUGMESSAGE(("Received a NULL PDOCUMENTPROPERTYHEADER"));

    return lRetValue;
}


// *******************************************************************************
// **  PRIVATE ROUTINES  *********************************************************
// *******************************************************************************

VOID FillComboWithOutputFormats(HWND hWnd, OUTPUTFORMAT *formats, int nFmtCount)
{
	SendMessage(hWnd, CB_RESETCONTENT, 0, 0);

    LPTSTR str = NULL;
    for (int i=0; i < nFmtCount; i++)
    {
        str = strCat(formats[i].strName, TEXT(" ("),
                     formats[i].strDescription, TEXT(")"), NULL);

        // the combo will make a copy of the passed string
		SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM) str);

        strFree(str);
    }

    // don't leave a null selection
    SendMessage(hWnd, CB_SETCURSEL, (WPARAM)0, 0);
}

BOOL GetComboText(OUT LPTSTR* lpStr, IN WPARAM itemIndex, IN HWND hwndCombo)
{
	LRESULT lres = SendMessage(hwndCombo, CB_GETLBTEXTLEN, itemIndex, 0);
	if (lres == CB_ERR)
        return FALSE;
    
	DWORD dwcStringLen = (DWORD) lres + 1;
	*lpStr = strAlloc(dwcStringLen);
	lres = SendMessage(hwndCombo, CB_GETLBTEXT, itemIndex, (LPARAM) *lpStr);
	if (lres == CB_ERR)
        return FALSE; 

    return TRUE;
}

INT GetComboCurSel(IN HWND hwndCombo)
{
	LRESULT lresIndex = ::SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
	if (lresIndex == CB_ERR)
        return -1;

    return lresIndex;
}

BOOL GetComboCurSelText(OUT LPTSTR* lpStr, IN HWND hwndCombo)
{
	LRESULT lresIndex = ::SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
	if (lresIndex == CB_ERR)
        return FALSE;

    return GetComboText(lpStr, (WPARAM) lresIndex, hwndCombo);
}

BOOL GetEditControlText(OUT LPTSTR* lpStr, IN HWND hwndText)
{
	DWORD len = (DWORD)::SendMessage(hwndText, EM_LINELENGTH, 0, 0) + 10;
	*lpStr = strAlloc(len);

    (*lpStr)[0] = (TCHAR)len;
    SendMessage(hwndText, EM_GETLINE, 0, (LPARAM) *lpStr);

    // testing SendMessage retvalue is not ok when getting text
    // out of an empty text control!
    return (*lpStr)[0] != (TCHAR)len;
}

VOID SelectFormat(IN HWND hDlg, IN LPCTSTR format)
{
    HWND hwndCombo = NULL;
    int index = -1;
    DWORD nId = -1;

    for (int i=0; i<ARRCOUNT(g_vectorFormats); i++) {
        LPCTSTR fmt = g_vectorFormats[i].strName;
        if (_tcsncmp(format, fmt, strLen(format)) == 0) {
            hwndCombo = GetDlgItem(hDlg, IDC_COMBO_VECTOR_FORMAT);
            index = i;
            nId = IDC_VECTOR_FORMAT_RADIOBOX;
            break;
        }
    }

    if (index == -1)
    {
        for (int i=0; i<ARRCOUNT(g_rasterFormats); i++) {
            LPCTSTR fmt = g_rasterFormats[i].strName;
            if (_tcsncmp(format, fmt, strLen(format)) == 0) {
                hwndCombo = GetDlgItem(hDlg, IDC_COMBO_RASTER_FORMAT);
                index = i;
                nId = IDC_RASTER_FORMAT_RADIOBOX;
                break;
            }
        }
    }

    SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM)index, 0);

    // select the radio button next to the combo which we just selected
    CheckRadioButton(hDlg, IDC_VECTOR_FORMAT_RADIOBOX, IDC_RASTER_FORMAT_RADIOBOX,
                     nId);
}

/* wxItemIdList implements RAII on top of ITEMIDLIST */
class wxItemIdList
{
public:
    // ctor takes ownership of the item and will free it
    wxItemIdList(LPITEMIDLIST pidl)
    {
        m_pidl = pidl;
    }

    static void Free(LPITEMIDLIST pidl)
    {
        if ( pidl )
        {
            LPMALLOC pMalloc;
            SHGetMalloc(&pMalloc);
            if ( pMalloc )
            {
                pMalloc->Free(pidl);
                pMalloc->Release();
            }
            else
            {
                DEBUGMESSAGE_LASTERROR(("SHGetMalloc"));
            }
        }
    }

    ~wxItemIdList()
    {
        Free(m_pidl);
    }

    // implicit conversion to LPITEMIDLIST
    operator LPITEMIDLIST() const { return m_pidl; }

    // get the corresponding path, returns empty string on error
    LPTSTR GetPath() const
    {
        LPTSTR path = strAlloc(MAX_PATH+1);
        if ( !SHGetPathFromIDList(m_pidl, path) )
        {
            DEBUGMESSAGE_LASTERROR(("SHGetPathFromIDList"));
        }

        return path;
    }

private:
    LPITEMIDLIST m_pidl;
};

LONG DirectDrvDocumentPropertySheets(PDOCUMENTPROPERTYHEADER pdph)
{
    //DEBUGMESSAGE(("DirectDrvDocumentPropertySheets - direct call by the spooler"));

    // CALLED DIRECTLY BY SPOOLER, RATHER THAN INDIRECTLY THROUGH CPSUI

    DWORD fMode = pdph->fMode;
    ASSERT(pdph != NULL);

    if (fMode == 0 || pdph->pdmOut == NULL)
    {
        DEBUGMESSAGE(("DirectDrvDocumentPropertySheets - "
                      "I was asked for EXTDEVMODE size (which is %d bytes)", 
                      extdmSizeof()));

        pdph->cbOut = (LONG) extdmSizeof();
        return pdph->cbOut;
    } 

    PDEVMODE pdmIn = pdph->pdmIn;
    PDEVMODE pdmOut = pdph->pdmOut;

    if (pdmIn && (fMode & DM_IN_BUFFER) &&
        pdmIn->dmSize == sizeof(DEVMODEW) && 
        pdmIn->dmDriverExtra == extdmSizeofPrivate())
    {
        // MSDN says:
        // The caller has supplied a DEVMODE structure pointer 
        // in pdmIn, and the DrvDocumentPropertySheets function 
        // should update its internal DEVMODE structure to reflect 
        // the contents of the supplied DEVMODE.

        DEBUGMESSAGE(("DirectDrvDocumentPropertySheets - I was asked to "
                      "update my internal DEVMODE from pdmIn"));

        // nothing to update actually
        return 1;   // success
    }

    if (pdmOut && (fMode & DM_OUT_BUFFER))
    {
        // MSDN says:
        // The caller has supplied a DEVMODE structure pointer in 
        // pdmOut, and the DrvDocumentPropertySheets function 
        // should copy the contents of its internal DEVMODE structure 
        // into the supplied DEVMODE.

        DEBUGMESSAGE(("DirectDrvDocumentPropertySheets - I was asked to "
                      "update pdmOut from my internal DEVMODE"));

        // create our internal devmode
        PEXTDEVMODE pExt = (PEXTDEVMODE)pdmOut;
        extdmSetToDefault(pExt);

        // get the devmode for current printer
        PDEVMODE pdmTemp = GetPrinterDevMode(pdph->pszPrinterName);
        if (pdmTemp != NULL) {
            extdmMerge(pExt, pdmTemp);
            free(pdmTemp);
        } 

        // copy our "internal" devmode in the output devmode
        dmCopy((LPBYTE) pdmOut, (PDEVMODE) pExt);

        return 1;   // success
    }

    if ((fMode & DM_ADVANCED) || (fMode & DM_IN_PROMPT))
    {
        // we were asked to create document property sheets in a
        // direct spooler call - this shouldn't happen!

        DEBUGMESSAGE(("DirectDrvDocumentPropertySheets - I was asked to create "
                      "sheets in a direct call - WARNING THIS SHOULDN'T HAPPEN"));

        return -1;
    }

    if ((fMode & DM_USER_DEFAULT) || (fMode & DM_OUT_DEFAULT))
    {
        DEBUGMESSAGE(("DirectDrvDocumentPropertySheets - DM_USER_DEFAULT and "
                      "DM_OUT_DEFAULT should not be used!"));

        return -1;
    }

    DEBUGMESSAGE(("DirectDrvDocumentPropertySheets - unhandled call with "
                  "fMode == %d, pdmIn == %d, pdmOut == %d", fMode, pdmIn, pdmOut));

    return -1;
}

LONG IndirectDrvDocumentPropertySheets(PPROPSHEETUI_INFO pPSUIInfo,
                                       PDOCUMENTPROPERTYHEADER pdph,
                                       LPARAM lParam)
{
    //DEBUGMESSAGE(("IndirectDrvDocumentPropertySheets - indirect call by the UI"));

    DWORD fMode = pdph->fMode;
    ASSERT(pdph != NULL && pPSUIInfo != NULL);

    if (fMode & DM_ADVANCED)
    {
        // MSDN say:
        // If set, the DrvDocumentPropertySheets function should only 
        // create the Advanced document page.
        // If not set, the DrvDocumentPropertySheets function should 
        // create both the Page Setup and Advanced document pages.
    }

    // WE ARE BEING CALLED BY CPSUI, TO SHOW TO THE USER 
    // THE CONFIGURABLE PROPERTY SHEETS FOR THIS DOCUMENT

    switch (pPSUIInfo->Reason)
    {
    case PROPSHEETUI_REASON_INIT:
        { 
            DEBUGMESSAGE(("IndirectDrvDocumentPropertySheets - initializing the sheet"));

            // alloc data
            DocumentPropDialogData *data = new DocumentPropDialogData();
            LPPROPSHEETPAGE page = (LPPROPSHEETPAGE) malloc(sizeof(PROPSHEETPAGE));

            if (data == NULL || page == NULL) {
                DEBUGMESSAGE(("OUT OF MEMORY"));
                return -1;   // failure
            }

            // initialize a page of the document property dialog
            ZeroMemory(page, sizeof(*page));
            page->dwSize = sizeof(*page);
            page->hInstance = g_hInstDLL;
            page->pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_DOCUMENTPROPERTIES);
            page->lParam = (LPARAM) data;
            page->pfnDlgProc = (DLGPROC) DocPropDlgProc;

            // from pm
            //page->dwFlags = PSP_USECALLBACK;
            //psp->pfnCallback = PropSheetPageProc;
     
            // save some stuff in the provided pointer
            pPSUIInfo->UserData = (ULONG_PTR) page;
            pPSUIInfo->Result   = CPSUI_CANCEL;


            // fill DocumentPropDialogData structure
            // -------------------------------------

            data->m_bAllowUserUpdate =
                (pdph->fMode & DM_OUT_BUFFER)  &&
                !(pdph->fMode & DM_NOPERMISSION) &&
                pdph->pdmOut != NULL;

            // set the initial devmode for this dialog
            if ((pdph->fMode & DM_IN_BUFFER) &&
                pdph->pdmIn != NULL &&
                extdmIsAnExtDevMode(pdph->pdmIn))
                dmCopy((LPBYTE)data->m_pExtdmCurrent, pdph->pdmIn);
            else
                extdmSetToDefault(data->m_pExtdmCurrent);

            data->m_pfnComPropSheet = pPSUIInfo->pfnComPropSheet;
            data->m_hComPropSheet = pPSUIInfo->hComPropSheet;

            // this call will actually create the sheet
            data->m_hPropSheetAdded = 
                (HANDLE) pPSUIInfo->pfnComPropSheet(
                                                    pPSUIInfo->hComPropSheet,
                                                    CPSFUNC_ADD_PROPSHEETPAGE,
                                                    (LPARAM)page, 
                                                    0
                                                    );

            if (data->m_hPropSheetAdded == NULL)
            {
                DEBUGMESSAGE(("COULD NOT CREATE THE PROPERTY SHEET PAGE"));
                delete data;
                if (page != NULL)
                    free(page);
                return -1;
            }

            // create the about sheet
            {
                LPPROPSHEETPAGE page = (LPPROPSHEETPAGE) malloc(sizeof(PROPSHEETPAGE));

                if (data == NULL || page == NULL) {
                    DEBUGMESSAGE(("OUT OF MEMORY"));
                    return -1;   // failure
                }

                // initialize a page of the document property dialog
                ZeroMemory(page, sizeof(*page));
                page->dwSize = sizeof(*page);
                page->hInstance = g_hInstDLL;
                page->pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_ABOUT);
                page->lParam = (LPARAM) data;
                page->pfnDlgProc = (DLGPROC) AboutDlgProc;

                // ignore error code - it's not fundamental
                (HANDLE) pPSUIInfo->pfnComPropSheet(
                                                    pPSUIInfo->hComPropSheet,
                                                    CPSFUNC_ADD_PROPSHEETPAGE,
                                                    (LPARAM)page, 
                                                    0
                                                    );
            }

            return 1;   // success
        }
        break;

    case PROPSHEETUI_REASON_SET_RESULT:
        {
            DEBUGMESSAGE(("IndirectDrvDocumentPropertySheets - saving the result"));

            LPPROPSHEETPAGE page = (LPPROPSHEETPAGE) pPSUIInfo->UserData;
            PSETRESULT_INFO psri = (PSETRESULT_INFO) lParam;
            if (psri->Result != CPSUI_OK)
                return 1;

            if (page == NULL) {
                DEBUGMESSAGE(("Unexpected NULL ptr"));
                return -1;
            }

            DocumentPropDialogData* data = 
                (DocumentPropDialogData *) page->lParam;
            if (data->m_bAllowUserUpdate)
            {
                // save the user-edited options in the given out PDEVMODE
                dmCopy((LPBYTE)pdph->pdmOut, (PCDEVMODE)data->m_pExtdmCurrent);
            }

            return 1;
        }
        break;

    case PROPSHEETUI_REASON_DESTROY:
        {
            DEBUGMESSAGE(("IndirectDrvDocumentPropertySheets - destroying the sheet"));

            LPPROPSHEETPAGE page = (LPPROPSHEETPAGE) pPSUIInfo->UserData;
            if (page == NULL) {
                DEBUGMESSAGE(("Unexpected NULL ptr"));
                return -1;
            }

            DocumentPropDialogData *data = (DocumentPropDialogData *) page->lParam;
            if (data != NULL)
                delete data;     // we won't need it anymore
            else
                DEBUGMESSAGE(("Unexpected NULL ptr"));

            free(page);
            pPSUIInfo->UserData = 0;

            return 1;
        } 
        break;

    default:
        break;
    }

    DEBUGMESSAGE(("IndirectDrvDocumentPropertySheets - unhandled reason %d", 
                 pPSUIInfo->Reason));

    return -1;  // unhandled reason...
}

BOOL OnNotify(HWND hDlg, LPNMHDR lpNMHdr)
{
    //DEBUGMESSAGE(("OnNotify"));

    UINT uiCode = lpNMHdr->code;
    switch (uiCode)
    {
    case PSN_APPLY:
        {
            DEBUGMESSAGE(("OnNotify - PSN_APPLY"));

            DocumentPropDialogData *data = 
                (DocumentPropDialogData *) GetWindowLongPtr(hDlg, DWL_USER);
            if (data == NULL) {
                DEBUGMESSAGE(("DocPropDlgProc - invalid internal data pointer"));
                return FALSE;
            }

            // which format combo should we use?
            LPTSTR format = NULL;
            if (IsDlgButtonChecked(hDlg, IDC_VECTOR_FORMAT_RADIOBOX) == BST_CHECKED)
            {
                INT sel = GetComboCurSel(GetDlgItem(hDlg, IDC_COMBO_VECTOR_FORMAT));
                format = strDuplicate(g_vectorFormats[sel].strName);
            }
            else if (IsDlgButtonChecked(hDlg, IDC_RASTER_FORMAT_RADIOBOX) == BST_CHECKED)
            {
                INT sel = GetComboCurSel(GetDlgItem(hDlg, IDC_COMBO_RASTER_FORMAT));
                format = strDuplicate(g_rasterFormats[sel].strName);
            }
            else
            {
                DEBUGMESSAGE(("DocPropDlgProc - unexpected condition"));
                return FALSE;
            }

            // get the output folder & validate it
            LPTSTR folder = NULL;
            if (!GetEditControlText(&folder, GetDlgItem(hDlg, IDC_OUTPUT_FOLDER))) {
                DEBUGMESSAGE(("DocPropDlgProc - could not get output folder text"));
                return FALSE;
            }
            if (!FolderExists(folder)) {
                ErrorMessage(hDlg, TEXT("Warning"),
                       TEXT("The given output directory does not exist!"));
                return FALSE;
            }

            // get the output filename & validate it
            LPTSTR filename = NULL;
            if (!GetEditControlText(&filename, GetDlgItem(hDlg, IDC_OUTPUT_FILENAME))) {
                DEBUGMESSAGE(("DocPropDlgProc - could not get output filename text"));
                return FALSE;
            }
            if (!IsValidFilename(filename)) {
                LPTSTR temp = strCat(TEXT("The given output filename is not valid!\n"),
                                     TEXT("It should not contain any of the '"),
                                     g_strFileNameForbiddenChars,
                                     TEXT("' characters."),
                                     NULL);
                ErrorMessage(hDlg, TEXT("Warning"), temp);
                strFree(temp);
                return FALSE;
            }

            // get the raster conv options
            LPTSTR rasteropt = NULL;
            if (!GetEditControlText(&rasteropt, GetDlgItem(hDlg, IDC_IMAGEMAGICK_OPTIONS))) {
                DEBUGMESSAGE(("DocPropDlgProc - could not get raster conv opt text"));
                return FALSE;
            }

            // get the postgen cmd
            LPTSTR postgen = NULL;
            if (!GetEditControlText(&postgen, GetDlgItem(hDlg, IDC_POSTGEN_CMD))) {
                DEBUGMESSAGE(("DocPropDlgProc - could not get postgen cmd text"));
                return FALSE;
            }

            // get override checkbox status
            BOOL override = 
                IsDlgButtonChecked(hDlg, IDC_OVERRIDE_CHECKBOX) == BST_CHECKED;

            // get crop checkbox
            BOOL crop = 
                IsDlgButtonChecked(hDlg, IDC_CROP_CHECKBOX) == BST_CHECKED;

            // get open-output checkbox
            BOOL openout = 
                IsDlgButtonChecked(hDlg, IDC_OPEN_VIEWER_CHECKBOX) == BST_CHECKED;

            // save all data in the EXTDEVMODE
            extdmSetPrivateData(data->m_pExtdmCurrent, 
                                format, filename, folder, rasteropt, postgen, 
                                override, openout, crop);

            // cleanup
            strFree(format);  
            strFree(filename);  
            strFree(folder);
            strFree(postgen);
            strFree(rasteropt);

            // call the _SET_RESULT callback
            PFNCOMPROPSHEET pfnComPropSheet = data->m_pfnComPropSheet;
            LONG lTemp = pfnComPropSheet(
                                        data->m_hComPropSheet, 
                                        CPSFUNC_SET_RESULT,
                                        (LPARAM) data->m_hPropSheetAdded,
                                        CPSUI_OK
                                        );

            return TRUE;
        }
        break;

    case PSN_RESET:
        break;

    case PSN_SETACTIVE:
        break;

    default:
        break;
    }

    return FALSE;
}

static int CALLBACK
BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
    switch(uMsg)
    {
#ifdef BFFM_SETSELECTION
        case BFFM_INITIALIZED:
            // sent immediately after initialisation and so we may set the
            // initial selection here
            //
            // wParam = TRUE => lParam is a string and not a PIDL
            ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
            break;
#endif // BFFM_SETSELECTION
    }

    return 0;
}

BOOL OnCommand(HWND hDlg, WORD wID, WORD code)
{
    //DEBUGMESSAGE(("OnCommand"));

    DocumentPropDialogData *data = 
        (DocumentPropDialogData *) GetWindowLongPtr(hDlg, DWL_USER);

    switch (wID)
    {
    case IDC_COMBO_VECTOR_FORMAT:
    case IDC_COMBO_RASTER_FORMAT:
        {
            DEBUGMESSAGE(("OnCommand - combo command"));
            switch (code)
            {
            case CBN_SELCHANGE:
                {
                    DWORD nId = 
                        (wID==IDC_COMBO_VECTOR_FORMAT) ? 
                            IDC_VECTOR_FORMAT_RADIOBOX : 
                            IDC_RASTER_FORMAT_RADIOBOX;

                    // select the radio button next to the combo which changed
                    CheckRadioButton(hDlg, 
                             IDC_VECTOR_FORMAT_RADIOBOX, IDC_RASTER_FORMAT_RADIOBOX,
                             nId);
                }
                break;

            default:
                break;
            }
        }
        break;

    case IDC_BROWSE_BUTTON:
        {
            BROWSEINFO bi;
            bi.hwndOwner      = hDlg;
            bi.pidlRoot       = NULL;
            bi.pszDisplayName = NULL;
            bi.lpszTitle      = TEXT("Choose output folder");
            bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
            bi.lpfn           = BrowseCallbackProc;

            LPTSTR folder = NULL;
            if (!GetEditControlText(&folder, GetDlgItem(hDlg, IDC_OUTPUT_FOLDER))) {
                DEBUGMESSAGE(("DocPropDlgProc - could not get output folder text"));
                return FALSE;
            }

            bi.lParam         = (LPARAM)folder; // param for the callback

            const int verComCtl32 = GetComCtl32Version();

            // we always add the edit box (it doesn't hurt anybody, does it?) if it is
            // supported by the system
            if ( verComCtl32 >= 471 )
            {
                bi.ulFlags |= BIF_EDITBOX;
            }

            // to have the "New Folder" button we must use the "new" dialog style which
            // is also the only way to have a resizable dialog
            if (verComCtl32 >= 500)
            {
                bi.ulFlags |= BIF_NEWDIALOGSTYLE;
            }

            // do show the dialog
            wxItemIdList pidl(SHBrowseForFolder(&bi));

            // cleanup
            wxItemIdList::Free((LPITEMIDLIST)bi.pidlRoot);
            strFree((LPTSTR)bi.lParam);

            if ( !pidl )
            {
                DEBUGMESSAGE(("user hit cancel"));
                return TRUE;
            }

            LPTSTR newPath = pidl.GetPath();
            if (newPath)
            {
                // change the text in the associated control
                SendMessage(GetDlgItem(hDlg, IDC_OUTPUT_FOLDER), 
                            WM_SETTEXT, 0, (LPARAM)newPath);
                strFree(newPath);
            }
        }
        break;

    default:
        break;
    }

    return FALSE;
}

static BOOL CALLBACK DocPropDlgProc(
                                    HWND hDlg,
                                    UINT message,
                                    WPARAM wParam,
                                    LPARAM lParam
                                    )
{
    //DEBUGMESSAGE(("DocPropDlgProc"));

    switch (message)
    {
    case WM_INITDIALOG:
        {
            DEBUGMESSAGE(("DocPropDlgProc - WM_INITDIALOG"));
            LPPROPSHEETPAGE page = (LPPROPSHEETPAGE) lParam;

            // attach to our property sheet page an instance of our data holder class
            DocumentPropDialogData *data = (DocumentPropDialogData *) page->lParam;
            SetWindowLongPtr(hDlg, DWL_USER, (LONG) data);

            // fill output format combos
            HWND hwndVectorFormats = GetDlgItem(hDlg, IDC_COMBO_VECTOR_FORMAT);
            FillComboWithOutputFormats(hwndVectorFormats, g_vectorFormats, 
                                       ARRCOUNT(g_vectorFormats));
            HWND hwndRasterFormats = GetDlgItem(hDlg, IDC_COMBO_RASTER_FORMAT);
            FillComboWithOutputFormats(hwndRasterFormats, g_rasterFormats,
                                       ARRCOUNT(g_rasterFormats));

            // select the right format
            SelectFormat(hDlg, extdmGetOutputFormat(data->m_pExtdmCurrent));

            // set default text control contents
            SendMessage(GetDlgItem(hDlg, IDC_OUTPUT_FOLDER), 
                        WM_SETTEXT, 0, (LPARAM)extdmGetOutputFolder(data->m_pExtdmCurrent));
            SendMessage(GetDlgItem(hDlg, IDC_OUTPUT_FILENAME), 
                        WM_SETTEXT, 0, (LPARAM)extdmGetOutputFilename(data->m_pExtdmCurrent));           SendMessage(GetDlgItem(hDlg, IDC_IMAGEMAGICK_OPTIONS), 
                        WM_SETTEXT, 0, (LPARAM)extdmGetConvertOptions(data->m_pExtdmCurrent));           SendMessage(GetDlgItem(hDlg, IDC_POSTGEN_CMD), 
                        WM_SETTEXT, 0, (LPARAM)extdmGetPostGenCmd(data->m_pExtdmCurrent));

            // check override box eventually
            CheckDlgButton(hDlg, IDC_OVERRIDE_CHECKBOX, 
                extdmAllowUserOverride(data->m_pExtdmCurrent) ? BST_CHECKED : BST_UNCHECKED);

            CheckDlgButton(hDlg, IDC_CROP_CHECKBOX, 
                extdmCropBorders(data->m_pExtdmCurrent) ? BST_CHECKED : BST_UNCHECKED);

            CheckDlgButton(hDlg, IDC_OPEN_VIEWER_CHECKBOX, 
                extdmViewOutputFile(data->m_pExtdmCurrent) ? BST_CHECKED : BST_UNCHECKED);
        } 
        break; // case WM_INITDIALOG

    case WM_COMMAND:
        return OnCommand(hDlg, LOWORD(wParam), HIWORD(wParam));

    case WM_NOTIFY:
        return OnNotify(hDlg, (LPNMHDR)lParam);

    default:
        //DEBUGMESSAGE(("DocPropDlgProc - unhandled message %d", message));
        return FALSE;
    }

    return TRUE;
}

static BOOL CALLBACK AboutDlgProc(
                                    HWND hDlg,
                                    UINT message,
                                    WPARAM wParam,
                                    LPARAM lParam
                                    )
{
    //DEBUGMESSAGE(("AboutDlgProc"));

    switch (message)
    {
    case WM_INITDIALOG:
        {
            DEBUGMESSAGE(("AboutDlgProc - WM_INITDIALOG"));
            LPPROPSHEETPAGE page = (LPPROPSHEETPAGE ) lParam;
    
            // attach to our property sheet page an instance of our data holder class
            CHyperLink *hyperLink = new CHyperLink();
            SetWindowLongPtr(hDlg, DWL_USER, (LONG) hyperLink);

            // transform the static text into an hyperlink!
            // it couldn't be easier - many thanks to Olivier Langlois
            hyperLink->ConvertStaticToHyperlink(
                        hDlg, IDC_PROJECT_URL,
                        TEXT("http://emfprinter.sourceforge.net"));

            // set about text
            TCHAR txt[256];
            StringCchPrintf(txt, 256, 
                TEXT("This is EmfPrinter %d.%d.%d by Francesco Montorsi (c) 2007.\n")
                TEXT("Licensed under the terms of the GNU General Public License."),
                emfVERSION_MAJOR, emfVERSION_MINOR, emfVERSION_RELEASE);
            HWND hwnd = GetDlgItem(hDlg, IDC_ABOUT_TEXT);
            SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)txt);
        } 
        break;

    default:
        //DEBUGMESSAGE(("AboutDlgProc - unhandled message %d", message));
        return FALSE;
    }

    return TRUE;
}
