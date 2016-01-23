/*
 * spoolerdata.cpp
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
#include "spoolerdata.h"
#include "buffer.h"
#include "devmode.h"
#include "utils.h"

// -----------------------------------------------------------------------------
// SpoolerFileIterator
// -----------------------------------------------------------------------------

class SpoolerFileIterator
{
public:
    DWORD m_dwSize;
    DWORD m_dwOffset;
    union {
        PCBYTE m_pb;
        PCSPL_HEADER m_psplheader;
        PCSMR m_psmr;
        PCSMR_PAGE m_psmrpage;
        PCSMR_DEVMODE m_psmrdm;
    };

public:
    SpoolerFileIterator(IN const Buffer& buff, IN DWORD dwOffset);
    BOOL Next();
};


// -----------------------------------------------------------------------------
// SpoolerData
// -----------------------------------------------------------------------------

BOOL SpoolerData::LoadSpoolerFile(LPCTSTR fileName)
{
    DEBUGMESSAGE(("SpoolerData::LoadSpoolerFile"));

    // open the spooler file
    HANDLE hFile = CreateFile( 
			                fileName,
			                FILE_READ_DATA,           
			                FILE_SHARE_READ,        
			                NULL,                   
			                OPEN_EXISTING,          
			                FILE_ATTRIBUTE_NORMAL,  
			                NULL                     
			                );

    if (hFile == INVALID_HANDLE_VALUE) {
        DEBUGMESSAGE_LASTERROR(("SpoolerData::LoadSpoolerFile - cannot open '%ws'", fileName));
        return FALSE;
    }

    DWORD nSize = GetFileSize(hFile, NULL);
    if (nSize == INVALID_FILE_SIZE)  {
        DEBUGMESSAGE(("SpoolerData::LoadSpoolerFile - cannot get size of '%ws'", fileName));
        CloseHandle(hFile);
        return FALSE;
    }

    // allocate an array big enough for this spooler file
    try {
        m_data.RequireMinSize(nSize + 1024);
    } catch (...) {
		DEBUGMESSAGE_LASTERROR(("SpoolerData::LoadSpoolerFile - malloc failed"));
        CloseHandle(hFile);
        return FALSE;
	}

    DWORD dwRead = 0;
    if (!ReadFile(hFile, m_data.GetPtr(), nSize, &dwRead, NULL)) {
        DEBUGMESSAGE(("SpoolerData::LoadSpoolerFile - cannot read '%ws'", fileName));
        CloseHandle(hFile);
        return FALSE;
    }

    CloseHandle(hFile);

    return BuildInternalArrayFromData();
}

BOOL SpoolerData::ReadPrinterData(HANDLE hPrinter)
{
    DEBUGMESSAGE(("SpoolerData::ReadPrinterData"));

    try {

        // alloc first a buffer with a fixed size...
        m_data.RequireMinSize(0xFF);

        DWORD nTotal = 0;
        DWORD dwcRead = 0;
        do {
            dwcRead = 1;
            BOOL b = ReadPrinter(hPrinter, (BYTE*)m_data.GetPtr()+nTotal,     
                                 m_data.GetSize()-nTotal, &dwcRead);

            if (b && dwcRead == m_data.GetSize()-nTotal)
            { 
                // not enough space in our buffer...
                m_data.RequireMinSize(m_data.GetSize()*2);
            }
            else if (!b)
            {
                // what the hell happened here?
                DEBUGMESSAGE_LASTERROR(("SpoolerData::ReadPrinterData - "
                                        "problem reading data"));
                return FALSE;
            }

            nTotal += dwcRead;
        } while (dwcRead != 0);
	} catch(...) {
		DEBUGMESSAGE_LASTERROR(("ReadPrinterData - malloc failed"));
        return FALSE;
	}

    return BuildInternalArrayFromData();
}

BOOL SpoolerData::BuildInternalArrayFromData()
{
    // now iterate over the page contained in this spooler data
    SpoolerFileIterator iter(m_data, 0);

    if (iter.m_psplheader->SIGNATURE != SPLMETA_SIGNATURE) {
        DEBUGMESSAGE(("SpoolerData::Open - WRONG SIGNATURE"));
        return FALSE;
    }

    uint nPages = 0;
    while (iter.Next())
    {
        DWORD dwType = iter.m_psmr->iType;
        switch(dwType)
        {
        case SRT_PAGE:
            {
                PageInfo pi;
                pi.dwOffset = iter.m_dwOffset;
//                pi.uiDMIndex = (uint) m_vdmPages.size() - 1;
                m_vpi.push_back(pi);
            }
            break;

        case SRT_DEVMODE:
            {
                PCSMR_DEVMODE p = iter.m_psmrdm;
//                PDEVMODE pdmTemp = dmDuplicate( (PCDEVMODE) p->DEVMODEData);
//                m_vdmPages.push_back( (PEXTDEVMODE) pdmTemp );

                // DEVMODE RECORD SEEMS TO FOLLOW THE EMF
                if (m_vpi.size() != 0) {
                    uint uiPage = (uint) m_vpi.size() - 1;
                    PageInfo& pi = m_vpi[uiPage];
  //                  pi.uiDMIndex = (uint) m_vdmPages.size() - 1;
                }
            }
            break;

        default:
            break;
        }
    }

    return TRUE;
}

uint SpoolerData::GetPageCount() const
{
    return (uint) m_vpi.size();
}

size_t SpoolerData::GetPageEMFSize(uint uiPage) const
{
    ASSERT(uiPage < m_vpi.size());

    PageInfo pi = m_vpi[uiPage];
    PCSMR_PAGE psmrpage = (PCSMR_PAGE) ((BYTE*)m_data.GetPtr() + pi.dwOffset);
    return psmrpage->smr.nSize;
}

HENHMETAFILE SpoolerData::GetPageEMF(uint uiPage) const
{
    DEBUGMESSAGE(("SpoolerData::GetPageEMF"));

    HENHMETAFILE hMeta = NULL;
    ASSERT(uiPage < m_vpi.size());

    try {
        PageInfo pi = m_vpi[uiPage];
        PCSMR_PAGE psmrpage = (PCSMR_PAGE) ((BYTE*)m_data.GetPtr() + pi.dwOffset);
        hMeta = SetEnhMetaFileBits(psmrpage->smr.nSize, psmrpage->EMFData);
    } catch( ... ) {
        // RETURN FAILURE CODE OF NULL
    }

    return hMeta;
}

HMETAFILE SpoolerData::GetPageWMF(uint uiPage) const
{
    DEBUGMESSAGE(("SpoolerData::GetPageWMF"));

    HENHMETAFILE hEMF = GetPageEMF(uiPage);
    if (!hEMF)
        return NULL;

    // convert to WMF

    HDC hdc = ::GetDC(NULL);

    // first get the buffer size and alloc memory
    size_t size = ::GetWinMetaFileBits(hEMF, 0, NULL, MM_ANISOTROPIC, hdc);
    if (!size) {
        DEBUGMESSAGE_LASTERROR((
            "SpoolerData::GetPageWMF - CANNOT GET DATA SIZE FOR %d-th PAGE",
            uiPage));
        ::ReleaseDC(NULL, hdc);
        return NULL;
    }

    BYTE *bits = (BYTE *)malloc(size);
    if (!bits) {
        ::ReleaseDC(NULL, hdc);
        return NULL;
    }

    // then get the enh metafile bits
    if ( !::GetWinMetaFileBits(hEMF, size, bits, MM_ANISOTROPIC, hdc) ) {
        DEBUGMESSAGE_LASTERROR((
            "SpoolerData::GetPageWMF - CANNOT GET DATA FOR %d-th PAGE",
            uiPage));
        ::ReleaseDC(NULL, hdc);
        free(bits);
        return NULL;
    }

    // and finally convert them to the WMF
    HMETAFILE hMF = ::SetMetaFileBitsEx(size, bits);

    free(bits);
    ::ReleaseDC(NULL, hdc);

    return hMF;
}

BOOL SpoolerData::SaveEMF(uint uiPage, LPCTSTR fileName) const
{
    DEBUGMESSAGE(("SpoolerData::SaveEMF"));

    HENHMETAFILE hPage = GetPageEMF(uiPage);
    if (!hPage) {
        DEBUGMESSAGE_LASTERROR(("SpoolerData::SaveEMF - CANNOT GET %d-th PAGE",
                                uiPage));
        return FALSE;
    }

    HENHMETAFILE hFile = CopyEnhMetaFile(hPage, fileName);
    if (hFile == NULL){
        DEBUGMESSAGE_LASTERROR(("SpoolerData::SaveEMF - CANNOT WRITE %d-th PAGE",
                                uiPage));
        DeleteEnhMetaFile(hPage);
        return FALSE;
    }

    DeleteEnhMetaFile(hPage);
    DeleteEnhMetaFile(hFile);

    return TRUE;
}

BOOL SpoolerData::SaveWMF(uint uiPage, LPCTSTR fileName) const
{
    DEBUGMESSAGE(("SpoolerData::SaveWMF"));

    HMETAFILE hPage = GetPageWMF(uiPage);
    if (!hPage) {
        DEBUGMESSAGE_LASTERROR(("SpoolerData::SaveWMF - CANNOT GET %d-th PAGE",
                                uiPage));
        return FALSE;
    }

    HMETAFILE hFile = CopyMetaFile(hPage, fileName);
    if (hFile == NULL){
        DEBUGMESSAGE_LASTERROR(("SpoolerData::SaveWMF - CANNOT WRITE %d-th PAGE",
                                uiPage));
        DeleteMetaFile(hPage);
        return FALSE;
    }

    DeleteMetaFile(hPage);
    DeleteMetaFile(hFile);

    return TRUE;
}

BOOL SpoolerData::Save(uint uiPage, LPCTSTR fileName, 
                       LPCTSTR format, LPCTSTR convOpt,
                       EXECPROC fnCallback,
                       LPVOID callbackParam) const
{
    if (_tcscmp(format, TEXT("EMF")) == 0 ||
        _tcscmp(format, TEXT("emf")) == 0)
        return SaveEMF(uiPage, fileName);

    if (_tcscmp(format, TEXT("WMF")) == 0 ||
        _tcscmp(format, TEXT("wmf")) == 0)
        return SaveWMF(uiPage, fileName);


    // should be a raster format... use ImageMagick command-line utility
    // to get the conversion done ;)

    // first save the EMF into a temp file
    TCHAR outFile[MAX_PATH+1] = { L'\0' };
    if (!GetFullTempFileName(outFile, MAX_PATH+1))
        return FALSE;

    if (!SaveEMF(uiPage, outFile))
        return FALSE;

    LPTSTR fullPath = GetFullAppPath();
    if (!fullPath)
        return FALSE;

    LPTSTR cmdLine = strCat(TEXT("\""), fullPath, TEXT("convert.exe\" \""),
                            outFile, TEXT("\" "), convOpt,
                            TEXT(" -format "), format,
                            TEXT(" \""), fileName, TEXT("\""),
                            NULL);

    // FIXME: can be a long op, show a "working, please wait" window
    BOOL bExec = ExecuteProgram(cmdLine, TRUE /* console app */,    
                                TRUE /* wait */, fnCallback, callbackParam);

    strFree(fullPath);
    strFree(cmdLine);

    return bExec;
}


