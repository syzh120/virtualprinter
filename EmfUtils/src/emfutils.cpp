/////////////////////////////////////////////////////////////////////////////
// Name:        emfutils.cpp
// Purpose:     EMF fix utility
// Author:      Francesco Montorsi
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: minimal.cpp 47801 2007-07-29 00:31:51Z VZ $
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

//
// NOTE: to be able to use wxMetafile class we must build in GUI-mode,
//       i.e. we cannot create a console app
//

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
 
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
 
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/metafile.h"
#include "wx/filename.h"
#include "wx/cmdline.h"
#include "wx/dir.h"

#include "emfutils.h"
#include "emfrecordeditor.h"

#include <wingdi.h>


IMPLEMENT_APP(EmfUtilsApp)


// ============================================================================
// implementation of a WMF reader routine taken from the ENHMETA app
// ============================================================================

// Aldus Placeable Header ===================================================
// Since we are a 32bit app, we have to be sure this structure compiles to
// be identical to a 16 bit app's version. To do this, we use the #pragma
// to adjust packing, we use a WORD for the hmf handle, and a SMALL_RECT
// for the bbox rectangle.
#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
	DWORD		dwKey;
	WORD		hmf;
	SMALL_RECT	bbox;
	WORD		wInch;
	DWORD		dwReserved;
	WORD		wCheckSum;
} APMHEADER, *PAPMHEADER;
#pragma pack( pop )
// ==========================================================================

HENHMETAFILE ReadEnhMetaFile( LPCTSTR szFileName, HWND hWndParent )
{
	HENHMETAFILE	hTemp;
	HMETAFILE		hOld;
	DWORD			dwSize;
	LPBYTE			pBits;
	METAFILEPICT	mp;
	HDC				hDC;
	HANDLE			hFile;

	// First try to read it as an enhanced metafile
	// If it works, simply return the handle
	if( (hTemp = GetEnhMetaFile( szFileName )) != NULL )
		return hTemp;

	// It was apparently not an enhanced metafile, so try 16bit windows metafile
	if( (hOld = GetMetaFile( szFileName )) != NULL )
	{
		// Ok, it is a 16bit windows metafile
		// How big are the bits?
		if( (dwSize = GetMetaFileBitsEx( hOld, 0, NULL )) == 0 )
		{
			DeleteMetaFile( hOld );
			MessageBoxA( hWndParent, "Failed to Get MetaFile Bits Size", 
                         "Error Reading MetaFile", MB_OK );
			return NULL;
		}

		// Allocate that much memory
		if( (pBits = (LPBYTE)malloc( dwSize )) == NULL )
		{
			DeleteMetaFile( hOld );
			MessageBoxA( hWndParent, "Failed to Allocate Memory for Metafile Bits", 
                         "Error Reading MetaFile", MB_OK );
			return NULL;
		}

		// Get the metafile bits
		if( GetMetaFileBitsEx( hOld, dwSize, pBits ) == 0 )
		{
			free( pBits );
			DeleteMetaFile( hOld );
			MessageBoxA( hWndParent, "Failed to Get MetaFile Bits", 
                         "Error Reading MetaFile", MB_OK );
			return NULL;
		}

		// Fill out a METAFILEPICT structure
		mp.mm = MM_ANISOTROPIC;
		mp.xExt = 1000;
		mp.yExt = 1000;
		mp.hMF = NULL;

		// Get a reference DC
		hDC = GetDC( NULL );

		// Make an enhanced metafile from the windows metafile
		hTemp = SetWinMetaFileBits( dwSize, pBits, hDC, &mp );

		// Clean up
		ReleaseDC( NULL, hDC );
		DeleteMetaFile( hOld );
		free( pBits );
		if( hTemp == NULL )
			MessageBoxA( hWndParent, "Failed to Create MetaFile from Bits", 
                         "Error Reading MetaFile", MB_OK );
		return hTemp;
	}

	// If we got here, the file of interest is neither an enhanced metafile nor a
	// 16bit windows metafile. Let's assume it's a placeable metafile and move on.
	// First, open the file for reading
	hFile = CreateFile( szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
                        FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE )
	{
		MessageBoxA( hWndParent, "Error Opening File", "Error", MB_OK );
		return NULL;
	}

	// How big is the file?
	dwSize = GetFileSize( hFile, NULL );
	
    // Allocate enough memory to hold it
	pBits = (LPBYTE)malloc( dwSize );
	
    // Read it in
	ReadFile( hFile, pBits, dwSize, &dwSize, NULL );
	
    // Close the file
	CloseHandle( hFile );
	
    // Is it a placeable metafile? (check the key)
	if( ((PAPMHEADER)pBits)->dwKey != 0x9ac6cdd7l )
	{
		// Not a metafile that we know how to recognise - bail out
		free( pBits );
		MessageBoxA( hWndParent, "Not a Valid Metafile", "Error", MB_OK );
		return NULL;
	}
	
    // Ok, its a placeable metafile
	// Fill out a METAFILEPICT structure
	mp.mm = MM_ANISOTROPIC;
	mp.xExt = ((PAPMHEADER)pBits)->bbox.Right - ((PAPMHEADER)pBits)->bbox.Left;
	mp.xExt = ( mp.xExt * 2540l ) / (DWORD)(((PAPMHEADER)pBits)->wInch);
	mp.yExt = ((PAPMHEADER)pBits)->bbox.Bottom - ((PAPMHEADER)pBits)->bbox.Top;
	mp.yExt = ( mp.yExt * 2540l ) / (DWORD)(((PAPMHEADER)pBits)->wInch);
	mp.hMF = NULL;
	
    // Get a reference DC
	hDC = GetDC( NULL );
	
    // Create an enhanced metafile from the bits
	hTemp = SetWinMetaFileBits( dwSize, &(pBits[sizeof(APMHEADER)]), hDC, &mp );
	
    // Clean up
	ReleaseDC( NULL, hDC );
	free( pBits );
	if( hTemp == NULL )
		MessageBoxA( hWndParent, "Failed to Create MetaFile from Bits", 
                     "Error Reading MetaFile", MB_OK );
	return hTemp;
}




