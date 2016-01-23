/////////////////////////////////////////////////////////////////////////////
// Name:        emfrecordeditor.cpp
// Purpose:     Frame for editing at low-level the EMFs
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

#include <wx/aboutdlg.h>
#include <wx/listctrl.h>
#include <wx/file.h>
#include <wx/print.h>

#include "emfutils.h"
#include "emfrecordeditor.h"
#include "emfpreview.h"

#if (WINVER < 0x0500)
    #error Too old windows!
#endif


// ----------------------------------------------------------------------------
// record names
// ----------------------------------------------------------------------------

// VERY IMPORTANT: the records whose name ends with * are records which
//                 appear only in Windows Spooler format
char g_szRecordStrings[EMR_MAX][40] =
{
	"EMR_HEADER",				"EMR_POLYBEZIER",			"EMR_POLYGON", 
	"EMR_POLYLINE",				"EMR_POLYBEZIERTO",			"EMR_POLYLINETO",
	"EMR_POLYPOLYLINE",			"EMR_POLYPOLYGON",			"EMR_SETWINDOWEXTEX",
	"EMR_SETWINDOWORGEX",		"EMR_SETVIEWPORTEXTEX",		"EMR_SETVIEWPORTORGEX",
	"EMR_SETBRUSHORGEX",		"EMR_EOF",					"EMR_SETPIXELV",
	"EMR_SETMAPPERFLAGS",		"EMR_SETMAPMODE",			"EMR_SETBKMODE",
	"EMR_SETPOLYFILLMODE",		"EMR_SETROP2",				"EMR_SETSTRETCHBLTMODE",
	"EMR_SETTEXTALIGN",			"EMR_SETCOLORADJUSTMENT",	"EMR_SETTEXTCOLOR",
	"EMR_SETBKCOLOR",			"EMR_OFFSETCLIPRGN",		"EMR_MOVETOEX",
	"EMR_SETMETARGN",			"EMR_EXCLUDECLIPRECT",		"EMR_INTERSECTCLIPRECT", 
	"EMR_SCALEVIEWPORTEXTEX",	"EMR_SCALEWINDOWEXTEX",		"EMR_SAVEDC",
	"EMR_RESTOREDC",			"EMR_SETWORLDTRANSFORM",	"EMR_MODIFYWORLDTRANSFORM",
	"EMR_SELECTOBJECT",			"EMR_CREATEPEN",			"EMR_CREATEBRUSHINDIRECT",
	"EMR_DELETEOBJECT",			"EMR_ANGLEARC",				"EMR_ELLIPSE",
	"EMR_RECTANGLE",			"EMR_ROUNDRECT",			"EMR_ARC",
	"EMR_CHORD",				"EMR_PIE",					"EMR_SELECTPALETTE",
	"EMR_CREATEPALETTE",		"EMR_SETPALETTEENTRIES",	"EMR_RESIZEPALETTE",
	"EMR_REALIZEPALETTE",		"EMR_EXTFLOODFILL",			"EMR_LINETO",
	"EMR_ARCTO",				"EMR_POLYDRAW",				"EMR_SETARCDIRECTION",
	"EMR_SETMITERLIMIT",		"EMR_BEGINPATH",			"EMR_ENDPATH",
	"EMR_CLOSEFIGURE",			"EMR_FILLPATH",				"EMR_STROKEANDFILLPATH",
	"EMR_STROKEPATH",			"EMR_FLATTENPATH",			"EMR_WIDENPATH",
	"EMR_SELECTCLIPPATH",		"EMR_ABORTPATH",			"NO METAFILE RECORD 69",
	"EMR_GDICOMMENT",			"EMR_FILLRGN",				"EMR_FRAMERGN",
	"EMR_INVERTRGN",			"EMR_PAINTRGN",				"EMR_EXTSELECTCLIPRGN",
	"EMR_BITBLT",				"EMR_STRETCHBLT",			"EMR_MASKBLT",
	"EMR_PLGBLT",				"EMR_SETDIBITSTODEVICE",	"EMR_STRETCHDIBITS",
	"EMR_EXTCREATEFONTINDIRECTW","EMR_EXTTEXTOUTA",			"EMR_EXTTEXTOUTW",
	"EMR_POLYBEZIER16",			"EMR_POLYGON16",			"EMR_POLYLINE16",
	"EMR_POLYBEZIERTO16",		"EMR_POLYLINETO16",			"EMR_POLYPOLYLINE16",
	"EMR_POLYPOLYGON16",		"EMR_POLYDRAW16",			"EMR_CREATEMONOBRUSH",
	"EMR_CREATEDIBPATTERNBRUSHPT","EMR_EXTCREATEPEN",		"EMR_POLYTEXTOUTA",
	"EMR_POLYTEXTOUTW",			"EMR_SETICMMODE",			"EMR_CREATECOLORSPACE",
	"EMR_SETCOLORSPACE",		"EMR_DELETECOLORSPACE",		"EMR_GLSRECORD",
	"EMR_GLSBOUNDEDRECORD",		"EMR_PIXELFORMAT",          "EMR_DRAWESCAPE*",
    "EMR_EXTESCAPE*",           "EMR_STARTDOC*",            "EMR_SMALLTEXTOUT*",
    "EMR_FORCEUFIMAPPING*",     "EMR_NAMEDESCAPE*",         "EMR_COLORCORRECTPALETTE",
    "EMR_SETICMPROFILEA",       "EMR_SETICMPROFILEW",       "EMR_ALPHABLEND",
    "EMR_SETLAYOUT",            "EMR_TRANSPARENTBLT",       "EMR_TRANSPARENTDIB*",
    "EMR_GRADIENTFILL",         "EMR_SETLINKEDUFIS*",       "EMR_SETTEXTJUSTIFICATION*",
    "EMR_COLORMATCHTOTARGETW",  "EMR_CREATECOLORSPACEW"
};


