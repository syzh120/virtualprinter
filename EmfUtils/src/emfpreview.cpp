/////////////////////////////////////////////////////////////////////////////
// Name:        emfpreview.cpp
// Purpose:     Shows an EMF in preview
// Author:      Francesco Montorsi
// Modified by: 
// Created:     06/10/2007 12:40:13
// RCS-ID:      $Id$
// Copyright:   (c) Francesco Montorsi
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/window.h>
#include "emfpreview.h"
#include "emfutils.h"


// ----------------------------------------------------------------------------
// EMFPreviewFrame
// ----------------------------------------------------------------------------

IMPLEMENT_CLASS( EMFPreviewFrame, wxFrame )
BEGIN_EVENT_TABLE( EMFPreviewFrame, wxFrame )
END_EVENT_TABLE()

EMFPreviewFrame::EMFPreviewFrame(wxWindow *parent)
{
    Create(parent);
}

bool EMFPreviewFrame::Create(wxWindow *parent)
{
    // important: don't mess with wxFRAME_FLOAT_ON_PARENT
    //            as it would interfere with our always-on-top handling

    if (!wxFrame::Create( parent, wxID_ANY, wxT("EMF Preview window"),
                          wxDefaultPosition, wxSize(300, 200),
                          wxCAPTION|wxFRAME_TOOL_WINDOW|
                          wxRESIZE_BORDER|wxFRAME_NO_TASKBAR ))
        return false;

    m_pOldParent = parent;

    m_preview = new EMFPreviewWindow(this);

    wxBoxSizer *sz = new wxBoxSizer(wxHORIZONTAL);
    sz->Add(m_preview, 1, wxGROW);
    SetSizer(sz);

    return true;
}

EMFPreviewFrame::~EMFPreviewFrame()
{
}

void EMFPreviewFrame::AlwaysOnTop(bool enable)
{ 
    LONG style = ::GetWindowLong(GetHwnd(), GWL_EXSTYLE);

    if (enable)
    {
        ::SetWindowLong(GetHwnd(), GWL_EXSTYLE, style | WS_EX_TOPMOST);
        ::SetWindowPos(GetHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
    }
    else
    {
        ::SetWindowLong(GetHwnd(), GWL_EXSTYLE, style & ~WS_EX_TOPMOST);
        ::SetWindowPos(GetHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
    }
}

// ----------------------------------------------------------------------------
// EMFPreviewWindow
// ----------------------------------------------------------------------------

IMPLEMENT_CLASS( EMFPreviewWindow, wxWindow )
BEGIN_EVENT_TABLE( EMFPreviewWindow, wxWindow )
    EVT_PAINT( EMFPreviewWindow::OnPaint )
END_EVENT_TABLE()

void EMFPreviewWindow::DrawEMF(const wxString &file)
{
    m_meta = ReadEnhMetaFile(file.c_str(), GetHwnd());
    Refresh();
}

void EMFPreviewWindow::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc(this);

    dc.Clear();
    
    RECT rc;
    rc.top = 0;
    rc.left = 0;
    rc.bottom = GetClientSize().GetHeight();
    rc.right = GetClientSize().GetWidth();

    PlayEnhMetaFile((HDC)dc.GetHDC(), m_meta, &rc);
}



