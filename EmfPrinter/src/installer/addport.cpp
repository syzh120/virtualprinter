/*
 * addport.cpp
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

#include "addport.h"
#include <windows.h>
#include <winspool.h>


#if 1

// ****
// **** TYPEDEFS AND PROTOTYPES PRIVATE TO MODULE
// ****

struct ThreadParam {
	LPCTSTR lptstrMonitorName;
	BOOL bAddPortSucess;
};

DWORD WINAPI AddPortThreadProc( LPVOID lpParam ) ;
BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lParam) ;

// RETURN FALSE TO ABORT
typedef BOOL (*WaitProc)(void);

// ****
// **** PUBLIC ROUTINES
// ****

BOOL AddPort(LPCTSTR lptstrPortName)
{
    DWORD dwMaxWaitMS = INFINITE;
    DWORD dwNewThreadSliceMS = 5;
    WaitProc pfn = NULL;
    LPCTSTR lptstrMonitorName = NULL;


	const TCHAR atstrDefaultMonitorName[] = TEXT("Local Port");
	BOOL bRetValue = FALSE;
	DWORD dwMSStart = ::GetTickCount();

	ThreadParam tp = { lptstrMonitorName == NULL ? 
            atstrDefaultMonitorName : lptstrMonitorName, FALSE };

	DWORD dwThreadID;
	HANDLE hThread = ::CreateThread(
		NULL,
		0,
		AddPortThreadProc,
		(LPVOID) &tp,
		0,
		&dwThreadID
		);

	if (hThread != NULL) {
		DWORD dwWaitResult = WAIT_TIMEOUT;
		BOOL bKeepWaiting = TRUE;
		HWND hwnd = NULL;
		HWND hwndEdit = NULL;
		while( 
			(hwnd == NULL || hwndEdit == NULL) &&
			(dwMaxWaitMS == INFINITE || dwMaxWaitMS >= (GetTickCount() - dwMSStart)) &&
			dwWaitResult == WAIT_TIMEOUT &&
			bKeepWaiting
		) {
			if (hwnd == NULL) {
				EnumThreadWindows(dwThreadID, EnumThreadWndProc, (LPARAM) &hwnd);
			}
			if (hwnd != NULL) {
				hwndEdit = ::FindWindowEx(hwnd, NULL, TEXT("Edit"), NULL);
			}
			if (pfn != NULL) {
				bKeepWaiting = pfn();
			} 
			if (dwNewThreadSliceMS != 0) {
				dwWaitResult = ::WaitForSingleObject(hThread, dwNewThreadSliceMS);
			}
		}

		if (bKeepWaiting) {
			if (hwnd != NULL && hwndEdit != NULL) {
				// ENTER THE PORT NAME FOR THE USER
				::SetWindowText(hwndEdit, lptstrPortName);
				// HIT THE OK BUTTON
				::PostMessage(hwnd, WM_COMMAND, (WPARAM) IDOK, 0);
				// FOR ADDED SAFETY, HIT THE RETURN KEY ALSO
				UINT uCode = VK_RETURN;	
				UINT uScanCode = ::MapVirtualKey(uCode, 0);
				::PostMessage(hwnd, WM_CHAR, (WPARAM) uCode, (LPARAM) uScanCode);
			}

			while(
				bKeepWaiting &&
				(dwMaxWaitMS == INFINITE || dwMaxWaitMS >= (GetTickCount() - dwMSStart)) &&
				dwWaitResult == WAIT_TIMEOUT
			) {
				if (pfn != NULL) {
					bKeepWaiting = pfn();
				}
				dwWaitResult = ::WaitForSingleObject(hThread, dwNewThreadSliceMS);
			}
		}
		if (dwWaitResult != WAIT_OBJECT_0) {
			::TerminateThread(hThread, 0);
		} else {
			bRetValue = tp.bAddPortSucess;
		}
		::CloseHandle(hThread);
	}

	return bRetValue;
}

// ****
// **** PRIVATE ROUTINES
// ****

static DWORD WINAPI AddPortThreadProc( LPVOID lpParam )
{
	ThreadParam *ptp = (ThreadParam *) lpParam;
	LPCTSTR lptstrMonitorName = ptp->lptstrMonitorName;
	AddPort(NULL, NULL, const_cast<LPTSTR>(lptstrMonitorName) );
	ptp->bAddPortSucess = TRUE;
	return 0;
}

static BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lParam)
{
	HWND *phwnd = (HWND *) lParam;
	*phwnd = hwnd;
	return FALSE;
}


//  AN ALTERNATIVE WAY TO ADD PORT WITHOUT USER INTERACTION
// 
//  http:// groups.google.com/group/microsoft.public.development.device.drivers/browse_thread/thread/3422cc39b6afc1bd/ac37398846c3ab1d?lnk=st&q=xcvdata+printer+handle&rnum=3&hl=en#ac37398846c3ab1d
// From:	 	"Ashwin [MS]" - view profile
// Date:		Mon, Apr 22 2002 7:40 pm
// Email: 		ashw...@online.microsoft.com ("Ashwin [MS]")
// Groups: 		microsoft.public.development.device.drivers
// Not yet rated
// Rating:	 
// show options
// 
// Reply | Reply to Author | Forward | Print | Individual Message | Show original | Report Abuse | Find messages by this author
// 
// AFAIK, there is no such procedure for any of the 9x platforms. But I am not
// a 9x expert....so I maybe wrong.
// 
// For silent programmatic installation of custom local ports, the only
// methods available are AddPortEx and XcvData. If you are not able to use
// AddPortEx and have to use AddPort, but without user intervention, another
// dirty trick is to get the handle of the the dialog box which comes up for
// the AddPort popup, then get the handle for the edit control box for filling
// up the port name, which the user would otherwise have to do; then call the
// Win32 api SetWindowText (please check on this) to fill the port name text
// string yourself, programmatically; finally, simulate the click on the "OK"
// button by calling ID_OK.
// 
// Here are some additional steps from the SDK docs:
// 
// The function should perform the following operations:
// 1.      Call OpenPrinter, specifying a printer name with the following format:
// \\ServerName\,XcvMonitor MonitorName
// where ServerName and MonitorName are the server and monitor names received
// as AddPortUI function arguments.
// The call to OpenPrinter requires a PRINTER_DEFAULTS structure, which is
// described in the Platform SDK documentation. The structure's DesiredAccess
// member must be set to SERVER_ACCESS_ADMINISTER. Its pDatatype and pDevMode
// members can be NULL.
// This call causes the print monitor server DLL's XcvOpenPort function to be
// called.
// 2.      Obtain a port name from the user by displaying a dialog box.
// 3.      Call XcvData, specifying the following input arguments:
// o       The handle received from OpenPrinter
// o       The port name received from the user
// o       A customized data name string, such as "PortExists"
// This call causes the server DLL's XcvDataPort function to be called. The
// XcvDataPort function should return a value that indicates whether the
// specified port name has already been used. If it has, the UI DLL should
// request another name from the user and call XcvData again.
// 4.      After a valid new port name has been received, call XcvData again, this
// time specifying the following input arguments:
// o       The handle received from OpenPrinter
// o       The validated port name received from the user
// o       A data name string of "AddPort"
// This call causes the server DLL's XcvDataPort function to be called again.
// 5.      Obtain port configuration parameters from the user by displaying a
// dialog box.
// 6.      Call XcvData one or more times, specifying customized data name strings,
// to send each configuration parameter to the server DLL. Each XcvData call
// causes the server's XcvDataPort function to be called.
// 7.      Call ClosePrinter, specifying the handle received from OpenPrinter. This
// causes the server DLL's XcvClosePort function to be called.
// 
// Hope this helps
// 
//- Ashwin 
/*
  WriteRegStr HKEY_LOCAL_MACHINE "SYSTEM\CurrentControlSet\Control\Print\Monitors\ImagePrinter Port" "Driver" "imgport.dll"
  WriteRegStr HKEY_LOCAL_MACHINE "SYSTEM\CurrentControlSet\Control\Print\Monitors\ImagePrinter Port" "name" "ImagePrinter Port"
  WriteRegStr HKEY_LOCAL_MACHINE "SYSTEM\CurrentControlSet\Control\Print\Monitors\ImagePrinter Port" "path" "C:\"
  WriteRegStr HKEY_LOCAL_MACHINE "SYSTEM\CurrentControlSet\Control\Print\Monitors\ImagePrinter Port" "format" "tif"
  WriteRegStr HKEY_LOCAL_MACHINE "SYSTEM\CurrentControlSet\Control\Print\Monitors\ImagePrinter Port" "format_ext" "LZW"
  
*/
#else

