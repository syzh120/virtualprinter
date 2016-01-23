/*
 * workerthread.h
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

#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include "stdheaderwrapper.h"

// NOTE:  IF THE WORKER THREAD ACCESSES ANY DATA OTHER THAN DATA WHICH IS 
//        PRIVATE TO THE DERIVED CLASS, THAT DATA MUST BE SYNCHRONIZED.
//        FOR EXAMPLE, DATA PASSED INTO ITS CONSTRUCTOR.
//        SAME IS TRUE OF ANY OF ITS PRIVATE DATA THAT ARE ACCESSED VIA 
//        PUBLIC METHODS (POSSIBLY BY A DIFFERENT THREAD)

class WorkerThread
{
public:
	enum {
		eDefaultTimeSliceMS = 50,
		eDefaultTotalWaitMS = INFINITE
	};

	WorkerThread();

	// DERIVED CLASS DESTRUCTOR MUST CALL
	// WaitOnTerminate OR PeekMessagesUntilTerminate
	// TO MAKE SURE THE CLASS IS NOT DESTROYED BEFORE THE THREAD TERMINATES
    //
	// WorkerThread CAN'T DO THIS, BECAUSE THE VTABLE HAS BEEN REDUCED BY THE TIME
	// ITS DESTRUCTOR IS CALLED, SO THAT DoWork IS NOT THE DERIVED CLASSES DoWork
	// AND THIS COULD RESULT IN A PURE VIRTUAL CALL
	virtual ~WorkerThread();

	// AT MOST ONE WORKER THREAD PER CLASS INSTANCE IS SUPPORTED;
    // AFTER A CALL TO StartNewDoWorkThread, CLIENT MUST 
    // CALL WaitOnTerminate OR PeekMessagesUntilTerminate
	// UNTIL THEY RETURN TRUE BEFORE CALLING StartNewDoWorkThread AGAIN
	BOOL StartNewDoWorkThread();

	BOOL PeekMessagesUntilTerminate(DWORD dwWaitTimeSlice = eDefaultTimeSliceMS,
                                    DWORD dwTotalWaitMS = eDefaultTotalWaitMS);
	BOOL WaitOnTerminate(DWORD dwWaitTimeSliceMS = eDefaultTimeSliceMS);

protected:
	virtual void DoWork() = 0;

private:
	HANDLE m_hThread;

    // real wnd procedure:
	static void __cdecl TheirThreadProc(void *pv);
};

#endif