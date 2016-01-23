/*
 * workerthread.cpp
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

#include "workerthread.h"
#include "utils.h"
#include <process.h>


// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

WorkerThread::WorkerThread()
{
	m_hThread = NULL;
}

WorkerThread::~WorkerThread()
{
	ASSERT(m_hThread == NULL);
}

BOOL WorkerThread::StartNewDoWorkThread()
{
	m_hThread = (HANDLE) ::_beginthread( WorkerThread::TheirThreadProc, 0,  this);
	BOOL bRetValue = (m_hThread != NULL);
	return bRetValue;
}

BOOL WorkerThread::PeekMessagesUntilTerminate(DWORD dwWaitTimeSlice, DWORD dwTotalWaitMS)
{
	BOOL bTerminated = FALSE;
	DWORD dwStartMS = ::GetTickCount();
	while( 
		!bTerminated && 
		(dwTotalWaitMS == INFINITE || GetTickCount() - dwStartMS < dwTotalWaitMS)
	) {
		bTerminated = this->WaitOnTerminate(dwWaitTimeSlice);
	}
	return bTerminated;
}

BOOL WorkerThread::WaitOnTerminate(DWORD dwWaitTimeSliceMS)
{
	BOOL bRetValue = TRUE;
	if (m_hThread != NULL) {
		DWORD dw = ::WaitForSingleObject(m_hThread, dwWaitTimeSliceMS);
		bRetValue = (dw != WAIT_TIMEOUT);
		if (bRetValue) {
			m_hThread = NULL;
		}
	}
	return bRetValue;
}

void __cdecl WorkerThread::TheirThreadProc(void *pv)
{
	WorkerThread *pwt = (WorkerThread *) pv;

    // do our work
	pwt->DoWork();

    // 7/9 REMOVE THIS, SEEING SOME UNEXPLAINED THREAD LOCKING
	// THIS IS NOT REQUIRED FOR THREADS STARTED WITH "beginthread"
	//_endthread();
}
