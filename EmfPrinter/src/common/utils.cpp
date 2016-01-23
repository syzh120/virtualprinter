/*
 * utils.cpp
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
#include "utils.h"
#include "debug.h"
#include "globals.h"

// forward declaration
LPTSTR strVCatStringsString(BOOL bIncludeNullTerms, LPCTSTR lptstrArg, va_list& in_vaList);

TCHAR g_strFileNameForbiddenChars[] = 
        {
            /* for short 8.3 filenames */
            ';', '=', '+', '<', '>', '|', '"', '[', ']', '\\', '/', '\'',

            /* for long filenames */
            ':', '*', '?', NULL 
        };


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

LPTSTR MakeFullFileName(LPCTSTR lptstrDir, LPCTSTR lptstrFileName, IN LPCTSTR lptstrExt)
{
    return strCat(lptstrDir, TEXT("\\"), 
                  lptstrFileName, lptstrExt, NULL);
}

LPTSTR GetErrorDesc(DWORD dw)
{
    LPTSTR lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    return lpMsgBuf;
}

VOID ErrorMessage(IN HWND hwndParent, IN LPCTSTR caption, 
                  IN LPCTSTR lptstrMessage)
{
    MessageBox(hwndParent, lptstrMessage, caption, MB_OK | MB_ICONERROR);
}

BOOL SafeDeleteFile( LPCTSTR lptstrFileName )
{
    if (lptstrFileName == NULL)
        return TRUE;        // consider this OK
    
    if (::lstrlen(lptstrFileName) == 0)
        return FALSE;
    
    BOOL b = DeleteFile( lptstrFileName );

    if (!b && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        DEBUGMESSAGE(("SafeDeleteFile - file '%ws' does not already exist", 
                    lptstrFileName));

        // consider this ok as our aim is to get rid of the
        // file - if it already does not exist, it's ok
        return TRUE;
    }

    // TODO: MSDN says we should remove the read-only attribute
    //       in case of the ERROR_ACCESS_DENIED errcode

    if (!b)
    {
        DEBUGMESSAGE(("SafeDeleteFile - Cannot delete file '%ws'", lptstrFileName));
        return FALSE;
    }

    return TRUE;
}

BOOL WriteFile(HANDLE hFile, LPCVOID lpData, DWORD dwcData)
{
    DWORD dwcWritten;
    BOOL b = WriteFile(hFile, lpData, dwcData, &dwcWritten, NULL);
    if (b && dwcData == dwcWritten) {
        DEBUGMESSAGE(("WriteFile - wrote %d bytes", dwcData));
        return TRUE;
    }
    
    DEBUGMESSAGE_LASTERROR(("WriteFile - problem writing %d bytes from %p to %p",
                           dwcData, lpData, hFile));
    return FALSE;
}