// -----------------------------------------------------------------------------
// IMPLEMENTATION - SpoolerFileIterator
// -----------------------------------------------------------------------------

SpoolerFileIterator::SpoolerFileIterator(IN const Buffer& buff, IN DWORD dwOffset)
{
    m_dwSize = buff.GetSize();
    ASSERT(dwOffset < m_dwSize);
    m_pb = (PCBYTE) buff.GetPtr() + dwOffset;
    m_dwOffset = dwOffset;
}

BOOL SpoolerFileIterator::Next()
{
    BOOL bRetValue = FALSE;
    DWORD dwNewOffset;
    DWORD dwcInc;

    // (SPL FILE HEADER RECORD ALSO STARTS W/SMR, WITH "iType" BEING THE SIGNATURE)
    dwcInc = m_psmr->nSize;
    if (m_psmr->iType != SPLMETA_SIGNATURE)
        dwcInc += sizeof(SMR);

    dwNewOffset = m_dwOffset + dwcInc;
    if (dwNewOffset < m_dwSize)
    {
        m_dwOffset = dwNewOffset;
        m_pb += dwcInc;
        if (m_psmr->iType != SRT_PAGE ||
            m_psmr->nSize != 0
            ) {
            bRetValue = TRUE;
        } 
    }

    return bRetValue;
}



