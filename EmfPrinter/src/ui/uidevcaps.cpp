/*
 * uidevcaps.cpp
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
#include "uidevcaps.h"
#include "uidevmode.h"
#include "extdevmode.h"
#include "utils.h"
#include "globals.h"


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

// DrvDeviceCapabilities function returns requested information
// about a printer's capabilities.
//
// NOTE:
//   WE NEED TO TAKE INTO ACCOUNT DIFFERENCE BETWEEN OUR CAPS, TARGET CAPS
DWORD DrvDeviceCapabilities(
                            HANDLE    hPrinter,
                            PWSTR     pDeviceName,
                            WORD      iDevCap,
                            VOID      *pvOutput,
                            PDEVMODE  pDevMode
	                        )
{
    //DEBUGMESSAGE(("DrvDeviceCapabilities"));

	DWORD dwRetValue = GDI_ERROR;
    WORD *pwOutput = (WORD*)pvOutput;
    POINTS *pptsdwRet = ((POINTS *)&dwRetValue);

    switch (iDevCap)
    {
    case DC_PAPERNAMES:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_PAPERNAMES"));
    case DC_MEDIAREADY:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_MEDIAREADY"));
        if (pvOutput) {
            TCHAR str[] = TEXT("None (the printer is virtual!)");
            // each paper name is 64 chars max
            strCopy((LPTSTR)pvOutput, str, strBufferSize(str));
        }
        return 1;

    case DC_PAPERS:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_PAPERS"));
        if (pvOutput)
            *pwOutput = DMPAPER_A4;     // A4 is 210x297 mm
        return 1;

    case DC_PAPERSIZE:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_PAPERSIZE"));
        if (pvOutput) {
            POINT *arr = (POINT*)pvOutput;
            arr[0].x = A4_WIDTH_MM*10;
            arr[0].y = A4_HEIGHT_MM*10;
        }
        return 1;

    case DC_BINNAMES:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_BINNAMES"));
        if (pvOutput)
            // each bin name is 24 chars max
            strCopy((LPTSTR)pvOutput, TEXT("Bin"), 3*sizeof(TCHAR));
        return 1;

    case DC_BINS:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_BINS"));
        if (pvOutput)
            *pwOutput = DMBIN_ONLYONE;
        return 1;

    case DC_COPIES:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_COPIES"));
        return 1;

    case DC_STAPLE:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_STAPLE"));
        return 0;   // not supported
    case DC_COLLATE:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_COLLATE"));
        return 0;   // not supported
    case DC_DUPLEX:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_DUPLEX"));
        return 0;   // not supported

    case DC_ENUMRESOLUTIONS:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_ENUMRESOLUTIONS"));
        if (pvOutput) {
            LONG *arr = (LONG*)pvOutput;
            arr[0] = emfDRIVER_WIDTH_DPI;
            arr[1] = emfDRIVER_HEIGHT_DPI;
        }
        return 1;

        // info about our driver's devmode

    case DC_DRIVER:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_DRIVER"));
        return emfDRIVER_REAL_VERSION;
    case DC_EXTRA:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_EXTRA"));
        return extdmSizeofPrivate();
    case DC_SIZE:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_SIZE"));
        return sizeof(DEVMODEW);    // size of only public members
    case DC_VERSION:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_VERSION"));
        return DM_SPECVERSION;
    case DC_FIELDS:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_FIELDS"));
        return extdmDEFAULT_FIELDS;
    case DC_FILEDEPENDENCIES:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_FILEDEPENDENCIES"));
        return 0;

    case DC_MAXEXTENT:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_MAXEXTENT"));
        pptsdwRet->x = A4_WIDTH_MM*10;
        pptsdwRet->y = A4_HEIGHT_MM*10;
        return dwRetValue;

    case DC_MINEXTENT:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_MINEXTENT"));
        // min is 1 mm
        pptsdwRet->x = 10;
        pptsdwRet->y = 10;
        return dwRetValue;

    case DC_ORIENTATION:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_ORIENTATION"));
        return 90;


    case DC_TRUETYPE:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_TRUETYPE"));
        return DCTT_BITMAP;//|DCTT_SUBDEV|DCTT_DOWNLOAD;

    case DC_PERSONALITY:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_PERSONALITY"));
        if (pvOutput)
            // each printer desc lang name is 32 chars max
            strCopy((LPTSTR)pvOutput, TEXT("EMF"), 3*sizeof(TCHAR));
        return 1;

    case DC_COLORDEVICE:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_COLORDEVICE"));
        return 1;

    //case DC_SPL_MEDIAREADY:

    case DC_NUP:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_NUP"));
        return 1;

    case DC_PRINTERMEM:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_PRINTERMEM"));
        return 4096;        // simulate 4MB of internal mem

        // 60 pages per minute: we're fast :)
    case DC_PRINTRATE:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_PRINTRATE"));
    case DC_PRINTRATEPPM:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_PRINTRATEPPM"));
        return 60;
    case DC_PRINTRATEUNIT:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_PRINTRATEUNIT"));
        return PRINTRATEUNIT_PPM;

    case DC_BINADJUST:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_BINADJUST"));
        // should be not used for WinXP/2000 but I've tested
        // that MS Word calls WinXP drivers with this code too!
        return DCBA_FACEUPNONE;

    case DC_EMF_COMPLIANT:
        DEBUGMESSAGE(("DrvDeviceCapabilities - DC_EMF_COMPLIANT"));
        // should be not used for WinXP/2000 but just to be sure...
        return 1;

        // other win98 only codes:
    //case DC_DATATYPE_PRODUCED:
    //case DC_MANUFACTURER:
    //case DC_MODEL:

    default:
        DEBUGMESSAGE(("Unexpected device capability required: %d", iDevCap));
        return GDI_ERROR;
    }

	return dwRetValue;
}

