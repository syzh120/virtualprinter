/*
 * uidevmode.cpp
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

#define UIDEVMODE_C

#include "uidevmode.h"
#include "devmode.h"
#include "devmodeV351.h"
#include "devmodeV400.h"
#include "devmodeV401.h"
#include "extdevmode.h"
#include "utils.h"


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

// DrvConvertDevMode function is responsible for converting a printer's 
// DEVMODEW structure from one version to another.
BOOL DrvConvertDevMode(
	                    LPTSTR    lptstrPrinterName,
	                    PDEVMODE  pdmIn,			
	                    PDEVMODE  pdmOut,			
	                    PLONG     pcbNeeded,
	                    DWORD     fMode
	                    )
{
    DEBUGMESSAGE(("DrvConvertDevMode"));

	DWORD dwcNeeded = 0;

	if (pcbNeeded == NULL) {
        DEBUGMESSAGE(("DrvConvertDevMode - INVALID pcbNeeded"));
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	switch(fMode)
    {
	case CDM_CONVERT: 
		if (pdmOut == NULL) {
            DEBUGMESSAGE(("DrvConvertDevMode - INVALID pdmOut"));
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
		}
		if (pdmOut->dmSize == GetDevModeV351Size()) {
			pdmOut->dmSpecVersion = 0x320;
		} else if (pdmOut->dmSize == GetDevModeV400Size()) { 
			pdmOut->dmSpecVersion = 0x400;
		} else if (pdmOut->dmSize == GetDevModeV401Size()) {
			pdmOut->dmSpecVersion = 0x401;
		} 
		dwcNeeded = dmGetTotalSize(pdmOut);
		break;
	case CDM_CONVERT351:
		dwcNeeded = GetDevModeV351Size() + pdmOut->dmDriverExtra;
 		break;
	case CDM_DRIVER_DEFAULT:
		dwcNeeded = (DWORD) ::extdmSizeof();
		break;

	default:
        DEBUGMESSAGE(("DrvConvertDevMode - INVALID fMode"));
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (*pcbNeeded < (LONG) dwcNeeded) {
		if (fMode == CDM_CONVERT) {
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
		} else {
			*pcbNeeded = dwcNeeded;
            DEBUGMESSAGE(("DrvConvertDevMode - NOT ENOUGH SPACE IN GIVEN BUFFER"));
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
			return FALSE;
		}
	}

	if (pdmOut == NULL) {
        DEBUGMESSAGE(("DrvConvertDevMode - INVALID pdmOut"));
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (fMode == CDM_DRIVER_DEFAULT) {
		extdmSetToDefault( (PEXTDEVMODE) pdmOut );
	} else {
		if (pdmIn == NULL) {
            DEBUGMESSAGE(("DrvConvertDevMode - INVALID pdmIn"));
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
		}
		*pcbNeeded = dwcNeeded;
		dmConvert(pdmOut, pdmIn, fMode == CDM_CONVERT351);
	}

	return TRUE;
}