BOOL FileExists(IN LPCTSTR strFileName)
{
    DWORD ret = ::GetFileAttributes(strFileName);
    return (ret != (DWORD)-1) && !(ret & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL FolderExists(IN LPCTSTR lptstrFolderName)
{
    DWORD ret = ::GetFileAttributes(lptstrFolderName);
    return (ret != (DWORD)-1) && (ret & FILE_ATTRIBUTE_DIRECTORY);
}

BOOL GetFullTempFileName(OUT LPTSTR lptstrTempFileName, IN DWORD dwcChBuff)
{
	ASSERT(dwcChBuff >= MAX_PATH+1);

    // get the temp folder
	TCHAR strTempPath[MAX_PATH+1];
	DWORD dwcChLength = GetTempPath(MAX_PATH+1, strTempPath);
    if (dwcChLength == 0 || dwcChLength > MAX_PATH) {
        DEBUGMESSAGE(("GetFullTempFileName - FAILED TO GET TEMP PATH"));
		//StringCchCopy(strTempPath, MAX_TEMP_PATH_CCH+1, TEXT("."));
        return FALSE;
    }

	return GetTempFileName(strTempPath, TEXT("emfpr"), 0, lptstrTempFileName) != 0;
}

BOOL IsValidFilename(IN LPCTSTR lptstrFileName)
{
    if (lptstrFileName == NULL)
        return FALSE;

    int len=lstrlen(lptstrFileName);
    if (len == 0)
        return FALSE;
    
    // check for common invalid chars
    if (_tcspbrk(lptstrFileName, g_strFileNameForbiddenChars) != NULL)
        return FALSE;

    return TRUE;
}

LPTSTR GetFileName(IN LPCTSTR lptstrFileName, IN LPCTSTR strDocName, IN DWORD nPage)
{
    if (!IsValidFilename(lptstrFileName) ||
        !IsValidFilename(strDocName)) {
        DEBUGMESSAGE(("GetFileName - invalid filename '%ws' or docname '%ws'",
                      lptstrFileName, strDocName));
        return NULL;
    }

    DWORD retSize = strLen(lptstrFileName) + strLen(strDocName) + 
                    MAX_PRINTED_DIGITS(nPage);
    LPTSTR ret = strAlloc(retSize);
    if (!ret)
        return NULL;

    // get page str
    TCHAR strPage[32];
    StringCchPrintf(strPage, 32, TEXT("%d"), nPage);

    // we allow %docname% and %page% substitutions
    strCopy(ret, lptstrFileName, strBufferSize(lptstrFileName));
    strReplace(ret, retSize, TEXT("%docname%"), strDocName);
    strReplace(ret, retSize, TEXT("%page%"), strPage);

    return ret;
}


struct DLLVERSIONINFO
{
    DWORD cbSize;
    DWORD dwMajorVersion;                   // Major version
    DWORD dwMinorVersion;                   // Minor version
    DWORD dwBuildNumber;                    // Build number
    DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
};

/* taken & adapted from wxWidgets sources */
DWORD GetComCtl32Version()
{
    // cache the result
    //
    // NB: this is MT-ok as in the worst case we'd compute s_verComCtl32 twice,
    //     but as its value should be the same both times it doesn't matter
    static int s_verComCtl32 = -1;

    if ( s_verComCtl32 != -1 )
        return s_verComCtl32;


    // initally assume no comctl32.dll at all
    s_verComCtl32 = 0;

    HMODULE dllComCtl32 = ::LoadLibrary(TEXT("comctl32.dll"));

    // if so, then we can check for the version
    if ( dllComCtl32 )
    {
        #define wxDYNLIB_FUNCTION(type, name, dynlib) \
            type pfn ## name = (type)GetProcAddress(dynlib, #name)
        typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

        // now check if the function is available during run-time
        wxDYNLIB_FUNCTION( DLLGETVERSIONPROC, DllGetVersion, dllComCtl32 );
        if ( pfnDllGetVersion )
        {
            DLLVERSIONINFO dvi;
            dvi.cbSize = sizeof(dvi);

            HRESULT hr = (*pfnDllGetVersion)(&dvi);
            if ( FAILED(hr) )
            {
                DEBUGMESSAGE_LASTERROR(("DllGetVersion"));
            }
            else
            {
                // this is incompatible with _WIN32_IE values, but
                // compatible with the other values returned by
                // GetComCtl32Version()
                s_verComCtl32 = 100*dvi.dwMajorVersion +
                                    dvi.dwMinorVersion;
            }
        }

        // if DllGetVersion() is unavailable either during compile or
        // run-time, try to guess the version otherwise
        if ( !s_verComCtl32 )
        {
            // InitCommonControlsEx is unique to 4.70 and later
            void *pfn = GetProcAddress(dllComCtl32, "InitCommonControlsEx");
            if ( !pfn )
            {
                // not found, must be 4.00
                s_verComCtl32 = 400;
            }
            else // 4.70+
            {
                // many symbols appeared in comctl32 4.71, could use any of
                // them except may be DllInstall()
                pfn = GetProcAddress(dllComCtl32, "InitializeFlatSB");
                if ( !pfn )
                {
                    // not found, must be 4.70
                    s_verComCtl32 = 470;
                }
                else
                {
                    // found, must be 4.71 or later
                    s_verComCtl32 = 471;
                }
            }
        }
    }

    return s_verComCtl32;
}

LPTSTR GetFullAppPath()
{
    const TCHAR strRegKey[] = TEXT("SOFTWARE\\EmfPrinter");

    HKEY hkey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, strRegKey, 
                     0, KEY_QUERY_VALUE, &hkey) != ERROR_SUCCESS) {
        DEBUGMESSAGE(("Could not open the '%ws' registry key", strRegKey));
        return NULL;
    }

    DWORD dwType=REG_NONE;
    DWORD dwSize=MAX_PATH;
    LPTSTR ret = strAlloc(dwSize+1);

    if (RegQueryValueEx(hkey, TEXT("Path"), NULL, &dwType, 
                        (LPBYTE)ret, &dwSize) != ERROR_SUCCESS) {
        DEBUGMESSAGE(("Could not get the '%ws' registry key", strRegKey));
        return NULL;
    }

    if (ret[strLen(ret)-1] == TEXT('\\'))
        return ret;

    LPTSTR newRet = strCat(ret, TEXT("\\"), NULL);
    strFree(ret);
     
    return newRet;
}

