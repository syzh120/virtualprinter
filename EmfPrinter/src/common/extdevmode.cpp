/*
 * extdevmode.cpp
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

#define EXTDEVMODE_C

#include "stdheaderwrapper.h"
#include "extdevmode.h"
#include "devModeV400.h"
#include "devmode.h"
#include "utils.h"

#include "globals.h"


// -----------------------------------------------------------------------------
// DATATYPES
// -----------------------------------------------------------------------------

struct PRIVDEVMODE
{
    TCHAR strOutputFormat[32];
    TCHAR strOutputFolder[512];
    TCHAR strOutputFileName[256];
    TCHAR strRasterConvOptions[256];
    TCHAR strPostGenCmd[256];
    BOOL bAllowOverride;
    BOOL bCropVectorFormat;
    BOOL bOpenOutputFile;
};

typedef PRIVDEVMODE *PPRIVDEVMODE;
typedef const PRIVDEVMODE* PCPRIVDEVMODE;

// the devmode is composed of public members and private ones
struct EXTDEVMODE
{
    DEVMODE dm;
    PRIVDEVMODE privdm;
};



// -----------------------------------------------------------------------------
// PRIVATE UTILS
// -----------------------------------------------------------------------------

//VOID extdmInit(PEXTDEVMODE pextdm);

PCPRIVDEVMODE extdmGetPrivateDM(PCEXTDEVMODE pextdm);
PPRIVDEVMODE extdmGetPrivateDM(PEXTDEVMODE pextdm);

VOID extdmSetPrivateDM(OUT PEXTDEVMODE pdmOut, IN PCPRIVDEVMODE pprivdmIn);

VOID extdmSetToDefaultPublicDM(OUT PDEVMODE pdmOut);
VOID extdmSetToDefaultPrivateDM(OUT PPRIVDEVMODE pdmOut);



// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

PEXTDEVMODE extdmNew()
{
    PEXTDEVMODE pextdm = (PEXTDEVMODE) malloc(sizeof(EXTDEVMODE));
    return pextdm;
}

VOID extdmDelete(PEXTDEVMODE pextdm)
{
    if (pextdm != NULL)
        free(pextdm);
}

DWORD extdmGetDriverExtra()
{
    return sizeof(PRIVDEVMODE);
}

DWORD extdmGetSize()
{
    return sizeof(DEVMODE);
}

DWORD extdmGetSpecVersion()
{
    return DM_SPECVERSION;
}

size_t extdmSizeof() 
{
    return sizeof(EXTDEVMODE);
}

size_t extdmSizeofPrivate() 
{
    return sizeof(PRIVDEVMODE);
}

BOOL extdmIsAnExtDevMode(IN PCDEVMODE pdm)
{
    if (pdm != NULL && pdm->dmDriverExtra == sizeof(PRIVDEVMODE))
        return TRUE;
    return FALSE;
}

VOID extdmSetToDefault(OUT PEXTDEVMODE pextdm)
{
    ZeroMemory(pextdm, sizeof(*pextdm));

    extdmSetToDefaultPublicDM(extdmGetPublicDM(pextdm));
    extdmSetToDefaultPrivateDM(extdmGetPrivateDM(pextdm));
}

PCDEVMODE extdmGetPublicDM(PCEXTDEVMODE pextdm)
{
    return (PCDEVMODE)pextdm;       // the public section comes first
}

PDEVMODE extdmGetPublicDM(PEXTDEVMODE pextdm)
{
    return (PDEVMODE)pextdm;       // the public section comes first
}

VOID extdmMerge(OUT PEXTDEVMODE pextdmOut, IN PCDEVMODE pdmIn)
{
	if (extdmIsAnExtDevMode(pdmIn))
    {
		PCPRIVDEVMODE pprivdmIn = extdmGetPrivateDM( (PCEXTDEVMODE) pdmIn);
		extdmSetPrivateDM(pextdmOut, pprivdmIn);
	}
}

VOID extdmSetPrivateData(OUT PEXTDEVMODE pextdmOut,
                         IN LPCTSTR lpFormatName,
                         IN LPCTSTR lpOutputFile,
                         IN LPCTSTR lpOutputFolder,
                         IN LPCTSTR lpRasterConvOptions,
                         IN LPCTSTR lpPostGenCommand,
                         IN INT bAllowOverride,
                         IN INT bOpenOutputFile,
                         IN INT bCrop)
{
    PPRIVDEVMODE ppriv = extdmGetPrivateDM(pextdmOut);

    // strings
    if (lpFormatName)
        strCopy(ppriv->strOutputFormat, lpFormatName, 
                strBufferSize(lpFormatName));
    if (lpOutputFolder)
        strCopy(ppriv->strOutputFolder, lpOutputFolder, 
                strBufferSize(lpOutputFolder));
    if (lpOutputFile)
        strCopy(ppriv->strOutputFileName, lpOutputFile, 
                strBufferSize(lpOutputFile));
    if (lpPostGenCommand)
        strCopy(ppriv->strPostGenCmd, lpPostGenCommand, 
                strBufferSize(lpPostGenCommand));
    if (lpRasterConvOptions)
        strCopy(ppriv->strRasterConvOptions, lpRasterConvOptions, 
                strBufferSize(lpRasterConvOptions));

    // booleans
    if (bAllowOverride != -1)
        ppriv->bAllowOverride = bAllowOverride;
    if (bCrop != -1)
        ppriv->bCropVectorFormat = bCrop;
    if (bOpenOutputFile != -1)
        ppriv->bOpenOutputFile = bOpenOutputFile;
}

VOID extdmCopyPrivateDM(PEXTDEVMODE pout, PCEXTDEVMODE pin)
{
    extdmSetPrivateDM(pout, extdmGetPrivateDM(pin));
}

LPCTSTR extdmGetOutputFolder(IN PEXTDEVMODE pextdmIn)
{
    PPRIVDEVMODE ppriv = extdmGetPrivateDM(pextdmIn);
    return ppriv->strOutputFolder;
}

LPCTSTR extdmGetOutputFilename(IN PEXTDEVMODE pextdmIn)
{
    PPRIVDEVMODE ppriv = extdmGetPrivateDM(pextdmIn);
    return ppriv->strOutputFileName;
}

LPCTSTR extdmGetOutputFormat(IN PEXTDEVMODE pextdmIn)
{
    PPRIVDEVMODE ppriv = extdmGetPrivateDM(pextdmIn);
    return ppriv->strOutputFormat;
}

LPCTSTR extdmGetConvertOptions(IN PEXTDEVMODE pextdmIn)
{
    PPRIVDEVMODE ppriv = extdmGetPrivateDM(pextdmIn);
    return ppriv->strRasterConvOptions;
}

LPCTSTR extdmGetPostGenCmd(IN PEXTDEVMODE pextdmIn)
{
    PPRIVDEVMODE ppriv = extdmGetPrivateDM(pextdmIn);
    return ppriv->strPostGenCmd;
}

BOOL extdmAllowUserOverride(IN PEXTDEVMODE pextdmIn)
{
    PPRIVDEVMODE ppriv = extdmGetPrivateDM(pextdmIn);
    return ppriv->bAllowOverride;
}

BOOL extdmCropBorders(IN PEXTDEVMODE pextdmIn)
{
    PPRIVDEVMODE ppriv = extdmGetPrivateDM(pextdmIn);
    return ppriv->bCropVectorFormat;
}

BOOL extdmViewOutputFile(IN PEXTDEVMODE pextdmIn)
{
    PPRIVDEVMODE ppriv = extdmGetPrivateDM(pextdmIn);
    return ppriv->bOpenOutputFile;
}

// private functions

static PCPRIVDEVMODE extdmGetPrivateDM(PCEXTDEVMODE pdm)
{
    PCPRIVDEVMODE pprivdm;

    ASSERT(extdmIsAnExtDevMode( (PCDEVMODE) pdm));
    
    pprivdm = (PPRIVDEVMODE) ( (PCBYTE) pdm + pdm->dm.dmSize );
    return pprivdm;
}

static PPRIVDEVMODE extdmGetPrivateDM(PEXTDEVMODE pdm)
{
    PPRIVDEVMODE pprivdm;

    ASSERT(extdmIsAnExtDevMode( (PDEVMODE) pdm));
    
    pprivdm = (PPRIVDEVMODE) ( (PCBYTE) pdm + pdm->dm.dmSize );
    return pprivdm;
}

static VOID extdmSetToDefaultPublicDM(OUT PDEVMODE pdm)
{
    StringCchCopy(pdm->dmDeviceName, 
                  ARRCOUNT(pdm->dmDeviceName), 
                  emfPRINTER_NAME);

    pdm->dmSpecVersion = DM_SPECVERSION;
    pdm->dmDriverVersion = emfDRIVER_REAL_VERSION;

    pdm->dmSize = sizeof(DEVMODEW);                     // size of public section
    pdm->dmDriverExtra = (WORD)extdmSizeofPrivate();    // size of private section

    pdm->dmFields = extdmDEFAULT_FIELDS;
    
    pdm->dmOrientation = DMORIENT_PORTRAIT;
    pdm->dmPaperSize = DMPAPER_A4;
    pdm->dmPaperLength = 0;     // we don't use it
    pdm->dmPaperWidth = 0;      // we don't use it
    pdm->dmScale = 100;         // 100% of proportion
    pdm->dmCopies = 1;
    pdm->dmDefaultSource = DMBIN_FIRST;

    pdm->dmColor = DMCOLOR_COLOR;
    pdm->dmDuplex = DMDUP_SIMPLEX;

    // see MSDN
    pdm->dmPrintQuality = emfDRIVER_WIDTH_DPI;
    pdm->dmYResolution = emfDRIVER_HEIGHT_DPI;

    pdm->dmTTOption = DMTT_BITMAP;
    pdm->dmCollate = DMCOLLATE_FALSE;
    strCopy(pdm->dmFormName, TEXT("Legal"), CCHFORMNAME);

    pdm->dmLogPixels = 0;   // unused by printers
    pdm->dmBitsPerPel = 0;   // unused by printers
    pdm->dmPelsWidth = 0;   // unused by printers
    pdm->dmPelsHeight = 0;   // unused by printers

    pdm->dmNup = DMNUP_ONEUP;
    pdm->dmDisplayFrequency = 0;   // unused by printers
    pdm->dmICMMethod = DMICMMETHOD_SYSTEM;
    pdm->dmICMIntent = DMICM_SATURATE;
    pdm->dmMediaType = DMMEDIA_STANDARD;
    pdm->dmDitherType = DMDITHER_NONE;
    pdm->dmPanningWidth = 0;        // reserved for use by the system
    pdm->dmPanningHeight = 0;        // reserved for use by the system
}

static VOID extdmSetToDefaultPrivateDM(OUT PPRIVDEVMODE pdm)
{
    LPCTSTR fmt = TEXT("EMF");
    LPCTSTR filename = TEXT("%docname%%page%");
    LPCTSTR foldername = TEXT("C:\\");

    strCopy(pdm->strOutputFormat, fmt, strBufferSize(fmt));
    strCopy(pdm->strOutputFileName, filename, strBufferSize(filename));
    strCopy(pdm->strOutputFolder, foldername, strBufferSize(foldername));

    pdm->bCropVectorFormat = TRUE;
    pdm->bOpenOutputFile = FALSE;
    pdm->bAllowOverride = TRUE;
}

static VOID extdmSetPrivateDM(OUT PEXTDEVMODE pdmOut, 
                              IN PCPRIVDEVMODE pprivdmIn)
{
    CopyMemory(pdmOut->privdm.strOutputFormat,
		       pprivdmIn->strOutputFormat,
		       sizeof(pprivdmIn->strOutputFormat));
}

