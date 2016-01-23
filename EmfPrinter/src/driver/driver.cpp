/*
 * driver.cpp
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
#include "driver.h"
#include "utils.h"
#include "debug.h"
#include "extdevmode.h"
#include "devmode.h"
#include "globals.h"

#include "spoolerdata.h"

#include "waitdlg.h"
#include "resource.h"

#include <winddi.h>
#include <malloc.h>
#include <commdlg.h>

#define SVG                 0           // work in progress!
#define MINIMAL_DRIVER      0           // useful for debugging


// -----------------------------------------------------------------------------
// DATA STRUCTURES
// -----------------------------------------------------------------------------

// PRIVATE TYPEDEFS

typedef struct _PDEV
{
    HPALETTE hPalDefault;
    HDEV hDev;
    HSURF hSurface;
    //HBITMAP hBmp;
    ULONG ulDitherFormat;

#if !MINIMAL_DRIVER
    PEXTDEVMODE pExtDM;
    LPTSTR pStrDocName;
    LPTSTR pStrPrinterName;
    SpoolerData *pSpoolerData;
#endif

#if SVG
    TCHAR strTempBaseFileName[MAX_PATH+1];
    HANDLE hCurrFile;
    DWORD nCurrPage;
#endif
} PDEV;

typedef PDEV* PPDEV;

// PRIVATE HELPERS

LPTSTR FilterDocName(IN LPCTSTR original);

BOOL AskUser(IN HWND hDlg, 
             IN LPCTSTR strDocName, 
             IN LPCTSTR strPrinterName, 
             OUT PEXTDEVMODE pdevmode);

BOOL GetSpoolFileName(OUT LPTSTR SpoolFileName, 
                      IN DWORD JobId, 
                      IN HANDLE hPrinter);

VOID SpoolerSaveCallback(LPVOID param);


// GLOBALS

HANDLE g_hModule = NULL;
//ULONG g_iEngineVersion;

static DRVFN g_RasterFuncs[] =
{
    {INDEX_DrvEnablePDEV       , (PFN)DrvEnablePDEV          },
    {INDEX_DrvCompletePDEV     , (PFN)DrvCompletePDEV        },
    {INDEX_DrvDisablePDEV      , (PFN)DrvDisablePDEV         },
    {INDEX_DrvResetPDEV        , (PFN)DrvResetPDEV           },
    {INDEX_DrvEnableSurface    , (PFN)DrvEnableSurface       },
    {INDEX_DrvDisableSurface   , (PFN)DrvDisableSurface      },
    {INDEX_DrvDisableDriver    , (PFN)DrvDisableDriver       },

    {INDEX_DrvStartBanding     , (PFN)DrvStartBanding        },
    {INDEX_DrvNextBand         , (PFN)DrvNextBand            },
    {INDEX_DrvStartDoc         , (PFN)DrvStartDoc            },
    {INDEX_DrvEndDoc           , (PFN)DrvEndDoc              },
    {INDEX_DrvStartPage        , (PFN)DrvStartPage           },
    {INDEX_DrvSendPage         , (PFN)DrvSendPage            },

    // these MUST always be implemented for device-managed surfaces
    // (see DrvEnableSurface for more details):
    {INDEX_DrvBitBlt           , (PFN)DrvBitBlt              }, // for HOOK_BITBLT
    {INDEX_DrvStrokePath       , (PFN)DrvStrokePath          }, // for HOOK_STROKEPATH
    {INDEX_DrvTextOut          , (PFN)DrvTextOut             }, // for HOOK_TEXTOUT
    {INDEX_DrvCopyBits         , (PFN)DrvCopyBits            }, // for HOOK_COPYBITS

#if SVG
    // for SVG creation we'd need to implement these!
    {INDEX_DrvAlphaBlend       , (PFN)DrvAlphaBlend          }, // for HOOK_ALPHABLEND
    {INDEX_DrvGradientFill     , (PFN)DrvGradientFill        }, // for HOOK_GRADIENTFILL
    {INDEX_DrvFillPath         , (PFN)DrvFillPath            }, // for HOOK_FILLPATH
    {INDEX_DrvStrokeAndFillPath, (PFN)DrvStrokeAndFillPath   }, // for HOOK_STROKEANDFILLPATH
    {INDEX_DrvLineTo           , (PFN)DrvLineTo              }, // for HOOK_LINETO
    {INDEX_DrvStretchBlt       , (PFN)DrvStretchBlt          }, // for HOOK_STRETCHBLT
    {INDEX_DrvStretchBltROP    , (PFN)DrvStretchBltROP       }, // for HOOK_STRETCHBLTROP
    {INDEX_DrvPlgBlt           , (PFN)DrvPlgBlt              }, // for HOOK_PLGBLT
    {INDEX_DrvSynchronize      , (PFN)DrvSynchronize         }, // for HOOK_SYNCHRONIZE
    {INDEX_DrvTransparentBlt   , (PFN)DrvTransparentBlt      }, // for HOOK_TRANSPARENTBLT
    {INDEX_DrvRealizeBrush     , (PFN)DrvRealizeBrush        },
    {INDEX_DrvCreateDeviceBitmap,(PFN)DrvCreateDeviceBitmap  },
    {INDEX_DrvDeleteDeviceBitmap,(PFN)DrvDeleteDeviceBitmap  },
    {INDEX_DrvEscape           , (PFN)DrvEscape              },
    {INDEX_DrvDrawEscape       , (PFN)DrvDrawEscape          }
#endif
};

/*
other DrvXXXX functions currently missing:

#define INDEX_DrvAssertMode                      5L
#define INDEX_DrvDitherColor                    13L
#define INDEX_DrvSetPalette                     22L
#define INDEX_DrvQueryFont                      26L
#define INDEX_DrvQueryFontTree                  27L
#define INDEX_DrvQueryFontData                  28L
#define INDEX_DrvSetPointerShape                29L
#define INDEX_DrvMovePointer                    30L
#define INDEX_DrvGetGlyphMode                   37L
#define INDEX_DrvSynchronize                    38L
#define INDEX_DrvSaveScreenBits                 40L
#define INDEX_DrvGetModes                       41L
#define INDEX_DrvFree                           42L
#define INDEX_DrvDestroyFont                    43L
#define INDEX_DrvQueryFontCaps                  44L
#define INDEX_DrvLoadFontFile                   45L
#define INDEX_DrvUnloadFontFile                 46L
#define INDEX_DrvFontManagement                 47L
#define INDEX_DrvQueryTrueTypeTable             48L
#define INDEX_DrvQueryTrueTypeOutline           49L
#define INDEX_DrvGetTrueTypeFile                50L
#define INDEX_DrvQueryFontFile                  51L
#define INDEX_DrvMovePanning                    52L
#define INDEX_DrvQueryAdvanceWidths             53L
#define INDEX_DrvSetPixelFormat                 54L
#define INDEX_DrvDescribePixelFormat            55L
#define INDEX_DrvSwapBuffers                    56L
#define INDEX_DrvGetDirectDrawInfo              59L
#define INDEX_DrvEnableDirectDraw               60L
#define INDEX_DrvDisableDirectDraw              61L
#define INDEX_DrvQuerySpoolType                 62L
#define INDEX_DrvIcmCreateColorTransform        64L
#define INDEX_DrvIcmDeleteColorTransform        65L
#define INDEX_DrvIcmCheckBitmapBits             66L
#define INDEX_DrvIcmSetDeviceGammaRamp          67L
#define INDEX_DrvPlgBlt                         70L
#define INDEX_DrvAlphaBlend                     71L
#define INDEX_DrvSynthesizeFont                 72L
#define INDEX_DrvGetSynthesizedFontFiles        73L
#define INDEX_DrvTransparentBlt                 74L
#define INDEX_DrvQueryPerBandInfo               75L
#define INDEX_DrvQueryDeviceSupport             76L
*/

#if SVG

static char g_svgHeader[] =
    "<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
    "<!-- Created with EmfPrinter (http://emfprinter.sourceforge.net/) -->\n"
    "<svg\n"
    "   xmlns:svg='http://www.w3.org/2000/svg'\n"
    "   xmlns='http://www.w3.org/2000/svg'\n"
//    "   width='744.09448819'\n"
//    "   height='1052.3622047'\n"
    "   id='svg2'>\n"
    "  <g id='group1'>";

