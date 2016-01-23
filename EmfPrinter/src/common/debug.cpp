/*
 * debug.cpp
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
#include "debug.h"
#include "types.h"
#include "utils.h"

#include <time.h>


#if ENABLE_DEBUGGING

#define MAX_DEBUGPRINT_CCH				256
char g_strError[MAX_ERROR_CHARS];


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

void SendDebugMessage(const char *errorStr)
{
    // first send it to the debugger, if it's running.
    // Typically this string is either displayed by MSVC debugger
    // or (if it's not running) by DebugView utility (if running).
    OutputDebugStringA(errorStr);

    // also save this debug message to a file
	FILE *pf = fopen(DEBUGLOGFILE, "a+");
	if (pf != NULL)
    {
        char time_str[64];
        _strtime(time_str);

        fprintf(pf, "%s - %s", time_str, errorStr);
		fclose(pf);
	}
    else
        OutputDebugStringA("Could not open the debug logging file.");
}

void AppendDebugStr(const char *pszFormat, ...)
{
	char astrBuff[MAX_DEBUGPRINT_CCH+1];

    va_list vaList;
    va_start(vaList, pszFormat);
	::StringCchVPrintfA(astrBuff, MAX_DEBUGPRINT_CCH, pszFormat, vaList);
	va_end(vaList);

	astrBuff[MAX_DEBUGPRINT_CCH] = 0;
    
    ::StringCchCatA(g_strError, MAX_ERROR_CHARS, astrBuff);
}

void ErrorExit(LPTSTR lpszFunction) 
{
	LPTSTR lpFormat = TEXT("%s failed with error %d: %s");
    LPTSTR lpMsgBuf = GetErrorDesc(GetLastError());
    LPTSTR lpDisplayBuf;
	size_t dwcBuff;

	dwcBuff = strBufferSize(lpMsgBuf) +
		        strBufferSize(lpszFunction) +
		        strBufferSize(lpFormat);

    lpDisplayBuf = (LPTSTR) LocalAlloc(LMEM_ZEROINIT, dwcBuff);
	StringCbPrintf(lpDisplayBuf, dwcBuff, lpFormat, lpszFunction, 
                   GetLastError(), lpMsgBuf);

    MessageBox(NULL, lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(GetLastError()); 
}

#endif      // DBG