#include "installerutils.h"
#include "utils.h"
/*
BOOL AddPort(LPCTSTR lptstrPortName, LPCTSTR strDriverFileName)
{
    const TCHAR strRegKey[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors");

    HKEY hkey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, strRegKey, 
                     0, KEY_SET_VALUE, &hkey) != ERROR_SUCCESS) {
        SER(TEXT("Could not open the '%s' registry key"), strRegKey);
        return FALSE;
    }

    HKEY portkey;
    if (RegCreateKeyEx(hkey, lptstrPortName, 0, NULL, 0, KEY_SET_VALUE, NULL,
                       &portkey, NULL) != ERROR_SUCCESS) {
        SER(TEXT("Could not create the '%s' registry key"), lptstrPortName);
        RegCloseKey(hkey);
        return FALSE;
    }

    if (RegSetValueEx(portkey, TEXT("Driver"), 0, REG_SZ, 
                      (LPBYTE) strDriverFileName, strBufferSize(strDriverFileName)+1)) {
        SER(TEXT("Cannot set the driver name for the '%s' port"), lptstrPortName);
        RegCloseKey(portkey);
        RegCloseKey(hkey);
        return FALSE;
    }

    if (RegSetValueEx(portkey, TEXT("name"), 0, REG_SZ, 
                      (LPBYTE) lptstrPortName, strBufferSize(lptstrPortName)+1)) {
        SER(TEXT("Cannot set the driver name for the '%s' port"), lptstrPortName);
        RegCloseKey(portkey);
        RegCloseKey(hkey);
        return FALSE;
    }

    RegCloseKey(portkey);
    RegCloseKey(hkey);

    return TRUE;
}
*/

