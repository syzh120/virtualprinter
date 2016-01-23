/*
 * utils.h
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

#ifndef UTILS_H
#define UTILS_H

#include "debug.h"
#include "stdheaderwrapper.h"
#include "types.h"


// -----------------------------------------------------------------------------
// MACROS
// -----------------------------------------------------------------------------

#define ARRCOUNT(a)		( sizeof(a) / sizeof((a)[0]) )

// IN pstr
// OUT pwstr
// IN cch: size of pwstr buffer
#define ANSI_TO_UNICODE(pstr,pwstr,cch)    \
            MultiByteToWideChar(CP_ACP,0,(pstr),(cch),(pwstr),(cch))

// OUT pstr
// IN pwstr
// IN cch: size of pstr buffer
#define UNICODE_TO_ANSI(pstr,pwstr,cch)    \
            WideCharToMultiByte(CP_ACP,0,(pwstr),(cch),(pstr),(cch),NULL,NULL)

// WHEN CONVERTING AN INTEGRAL TYPE (DWORD, INT, ETC) TO A STRING, 
// NO MORE THAN 4 DIGITS PER BYTE IS NEEDED
#define MAX_PRINTED_DIGITS(integral_type)			(sizeof(integral_type) * 4)


// -----------------------------------------------------------------------------
// GLOBALS
// -----------------------------------------------------------------------------

extern TCHAR g_strFileNameForbiddenChars[32];



// -----------------------------------------------------------------------------
// MISC FUNCTIONS
// -----------------------------------------------------------------------------

// CALLER IS RESPONSIBLE FOR FREEING THE MALLOC'D BUFFER RETURNED
LPTSTR MakeFullFileName(IN LPCTSTR lptstrDir, IN LPCTSTR lptstrFileName, 
                        IN LPCTSTR lptstrExt = NULL);
LPTSTR GetFileName(IN LPCTSTR lptstrFileName, IN LPCTSTR strDocName, IN DWORD nPage);
LPTSTR GetErrorDesc(IN DWORD dwErrCode);
LPTSTR GetFullAppPath();

BOOL SafeDeleteFile(IN LPCTSTR lptstrFileName);
BOOL WriteFile(IN HANDLE hFile, IN LPCVOID lpData, IN DWORD dwcData);
BOOL FileExists(IN LPCTSTR strFileName);
BOOL FolderExists(IN LPCTSTR lptstrFolderName);
BOOL GetFullTempFileName(OUT LPTSTR lptstrTempFileName, IN DWORD dwcChBuff);
BOOL IsValidFilename(IN LPCTSTR lptstrFolderName);
BOOL ExecuteProgram(IN LPTSTR commandLine, 
                    IN BOOL bConsole, 
                    IN BOOL bWait, 
                    IN EXECPROC fnCallback = NULL,
                    LPVOID callbackParam = NULL);

DWORD GetComCtl32Version();

// NOTE: this is not a debug macro! It must always be compiled (regardless of DBG)
VOID ErrorMessage(IN HWND hwndParent, IN LPCTSTR caption, IN LPCTSTR lptstrMessage);

// uses ImageMagick to init raster format list
BOOL InitRasterFormats();


// -----------------------------------------------------------------------------
// STRING UTILITIES
// -----------------------------------------------------------------------------

size_t strLen(IN LPCTSTR lptstr);               // returned value is in num of chars
size_t strBufferSize(IN LPCTSTR lptstr);        // returned value is in bytes!!
VOID strFree(IN LPCTSTR lptstr);
VOID strCopy(OUT LPTSTR lptOut, IN LPCTSTR lptstr, IN size_t nLen /* in bytes! */);

LPTSTR strAlloc(IN size_t nChars /* as number of chars for returned buffer */);
LPTSTR strDuplicate(IN LPCTSTR lptstr);
LPTSTR strCat(IN LPCTSTR lptstrArg, ...);
    // CALLER IS RESPONSIBLE FOR FREEING THE MALLOC'D BUFFER RETURNED
    // THE LAST ARGUMENT MUST BE NULL

BOOL strReplace(OUT LPTSTR strOut, IN DWORD dwOutSize, 
                IN LPCTSTR strToFind, IN LPCTSTR strReplacement);

#endif
