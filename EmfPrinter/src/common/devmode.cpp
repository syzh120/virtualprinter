/*
 * devmode.cpp
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
#include "devmode.h"
#include "devModeV351.h"
//#include "devModeV400.h"
//#include "devModeV401.h"


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

DWORD dmGetTotalSize(IN PCDEVMODE pdm)
{
    return pdm->dmSize + pdm->dmDriverExtra;
}

void dmCopy(OUT LPBYTE lpBuff, IN PCDEVMODE pdm)
{
    DWORD dwc = dmGetTotalSize(pdm);
    CopyMemory(lpBuff, pdm, dwc);
}

PDEVMODE dmDuplicate(IN PCDEVMODE pdmSrc)
{
    DWORD dwc = dmGetTotalSize(pdmSrc);
    PDEVMODE pdm = (PDEVMODE) malloc(dwc);
    if (pdm != NULL)
        dmCopy((LPBYTE) pdm, pdmSrc);
    else
        SetLastError(ERROR_OUTOFMEMORY);
    
    return pdm;
}

void dmConvert(OUT PDEVMODE pdmOut, IN PCDEVMODE pdmIn, BOOL b351)
{
    WORD dmSpecVersion, dmSize, dmDriverVersion, dmDriverExtra;
    DWORD dwcCopy;

    if (b351) {
        pdmOut->dmSize = (WORD) GetDevModeV351Size();
        pdmOut->dmSpecVersion = 0x320;
    }

    dmSpecVersion = pdmOut->dmSpecVersion;
    dmSize = pdmOut->dmSize;
    dmDriverVersion = pdmOut->dmDriverVersion;
    dmDriverExtra = pdmOut->dmDriverExtra;

    dwcCopy = min( pdmOut->dmSize, pdmIn->dmSize );
    CopyMemory( pdmOut, pdmIn, dwcCopy );

    pdmOut->dmSpecVersion = dmSpecVersion;
    pdmOut->dmSize = dmSize;        
    pdmOut->dmDriverVersion = dmDriverVersion;
    pdmOut->dmDriverExtra = dmDriverExtra;

    dwcCopy = min( pdmOut->dmDriverExtra, pdmIn->dmDriverExtra );
    CopyMemory( (LPBYTE) pdmOut + dmSize, (LPBYTE) pdmIn + pdmIn->dmSize, dwcCopy );
}
