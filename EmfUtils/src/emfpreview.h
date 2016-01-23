/////////////////////////////////////////////////////////////////////////////
// Name:        emfpreview.h
// Purpose:     Shows an EMF in "preview"
// Author:      Francesco Montorsi
// Modified by: 
// Created:     06/10/2007 12:40:13
// RCS-ID:      $Id$
// Copyright:   (c) Francesco Montorsi
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef _PREVIEW_FRAME_H_
#define _PREVIEW_FRAME_H_

/*!
 * Includes
 */

#include <wx/frame.h>
#include <wx/metafile.h>


/*!
 * EMFPreviewWindow class declaration
 */

class EMFPreviewWindow: public wxWindow
{    
    DECLARE_CLASS( EMFPreviewWindow )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    EMFPreviewWindow(wxWindow *parent)
        : wxWindow(parent, wxID_ANY) { m_meta=NULL; }
    ~EMFPreviewWindow()
        { ReleaseEMF(); }

public:     // misc

    void DrawEMF(const wxString &file);
    void OnPaint(wxPaintEvent &ev);

    // just releases the METAFILE so that the
    // relative file can be overwritten
    void ReleaseEMF()
        { DeleteEnhMetaFile(m_meta); m_meta = NULL; }

    // like ReleaseEMF but also clears the window to the background
    void Clear()
        { ReleaseEMF(); Refresh(); }

protected:
    HENHMETAFILE m_meta;
};


/*!
 * EMFPreviewFrame class declaration
 */

class EMFPreviewFrame: public wxFrame
{    
    DECLARE_CLASS( EMFPreviewFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    EMFPreviewFrame(wxWindow *parent);

    /// Destructor
    ~EMFPreviewFrame();

public:     // misc

    bool Create(wxWindow *parent);

    void AlwaysOnTop(bool enable);

    void DrawEMF(const wxString &file)
        { m_preview->DrawEMF(file); }
    void Clear()
        { m_preview->Clear(); }
    void ReleaseEMF()
        { m_preview->ReleaseEMF(); }

public:
    EMFPreviewWindow *m_preview;
    wxWindow *m_pOldParent;
};


#endif
    // _PREVIEW_FRAME_H_