// ============================================================================
// implementation
// ============================================================================

bool EmfUtilsApp::CropMargins(const wxString& fileName)
{
    if (!m_quiet)
        wxLogMessage(wxT("Cropping '%s'..."), fileName);
    
    wxString fileTemp = wxFileName::CreateTempFileName(wxT("emfutil"));

    {
        wxMetafile file(fileName);
        if (!file.IsOk())
            return false;

        wxMetafileDC dc(fileTemp);
        if (!dc.IsOk())
            return false;

        // ignore eventually play errors as they are not fatal
        if (!file.Play(&dc) && !m_quiet)
            wxLogWarning(wxT(" ...the file '%s' did not playback perfectly"), fileName);

        // close the DC and destroy the associated metafile;
        // we just want to keep the version on the disk
        delete dc.Close();
    }

    if (!wxCopyFile(fileTemp, fileName))
        return false;

    wxRemoveFile(fileTemp);
    
    return true;
}

bool EmfUtilsApp::Rotate(const wxString& fileName)
{
    if (!m_quiet)
        wxLogMessage(wxT("Rotating '%s'..."), fileName);
    
    wxString fileTemp = wxFileName::CreateTempFileName(wxT("emfutil"));

    {
        wxMetafile file(fileName);
        if (!file.IsOk())
            return false;

        wxMetafileDC dc(fileTemp);
        if (!dc.IsOk())
            return false;

        HDC hdc = (HDC)dc.GetHDC();

        if (SetGraphicsMode(hdc, GM_ADVANCED) == 0)
            wxLogLastError(wxT(" ...could not set graphics mode"));

        double q = m_nDegrees*PI/180.0;   // in radiants
        XFORM xform; 
        xform.eM11 = cos(q);
        xform.eM12 = sin(q);
        xform.eM21 = -sin(q);
        xform.eM22 = cos(q);
        xform.eDx  = (FLOAT) 0.0; 
        xform.eDy  = (FLOAT) 0.0; 
        if (!SetWorldTransform(hdc, &xform))
            wxLogLastError(wxT(" ...could not set world transform matrix"));

        // ignore eventually play errors as they are not fatal
        if (!file.Play(&dc))
            wxLogWarning(wxT(" ...the file '%s' did not playback perfectly"), fileName);

        // close the DC and destroy the associated metafile;
        // we just want to keep the version on the disk
        delete dc.Close();
    }

    if (!wxCopyFile(fileTemp, fileName))
        return false;

    wxRemoveFile(fileTemp);
    
    return true;
}

