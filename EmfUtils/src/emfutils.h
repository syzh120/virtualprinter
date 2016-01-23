/////////////////////////////////////////////////////////////////////////////
// Name:        emfutils.h
// Purpose:     EMF fix utility
// Author:      Francesco Montorsi
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: minimal.cpp 47801 2007-07-29 00:31:51Z VZ $
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _EMFUTILS_H_
#define _EMFUTILS_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/app.h"
#include "wx/filename.h"
#include "wx/cmdline.h"
#include "wx/dir.h"

#ifndef __WXMSW__
    #error Sorry, this apps builds only on wxMSW
#endif
#if !wxUSE_UNICODE
    #error Please build in Unicode mode
#endif

#define PI  3.14159265


// ----------------------------------------------------------------------------
// EmfUtilsApp
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class EmfUtilsApp : public wxApp
{
public:
    virtual int OnRun();
    virtual bool OnInit();

public:     // implementation
        
    bool CropMargins(const wxString& fileName);
    bool Rotate(const wxString& fileName);
    bool Convert(const wxString& fileName);

public:     // control routines

    bool Process(const wxString& fileName);
    void AddMatchingFiles(const wxString &pattern);

protected:
    wxString m_mode;
    wxArrayString m_input;
    bool m_backup;
    bool m_quiet;

    long m_nDegrees;
};


// ----------------------------------------------------------------------------
// utilities
// ----------------------------------------------------------------------------

HENHMETAFILE ReadEnhMetaFile( LPCTSTR szFileName, HWND hWndParent );

#endif      // _EMFUTILS_H_