BOOL ExecuteProgram(IN LPTSTR commandLine, IN BOOL bConsole, 
                    IN BOOL bWait, IN EXECPROC fnCallback,
                    LPVOID callbackParam)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    ZeroMemory( &pi, sizeof(pi) );

    si.cb = sizeof(si);
    if (bConsole)
    {
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }

    // Start the child process. 
    if ( !CreateProcess( 
                NULL,             // No module name (use command line). 
                commandLine,      // Command line. 
                NULL,             // Process handle not inheritable. 
                NULL,             // Thread handle not inheritable. 
                FALSE,            // Set handle inheritance to FALSE. 
                0,                // No creation flags. 
                NULL,             // Use parent's environment block. 
                NULL,             // Use parent's starting directory. 
                &si,              // Pointer to STARTUPINFO structure.
                &pi )             // Pointer to PROCESS_INFORMATION structure.
        ) 
    {
        DEBUGMESSAGE(( "CreateProcess failed." ));
        return FALSE;
    }

    // Wait until child process exits.
    if (!bWait)
    {
        //CloseHandle( pi.hProcess );
        //CloseHandle( pi.hThread );

        return TRUE;
    }

    while (1)
    {
        if (WaitForSingleObject( pi.hProcess, /*INFINITE*/ 10 /* 10 ms */) == WAIT_OBJECT_0)
            break;      // process completed

        // call the callback ;)
        if (fnCallback)
            (*fnCallback)(callbackParam);
    }

    DWORD exitCode;
    if (!GetExitCodeProcess(pi.hProcess, &exitCode))
    {
        DEBUGMESSAGE(( "CreateProcess failed." ));
        return FALSE;
    }

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

    return exitCode == 0;
}



// -----------------------------------------------------------------------------
// STRING UTILITIES
// -----------------------------------------------------------------------------

LPTSTR strAlloc(IN size_t nChars)
{
    return (LPTSTR)malloc(nChars * sizeof(TCHAR));
}

size_t strLen(IN LPCTSTR lptstr)
{
    size_t len;
    if (StringCchLength(lptstr, STRSAFE_MAX_CCH, &len) != S_OK) {
        DEBUGMESSAGE(("StringCchLength failed for '%ws'", lptstr));
        return 0;
    }

    return len;
}

