/*
 * extdevmode.h
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

#ifndef EXTDEVMODE_H
#define EXTDEVMODE_H

#include "types.h"

// NOTE: the device mode is a very important part of the printer driver/driverUI
//       because it's the structure which Windows will pass among func calls 
//       between the driver/driverUI parts; it should contain
//       all info needed for doing the printing!


// -----------------------------------------------------------------------------
// TYPES
// -----------------------------------------------------------------------------

// TYPEDEFS 

struct EXTDEVMODE;      // our "extended" devmode is private!

typedef EXTDEVMODE*         PEXTDEVMODE;
typedef const EXTDEVMODE*   PCEXTDEVMODE;

// used by DrvDeviceCaps and extdm functions
#define extdmDEFAULT_FIELDS                                     \
    DM_ORIENTATION|DM_COPIES|DM_PAPERSIZE|DM_SCALE|DM_NUP|      \
    DM_DEFAULTSOURCE|DM_PRINTQUALITY|DM_COLOR|DM_DUPLEX|        \
    DM_YRESOLUTION|DM_TTOPTION|DM_COLLATE


// -----------------------------------------------------------------------------
// EXTENDED DEVMODE UTILITIES
// -----------------------------------------------------------------------------

PEXTDEVMODE extdmNew();
VOID extdmDelete(PEXTDEVMODE pextdm);

DWORD extdmGetDriverExtra();
DWORD extdmGetSize();
DWORD extdmGetSpecVersion();
BOOL extdmIsAnExtDevMode(IN PCDEVMODE pdm);
size_t extdmSizeof();
size_t extdmSizeofPrivate();

VOID extdmSetToDefault(OUT PEXTDEVMODE pextdm);

PCDEVMODE extdmGetPublicDM(PCEXTDEVMODE pextdm);
PDEVMODE extdmGetPublicDM(PEXTDEVMODE pextdm);

VOID extdmMerge(OUT PEXTDEVMODE pextdmOut, IN PCDEVMODE pdmIn);
VOID extdmCopyPrivateDM(PEXTDEVMODE pout, PCEXTDEVMODE pin);

VOID extdmSetPrivateData(OUT PEXTDEVMODE pextdmOut, 
                         IN LPCTSTR lpFormatName,
                         IN LPCTSTR lpOutputFile,
                         IN LPCTSTR lpOutputFolder,
                         IN LPCTSTR lpRasterConvOptions,
                         IN LPCTSTR lpPostGenCommand,
                         IN INT bAllowOverride,
                         IN INT bOpenOutputFile,
                         IN INT bCrop);

LPCTSTR extdmGetOutputFormat(IN PEXTDEVMODE pextdmIn);
LPCTSTR extdmGetOutputFolder(IN PEXTDEVMODE pextdmIn);
LPCTSTR extdmGetOutputFilename(IN PEXTDEVMODE pextdmIn);
LPCTSTR extdmGetConvertOptions(IN PEXTDEVMODE pextdmIn);
LPCTSTR extdmGetPostGenCmd(IN PEXTDEVMODE pextdmIn);

BOOL extdmAllowUserOverride(IN PEXTDEVMODE pextdmIn);
BOOL extdmCropBorders(IN PEXTDEVMODE pextdmIn);
BOOL extdmViewOutputFile(IN PEXTDEVMODE pextdmIn);

#endif