static char g_svgFooter[] = 
    "  </g>\n"
    "</svg>\n";

#endif


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

BOOL CALLBACK DllMain(
                      HANDLE      hModule,
                      ULONG       ulReason,
                      PCONTEXT    pContext
                      )
{
    DEBUGMESSAGE(("Driver DllMain [%s]", __TIME__));

    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:
        {
            g_hModule = hModule;
            //DisableThreadLibraryCalls( (HMODULE)hModule );
            break;
        }

    case DLL_PROCESS_DETACH:
        g_hModule = NULL;
        break;
    }

    return TRUE;
}




// -----------------------------------------------------------------------------
// Drv*Driver* functions
// -----------------------------------------------------------------------------

// DrvEnableDriver function is the initial driver entry point exported by 
// the driver DLL. It fills in a DRVENABLEDATA structure with the driver's 
// graphics DDI version number and the calling addresses of all graphics 
// DDI functions supported by the driver.
BOOL CALLBACK DrvEnableDriver(
                              ULONG iEngineVersion,
                              ULONG cj,
                              PDRVENABLEDATA pded
                              )
{
    DEBUGMESSAGE(("DrvEnableDriver"));

    // DDI_DRIVER_VERSION is now out-dated. See winddi.h
    // DDI_DRIVER_VERSION_NT4 is equivalent to the old DDI_DRIVER_VERSION

    if (iEngineVersion < DDI_DRIVER_VERSION_NT4) {
        DEBUGMESSAGE(("DrvEnableDriver - the running GDI is too old!"));
        return FALSE;
    }

    if (cj < sizeof(DRVENABLEDATA)) {
        DEBUGMESSAGE(("DrvEnableDriver - the passed DRVENABLEDATA structure is too old!"));
        return FALSE;
    }

    if (pded == NULL) {
        DEBUGMESSAGE(("DrvEnableDriver - invalid DRVENABLEDATA pointer"));
        return FALSE;
    }

    //g_iEngineVersion = iEngineVersion;

    pded->iDriverVersion = DDI_DRIVER_VERSION_NT5;
    pded->c = ARRCOUNT(g_RasterFuncs); 
    pded->pdrvfn = g_RasterFuncs;

    return TRUE;
}

