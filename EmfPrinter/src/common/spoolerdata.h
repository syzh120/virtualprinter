/*
 * spoolerdata.h
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

#ifndef SPOOLERDATAFILE_H
#define SPOOLERDATAFILE_H

#include "extdevmode.h"
#include "undocumented.h"
#include "buffer.h"
#include <windows.h>
#include <vector>

typedef const SPL_HEADER *PCSPL_HEADER;
typedef const SMR *PCSMR;
typedef const SMR_PAGE *PCSMR_PAGE;
typedef const SMR_DEVMODE *PCSMR_DEVMODE;

// this is an in-memory SPOOLER DATAFILE...
class SpoolerData
{
public:
    SpoolerData() {}
    SpoolerData(const SpoolerData& tocopy)
        : m_vpi(tocopy.m_vpi), m_data(tocopy.m_data) {}

    virtual ~SpoolerData() {}

    // initialization

    // reads data directly from the printer enlarging the internal
    // buffer as needed
    virtual BOOL ReadPrinterData(HANDLE hPrinter);

    // loads data from the spooler file with the given name
    virtual BOOL LoadSpoolerFile(LPCTSTR fileName);

    // misc

    virtual size_t GetPageEMFSize(uint uiPage) const;
	virtual uint GetPageCount() const;

    // page-extraction
    
    virtual HMETAFILE GetPageWMF(uint uiPage) const;
	virtual HENHMETAFILE GetPageEMF(uint uiPage) const;

    // vector formats:

    virtual BOOL SaveEMF(uint uiPage, LPCTSTR fileName) const;
    virtual BOOL SaveWMF(uint uiPage, LPCTSTR fileName) const;

    // any-format save:

    virtual BOOL Save(uint uiPage, LPCTSTR fileName, 
                      LPCTSTR format, LPCTSTR convOpt = NULL,
                      EXECPROC fnCallback = NULL,
                      LPVOID callbackParam = NULL) const;

private:

    // init the m_vpi array from m_data
    BOOL BuildInternalArrayFromData();

	struct PageInfo
    {
		DWORD dwOffset;
		uint uiDMIndex; // INDEX INTO m_vdmPages FOR THIS PAGE
	};

	std::vector<PageInfo> m_vpi;
	Buffer m_data;

    // FIXME: what are these EXTDEVMODE useful for?
	//std::vector<PEXTDEVMODE> m_vdmPages;
};

#endif