bool EmfUtilsApp::Convert(const wxString& fileName)
{
    if (!m_quiet)
        wxLogMessage(wxT("Converting '%s' from WMF to EMF..."), fileName.c_str());

    // get a handle to the source metafile
    HENHMETAFILE hEmf = ReadEnhMetaFile(fileName.c_str(), NULL);

    // create a disk-based destination metafile
    wxFileName fn(fileName);
    fn.SetExt(wxT("emf"));
    HENHMETAFILE hNewEmf = CopyEnhMetaFile(hEmf, fn.GetFullPath().wc_str());
    if (hNewEmf == NULL)
    {
        wxLogLastError(wxT("Cannot create the final EMF..."));
        return false;
    }

    // cleanup
    DeleteEnhMetaFile(hEmf);
    DeleteEnhMetaFile(hNewEmf);

    return true;
}


// ----------------------------------------------------------------------------
// EmfUtilsApp control routines
// ----------------------------------------------------------------------------

bool EmfUtilsApp::OnInit()
{
    m_backup = false;
    m_quiet = false;

    // don't do wx cmd line processing, do our own

    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_SWITCH, "h", "help", "show this help message",
            wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_SWITCH, "q", "quiet", "be quiet" },
        { wxCMD_LINE_SWITCH, "b", "backup", "do a backup copy" },

        { wxCMD_LINE_PARAM,  NULL, NULL, "mode", wxCMD_LINE_VAL_STRING, 0 },

        { wxCMD_LINE_PARAM,  NULL, NULL, "mode parameters or input-files",
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL|wxCMD_LINE_PARAM_MULTIPLE },

        { wxCMD_LINE_NONE }
    };

    wxCmdLineParser parser(cmdLineDesc, argc, argv);

    switch ( parser.Parse() )
    {
        case 0:
            m_quiet = parser.Found(wxT("quiet"));
            m_backup = parser.Found(wxT("backup"));
            m_mode = parser.GetParam();
            for (int i=1; i<parser.GetParamCount(); i++)
            {
                if (wxIsWild(parser.GetParam(i)))
                    AddMatchingFiles(parser.GetParam(i));
                else
                    m_input.Add(parser.GetParam(i));
            }
            return true;

        default:
            wxMessageBox(wxT("The allowed modes are: 'edit', 'crop', 'rotate', 'convert'."),
                         wxT("emfutils mode help"));
            return false;
    }

    return true;
}

int EmfUtilsApp::OnRun()
{
    if (m_mode.CmpNoCase(wxT("edit")) == 0)
    {
        // at max 1 filename is allowed
        if (m_input.GetCount() > 1)
        {
            wxLogError(wxT("This workmode does not support more than a filename!"));
            return 1;
        }

        wxString arg;
        if (m_input.GetCount() == 1)
            arg = m_input[0];

        // show the GUI
        EMFRecordEditorFrame *frame = new EMFRecordEditorFrame(arg);
        frame->Show(true);

        // leave control to the main loop
        return wxApp::OnRun();
    }
    else if (m_mode.CmpNoCase(wxT("rotate")) == 0)
    {
        if (m_input.GetCount() < 2)
        {
            wxLogError(wxT("Rotate mode requires the number of degree and then ")
                       wxT("at least a filename."));
            return 1;
        }

        if (!m_input[0].ToLong(&m_nDegrees))
        {
            wxLogError(wxT("Invalid number of degrees: '%s'"), m_input[0]);
            return 1;
        }

        m_input.RemoveAt(0);        // remove first argument
    }
    else
    {
        // all other work mode require at least 1 filename!
        if (m_input.GetCount() == 0)
        {
            wxLogError(wxT("This workmode requires at least one filename!"));
            return 1;
        }
    }

    for (int i=0; i<m_input.GetCount(); i++)
    {
        if (!Process(m_input[i]))
            wxLogError(wxT("Error while processing '%s'"), m_input[i]);
    }

    return 0;
}

void EmfUtilsApp::AddMatchingFiles(const wxString &pattern)
{
    wxArrayString files;
    wxDir::GetAllFiles(wxGetCwd(), &files, wxEmptyString, wxDIR_FILES);

    for (int i=0; i<files.GetCount(); i++)
        if (wxMatchWild(pattern, files[i]))
            m_input.Add(files[i]);
}

bool EmfUtilsApp::Process(const wxString& fileName)
{
    if (m_backup)
        wxCopyFile(fileName, fileName + wxT(".bak"));

    if (m_mode.CmpNoCase(wxT("crop")) == 0)
        return CropMargins(fileName);
    else if (m_mode.CmpNoCase(wxT("rotate")) == 0)
        return Rotate(fileName);
    else if (m_mode.CmpNoCase(wxT("convert")) == 0)
        return Convert(fileName);
    else
        wxLogError(wxT("Invalid mode '%s'"), m_mode);

    return true;
}