/*
BOOL AddPort(LPCTSTR lptstrPortName, LPCTSTR strDriverFileName)
{
    const TCHAR strRegKey[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors");


_MONITORINIT {
  DWORD  cbSize;
  HANDLE  hSpooler;
  HANDLE  hckRegistryRoot;
  PMONITORREG  pMonitorReg;
  BOOL  bLocal;
  LPCWSTR  pszServerName;
} MONITORINIT, *PMONITORINIT;


    InitializePrintMonitor2();
    AddPortEx

    HKEY hkey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, strRegKey, 
                     0, KEY_SET_VALUE, &hkey) != ERROR_SUCCESS) {
        SER(TEXT("Could not open the '%s' registry key"), strRegKey);
        return FALSE;
    }

    HKEY portkey;
    if (RegCreateKeyEx(hkey, lptstrPortName, 0, NULL, 0, KEY_SET_VALUE, NULL,
                       &portkey, NULL) != ERROR_SUCCESS) {
        SER(TEXT("Could not create the '%s' registry key"), lptstrPortName);
        RegCloseKey(hkey);
        return FALSE;
    }

    if (RegSetValueEx(portkey, TEXT("Driver"), 0, REG_SZ, 
                      (LPBYTE) strDriverFileName, strBufferSize(strDriverFileName)+1)) {
        SER(TEXT("Cannot set the driver name for the '%s' port"), lptstrPortName);
        RegCloseKey(portkey);
        RegCloseKey(hkey);
        return FALSE;
    }

    if (RegSetValueEx(portkey, TEXT("name"), 0, REG_SZ, 
                      (LPBYTE) lptstrPortName, strBufferSize(lptstrPortName)+1)) {
        SER(TEXT("Cannot set the driver name for the '%s' port"), lptstrPortName);
        RegCloseKey(portkey);
        RegCloseKey(hkey);
        return FALSE;
    }

    RegCloseKey(portkey);
    RegCloseKey(hkey);

    return TRUE;
}*/


BOOL AddPort(LPCTSTR pszPortName)
{
	DWORD cbneed,cbstate;
    PBYTE pOutputData;
	HANDLE hXcv = INVALID_HANDLE_VALUE;
	PRINTER_DEFAULTS Defaults = { NULL,NULL,SERVER_ACCESS_ADMINISTER };		
	
    pOutputData=(PBYTE)malloc(MAX_PATH);

	if (!OpenPrinter(TEXT(",XcvMonitor Local Port"), &hXcv, &Defaults))
	{
		SER(TEXT("CANNOT OPEN LOCAL PORT MONITOR"));
		free(pOutputData);
		return FALSE;
	}
    
    WCHAR *pwPortName=emfPORT_NAME; 
    if (!XcvData(hXcv, TEXT("AddPort"), (PBYTE)pwPortName,
                 sizeof(pwPortName), (PBYTE)pOutputData, MAX_PATH, &cbneed, &cbstate))
	{
		SER(TEXT("CANNOT ADD LOCAL PORT '%s'"), pszPortName);
		free(pOutputData);
		return FALSE;
	}

	free(pOutputData);
    ClosePrinter(hXcv);

	return TRUE;
}
#endif
 