// ----------------------------------------------------------------------------
// data for the current EMF
// ----------------------------------------------------------------------------

ENHMETAHEADER g_hMetaHeader;

HANDLETABLE *g_pHandleTable = NULL;
int g_nHandles;

ENHMETARECORD **g_pMetaRecords = NULL;
int g_nRecords, g_nRecordsMax;

void FreeEMFStuff()
{
    if (g_pMetaRecords)
    {
        for (int i=0; i<g_nRecords; i++)
            free((void*)g_pMetaRecords[i]);

        free(g_pMetaRecords);
        g_pMetaRecords = NULL;
    }

    if (g_pHandleTable)
    {
        free(g_pHandleTable);
        g_pHandleTable = NULL;
    }
}

// ----------------------------------------------------------------------------
// EMFRecordEditorFrame
// ----------------------------------------------------------------------------

IMPLEMENT_CLASS( EMFRecordEditorFrame, wxFrame )
BEGIN_EVENT_TABLE( EMFRecordEditorFrame, wxFrame )

////@begin EMFRecordEditorFrame event table entries
    EVT_MENU( wxID_OPEN, EMFRecordEditorFrame::OnOpen )

    EVT_MENU( ID_RELOAD, EMFRecordEditorFrame::OnReload )

    EVT_MENU( wxID_SAVE, EMFRecordEditorFrame::OnSave )

    EVT_MENU( wxID_PRINT, EMFRecordEditorFrame::OnPrint )

    EVT_MENU( wxID_EXIT, EMFRecordEditorFrame::OnExit )

    EVT_MENU( ID_SHOW_PREVIEW_WINDOW, EMFRecordEditorFrame::OnShowPreviewWindowClick )

    EVT_MENU( ID_PREVIEW_WINDOW_ALWAYS_TOP, EMFRecordEditorFrame::OnPreviewWindowAlwaysTopClick )

    EVT_MENU( wxID_ABOUT, EMFRecordEditorFrame::OnAbout )

    EVT_BUTTON( ID_DELETE_RECORD, EMFRecordEditorFrame::OnDeleteRecordClick )
    EVT_UPDATE_UI( ID_DELETE_RECORD, EMFRecordEditorFrame::OnDeleteRecordUpdate )

    EVT_BUTTON( ID_EDIT_RECORD, EMFRecordEditorFrame::OnEditRecordClick )
    EVT_UPDATE_UI( ID_EDIT_RECORD, EMFRecordEditorFrame::OnEditRecordUpdate )

    EVT_BUTTON( ID_MOVE_UP, EMFRecordEditorFrame::OnMoveUpClick )
    EVT_UPDATE_UI( ID_MOVE_UP, EMFRecordEditorFrame::OnMoveUpUpdate )

    EVT_BUTTON( ID_MOVE_DOWN, EMFRecordEditorFrame::OnMoveDownClick )
    EVT_UPDATE_UI( ID_MOVE_DOWN, EMFRecordEditorFrame::OnMoveDownUpdate )

    EVT_BUTTON( ID_INSERTNEW_RECORD, EMFRecordEditorFrame::OnInsertnewRecordClick )