size_t strBufferSize(IN LPCTSTR lptstr)
{
    size_t szRetValue = 0;
    const TCHAR atstr[] = TEXT("");
    HRESULT hr = StringCbLength(lptstr, STRSAFE_MAX_CCH * sizeof(TCHAR), &szRetValue);
    ASSERT(hr == S_OK);
    szRetValue += sizeof(atstr);
    return szRetValue;      // this is in bytes!
}

LPTSTR strDuplicate(IN LPCTSTR lptstr)
{
    size_t sz = strBufferSize(lptstr);
    LPTSTR lptstrDup = (LPTSTR) malloc(sz);
    CopyMemory(lptstrDup, lptstr, sz);
    return lptstrDup;
}

VOID strFree(IN LPCTSTR lptstr)
{
    free((void*)lptstr);
}

VOID strCopy(OUT LPTSTR lptOut, IN LPCTSTR lptstr, size_t nLen)
{
    if (!lptOut || !lptstr) {
        DEBUGMESSAGE(("strCopy - invalid pointers!!"));
        return;
    }

    CopyMemory(lptOut, lptstr, nLen);   // nLen must be in bytes
}

LPTSTR strCat(LPCTSTR lptstrArg, ...)
{
    va_list vaList;
    va_start(vaList, lptstrArg);
    LPTSTR lptstrRetValue = strVCatStringsString(FALSE, lptstrArg, vaList);
    va_end(vaList);
    return lptstrRetValue;
}

LPTSTR strVCatStringsString(BOOL bIncludeNullTerms, LPCTSTR lptstrArg, 
                            va_list& in_vaList)
{
    DWORD dwCchExtra = (bIncludeNullTerms ? 1 : 0);
    LPTSTR lptstrRetValue = NULL;

    va_list vaList = in_vaList;
    DWORD dwCch = 0;
    for (LPCTSTR lptstr = lptstrArg; lptstr != NULL; lptstr = va_arg(vaList, LPCTSTR))
        dwCch += ::lstrlen(lptstr) + dwCchExtra;
    ++ dwCch;

    lptstrRetValue = strAlloc(dwCch);
    if (lptstrRetValue != NULL) {
        LPTSTR lptstrCtr = lptstrRetValue;
        vaList = in_vaList;
        for (LPCTSTR lptstr = lptstrArg; lptstr != NULL; lptstr = va_arg(vaList, LPCTSTR))
        {
            dwCch = ::lstrlen(lptstr) + dwCchExtra;
            memcpy(lptstrCtr, lptstr, dwCch * sizeof(TCHAR));
            lptstrCtr += dwCch;
        }
        *lptstrCtr = 0;
    }

    return lptstrRetValue;
}

BOOL strReplace(OUT LPTSTR strOut, IN DWORD dwOutSize, 
                IN LPCTSTR poldStr, IN LPCTSTR pnewStr)
{
    size_t nOutChars = strLen(strOut);
    size_t nOldChars = strLen(poldStr);
    size_t nNewChars = strLen(pnewStr);

    LPTSTR pFirstOccBegin = _tcsstr(strOut, poldStr);
    if (!pFirstOccBegin)
        return FALSE;      // nothing to replace

    size_t nFirstOccBeginIdx = pFirstOccBegin - strOut;
    LPTSTR pFirstOccEnd = pFirstOccBegin + nOldChars;

    LPTSTR temp = strAlloc(nOutChars+1);
    if (StringCchCopy(temp, nOutChars+1, pFirstOccEnd) != S_OK)
        return FALSE;

    // do the replace
    if (StringCchCopy(pFirstOccBegin, dwOutSize-nFirstOccBeginIdx, pnewStr) != S_OK)
        return FALSE;

    // then copy the portion which was placed after the replaced token back
    // in the original buffer
    if (StringCchCopy(pFirstOccBegin+nNewChars, 
                      dwOutSize-nFirstOccBeginIdx-nNewChars, temp) != S_OK)
        return FALSE;

    strFree(temp);
    return TRUE;
}

