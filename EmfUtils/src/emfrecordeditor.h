/////////////////////////////////////////////////////////////////////////////
// Name:        emfrecordeditor.h
// Purpose:     Frame for editing at low-level the EMFs
// Author:      Francesco Montorsi
// Modified by: 
// Created:     06/10/2007 12:40:13
// RCS-ID:      $Id$
// Copyright:   (c) Francesco Montorsi
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef _EMFRECORDEDITOR_H_
#define _EMFRECORDEDITOR_H_

/*!
 * Includes
 */

#include "wx/frame.h"
#include "wx/filename.h"
#include "emfpreview.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_RELOAD 10008
#define ID_SHOW_PREVIEW_WINDOW 10006
#define ID_PREVIEW_WINDOW_ALWAYS_TOP 10007
#define ID_EMF_RECORDS 10000
#define ID_DELETE_RECORD 10001
#define ID_EDIT_RECORD 10003
#define ID_MOVE_UP 10004
#define ID_MOVE_DOWN 10005
#define ID_INSERTNEW_RECORD 10002
#define SYMBOL_EMFRECORDEDITORFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_EMFRECORDEDITORFRAME_TITLE _("EMF Record Editor")
#define SYMBOL_EMFRECORDEDITORFRAME_IDNAME wxID_ANY
#define SYMBOL_EMFRECORDEDITORFRAME_SIZE wxSize(400, 300)
#define SYMBOL_EMFRECORDEDITORFRAME_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * EMFRecordEditorFrame class declaration
 */

class EMFRecordEditorFrame: public wxFrame
{    
    DECLARE_CLASS( EMFRecordEditorFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    EMFRecordEditorFrame(const wxString &fileName);

    /// Destructor
    ~EMFRecordEditorFrame();

public:     // event handlers

////@begin EMFRecordEditorFrame event handler declarations

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_OPEN
    void OnOpen( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_RELOAD
    void OnReload( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_SAVE
    void OnSave( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_PRINT
    void OnPrint( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
    void OnExit( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_SHOW_PREVIEW_WINDOW
    void OnShowPreviewWindowClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_PREVIEW_WINDOW_ALWAYS_TOP
    void OnPreviewWindowAlwaysTopClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ABOUT
    void OnAbout( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DELETE_RECORD
    void OnDeleteRecordClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_DELETE_RECORD
    void OnDeleteRecordUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EDIT_RECORD
    void OnEditRecordClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_EDIT_RECORD
    void OnEditRecordUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_MOVE_UP
    void OnMoveUpClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_MOVE_UP
    void OnMoveUpUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_MOVE_DOWN
    void OnMoveDownClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_MOVE_DOWN
    void OnMoveDownUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_INSERTNEW_RECORD
    void OnInsertnewRecordClick( wxCommandEvent& event );

////@end EMFRecordEditorFrame event handler declarations

public:     // misc

    bool Create();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    void GetRecordListFromFile();
    //void UpdateRecordList();
    void UpdatePreview();
    void SetDirty(bool dirty = true);

    void Open(const wxString &file);
    bool Save(const wxString &file);
    
    void DeleteRecord(int idx);
    void SwapRecords(int idx1, int idx2);
    wxArrayInt GetSelectedItems();

protected:      // member variables

    wxFileName m_file;              // the EMF being edited
    wxFileName m_tempFile;          // the temporary EMF file used for preview
    EMFPreviewFrame *m_preview;

////@begin EMFRecordEditorFrame member variables
    wxListCtrl* m_pList;
////@end EMFRecordEditorFrame member variables
};

#endif
    // _EMFRECORDEDITOR_H_