// DrvQueryDriverInfo function returns requested driver-specific information.
BOOL CALLBACK DrvQueryDriverInfo(
                                DWORD dwMode, 
                                PVOID pBuffer, 
                                DWORD cbBuf, 
                                PDWORD pcbNeeded
                                )
{
    DEBUGMESSAGE(("DrvQueryDriverInfo"));

    switch (dwMode)
    {
    case DRVQUERY_USERMODE:
        if (pcbNeeded)
            *pcbNeeded = sizeof(DWORD);

        if (pBuffer == NULL || cbBuf < sizeof(DWORD))
        {
            DEBUGMESSAGE(("DrvQueryDriverInfo - BUFFER TOO SMALL"));
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

		// we do execute this driver in USER MODE
        *((PDWORD) pBuffer) = TRUE;
        return TRUE;

    default:
        DEBUGMESSAGE(("DrvQueryDriverInfo - INVALID PARAMETER"));
        SetLastError(ERROR_INVALID_PARAMETER);
        break;
    }

    return FALSE;
}

// The DrvDisableDriver function is used by GDI to notify a driver that 
// it no longer requires the driver and is ready to unload it.
VOID CALLBACK DrvDisableDriver()
{
    DEBUGMESSAGE(("DrvDisableDriver"));
}




// -----------------------------------------------------------------------------
// Drv*PDEV* functions
// -----------------------------------------------------------------------------

// DrvEnablePDEV function returns a description of the physical device's 
// characteristics to GDI.
// NOTE: "NU" below stands for Not Used
DHPDEV CALLBACK DrvEnablePDEV(
        IN DEVMODEW *pdm,                   // Pointer to DEVMODE
        IN LPWSTR    pwszLogAddress /*NU*/, // Logical address
        IN ULONG     cPat /*NU*/,           // number of patterns
        OUT HSURF*   phSurfacePatterns /*NU*/, // Return standard patterns
        IN ULONG     cjCaps,                // Length of memory pointed to by pGdiInfo
        OUT ULONG    *pdevcaps,             // Pointer to GdiInfo structure
        IN ULONG     cjDevInfo,             // Length of following PDEVINFO structure
        OUT DEVINFO  *pdi,                  // physical device information structure
        IN HDEV      hDev /*NU*/,           // HDEV, used for callbacks
        IN LPWSTR    pwszDeviceName /*NU*/, // DeviceName - not used
        IN HANDLE    hDriver                // Handle to base driver
        )
{
    DEBUGMESSAGE(("DrvEnablePdev"));

    // NOTE: phSurfacePatterns don't need to be handled by us as
    //       we are a vector driver, not a raster one; DrvRealizeBrush() 
    //       will take care of brushes


    // fill GDIINFO
    // ---------------------

    if (pdevcaps == NULL) {
        DEBUGMESSAGE(("DrvEnablePdev - invalid GDIINFO pointer"));
        return NULL;
    }

    PGDIINFO pGdiInfo = (PGDIINFO) pdevcaps;
    
    ZeroMemory(pGdiInfo, sizeof(GDIINFO));

    pGdiInfo->ulVersion = emfDRIVER_REAL_VERSION;
    pGdiInfo->ulTechnology = DT_PLOTTER;        // we are a VECTOR device!!

    // following are in millimeters
    // NOTE: don't use floats as they could give problems inside a driver
    //       (see EngSaveFloatingPointState etc etc)
    int scaleFactor = pdm->dmScale;
    pGdiInfo->ulHorzSize = (int)(A4_WIDTH_MM * scaleFactor / 100);
    pGdiInfo->ulVertSize = (int)(A4_HEIGHT_MM * scaleFactor / 100);

    // in pixels
    pGdiInfo->ulHorzRes = emfDRIVER_WIDTH_PIXEL;
    pGdiInfo->ulVertRes = emfDRIVER_HEIGHT_PIXEL;

    // in dpi
    pGdiInfo->ulLogPixelsX = emfDRIVER_WIDTH_DPI;
    pGdiInfo->ulLogPixelsY = emfDRIVER_HEIGHT_DPI;

    pGdiInfo->cBitsPixel = 24;      // see also pGdiInfo->ulHTOutputFormat, 
                                    // pDevInfo->iDitherFormat
    pGdiInfo->cPlanes = 1;
  
    pGdiInfo->ulNumColors = 256;

    pGdiInfo->ulDACRed     = 0;
    pGdiInfo->ulDACGreen   = 0;
    pGdiInfo->ulDACBlue    = 0;
    pGdiInfo->flRaster     = 0;
    pGdiInfo->flTextCaps   = 0;
    pGdiInfo->xStyleStep   = 1;
    pGdiInfo->yStyleStep   = 1;

    // Assume the device has a 1:1 aspect ratio
    pGdiInfo->ulAspectX    = 10;
    pGdiInfo->ulAspectY    = 10;
    pGdiInfo->ulAspectXY   = 14;        // sqrt(10^2 + 10^2) = 14

    pGdiInfo->denStyleStep = 10;
    
    pGdiInfo->ptlPhysOffset.x = 0;
    pGdiInfo->ptlPhysOffset.y = 0;

    pGdiInfo->szlPhysSize.cx = pGdiInfo->ulHorzRes;
    pGdiInfo->szlPhysSize.cy = pGdiInfo->ulVertRes;
    
    pGdiInfo->ulNumPalReg = 0;
    
    COLORINFO ciDevice = 
    {
        { 6810, 3050,     0 },  // xr, yr, Yr
        { 2260, 6550,     0 },  // xg, yg, Yg
        { 1810,  500,     0 },  // xb, yb, Yb
        { 2000, 2450,     0 },  // xc, yc, Yc
        { 5210, 2100,     0 },  // xm, ym, Ym
        { 4750, 5100,     0 },  // xy, yy, Yy
        { 3324, 3474, 10000 },  // xw, yw, Yw

        10000,                  // R gamma
        10000,                  // G gamma
        10000,                  // B gamma

        1422,  952,             // M/C, Y/C
        787,  495,              // C/M, Y/M
        324,  248               // C/Y, M/Y
    };

    pGdiInfo->ciDevice = ciDevice;
    
    pGdiInfo->ulDevicePelsDPI = pGdiInfo->ulLogPixelsX;
    
    pGdiInfo->ulHTOutputFormat = HT_FORMAT_24BPP;
    pGdiInfo->ulHTPatternSize =  HT_PATSIZE_16x16_M;
    pGdiInfo->flHTFlags = HT_FLAG_HAS_BLACK_DYE;
    pGdiInfo->ulPrimaryOrder = PRIMARY_ORDER_ABC;

    // ignored stuff which must be set to zero:

    pGdiInfo->ulVRefresh = 0;
    pGdiInfo->ulBltAlignment = 0;
    
    pGdiInfo->ulPanningHorzRes = 0;
    pGdiInfo->ulPanningVertRes = 0;
    pGdiInfo->xPanningAlignment = 0;
    pGdiInfo->yPanningAlignment = 0;
    
    pGdiInfo->cxHTPat = 0;
    pGdiInfo->cyHTPat = 0;
    pGdiInfo->pHTPatA = NULL;
    pGdiInfo->pHTPatB = NULL;
    pGdiInfo->pHTPatC = NULL;
    pGdiInfo->flShadeBlend = SB_GRAD_RECT;

    pGdiInfo->ulPhysicalPixelCharacteristics = PPC_DEFAULT;
    pGdiInfo->ulPhysicalPixelGamma = PPG_DEFAULT;


    // fill the DEVINFO
    // -------------------------

    if (pdi == NULL) {
        DEBUGMESSAGE(("DrvEnablePdev - invalid DEVINFO pointer"));
        return NULL;
    }

    PDEVINFO pDevInfo = (PDEVINFO) pdi;

    ZeroMemory(pDevInfo, sizeof(DEVINFO));

    pDevInfo->iDitherFormat = BMF_24BPP;

    // we _must_ create the "palette" otherwise the driver initialization will fail
    // Note that specifying PAL_BGR we inform GDI that this device accepts RGB colors 
    // directly, with B (blue) as the least significant byte and does not really use
    // an indexed palette.
    pDevInfo->hpalDefault = EngCreatePalette(PAL_BGR, 0, 0, 0, 0, 0);

#if MINIMAL_DRIVER
    pDevInfo->flGraphicsCaps = 0;
#else
    pDevInfo->flGraphicsCaps =
        GCAPS_ALTERNATEFILL     |
        GCAPS_HORIZSTRIKE       |
        GCAPS_VERTSTRIKE        |
        GCAPS_VECTORFONT        | 
        GCAPS_HALFTONE          | 
        GCAPS_OPAQUERECT        |
        GCAPS_BEZIERS           |
        GCAPS_WINDINGFILL;
#endif


    // fill the PDEV
    // -------------------------

    // alloc our custom PDEV structure
    PPDEV ppdev = (PPDEV) malloc(sizeof(*ppdev));
    if (ppdev == NULL) {
        DEBUGMESSAGE(("OUT OF MEMORY"));
        return NULL;
    }

    ZeroMemory(ppdev, sizeof(*ppdev));

#if !MINIMAL_DRIVER

    ppdev->ulDitherFormat = pDevInfo->iDitherFormat;
    ppdev->hPalDefault = pDevInfo->hpalDefault;

    ppdev->pExtDM = extdmNew();
    if (extdmIsAnExtDevMode(pdm))
    {
        // copy the ext DM to the user's DM
        memcpy(ppdev->pExtDM, pdm, extdmSizeof());
    }
    else
    {
        DEBUGMESSAGE(("DrvEnablePdev - the given DEVMODE is not an extended "
                      "DEVMODE! Falling back to the defaults"));
        extdmSetToDefault(ppdev->pExtDM);
    }

    ppdev->pStrPrinterName = strDuplicate(extdmGetPublicDM(ppdev->pExtDM)->dmDeviceName);

    // note: ppdev->hDev will be set in DrvCompletePDEV
    //       ppdev->hSurface will be set in DrvEnableSurface
    //       ppdev->pStrDocName will be set in DrvStartDoc
    //       ppdev->pSpoolerData will be set in DrvStartDoc
#endif

    return (DHPDEV) ppdev;
}

// DrvCompletePDEV function stores the GDI handle of the physical device being created. 
VOID CALLBACK DrvCompletePDEV(DHPDEV dhpdev, HDEV hDev)
{
    DEBUGMESSAGE(("DrvCompletePDEV"));

    PPDEV ppdev = (PPDEV) dhpdev;
    if (ppdev)
        ppdev->hDev = hDev;
}

// DrvDisablePDEV function is used by GDI to notify a driver that the 
// specified PDEV is no longer needed.
// NOTE: THIS ROUTINE IS ALSO CALLED INTERNALLY FROM DrvEnablePDEV IN 
//       CASE OF FAILURE TO FREE ALLOCATED RESOURCES BEFORE RETURNING NULL
VOID CALLBACK DrvDisablePDEV(DHPDEV dhpdev)
{
    DEBUGMESSAGE(("DrvDisablePDEV"));

#if !MINIMAL_DRIVER
    PPDEV ppdev = (PPDEV) dhpdev;
    if (ppdev == NULL)
        return;

    if (ppdev->hPalDefault != NULL) {
        EngDeletePalette(ppdev->hPalDefault);
        ppdev->hPalDefault = NULL;
    }

    if (ppdev->pStrDocName)
        strFree(ppdev->pStrDocName);

    if (ppdev->pStrPrinterName)
        strFree(ppdev->pStrPrinterName);

    if (ppdev->pExtDM)
        free(ppdev->pExtDM);

    if (ppdev->pSpoolerData)
        delete ppdev->pSpoolerData;

    // at last free the PPDEV structure itself
    free(ppdev);
#endif
}

// The DrvResetPDEV function allows a graphics driver to transfer the 
// state of the driver from an old PDEV structure to a new PDEV structure 
// when a Win32 application calls ResetDC.
BOOL  CALLBACK DrvResetPDEV(DHPDEV dhpdevOld, DHPDEV dhpdevNew)
{
    DEBUGMESSAGE(("DrvResetPDEV"));

    PPDEV ppold = (PPDEV) dhpdevOld;
    PPDEV ppnew = (PPDEV) dhpdevNew;
    if (ppold == NULL || ppnew == NULL)
        return FALSE;

#if !MINIMAL_DRIVER
    try {
        if (!ppnew->pSpoolerData && ppold->pSpoolerData)
            ppnew->pSpoolerData = new SpoolerData(*ppold->pSpoolerData);
    } catch(...) {
		DEBUGMESSAGE_LASTERROR(("DrvResetPDEV - malloc failed"));
        return FALSE;
	}

    // copy doc name if required

    if (ppnew->pStrDocName && strLen(ppnew->pStrDocName) == 0) {
        strFree(ppnew->pStrDocName);
        ppnew->pStrDocName = NULL;
    }
    if (!ppnew->pStrDocName && ppold->pStrDocName)
        ppnew->pStrDocName = strDuplicate(ppold->pStrDocName);

    // copy printer name if required

    if (ppnew->pStrPrinterName && strLen(ppnew->pStrPrinterName) == 0) {
        strFree(ppnew->pStrPrinterName);
        ppnew->pStrPrinterName = NULL;
    }
    if (!ppnew->pStrPrinterName && ppold->pStrPrinterName)
        ppnew->pStrPrinterName = strDuplicate(ppold->pStrPrinterName);

    // copy ext DM if required

    if (!ppnew->pExtDM && ppold->pExtDM)
    {
        ppnew->pExtDM = extdmNew();
        memcpy(ppnew->pExtDM, ppold->pExtDM, extdmSizeof());
    }
    else if (ppold->pExtDM)
        // always copy our private extended part of the DM...
        extdmCopyPrivateDM(ppnew->pExtDM, ppold->pExtDM);
#endif

    return TRUE;
}



// -----------------------------------------------------------------------------
// other Drv* functions
// -----------------------------------------------------------------------------

BOOL CALLBACK DrvStartDoc(SURFOBJ *pso, LPWSTR pwszDocName, DWORD dwJobId)
{
    DEBUGMESSAGE(("DrvStartDoc"));

#if !MINIMAL_DRIVER
    // as DDK says, when pwszDocName == NULL, it means that a ResetDC()
    // has been called and this is not a real new document...
    if (!pwszDocName)
        return TRUE;        // ignore it

    if (!pso)
    {
        DEBUGMESSAGE(("DrvStartDoc - invalid SURFOBJ!"));
        return FALSE;
    }

    // get our ext devmode
    PPDEV ppdev = ((PPDEV)pso->dhpdev);
    if (!ppdev ||
        !extdmIsAnExtDevMode((PCDEVMODE)ppdev->pExtDM) ||
        !ppdev->pStrPrinterName)
    {
        DEBUGMESSAGE(("DrvStartDoc - invalid EXTDEVMODE or printer name!"));
        return FALSE;
    }

    // should we ask user for output filename/folder/format?
    if (extdmAllowUserOverride(ppdev->pExtDM))
    {
        if (!AskUser(GetForegroundWindow(), pwszDocName, 
                     ppdev->pStrPrinterName, ppdev->pExtDM)) {
            DEBUGMESSAGE(("DrvStartDoc - could not ask"));
            return FALSE;
        }
    }

    // save doc name
    if (pwszDocName && strLen(pwszDocName) > 0)
        ppdev->pStrDocName = strDuplicate(pwszDocName);

    const TCHAR atstrJob[] = TEXT(",Job ");
    DWORD dwcChBuff = lstrlen(ppdev->pStrPrinterName) + 
                      lstrlen(atstrJob) + 
                      MAX_PRINTED_DIGITS(dwJobId) + 1;

    LPTSTR lptstrPrinterJobName = strAlloc(dwcChBuff);
    if (lptstrPrinterJobName == NULL) {
        DEBUGMESSAGE(("DrvStartDoc - malloc failed"));
        return FALSE;
    }

    if (::StringCchPrintf(lptstrPrinterJobName, dwcChBuff, 
                          TEXT("%ws%ws%d"), 
                          ppdev->pStrPrinterName, atstrJob, dwJobId) != S_OK) {
        DEBUGMESSAGE(("DrvStartDoc - StringCchPrintf failed"));
        free(lptstrPrinterJobName);
        return FALSE;
    }

    // open printer for reading its data
    HANDLE hPrinter;
    if (::OpenPrinter(lptstrPrinterJobName, &hPrinter, NULL) == FALSE) {
        DEBUGMESSAGE(("DrvStartDoc - OpenPrinter failed"));
        free(lptstrPrinterJobName);
        return FALSE;
    }

    // not needed anymore...
    free(lptstrPrinterJobName);

    // now load into in-memory buffer the spooler file for this job;
    // the spooler file is basically EMF surrounded by some special records
    TCHAR spoolFile[MAX_PATH+1];
    if (GetSpoolFileName(spoolFile, dwJobId, hPrinter))
    {
        // NOTE: we cannot read directly spoolFile as it's in use by
        //       the spooler and it does not allow us read-access!
        //       thus we first copy it to a temporary file...
        TCHAR tempFile[MAX_PATH+1] = { L'\0' };
        if (!GetFullTempFileName(tempFile, MAX_PATH+1)) {
            DEBUGMESSAGE(("DrvStartDoc - could not get temp file name"));
            return FALSE;
        }

        if (!CopyFile(spoolFile, tempFile, FALSE)) {
            DEBUGMESSAGE(("DrvStartDoc - could not copy spooler data in a temp file"));
            return FALSE;
        }

        ppdev->pSpoolerData = new SpoolerData;
        if (!ppdev->pSpoolerData->LoadSpoolerFile(tempFile)) {
            DEBUGMESSAGE(("DrvStartDoc - LoadSpoolerFile failed"));
            return FALSE;
        }

        if (!DeleteFile(tempFile)) {
            DEBUGMESSAGE(("DrvStartDoc - could not remove the temp file"));
            return FALSE;
        }
    }
    else
    {
        // if we couldn't get the spooler data directly reading the file,
        // we can read the printer; this would be completely equivalent but
        // is less safe since ReadPrinterData() will need to constantly
        // reallocate its internal buffer...
        //
        // WARNING: sometimes using this method my whole system crashes
        //          and reboots!!!!! FIXME FIXME
        //
        ppdev->pSpoolerData = new SpoolerData;
        if (!ppdev->pSpoolerData->ReadPrinterData(hPrinter)) {
            DEBUGMESSAGE(("DrvStartDoc - ReadPrinterData failed"));
            return FALSE;
        }
    }

    ::ClosePrinter(hPrinter);
#endif

    return TRUE;
}

BOOL CALLBACK DrvEndDoc(SURFOBJ *pso, FLONG fl)
{
    DEBUGMESSAGE(("DrvEndDoc"));

#if !MINIMAL_DRIVER
    if (!pso)
    {
        DEBUGMESSAGE(("DrvEndDoc - invalid SURFOBJ structure!"));
        return FALSE;
    }

    // get our ext devmode
    PPDEV ppdev = ((PPDEV)pso->dhpdev);
    if (!ppdev ||
        !extdmIsAnExtDevMode((PCDEVMODE)ppdev->pExtDM) ||
        !ppdev->pSpoolerData ||
        !ppdev->pStrDocName)
    {
        DEBUGMESSAGE(("DrvEndDoc - invalid PPDEV structure!"));
        return FALSE;
    }

    // filter the docname
    LPTSTR docName = FilterDocName(ppdev->pStrDocName);
    if (!docName) {
        DEBUGMESSAGE(("DrvEndDoc - invalid docname"));

        ErrorMessage(NULL, TEXT("EmfPrinter"), 
                TEXT("Invalid document name."));

        return FALSE;
    }

    // the output folder should always end with a backslash
    LPCTSTR orig = extdmGetOutputFolder(ppdev->pExtDM);
    LPTSTR folder = NULL;
    if (orig[strLen(orig)-1] == TEXT('\\'))
        folder = strDuplicate(orig);
    else
        folder = strCat(orig, TEXT("\\"));

    // the output format
    LPTSTR fmt = strDuplicate(extdmGetOutputFormat(ppdev->pExtDM));
    fmt = _tcslwr(fmt);

    // the path to imageMagick utilities etc etc
    LPTSTR fullPath = GetFullAppPath();

    // show a work-in-progress window
    WaitDialogWindow *pwd = 
        new WaitDialogWindow((HINSTANCE)g_hModule);

    // save images
    MSG msg;
    BOOL bRet;
    SpoolerData *data = ppdev->pSpoolerData;
    for (unsigned int i=0; i < data->GetPageCount(); i++)
    {
        pwd->UpdateStatus(i, ppdev->pStrPrinterName);
        pwd->Pulse();

        // process messages for the wait window until there are messages...
        while (PeekMessage( &msg, pwd->GetDlgHwnd(), 0, 0, PM_NOREMOVE) != 0)
        {
            if (GetMessage( &msg, pwd->GetDlgHwnd(), 0, 0 ) != -1)
            { 
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            }
        }

        LPTSTR fileName = GetFileName(extdmGetOutputFilename(ppdev->pExtDM),
                                      docName, i);

        // get full output file name
        TCHAR strFullFileName[MAX_PATH];
        if (StringCchPrintf(strFullFileName, ARRCOUNT(strFullFileName), 
                            TEXT("%ws%ws.%ws"), 
                            folder, 
                            fileName,
                            fmt) != S_OK) {
            DEBUGMESSAGE(("DrvEndDoc - StringCchPrintf failed for '%ws'", 
                          docName));
            strFree(fileName);
            continue; // skip this
        }

        strFree(fileName);

        DEBUGMESSAGE(("DrvEndDoc - going to save %d-th page in '%ws'", 
                      i, strFullFileName));


        // save file
        if (!data->Save(i, strFullFileName, fmt, 
                        extdmGetConvertOptions(ppdev->pExtDM),
                        (EXECPROC)SpoolerSaveCallback, pwd)) {
            DEBUGMESSAGE(("DrvEndDoc - could not save as '%ws'", strFullFileName));

            LPTSTR temp = 
                strCat(
                    TEXT("Could not save as '"),
                    strFullFileName,
                    TEXT("' the printer output!\nInternal error.\n"),
                    TEXT("Please submit a bug report at http://emfprinter.sourceforge.net"),
                    NULL);

            ErrorMessage(NULL, TEXT("EmfPrinter"), temp);
            strFree(temp);
            return FALSE;
        }

        // crop it?
        // NOTE: fmt was previously "to-lowered"
        if ((_tcscmp(fmt, TEXT("emf")) == 0 ||
             _tcscmp(fmt, TEXT("wmf")) == 0) &&
            extdmCropBorders(ppdev->pExtDM))
        {
            DEBUGMESSAGE(("DrvEndDoc - cropping borders"));

            LPTSTR cmdLine = strCat(TEXT("\""), fullPath, 
                                    TEXT("emfutils.exe\" --quiet crop \""),
                                    strFullFileName, 
                                    TEXT("\""),
                                    NULL);

            BOOL bExec = ExecuteProgram(cmdLine, 
                                        FALSE /* console app */,
                                        TRUE /* do wait */);
            strFree(cmdLine);

            if (!bExec) {
                ErrorMessage(NULL, TEXT("EmfPrinter"), 
                        TEXT("Could not crop the output file!"));
                continue; // skip it
            }
        }

        // view it?
        if (extdmViewOutputFile(ppdev->pExtDM))
        {
            DEBUGMESSAGE(("DrvEndDoc - launching file viewer"));

            LPTSTR cmdLine = strCat(TEXT("\""), fullPath, 
                                    TEXT("imdisplay.exe\" \""),
                                    strFullFileName, 
                                    TEXT("\""),
                                    NULL);

            BOOL bExec = ExecuteProgram(cmdLine, 
                                        FALSE /* console app */,
                                        FALSE /* don't wait */);
            strFree(cmdLine);

            if (!bExec) {
                ErrorMessage(NULL, TEXT("EmfPrinter"), 
                        TEXT("Could not display the output file!"));
                continue; // skip it
            }
        }

        // exec a postgen cmd?
        if (strLen(extdmGetPostGenCmd(ppdev->pExtDM)) > 0)
        {
            DEBUGMESSAGE(("DrvEndDoc - executing the postgen cmds"));

            LPTSTR cmdLine = strAlloc(MAX_PATH*2);
            StringCchCopy(cmdLine, MAX_PATH*2, extdmGetPostGenCmd(ppdev->pExtDM));
            strReplace(cmdLine, MAX_PATH*2, TEXT("%outfile%"), strFullFileName);

            BOOL bExec = ExecuteProgram(cmdLine, 
                                        FALSE /* console app */,
                                        FALSE /* don't wait */);
            strFree(cmdLine);

            if (!bExec) {
                ErrorMessage(NULL, TEXT("EmfPrinter"), 
                        TEXT("Could not execute the post-generation custom command."));
                continue; // skip it
            }
        }
    }

    // cleanup
    strFree(folder);
    strFree(docName);
    strFree(fmt);
    strFree(fullPath);
    delete pwd;
#endif

    return TRUE;
}

// DrvEnableSurface function sets up a surface to be drawn on and 
// associates it with a given physical device.
HSURF CALLBACK DrvEnableSurface(DHPDEV dhpdev)
{
#if 1
    DEBUGMESSAGE(("DrvEnableSurface"));
    
    PPDEV ppdev = (PPDEV) dhpdev;
    SIZEL sizeSurface;
   
    sizeSurface.cx = emfDRIVER_WIDTH_PIXEL;//ppdev->pGDIInfo.ulHorzRes;
    sizeSurface.cy = emfDRIVER_HEIGHT_PIXEL;//ppdev->pGDIInfo.ulVertRes;

/*
    VOID *mem = EngAllocUserMem(sizeSurface.cy*10 + 1000, 'PfmE');
    if (mem == NULL) {
        DEBUGMESSAGE(("DrvEnableSurface - not enough memory"));
        return NULL;
    }

    EngFreeUserMem(mem);
*/

    if (ppdev->ulDitherFormat == 0)
        ppdev->ulDitherFormat = BMF_24BPP;

    ppdev->hSurface = 
        EngCreateDeviceSurface((DHSURF) ppdev, 
                                sizeSurface, 
                                ppdev->ulDitherFormat);

    if (ppdev->hSurface == NULL) {
        DEBUGMESSAGE(("DrvEnableSurface - EngCreateDeviceSurface failed"));
        return NULL;
    }

    // which GDI functions should we hook?
    // VERY IMPORTANT: if we don't hook these functions they won't be called
    LONG flHooks =
        HOOK_BITBLT |           // DrvBitBlt
        HOOK_STROKEPATH |       // DrvStrokePath
        HOOK_TEXTOUT |           // DrvTextOut
        HOOK_COPYBITS;

    BOOL b = EngAssociateSurface( 
                                ppdev->hSurface, 
                                ppdev->hDev,
                                flHooks
                                );
    if (!b) {
        DEBUGMESSAGE(("DrvEnableSurface - EngAssociateSurface failed"));
        EngDeleteSurface(ppdev->hSurface);
        ppdev->hSurface = NULL;
    }

    return ppdev->hSurface;
#else
    DEBUGMESSAGE(("DrvEnableSurface"));

    PPDEV ppdev = (PPDEV) dhpdev;
    if (!ppdev) {
        DEBUGMESSAGE(("DrvEnableSurface - invalid DHPDEV"));
        return NULL;
    }

    SIZEL sizeSurface;
    
    sizeSurface.cx = emfDRIVER_WIDTH_PIXEL;//ppdev->pGDIInfo.ulHorzRes;
    sizeSurface.cy = emfDRIVER_HEIGHT_PIXEL;//ppdev->pGDIInfo.ulVertRes;

    if (ppdev->ulDitherFormat == 0)
        ppdev->ulDitherFormat = BMF_24BPP;

    /*if (ppdev->hSurface) {
        DEBUGMESSAGE(("DrvEnableSurface - surface already exists"));
        EngDeleteSurface(ppdev->hSurface);
        ppdev->hSurface = NULL;
    }*/

    LONG lWidth;        // length of a bitmap line in bytes
    switch (ppdev->ulDitherFormat)
    {
    case BMF_4BPP:
        lWidth = sizeSurface.cx/2;
        break;
    case BMF_8BPP:
        lWidth = sizeSurface.cx;
        break;
    case BMF_16BPP:
        lWidth = sizeSurface.cx*2;
        break;
    case BMF_24BPP:
        lWidth = sizeSurface.cx*3;
        break;
    case BMF_32BPP:
        lWidth = sizeSurface.cx*4;
        break;
    }

    // alloc memory
    VOID *mem = EngAllocUserMem(sizeSurface.cy*lWidth + 1000, 'PfmE');
    if (mem == NULL) {
        DEBUGMESSAGE(("DrvEnableSurface - not enough memory"));
        return NULL;
    }
/*
    // create the bitmap
    ppdev->hBmp = 
        EngCreateBitmap(sizeSurface, lWidth, ppdev->ulDitherFormat, 
                        0, mem);
*/

    // create a device-managed surface associated with our PPDEV
    ppdev->hSurface = 
        EngCreateDeviceSurface((DHSURF) ppdev, sizeSurface, ppdev->ulDitherFormat);

    if (ppdev->hSurface == NULL) {
        DEBUGMESSAGE(("DrvEnableSurface - EngCreateDeviceSurface of size %d;%d failed",
                      sizeSurface.cx, sizeSurface.cy));
        return NULL;
    }

    // which GDI functions should we hook?
    // VERY IMPORTANT: if we don't hook these functions they won't be called
    FLONG flHooks;

    // VERY IMPORTANT: I found "empirically" that the following
    //                 three flags are the ones _required_ for
    //                 the driver to work!

    // from DDK "Surface types":
    // An opaque device-managed surface is one for which GDI has 
    // neither any information about the bitmap format nor a reference 
    // to the bits in the bitmap. For these reasons, the driver must 
    // support, at minimum, the DrvBitBlt, DrvTextOut, and DrvStrokePath 
    // functions. The type of such a surface is STYPE_DEVBITMAP.

    flHooks =
        HOOK_BITBLT |           // DrvBitBlt
        HOOK_STROKEPATH |       // DrvStrokePath
        HOOK_TEXTOUT;           // DrvTextOut

#if SVG
    flHooks |=
        HOOK_ALPHABLEND |       // DrvAlphaBlend
        HOOK_COPYBITS |         // DrvCopyBits
        HOOK_FILLPATH |         // DrvFillPath
        HOOK_GRADIENTFILL |     // DrvGradientFill
        HOOK_LINETO |           // DrvLineTo
        HOOK_PLGBLT |           // DrvPlgBlt
        HOOK_STRETCHBLT |       // DrvStretchBlt
        HOOK_STRETCHBLTROP |    // DrvStretchBltROP
        HOOK_STROKEANDFILLPATH| // DrvStrokeAndFillPath
        HOOK_SYNCHRONIZE |      // DrvSynchronize
        HOOK_TRANSPARENTBLT;    // DrvTransparentBlt
#endif
/*
    BOOL b = EngAssociateSurface( 
                                ppdev->hBmp, 
                                ppdev->hDev,
                                flHooks
                                );
  */                              

    // convert this device-managed surface from opaque to non-opaque
    // so that GDI can perform all the tasks it needs on it:
    //flHooks =0;
    BOOL b = EngModifySurface(ppdev->hSurface, ppdev->hDev, flHooks, 
                              0, NULL /* dhsurf member */, mem, lWidth, NULL);

    if (!b) {
        DEBUGMESSAGE(("DrvEnableSurface - EngAssociateSurface failed"));
        EngDeleteSurface(ppdev->hSurface);
        ppdev->hSurface = NULL;
    }

    // REMEMBER:
    // After DrvEnableSurface returns a handle to a primary surface, 
    // do not call EngModifySurface or EngAssociateSurface on that handle. 
    // Doing so can cause a bug check in certain circumstances. For more 
    // information, see  Microsoft Knowledge Base article 330248:
    // http://go.microsoft.com/fwlink/?LinkId=3100&ID=330248

    return ppdev->hSurface;
#endif
}

// DrvDisableSurface function is used by GDI to notify a driver that
// the surface created by DrvEnableSurface for the current device is no longer needed.
VOID CALLBACK DrvDisableSurface(DHPDEV dhpdev)
{
    DEBUGMESSAGE(("DrvDisableSurface"));

    PPDEV ppdev = (PPDEV) dhpdev;
    if (!ppdev) {
        DEBUGMESSAGE(("DrvDisableSurface - invalid DHPDEV"));
        return;
    }

    if (ppdev->hSurface) {
        EngDeleteSurface( ppdev->hSurface );
        ppdev->hSurface = NULL;
    }
}

BOOL CALLBACK DrvStartPage(SURFOBJ *pso)
{
    DEBUGMESSAGE(("DrvStartPage"));

#if SVG
    PPDEV ppdev = ((PPDEV)pso->dhpdev);

    TCHAR strFileName[MAX_PATH+1];
    StringCchPrintf(strFileName, MAX_PATH+1, TEXT("%s_page%d"), 
                    ppdev->strTempBaseFileName, ppdev->nCurrPage);

    // create a new file
	ppdev->hCurrFile = ::CreateFile( 
		                        strFileName,
		                        GENERIC_WRITE,           
		                        FILE_SHARE_READ,        
		                        NULL,                   
		                        CREATE_ALWAYS,          
		                        FILE_ATTRIBUTE_NORMAL,  
		                        NULL                     
		                        );

	if (ppdev->hCurrFile == INVALID_HANDLE_VALUE) {
        DEBUGMESSAGE_LASTERROR(("DrvStartPage - CANNOT CREATE FILE '%ws'",
                                strFileName));
        return FALSE;
    }

    // write the header inside it (except for the final NULL)
    if (!WriteFile(ppdev->hCurrFile, &g_svgHeader, sizeof(g_svgHeader)-1)) {
        DEBUGMESSAGE_LASTERROR(("DrvStartPage - COULD NOT WRITE INTO '%ws'",
                                strFileName));
        return FALSE;
    }

    ppdev->nCurrPage++;
#endif

    return TRUE;
}

BOOL CALLBACK DrvSendPage(SURFOBJ *pso)
{
    DEBUGMESSAGE(("DrvSendPage"));

#if SVG
    PPDEV ppdev = ((PPDEV)pso->dhpdev);

    // write the footer inside it (except for the final NULL)
    if (!WriteFile(ppdev->hCurrFile, &g_svgFooter, sizeof(g_svgFooter)-1)) {
        DEBUGMESSAGE_LASTERROR(("DrvSendPage - COULD NOT WRITE INTO '%ws'",
                                strFileName));
        return FALSE;
    }

    CloseHandle(ppdev->hCurrFile);
#endif

    return TRUE;
}

// DrvStartBanding function is called by GDI when it is ready to start 
// sending bands of a physical page to the driver for rendering.
BOOL CALLBACK DrvStartBanding( SURFOBJ *pso, POINTL *pptl )
{
     DEBUGMESSAGE(("DrvStartBanding"));

     // this function should never be called since EngMarkBandingSurface
     // was not called inside DrvEnableSurface

     return TRUE;
}

// DrvNextBand function is called by GDI when it has finished drawing 
// a band for a physical page, so the driver can send the next band to the printer.
BOOL CALLBACK DrvNextBand( SURFOBJ *pso, POINTL *pptl )
{
     DEBUGMESSAGE(("DrvNextBand"));

     // this function should never be called since EngMarkBandingSurface
     // was not called inside DrvEnableSurface

     return TRUE;
}

BOOL CALLBACK DrvStrokePath(
                            SURFOBJ   *pso,
                            PATHOBJ   *ppo,
                            CLIPOBJ   *pco,
                            XFORMOBJ  *pxo,
                            BRUSHOBJ  *pbo,
                            POINTL    *pptlBrushOrg,
                            LINEATTRS *plineattrs,
                            MIX        mix
                            )
{
    DEBUGMESSAGE(("DrvStrokePath"));

    return TRUE;
}

BOOL CALLBACK DrvBitBlt(SURFOBJ *psoTrg,    SURFOBJ  *psoSrc,   SURFOBJ  *psoMask,
                        CLIPOBJ *pco,       XLATEOBJ *pxlo,     RECTL    *prclTrg,
                        POINTL  *pptlSrc,   POINTL   *pptlMask, BRUSHOBJ *pbo,
                        POINTL  *pptlBrush, ROP4 rop4)
{
    DEBUGMESSAGE(("DrvBitBlt"));

    return TRUE;
}

BOOL CALLBACK DrvCopyBits(
                          SURFOBJ  *psoDest,
                          SURFOBJ  *psoSrc,
                          CLIPOBJ  *pco,
                          XLATEOBJ *pxlo,
                          RECTL    *prclDest,
                          POINTL   *pptlSrc
                          )
{
    DEBUGMESSAGE(("DrvCopyBits"));

    return TRUE;
}

BOOL CALLBACK DrvTextOut(
                        SURFOBJ  *pso,
                        STROBJ   *pstro,
                        FONTOBJ  *pfo,
                        CLIPOBJ  *pco,
                        RECTL    *prclExtra,
                        RECTL    *prclOpaque,
                        BRUSHOBJ *pboFore,
                        BRUSHOBJ *pboOpaque,
                        POINTL   *pptlOrg,
                        MIX       mix
                        )
{
    DEBUGMESSAGE(("DrvTextOut"));

#if SVG
    HANDLE hFile = ((PPDEV)pso->dhpdev)->hCurrFile;
    if (!pstro || !pstro->pwszOrg || !hFile)
        return FALSE;

    // build the string to print
    WCHAR temp[256];
    CopyMemory(temp, pstro->pwszOrg, pstro->cGlyphs * sizeof(WCHAR));
    temp[pstro->cGlyphs] = TEXT('\0');

    <text
       xml:space="preserve"
       style="font-size:12px;font-style:normal;font-weight:normal;fill:#000000;fill-opacity:1;stroke:none;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1;font-family:Bitstream Vera Sans"
       x="85.714287"
       y="380.93362"
       id="text"><tspan
         sodipodi:role="line"
         id="tspan2180"
         x="85.714287"
         y="380.93362">asdf</tspan></text>

    if (!WriteFile(hFile, &g_svgFooter, sizeof(g_svgFooter)-1))
        return FALSE;

    DEBUGMESSAGE(("DrvTextOut - writing '%ws'", temp));

#endif

    return TRUE;
}




// -----------------------------------------------------------------------------
// SVG-only Drv* functions
// -----------------------------------------------------------------------------

#if SVG

BOOL CALLBACK DrvRealizeBrush(BRUSHOBJ *pbo, SURFOBJ *psoTarget,
                              SURFOBJ  *psoPattern, SURFOBJ *psoMask,
                              XLATEOBJ *pxlo, ULONG iHatch)
{
    DEBUGMESSAGE(("DrvRealizeBrush"));

    return TRUE;
}

BOOL CALLBACK DrvFillPath(SURFOBJ  *pso,PATHOBJ  *ppo,CLIPOBJ  *pco,
                          BRUSHOBJ *pbo,POINTL   *pptlBrushOrg,MIX  mix,
                          FLONG     flOptions)
{
    DEBUGMESSAGE(("DrvFillPath"));

    return TRUE;
}


BOOL CALLBACK DrvStrokeAndFillPath(SURFOBJ   *pso,PATHOBJ   *ppo,
                                   CLIPOBJ   *pco,XFORMOBJ  *pxo,
                                   BRUSHOBJ  *pboStroke,
                                   LINEATTRS *plineattrs,
                                   BRUSHOBJ  *pboFill,
                                   POINTL    *pptlBrushOrg,
                                   MIX        mixFill,
                                   FLONG      flOptions)
{
    DEBUGMESSAGE(("DrvStrokeAndFillPath"));

    return TRUE;
}

BOOL CALLBACK DrvLineTo(SURFOBJ *pso, CLIPOBJ *pco,        BRUSHOBJ  *pbo,
                        LONG     x1,  LONG     y1,         LONG       x2,
                        LONG     y2,  RECTL   *prclBounds, MIX        mix)
{
    DEBUGMESSAGE(("DrvLineTo"));

    return TRUE;
}

HBITMAP CALLBACK DrvCreateDeviceBitmap(
                                        DHPDEV  dhpdev,
                                        SIZEL  sizl,
                                        ULONG  iFormat
                                        )
{
    DEBUGMESSAGE(("DrvCreateDeviceBitmap"));

    return 0;
}

VOID CALLBACK DrvDeleteDeviceBitmap(DHSURF  dhSurface)
{
    DEBUGMESSAGE(("DrvDeleteDeviceBitmap"));
}

BOOL CALLBACK DrvStretchBlt(SURFOBJ * psoDest, SURFOBJ * psoSrc,
                            SURFOBJ * psoMask, CLIPOBJ * pco,
                            XLATEOBJ * pxlo, COLORADJUSTMENT * pca,
                            POINTL * pptlHTOrg, RECTL * prclDest,
                            RECTL * prclSrc, POINTL * pptlMask,
                            ULONG iMode)
{
    DEBUGMESSAGE(("DrvStretchBlt"));

    return TRUE;
}

BOOL CALLBACK DrvStretchBltROP( SURFOBJ  *pTargetSurfObj, SURFOBJ   *pSourceSurfObj,
        SURFOBJ                 *pMaskSurfObj, CLIPOBJ   *pClipObj,
        XLATEOBJ                *pXlateObj, COLORADJUSTMENT    *pColorAdjustment,
        POINTL                  *pHalfToneBrushOriginPointl, RECTL     *pTargetRectl,
        RECTL                   *pSourceRectl, POINTL     *pMaskOffsetPointl,
        ULONG                   mode, BRUSHOBJ    *pBrushObj,
        ROP4                    rop4 //  some  places   this   also  referred  as  mix.
        )
{
    DEBUGMESSAGE(("DrvStretchBltROP"));

    return TRUE;
}

BOOL CALLBACK DrvTransparentBlt(
                                IN SURFOBJ  *psoDst,
                                IN SURFOBJ  *psoSrc,
                                IN CLIPOBJ  *pco,
                                IN XLATEOBJ  *pxlo,
                                IN RECTL  *prclDst,
                                IN RECTL  *prclSrc,
                                IN ULONG  iTransColor,
                                IN ULONG  ulReserved
                                )
{
    DEBUGMESSAGE(("DrvTransparentBlt"));

    return TRUE;
}

VOID CALLBACK DrvSynchronize(
                            IN DHPDEV  dhpdev,
                            IN RECTL  *prcl
                            )
{
    DEBUGMESSAGE(("DrvSynchronize"));
}

BOOL CALLBACK DrvPlgBlt(
                        IN SURFOBJ  *psoTrg,
                        IN SURFOBJ  *psoSrc,
                        IN SURFOBJ  *psoMsk,
                        IN CLIPOBJ  *pco,
                        IN XLATEOBJ  *pxlo,
                        IN COLORADJUSTMENT  *pca,
                        IN POINTL  *pptlBrushOrg,
                        IN POINTFIX  *pptfx,
                        IN RECTL  *prcl,
                        IN POINTL  *pptl,
                        IN ULONG  iMode
                        )
{
    DEBUGMESSAGE(("DrvPlgBlt"));

    return TRUE;
}

BOOL CALLBACK DrvGradientFill(
                            IN SURFOBJ  *psoDest,
                            IN CLIPOBJ  *pco,
                            IN XLATEOBJ  *pxlo,
                            IN TRIVERTEX  *pVertex,
                            IN ULONG  nVertex,
                            IN PVOID  pMesh,
                            IN ULONG  nMesh,
                            IN RECTL  *prclExtents,
                            IN POINTL  *pptlDitherOrg,
                            IN ULONG  ulMode
                            )
{
    DEBUGMESSAGE(("DrvGradientFill"));

    return TRUE;
}

BOOL CALLBACK DrvAlphaBlend(
                            IN SURFOBJ  *psoDest,
                            IN SURFOBJ  *psoSrc,
                            IN CLIPOBJ  *pco,
                            IN XLATEOBJ  *pxlo,
                            IN RECTL  *prclDest,
                            IN RECTL  *prclSrc,
                            IN BLENDOBJ  *pBlendObj
                            )
{
    DEBUGMESSAGE(("DrvAlphaBlend"));

    return TRUE;
}

ULONG CALLBACK DrvEscape(SURFOBJ *pso,
                        ULONG    iEsc,
                        ULONG    cjIn,
                        PVOID    pvIn,
                        ULONG    cjOut,
                        PVOID    pvOut)
{
    DEBUGMESSAGE(("DrvEscape"));
/*
    ULONG uRetVal = 0;
    DUMPMSG("DrvEscape");
    struct DEVDATAUI{
        DWORD dwJobId;
        int Pages;
        WCHAR *pSpoolFileName;
        HANDLE hPDriver;
        BOOL *pResetDC;
    };
    switch(iEsc)
    {
        // while playing back on the spooler we don't get jobid 
    case PDEV_ESCAPE: 
        {
            PPDEV pPDEV = (DEVDATA *)pso->dhpdev;
            DEVDATAUI *pDevUI = (DEVDATAUI *)pvOut;
            pDevUI->dwJobId = pPDEV->dwJobId;
            pDevUI->Pages = pPDEV->Pages;
            return TRUE; 
        }
        break;
    }
    return uRetVal;
*/
    return 0;
}

ULONG CALLBACK DrvDrawEscape(SURFOBJ *pso,  ULONG iEsc, CLIPOBJ *pco,
                             RECTL   *prcl, ULONG cjIn, PVOID    pvIn)
{
    DEBUGMESSAGE(("DrvDrawEscape"));
    return 0xFFFFFFFF;
}

#endif  // SVG


// -----------------------------------------------------------------------------
// INTERNAL UTILITIES (NOT EXPOSED IN THE DLL INTERFACE)
// -----------------------------------------------------------------------------

#if !MINIMAL_DRIVER

LPTSTR FilterDocName(IN LPCTSTR original)
{
    // NOTE: we want the len in chars and not in bytes thus we don't use strBufferSize
    size_t lenOrig = strLen(original), lenFixed = 0;

    if (lenOrig == 0)
        return strDuplicate(TEXT("[emptydoc]"));

    // start copying docname from the last slash
    // (some programs set as document name e.g. C:\\temp\\tempfile.txt)
    LPCTSTR pBegin = _tcschr(original, TEXT('\\'));
    if (pBegin)
        lenOrig -= pBegin - original;
    else
        pBegin = original;

    // alloc returned string
    LPTSTR ret = strAlloc(lenOrig);

    // remove spaces & invalid chars from docname
    for (size_t i=0; i < lenOrig && lenFixed < lenOrig; i++)
    {
        BOOL isOk = TRUE;
        for (size_t j=0; j < strLen(g_strFileNameForbiddenChars); j++)
            if (pBegin[i] == g_strFileNameForbiddenChars[j])
                isOk = FALSE;
        if (pBegin[i] == TEXT(' '))
            isOk = FALSE;

        if (isOk)
            ret[lenFixed++] = pBegin[i];
    }

    ret[lenFixed] = TEXT('\0');
    return ret;
}

BOOL AskUser(IN HWND hDlg, IN LPCTSTR strDocName, IN LPCTSTR strPrinterName, 
             OUT PEXTDEVMODE pdevmode)
{
    const size_t vectorCount = ARRCOUNT(g_vectorFormats);
    const size_t rasterCount = ARRCOUNT(g_rasterFormats);

    // reserve space for some strings

	TCHAR atstrFileNameBuff[MAX_PATH];
    StringCchCopy(atstrFileNameBuff, MAX_PATH, extdmGetOutputFilename(pdevmode));

	TCHAR atstrFileTitleBuff[MAX_PATH];
    ::ZeroMemory(atstrFileTitleBuff, sizeof(atstrFileTitleBuff));

	TCHAR atstrCustomFilterBuff[32];
	::ZeroMemory(atstrCustomFilterBuff, sizeof(atstrCustomFilterBuff));


    // build the filter string

    size_t nFilterBuffSize = 128*(vectorCount+rasterCount)*sizeof(TCHAR);
    TCHAR *atstrFilterBuff = (TCHAR*)malloc(nFilterBuffSize);
    if (!atstrFilterBuff) {
        DEBUGMESSAGE(("AskUser - malloc failed"));
        return FALSE;
    }
	::ZeroMemory(atstrFilterBuff, nFilterBuffSize);

    LPTSTR pPos = atstrFilterBuff,
           pEnd = pPos + nFilterBuffSize - 1;

    size_t formatIdx = -1;
    for (size_t i=0; i<vectorCount+rasterCount; i++)
    {
        OUTPUTFORMAT fmt;
        if (i<vectorCount)
            fmt = g_vectorFormats[i];
        else
            fmt = g_rasterFormats[i-vectorCount];

        if (_tcscmp(fmt.strName, extdmGetOutputFormat(pdevmode)) == 0)
            formatIdx = i;

        StringCchCat(pPos, pEnd-pPos, fmt.strName);
        StringCchCat(pPos, pEnd-pPos, TEXT(" ("));
        StringCchCat(pPos, pEnd-pPos, fmt.strDescription);
        StringCchCat(pPos, pEnd-pPos, TEXT(")"));

        pPos += strLen(fmt.strName) + 
                strLen(fmt.strDescription) + 
                3 +     // additional text
                1;      // the null

        StringCchCat(pPos, pEnd-pPos, TEXT("*."));
        StringCchCat(pPos, pEnd-pPos, fmt.strName);
        pPos += strLen(fmt.strName) + 3;
    }

    // init the openfilename data struct

	::OPENFILENAME ofn;
	::ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.Flags = OFN_HIDEREADONLY;
    ofn.lpstrTitle = strCat(strPrinterName,
                            TEXT(": save output for '"),
                            strDocName,
                            TEXT("'"),
                            NULL);
	ofn.hwndOwner = hDlg;

    // ???
	ofn.lpstrCustomFilter = atstrCustomFilterBuff;
	ofn.nMaxCustFilter = ARRCOUNT(atstrCustomFilterBuff)-1;

    // output format
	ofn.lpstrFilter = atstrFilterBuff;
	ofn.nFilterIndex = formatIdx+1;     // the index here is not zero-based

    // output file name + path + ext
	ofn.lpstrFile = atstrFileNameBuff;
	ofn.nMaxFile = ARRCOUNT(atstrFileNameBuff);

    // output file name + ext
	ofn.lpstrFileTitle = atstrFileTitleBuff;
	ofn.nMaxFileTitle = ARRCOUNT(atstrFileTitleBuff);

    // output folder
    ofn.lpstrInitialDir = extdmGetOutputFolder(pdevmode);

    // show the dialog
	if (!GetSaveFileName(&ofn))
    {
        strFree(ofn.lpstrTitle);
        free(atstrFilterBuff);

        if (CommDlgExtendedError() == 0)
        {
            // user cancelled the dialog without displaying any error message...
            return FALSE;
        }

        DEBUGMESSAGE_LASTERROR(("GetSaveFileName failed"));
        return FALSE;
	}

    strFree(ofn.lpstrTitle);
    free(atstrFilterBuff);

    // parse output

    ofn.nFilterIndex--;         // make it zero-based
    LPCTSTR format = NULL;
    if (ofn.nFilterIndex < vectorCount)
        format = g_vectorFormats[ofn.nFilterIndex].strName;
    else
        format = g_rasterFormats[ofn.nFilterIndex - vectorCount].strName;

    LPTSTR file = ofn.lpstrFileTitle;
    LPTSTR pExt = _tcsrchr(file, TEXT('.'));
    if (pExt)
        *pExt = TEXT('\0');

    LPTSTR folder = ofn.lpstrFile;
    LPTSTR pFile = _tcsrchr(folder, TEXT('\\'));
    if (pFile)
        *(pFile+1) = TEXT('\0');

    // save output

    extdmSetPrivateData(pdevmode,
                        format,
                        file,
                        folder,
                        NULL,
                        NULL,
                        -1,
                        -1,
                        -1);

    return TRUE;
}

BOOL GetSpoolFileName(OUT LPTSTR SpoolFileName, IN DWORD JobId, IN HANDLE hPrinter)
{ 
    DWORD cbNeeded;  
    DWORD dwType = REG_SZ;                   // data type

    if (GetPrinterData(
            hPrinter,                         // handle to printer or print server
            SPLREG_DEFAULT_SPOOL_DIRECTORY,
            &dwType,                         // data type
            NULL,                            // configuration data buffer
            0,                               // size of configuration data buffer
            &cbNeeded                        // bytes received or required
            ) != ERROR_MORE_DATA)
        return FALSE;

    LPBYTE pSpoolDirectory = (LPBYTE)malloc(cbNeeded+1);
    if (GetPrinterData(
            hPrinter,                        // handle to printer or print server
            SPLREG_DEFAULT_SPOOL_DIRECTORY,
            &dwType,                        // data type
            pSpoolDirectory,                // configuration data buffer
            cbNeeded,                       // size of configuration data buffer
            &cbNeeded                       // bytes received or required
            ) != ERROR_SUCCESS)
        return FALSE;

    TCHAR TempSpoolFileName[MAX_PATH];
    wsprintf(TempSpoolFileName, L"%s\\", pSpoolDirectory);

    TCHAR JobIdName[256];
    wsprintf(JobIdName, L"%d", JobId);
    int zeros = 5 - wcslen(JobIdName);
    for(; zeros > 0; zeros--)
    {
        wsprintf(SpoolFileName, L"%s0", TempSpoolFileName);
        wcscpy(TempSpoolFileName, SpoolFileName);
    } 

    wsprintf(SpoolFileName, L"%s%d.spl", TempSpoolFileName, JobId);

    WIN32_FIND_DATA FindFileData;
    if (FindFirstFile(SpoolFileName,
                      &FindFileData) == INVALID_HANDLE_VALUE)
        return FALSE;

    wcscpy(SpoolFileName, (TCHAR *)pSpoolDirectory);
    wcscat(SpoolFileName, L"\\");
    wcscat(SpoolFileName, FindFileData.cFileName);   

    free(pSpoolDirectory);

    return TRUE;
}

VOID SpoolerSaveCallback(LPVOID param)
{
    WaitDialogWindow *wnd = (WaitDialogWindow*)param;
    MSG msg;

    wnd->Pulse();

    while (PeekMessage( &msg, wnd->GetDlgHwnd(), 0, 0, PM_NOREMOVE) != 0)
    {
        if (GetMessage( &msg, wnd->GetDlgHwnd(), 0, 0 ) != -1)
        { 
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }
}

#endif      // MINIMAL_DRIVER