////@end EMFRecordEditorFrame event table entries

END_EVENT_TABLE()


EMFRecordEditorFrame::EMFRecordEditorFrame(const wxString &fileName)
{
    Init();
    Create();

    Open(fileName);
    // UpdatePreview() already called by Open
}

bool EMFRecordEditorFrame::Create()
{
    wxFrame::Create( NULL, wxID_ANY, wxT("EMF Record Editor") );

    CreateControls();
    Centre();

    SetSize(GetMinSize().Scale(1.6, 2.4));

    return true;
}

EMFRecordEditorFrame::~EMFRecordEditorFrame()
{
////@begin EMFRecordEditorFrame destruction
////@end EMFRecordEditorFrame destruction

    m_preview->Hide();
    m_preview->Destroy();

    if (m_tempFile.FileExists())
        wxRemoveFile(m_tempFile.GetFullPath());

    FreeEMFStuff();
}

void EMFRecordEditorFrame::Init()
{
////@begin EMFRecordEditorFrame member initialisation
    m_pList = NULL;
////@end EMFRecordEditorFrame member initialisation

    // this is a constant value only read after this line
    m_tempFile.AssignTempFileName(wxT("emfpreview"));
}

void EMFRecordEditorFrame::CreateControls()
{    
////@begin EMFRecordEditorFrame content construction
    EMFRecordEditorFrame* itemFrame1 = this;

    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* itemMenu3 = new wxMenu;
    itemMenu3->Append(wxID_OPEN, _("Open..."), _("Open a new EMF for editing..."), wxITEM_NORMAL);
    itemMenu3->Append(ID_RELOAD, _("Reload"), _("Reloads the currently open EMF..."), wxITEM_NORMAL);
    itemMenu3->Append(wxID_SAVE, _("Save"), _("Save current records in the EMF..."), wxITEM_NORMAL);
    itemMenu3->Append(wxID_PRINT, _("Print..."), _("Prints the EMF..."), wxITEM_NORMAL);
    itemMenu3->AppendSeparator();
    itemMenu3->Append(wxID_EXIT, _("Exit"), _("Exits the program..."), wxITEM_NORMAL);
    menuBar->Append(itemMenu3, _("File"));
    wxMenu* itemMenu10 = new wxMenu;
    itemMenu10->Append(ID_SHOW_PREVIEW_WINDOW, _("Show preview window"), _("Shows the preview window..."), wxITEM_CHECK);
    itemMenu10->Check(ID_SHOW_PREVIEW_WINDOW, true);
    itemMenu10->Append(ID_PREVIEW_WINDOW_ALWAYS_TOP, _("Preview window always on top"), _("Make the preview window always on top..."), wxITEM_CHECK);
    menuBar->Append(itemMenu10, _("Preview"));
    wxMenu* itemMenu13 = new wxMenu;
    itemMenu13->Append(wxID_ABOUT, _("About..."), _("Shows info about this program..."), wxITEM_NORMAL);
    menuBar->Append(itemMenu13, _("Help"));
    itemFrame1->SetMenuBar(menuBar);

    wxPanel* itemPanel15 = new wxPanel( itemFrame1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxVERTICAL);
    itemPanel15->SetSizer(itemBoxSizer16);

    m_pList = new wxListCtrl( itemPanel15, ID_EMF_RECORDS, wxDefaultPosition, wxSize(100, 100), wxLC_REPORT );
    itemBoxSizer16->Add(m_pList, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer16->Add(itemBoxSizer18, 0, wxGROW|wxBOTTOM, 5);

    wxButton* itemButton19 = new wxButton( itemPanel15, ID_DELETE_RECORD, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton20 = new wxButton( itemPanel15, ID_EDIT_RECORD, _("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton21 = new wxButton( itemPanel15, ID_MOVE_UP, _("Move up"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton22 = new wxButton( itemPanel15, ID_MOVE_DOWN, _("Move down"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer18->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton24 = new wxButton( itemPanel15, ID_INSERTNEW_RECORD, _("Insert new"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStatusBar* itemStatusBar25 = new wxStatusBar( itemFrame1, wxID_ANY, wxST_SIZEGRIP|wxNO_BORDER );
    itemStatusBar25->SetFieldsCount(3);
    int itemStatusBar25Widths[3];
    itemStatusBar25Widths[0] = -3;
    itemStatusBar25Widths[1] = -2;
    itemStatusBar25Widths[2] = -1;
    itemStatusBar25->SetStatusWidths(3, itemStatusBar25Widths);
    itemFrame1->SetStatusBar(itemStatusBar25);

////@end EMFRecordEditorFrame content construction

    itemBoxSizer16->SetSizeHints(itemPanel15);
    itemBoxSizer16->SetSizeHints(this);

    m_pList->InsertColumn(0, wxT("Record type"));
    m_pList->InsertColumn(1, wxT("Record arguments"));

    // the preview window is initially shown
    m_preview = new EMFPreviewFrame(this);
    m_preview->Show(true);
}

bool EMFRecordEditorFrame::ShowToolTips()
{
    return true;
}

wxBitmap EMFRecordEditorFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin EMFRecordEditorFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end EMFRecordEditorFrame bitmap retrieval
}

wxIcon EMFRecordEditorFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin EMFRecordEditorFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end EMFRecordEditorFrame icon retrieval
}

void EMFRecordEditorFrame::Open(const wxString &file)
{
    m_file = file;

    SetStatusText(file, 1);
    SetDirty(false);

    if (!file.IsEmpty())
    {
        GetRecordListFromFile();
        UpdatePreview();
    }
}

bool EMFRecordEditorFrame::Save(const wxString &filename)
{
    // save the METAFILE our own way ;)
    wxFile out;
    if (!out.Open(filename, wxFile::write))
        return false;

    if (out.Write(&g_hMetaHeader, sizeof(g_hMetaHeader)) != sizeof(g_hMetaHeader))
        return false;

    for (int i=0; i<g_nRecords; i++)
    {
        int size = g_pMetaRecords[i]->nSize;
        if (out.Write(g_pMetaRecords[i], size) != size)
            return false;
    }

    return true;
}

wxString GetRecordName(ENHMETARECORD *lpEMFR)
{
    // If it's a known record type, get the string from the global string array
    if ((lpEMFR->iType>=EMR_MIN) && (lpEMFR->iType<=EMR_MAX))
        return wxString::FromAscii(g_szRecordStrings[lpEMFR->iType - EMR_MIN]);
    
    return wxT("Unknown");
}

wxString GetRecordArgs(ENHMETARECORD *lpEMFR)
{    
    wxString args;

    // For each parameter DWORD, add the string representation to the string
    for (int i=0;i<(lpEMFR->nSize/sizeof(DWORD))-2;i++)
        args += wxString::Format( wxT(" %x"), lpEMFR->dParm[i] );

    return args;
}

/* EMF record enumeration callback */
int CALLBACK EnumEnhMetafileProc( HDC hDC, 
                                  HANDLETABLE *lpHTable, 
                                  ENHMETARECORD *lpEMFR, 
                                  int nObj, 
                                  LPARAM lpData )
{
    wxListCtrl *pList = (wxListCtrl *)lpData;

    // VERY IMPORTANT: skip EMR_HEADER record as it's identical to the
    //                 ENHMETAHEADER which we have already extracted
    if (GetRecordName(lpEMFR) == wxT("EMR_HEADER"))
        return 1;

    // Add the new string to the list
    int n = pList->GetItemCount();
    pList->InsertItem(n, GetRecordName(lpEMFR));
    pList->SetItem(n, 1, GetRecordArgs(lpEMFR));

    // is the record array big enough?
    if (g_nRecords == g_nRecordsMax)
    {
        // expand our copy of ENH records
        g_pMetaRecords = (ENHMETARECORD**)realloc(g_pMetaRecords, 
                            (g_nRecordsMax+100)*sizeof(ENHMETARECORD));
        if (g_pMetaRecords)
            g_nRecordsMax += 100;
    }

    // add this record to our private list
    if (g_pMetaRecords)
    {
        g_pMetaRecords[g_nRecords] = (ENHMETARECORD*)malloc(lpEMFR->nSize);

        if (g_pMetaRecords[g_nRecords])
            memcpy(g_pMetaRecords[g_nRecords], 
                   lpEMFR, 
                   lpEMFR->nSize);
    }

    if (!g_pHandleTable)
    {
        g_pHandleTable = (HANDLETABLE*)malloc(nObj*sizeof(HGDIOBJ));
        if (g_pHandleTable)
            memcpy(g_pHandleTable, lpHTable, nObj*sizeof(HGDIOBJ));

        g_nHandles = nObj;
    }

    g_nRecords++;

    //wxLogMessage(wxT("the nObj is %d; lpHTable is %p "), nObj, lpHTable);

    return 1;
}

void EMFRecordEditorFrame::GetRecordListFromFile()
{
    HENHMETAFILE hMeta = ReadEnhMetaFile(m_file.GetFullPath(), GetHwnd());
    if (!hMeta)
        return;

    // get meta header
    if (GetEnhMetaFileHeader(hMeta, sizeof(g_hMetaHeader), &g_hMetaHeader) != 
        sizeof(ENHMETAHEADER))
        return;

    // init data touched by the callback
    FreeEMFStuff();
    g_pHandleTable = NULL;
    g_nHandles = 0;
    g_pMetaRecords = NULL;
    g_nRecords = 0;
    g_nRecordsMax = 0;
    m_pList->DeleteAllItems();

    // get the list of meta records
    EnumEnhMetaFile( NULL, 
                     hMeta, 
                     (ENHMFENUMPROC)EnumEnhMetafileProc, 
                     (LPVOID)m_pList, 
                     NULL );

    DeleteEnhMetaFile(hMeta);

    // update GUI
    m_pList->SetColumnWidth(0, wxLIST_AUTOSIZE);
    m_pList->SetColumnWidth(1, wxLIST_AUTOSIZE);

    SetStatusText(wxString::Format(wxT("%d records"), g_nRecords), 2);
}
/* not used
void EMFRecordEditorFrame::UpdateRecordList()
{
    m_pList->ClearAll();

    for (int i=0; i<g_nRecords; i++)
    {
        m_pList->InsertItem(i, GetRecordName(g_pMetaRecords[i]));
        m_pList->SetItem(i, 1, GetRecordArgs(g_pMetaRecords[i]));
    }

    // update GUI
    m_pList->SetColumnWidth(0, wxLIST_AUTOSIZE);
    m_pList->SetColumnWidth(1, wxLIST_AUTOSIZE);

    SetStatusText(wxString::Format(wxT("%d records"), g_nRecords), 2);
}*/

wxArrayInt EMFRecordEditorFrame::GetSelectedItems()
{
    wxArrayInt ret;
    for (int i=0; i<m_pList->GetItemCount(); i++)
        if (m_pList->GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            ret.Add(i);

    return ret;
}

void EMFRecordEditorFrame::SetDirty(bool dirty)
{
    wxString title = wxString::Format(wxT("EMF Record Editor [%s"), 
                                      m_file.GetFullName());

    if (dirty)
        title += wxT(" *]");
    else
        title += wxT("]");
    SetTitle(title);
}

void EMFRecordEditorFrame::UpdatePreview()
{
    if (!m_preview->IsShown())
        return;

    m_preview->ReleaseEMF();

    // update temp file
    if (Save(m_tempFile.GetFullPath()))
        m_preview->DrawEMF(m_tempFile.GetFullPath());
    else
        m_preview->Clear();
}

void EMFRecordEditorFrame::DeleteRecord(int idx)
{
    if (!g_pMetaRecords)
        return;

    // update header
    g_hMetaHeader.nBytes -= g_pMetaRecords[idx]->nSize;
    g_hMetaHeader.nRecords--;

    // free record data
    free(g_pMetaRecords[idx]);

    // shift the array of records 1 position to the left
    for (int i=idx; i<g_nRecords-1; i++)
        g_pMetaRecords[i] = g_pMetaRecords[i+1];
    g_pMetaRecords[g_nRecords] = NULL;

    g_nRecords--;

    // update GUI
    m_pList->DeleteItem(idx);
    SetStatusText(wxString::Format(wxT("%d records"), g_nRecords), 2);

    SetDirty();
    UpdatePreview();
}

void EMFRecordEditorFrame::SwapRecords(int idx1, int idx2)
{
    if (!g_pMetaRecords)
        return;
    if (idx1 < 0 || idx1 >= m_pList->GetItemCount())
        return;
    if (idx2 < 0 || idx2 >= m_pList->GetItemCount())
        return;

    // swap records
    ENHMETARECORD *temp = g_pMetaRecords[idx1];
    g_pMetaRecords[idx1] = g_pMetaRecords[idx2];
    g_pMetaRecords[idx2] = temp;

#define MASK        wxLIST_STATE_SELECTED

    // swap GUI items
    wxString name = m_pList->GetItemText(idx1);
    int state = m_pList->GetItemState(idx1, MASK);
    m_pList->SetItemText(idx1, m_pList->GetItemText(idx2));
    m_pList->SetItemState(idx1, m_pList->GetItemState(idx2, MASK), MASK);
    m_pList->SetItemText(idx2, name);
    m_pList->SetItemState(idx2, state, MASK);

    // swap also 2nd column
    wxListItem item1;
    item1.SetId(idx1);
    item1.SetColumn(1);
    item1.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_STATE);
    m_pList->GetItem(item1);

    wxListItem item2;
    item2.SetId(idx2);
    item2.SetColumn(1);
    item2.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_STATE);
    m_pList->GetItem(item2);

    m_pList->SetItem(idx1, 1, item2.GetText());
    m_pList->SetItem(idx2, 1, item1.GetText());

    SetDirty();
    UpdatePreview();
}


// ----------------------------------------------------------------------------
// EMFRecordEditorFrame - menu handlers
// ----------------------------------------------------------------------------

void EMFRecordEditorFrame::OnOpen( wxCommandEvent& event )
{
    wxFileDialog  fd(this, wxT("Choose a file"), wxEmptyString,
                     wxEmptyString, wxT("EMF files (*.emf)|*.emf"), 
                     wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (fd.ShowModal() == wxID_OK)
        Open(fd.GetPath());
}

void EMFRecordEditorFrame::OnExit( wxCommandEvent& event )
{
    Close(true);
}

void EMFRecordEditorFrame::OnAbout( wxCommandEvent& event )
{
    wxAboutDialogInfo info;
    info.SetName(wxT("EMF Record Editor"));
    info.SetCopyright(wxT("Francesco Montorsi (c) 2007"));
    info.SetDescription(wxT("Tool for low-level EMF editing"));
    info.SetVersion(wxT("1.0.0"));
    info.SetWebSite(wxT("http://emfprinter.sourceforge.net"));
    wxAboutBox(info);
}
 
void EMFRecordEditorFrame::OnReload( wxCommandEvent& event )
{
    Open(m_file.GetFullPath());
        // Open() will already call UpdatePreview()
}

void EMFRecordEditorFrame::OnSave( wxCommandEvent& event )
{
    if (!Save(m_file.GetFullPath()))
        wxLogError(wxT("Could not save!"));
    else
        SetDirty(false);
}

void EMFRecordEditorFrame::OnShowPreviewWindowClick( wxCommandEvent& event )
{
    m_preview->Show(event.IsChecked());
    UpdatePreview();
}

void EMFRecordEditorFrame::OnPreviewWindowAlwaysTopClick( wxCommandEvent& event )
{
    m_preview->AlwaysOnTop(event.IsChecked());
}

void EMFRecordEditorFrame::OnPrint( wxCommandEvent& event )
{
    wxPrintDialogData printDialogData;
    wxPrinter printer(&printDialogData);

    wxDC *pDC = printer.PrintDialog(this);
    if (pDC && pDC->IsOk())
    {
        m_preview->ReleaseEMF();

        // update temp file
        if (Save(m_tempFile.GetFullPath()))
        {
            wxMetafile meta(m_tempFile.GetFullPath());
            if (meta.IsOk())
            {
                pDC->StartDoc(m_file.GetName());
                pDC->StartPage();
                meta.Play(pDC);
                pDC->EndPage();
                pDC->EndDoc();
            }
        }


        // make the preview get an handle to the temp EMF again
        UpdatePreview();
    }
}


// ----------------------------------------------------------------------------
// EMFRecordEditorFrame - event handlers
// ----------------------------------------------------------------------------
 
void EMFRecordEditorFrame::OnDeleteRecordClick( wxCommandEvent& event )
{
    wxArrayInt sel = GetSelectedItems();
    for (int i=0; i<sel.GetCount(); i++)
        DeleteRecord(sel[i]);
}

void EMFRecordEditorFrame::OnEditRecordClick( wxCommandEvent& event )
{
    wxMessageBox(wxT("sorry - not implemented yet"));
}

void EMFRecordEditorFrame::OnInsertnewRecordClick( wxCommandEvent& event )
{
    wxMessageBox(wxT("sorry - not implemented yet"));
}

void EMFRecordEditorFrame::OnMoveUpClick( wxCommandEvent& event )
{
    wxArrayInt sel = GetSelectedItems();
    for (int i=0; i<sel.GetCount(); i++)
        SwapRecords(sel[i], sel[i]-1);
}

void EMFRecordEditorFrame::OnMoveDownClick( wxCommandEvent& event )
{
    wxArrayInt sel = GetSelectedItems();
    for (int i=0; i<sel.GetCount(); i++)
        SwapRecords(sel[i], sel[i]+1);
}


// ----------------------------------------------------------------------------
// EMFRecordEditorFrame - UI update handlers
// ----------------------------------------------------------------------------
 
void EMFRecordEditorFrame::OnDeleteRecordUpdate( wxUpdateUIEvent& event )
{
    event.Enable(m_pList->GetSelectedItemCount() > 0);
}

void EMFRecordEditorFrame::OnEditRecordUpdate( wxUpdateUIEvent& event )
{
    event.Enable(m_pList->GetSelectedItemCount() == 1);
}

void EMFRecordEditorFrame::OnMoveUpUpdate( wxUpdateUIEvent& event )
{
    event.Enable(m_pList->GetSelectedItemCount() == 1);
}

void EMFRecordEditorFrame::OnMoveDownUpdate( wxUpdateUIEvent& event )
{
    event.Enable(m_pList->GetSelectedItemCount() == 1);
}


