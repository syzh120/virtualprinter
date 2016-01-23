//
// WARNING: ANY USE BY YOU OF THIS CODE IS AT YOUR OWN RISK. 
// Microsoft provides this code "as is" without warranty of any
// kind, either express or implied, including but not limited to the implied
// warranties of merchantability and/or fitness for a particular purpose.
//
/****************************************************************************

    FILE:    EnhMeta.C

    PURPOSE: Provide functions for manipulating metafiles

	FUNCTIONS:
		WinMain							- Calls initializations, message loop
		InitApplication					- Initialization for this application
		WndProc							- Window procedure for main window
		PlayEnhMetaFileAtOriginalSize	- Plays metafile at its intended size
		PlayEnhMetaFileInRect			- Scales metafile and plays in a RECT
		PrepareOpenGL					- Prepare's target DC to handle OpenGL
		CleanupOpenGL					- Makes the OpenGL RC non-current and deletes it
		GetEnhMetaFileFromClipboard		- Gets a metafile from the clipboard
		PutEnhMetaFileOnClipboard		- Puts the current metafile on the clipboard
		GetPrinterDC					- Gets a DC for the printer
		AbortProc						- Abort Procedure for printing
		PrintEnhMetaFile				- Gets a printer DC and plays metafile on it
		InfoDlgProc						- Dialog Procedure for "Metafile Info" dialog
		GetEnhMetaFileName				- Get user input for file name
		WriteEnhMetaFileAsPlaceable		- Writes metafile as 16bit aldus placeable
		ReadEnhMetaFile					- Reads metafile, converting if necessary
		WriteEnhMetaFileAsRegular		- Write metafile as 16bit normal metafile
		AboutDlgProc					- Dialog procedure for about dialog
		RecordsDlgProc					- Dialog procedure for "Metafile Records" dialog
		EnumEnhMetafileProc				- Metafile Enumeration callback procedure
		GetEnhancedMetafilePalette		- Gets the palette from a metafile
	
    HISTORY:	1/10/96 - wrote it - jmh
				10/7/97 - modified it to handle opengl - dfw
				5/26/98 - Modified to handle doube-buffered OpenGL - dfw

****************************************************************************/
#include <Windows.H>
#include "gl/gl.h"
#include "gl/glu.h"
#include "gl/glaux.h"
#include "Resource.h"


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



// Function Prototypes ======================================================
int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow);
BOOL InitApplication( HANDLE hInstance, int nCmdShow );
LRESULT CALLBACK WndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam );
BOOL PlayEnhMetaFileAtOriginalSize( HDC hDC, HENHMETAFILE hMeta );
BOOL PlayEnhMetaFileInRect( HDC hDC, HENHMETAFILE hMeta, RECT Rect );
BOOL PrepareOpenGL(HDC hDC, HENHMETAFILE hMeta);
BOOL CleanupOpenGL();
HENHMETAFILE GetEnhMetaFileFromClipboard( HWND hWnd );
BOOL PutEnhMetaFileOnClipboard( HWND hWnd, HENHMETAFILE hMeta );
HDC GetPrinterDC( void );
BOOL CALLBACK AbortProc( HDC hDC, int Error );
BOOL PrintEnhMetaFile( HENHMETAFILE hMeta, BOOL bPrintOriginalSize );
BOOL CALLBACK InfoDlgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
BOOL GetEnhMetaFileName( HWND hWndParent, LPTSTR szFileName, BOOL bSaving );
BOOL WriteEnhMetaFileAsPlaceable( HENHMETAFILE hMeta, LPTSTR szFileName );
HENHMETAFILE ReadEnhMetaFile( LPTSTR szFileName, HWND hWndParent );
BOOL WriteEnhMetaFileAsRegular( HENHMETAFILE hMeta, LPTSTR szFileName );
BOOL CALLBACK AboutDlgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK RecordsDlgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
int CALLBACK EnumEnhMetafileProc( HDC hDC, HANDLETABLE *lpHTable, ENHMETARECORD *lpEMFR, int nObj, LPARAM lpData );
HPALETTE GetEnhancedMetafilePalette( HENHMETAFILE hMeta );
// ==========================================================================



// Global Variables =========================================================
// The names of the records that can be recorded into an enhanced metafile
// This is used to show the names in the "records" dialog box
TCHAR	szRecordStrings[EMR_MAX][40] = {
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
	"EMR_GLSBOUNDEDRECORD",		"EMR_PIXELFORMAT" };


HINSTANCE		hInst;
HWND			hWndMain;
char			szAppName[] = "EnhMeta";
char			szTitle[] = "Enhanced Metafile Sample";
BOOL			bOriginalSize = TRUE;	// Are we displaying at original size?
HENHMETAFILE	hEnhMeta = NULL;		// Handle to the current enhanced metafile
HPALETTE		hEnhMetaPal = NULL;		// Palette for the current enhanced metafile

/* OpenGL-specific globals... */
HGLRC			hRC = NULL;
HINSTANCE		hOpengl32 = NULL;
HINSTANCE		hGlu32 = NULL;
BOOL			bSwapAfterPlayback = FALSE;

// End Global Variables
// ==========================================================================



// ==========================================================================
/*
  FUNCTION   : WinMain

  PARAMETERS :	HANDLE - this instance
				HANDLE - previous instance
				LPSTR  - command line
				int    - requested display state

  PURPOSE    : calls initialization function, processes message loop

  RETURNS    : int

  COMMENTS   : Windows recognizes this function by name as the initial entry
               point for the program.  This function calls the application
               initialization routine, if no other instance of the program is
               running, and always calls the instance initialization routine.
               It then executes a message retrieval and dispatch loop that is
               the top-level control structure for the remainder of execution.
               The loop is terminated when a WM_QUIT message is received, at
               which time this function exits the application instance by
               returning the value passed by PostQuitMessage().
               If this function must abort before entering the message loop,
               it returns the value FALSE.

  HISTORY    : 1/10/96 - Created
  
*/
int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	MSG msg;

	// Handle all initialization chores
	if( ! InitApplication( hInstance, nCmdShow ) )
	{
		return FALSE;
	}
	// If we have a command line, open that file
	if( lstrlen(lpszCmdLine) > 0 )
	{
		hEnhMeta = ReadEnhMetaFile( lpszCmdLine, hWndMain );
		hEnhMetaPal = GetEnhancedMetafilePalette( hEnhMeta );
		InvalidateRect( hWndMain, NULL, TRUE );
	}
	// Message Loop
	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	return msg.wParam;
}
// End WinMain
// ==========================================================================



// ==========================================================================
/*
  FUNCTION   : InitApplication

  PARAMETERS : HANDLE hInstance  - This instance
               int     nCmdShow  - Param for first ShowWindow() call
 
  PURPOSE    : Initializes window data and registers window class, creates window

  RETURNS    : BOOL - TRUE is Success, FALSE is failure

  COMMENTS   : This function is called at initialization time.  This function
               performs initialization tasks like initializing a window class
			   by filling out a data structure of type WNDCLASS and calling the
			   Windows RegisterClass() function. It then creates and shows the
			   main application window.

  HISTORY    : 4/17/95 - Created

*/
BOOL InitApplication( HANDLE hInstance, int nCmdShow )
{
	WNDCLASS wc;

	// Store the instance handle globally for future use
	hInst = hInstance;

	// Fill out a WNDCLASS struct so we can register our class
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon( hInstance, MAKEINTRESOURCE(ENHMETA_ICON) );
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = MAKEINTRESOURCE(ENHMETA_MENU);
	wc.lpszClassName = szAppName;

	// Register this new window class for our main window
	if( ! RegisterClass( &wc ) )
		return FALSE;

	// Create the main window from the new class
	hWndMain = CreateWindow(
		szAppName,
		szTitle,
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0,
		NULL,
		NULL,
		hInstance,
		NULL );

	// Did it create a main window?
	if( hWndMain == NULL )
		return FALSE;

	// We have a window, display it
	ShowWindow( hWndMain, nCmdShow );
	UpdateWindow( hWndMain );

	return TRUE;
}
// End InitInstance
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : MainWndProc

  PARAMETERS : HWND hWnd        -  window handle
               UINT message     -  type of message
               WPARAM wParam    -  additional information
               LPARAM lParam    -  additional information

  PURPOSE    : Processes messages for the main window.

  RETURNS    : LRESULT

  COMMENTS   : 

  HISTORY    : 1/10/96 - Created

*/
LRESULT CALLBACK WndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
	switch( Message )
	{
		// Palette has changed
		case WM_PALETTECHANGED:
			// If we caused the change, ignore the message
			if( (HWND)wParam == hWnd )
				break;
		// Here's our chance to get our palette entries in the system palette
		case WM_QUERYNEWPALETTE:
		{
			HDC			hDC;
			HPALETTE	hOldPal;

			// Does the current metafile even have a palette?
			if( hEnhMetaPal != NULL )
			{
				// Ok, need a DC to select the palette into
				hDC = GetDC( hWnd );
				// Select it in as a foreground palette
				hOldPal = SelectPalette( hDC, hEnhMetaPal, FALSE );
				// Realize the palette
				if( RealizePalette(hDC) )
					InvalidateRect( hWnd, NULL, TRUE );
				// clean up
				SelectPalette( hDC, hOldPal, TRUE );
				RealizePalette( hDC );
				ReleaseDC( hWnd, hDC );
			}
		}
		break; // End WM_PALETTECHANGED and WM_QUERYNEWPALETTE

		// Window needs painting
		case WM_PAINT:
		{
			PAINTSTRUCT	ps;
			HPALETTE	hOldPal;

			// Inform windows we will be painting (which also gets a DC)
			BeginPaint( hWnd, &ps );
			// Do we have a current enhanced metafile?
			if( hEnhMeta != NULL )
			{
				// We have a metafile, lets get to drawing it
				// Do we have a palette?
				if( hEnhMetaPal != NULL )
				{
					// Yes, we have a palette, select and realize it
					hOldPal = SelectPalette( ps.hdc, hEnhMetaPal, FALSE );
					RealizePalette( ps.hdc );
				}

				// Should we draw at the intended size?
				if( bOriginalSize )
				{
					// Yes, play the metafile at its intended size
					PlayEnhMetaFileAtOriginalSize( ps.hdc, hEnhMeta );
				}
				else
				{
					// No, stretch the metafile to fill the client area
					// Note that the "metafile device" (the RECT passed in
					// to the create call) will be mapped to the client area 
					RECT	Rect;
					GetClientRect( hWnd, &Rect );
					SetStretchBltMode( ps.hdc, COLORONCOLOR );
					PlayEnhMetaFileInRect( ps.hdc, hEnhMeta, Rect );
				}
				// If we are using a palette, select it back out
				if( hEnhMetaPal != NULL )
				{
					SelectPalette( ps.hdc, hOldPal, FALSE );
				}
			}
			else 
			{	
				// We have no current metafile
				// So we'll put up text saying so
				#define NO_META_TEXT	"No Metafile Selected"
				RECT	Rect;
				int		x, y;
				SIZE	size;

				// Simply center the text in the client area
				GetTextExtentPoint32( ps.hdc, NO_META_TEXT, lstrlen(NO_META_TEXT), &size );
				GetClientRect( hWnd, &Rect );
				x = Rect.left + ( (Rect.right - Rect.left - size.cx ) / 2 );
				y = Rect.top + ( (Rect.bottom - Rect.top - size.cy ) / 2 );
				TextOut( ps.hdc, x, y, NO_META_TEXT, lstrlen( NO_META_TEXT ) );
				#undef NO_META_TEXT
			}
			// Inform Windows that we are done painting
			EndPaint( hWnd, &ps );
		}
		break; // End WM_PAINT

		// Enable and Check menu items based on the current state of the program
		case WM_INITMENU:
			EnableMenuItem( (HMENU)wParam, ID_FILE_OPEN, MF_BYCOMMAND | MF_ENABLED );
			EnableMenuItem( (HMENU)wParam, ID_FILE_SAVEREGULAR, MF_BYCOMMAND | (hEnhMeta==NULL?MF_GRAYED:MF_ENABLED) );
			EnableMenuItem( (HMENU)wParam, ID_FILE_SAVEPLACEABLE, MF_BYCOMMAND | (hEnhMeta==NULL?MF_GRAYED:MF_ENABLED) );
			EnableMenuItem( (HMENU)wParam, ID_FILE_SAVEENHANCED, MF_BYCOMMAND | (hEnhMeta==NULL?MF_GRAYED:MF_ENABLED) );
			EnableMenuItem( (HMENU)wParam, ID_FILE_PRINT, MF_BYCOMMAND | (hEnhMeta==NULL?MF_GRAYED:MF_ENABLED) );
			EnableMenuItem( (HMENU)wParam, ID_FILE_EXIT, MF_BYCOMMAND | MF_ENABLED );
			EnableMenuItem( (HMENU)wParam, ID_EDIT_COPY, MF_BYCOMMAND | (hEnhMeta==NULL?MF_GRAYED:MF_ENABLED) );
			EnableMenuItem( (HMENU)wParam, ID_EDIT_PASTE, MF_BYCOMMAND | (IsClipboardFormatAvailable(CF_ENHMETAFILE)?MF_ENABLED:MF_GRAYED) );
			EnableMenuItem( (HMENU)wParam, ID_EDIT_ORIGINALSIZE, MF_BYCOMMAND | (hEnhMeta==NULL?MF_GRAYED:MF_ENABLED) );
			EnableMenuItem( (HMENU)wParam, ID_EDIT_STRETCHTOFIT, MF_BYCOMMAND | (hEnhMeta==NULL?MF_GRAYED:MF_ENABLED) );
			EnableMenuItem( (HMENU)wParam, ID_EDIT_METAFILEINFO, MF_BYCOMMAND | (hEnhMeta==NULL?MF_GRAYED:MF_ENABLED) );
			EnableMenuItem( (HMENU)wParam, ID_EDIT_METAFILERECORDS, MF_BYCOMMAND | (hEnhMeta==NULL?MF_GRAYED:MF_ENABLED) );
			CheckMenuItem( (HMENU)wParam, ID_EDIT_ORIGINALSIZE, MF_BYCOMMAND | (bOriginalSize?MF_CHECKED:MF_UNCHECKED) );
			CheckMenuItem( (HMENU)wParam, ID_EDIT_STRETCHTOFIT, MF_BYCOMMAND | (bOriginalSize?MF_UNCHECKED:MF_CHECKED) );
		break; // End WM_INITMENU

		// Handle menu items being chosen
		case WM_COMMAND:
			switch( LOWORD( wParam ) )
			{
				// File->Exit - shut down
				case ID_FILE_EXIT:
					PostMessage( hWnd, WM_CLOSE, 0, 0 );
				break;
				// File->Open - Open a metafile file
				case ID_FILE_OPEN:
				{
					TCHAR	szFileName[MAX_PATH] = "";
					// Get the file name
					if( GetEnhMetaFileName( hWnd, szFileName, FALSE ) )
					{
						// If we have a current metafile, delete it
						if( hEnhMeta != NULL )
							DeleteEnhMetaFile( hEnhMeta );
						// If we have a current palette, delete it
						if( hEnhMetaPal != NULL )
							DeleteObject( hEnhMetaPal );
						// Read in the new metafile
						hEnhMeta = ReadEnhMetaFile( szFileName, hWnd );
						// Get its palette if it has one
						hEnhMetaPal = GetEnhancedMetafilePalette( hEnhMeta );
						// Force a repaint
						InvalidateRect( hWnd, NULL, TRUE );
					}
				}
				break;
				// File->Save Regular - Save as 16bit windows metafile
				case ID_FILE_SAVEREGULAR:
				{
					TCHAR			szFileName[MAX_PATH] = "";
					// Get the file name
					if( GetEnhMetaFileName( hWnd, szFileName, TRUE ) )
					{
						// Write the current enhanced metafile as a 16bit wmf
						WriteEnhMetaFileAsRegular( hEnhMeta, szFileName );
					}
				}
				break;
				// File->Save Placeable - Save as 16bit aldus placeable metafile
				case ID_FILE_SAVEPLACEABLE:
				{
					TCHAR			szFileName[MAX_PATH] = "";
					// Get the file name
					if( GetEnhMetaFileName( hWnd, szFileName, TRUE ) )
					{
						// Write the current metafile as a 16bit aldus placeable metafile
						WriteEnhMetaFileAsPlaceable( hEnhMeta, szFileName );
					}
				}
				break;
				// File->Save Enhanced - Save as a 32bit enhanced metafile
				case ID_FILE_SAVEENHANCED:
				{
					TCHAR			szFileName[MAX_PATH] = "";
					HENHMETAFILE	hTemp;
					// Get the file name
					if( GetEnhMetaFileName( hWnd, szFileName, TRUE ) )
					{
						// Use the copy API to write it out
						hTemp = CopyEnhMetaFile( hEnhMeta, szFileName );
						DeleteEnhMetaFile( hTemp );
					}
				}
				break;
				// File->Print - print the current metafile
				case ID_FILE_PRINT:
					PrintEnhMetaFile( hEnhMeta, bOriginalSize );
				break;
				// Edit->Copy - Copy the current metafile to the clipboard
				case ID_EDIT_COPY:
					PutEnhMetaFileOnClipboard( hWnd, hEnhMeta );
				break;
				// Edit->Paste - Get a new current metafile from the clipboard
				case ID_EDIT_PASTE:
					// If we have a current metafile, delete it
					if( hEnhMeta != NULL )
						DeleteEnhMetaFile( hEnhMeta );
					// If we have a current palette, delete it
					if( hEnhMetaPal != NULL )
						DeleteObject( hEnhMetaPal );
					// Get the metafile from the clipboard
					hEnhMeta = GetEnhMetaFileFromClipboard( hWnd );
					// Get the palette if the metafile has one
					hEnhMetaPal = GetEnhancedMetafilePalette( hEnhMeta );
					// Force a repaint
					InvalidateRect( hWnd, NULL, TRUE );
				break;
				// Edit->Original Size - Show metafile at its intended size
				case ID_EDIT_ORIGINALSIZE:
					// Set the flag and force a repaint
					bOriginalSize = TRUE;
					InvalidateRect( hWnd, NULL, TRUE );
				break;
				// Edit->Stretch to Fit - Stretch metafile to fill client area
				case ID_EDIT_STRETCHTOFIT:
					// Set the flag and force a repaint
					bOriginalSize = FALSE;
					InvalidateRect( hWnd, NULL, TRUE );
				break;
				// Edit->Metafile Info ... - Show a dialog that displays the metafile header information
				case ID_EDIT_METAFILEINFO:
					DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_METAINFODLG ), hWnd, InfoDlgProc, (LPARAM)hEnhMeta );
				break;
				// Edit->Metafile Records ... - Show a ddialog that lists the records in the current metafile
				case ID_EDIT_METAFILERECORDS:
					DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_METARECORDSDLG ), hWnd, RecordsDlgProc, (LPARAM)hEnhMeta );
				break;
				// Help->About - a simple about dialog
				case ID_HELP_ABOUT:
					DialogBox( hInst, MAKEINTRESOURCE( IDD_ABOUTDLG ), hWnd, AboutDlgProc );
				break;
				default:
				break;
			}
		break; // End WM_COMMAND

		// Time to say "goodbye"
		case WM_CLOSE:
			DestroyWindow( hWnd );
			PostQuitMessage( 0 );
		break;

		default:
			return DefWindowProc( hWnd, Message, wParam, lParam );
		break;
	}
	return 0l;
}
// End WndProc
// ==========================================================================


// ==========================================================================
/*

  FUNCTION   : PlayEnhMetaFileAtOriginalSize

  PARAMETERS : HDC  hDC            -  DC in which to draw
               HENHMETAFILE hMeta  -  Enhanced metafile to play

  PURPOSE    : Plays the enhanced metafile on the DC at the size
               that the metafile was inteded to be displayed by
			   inspecting the header and extacting the size

  RETURNS    : BOOL - TRUE for success, FALSE for failure

  COMMENTS   : 
 
  HISTORY    :	1/10/96 - Created
				10/7/97 - Modified to handle OpenGL - dfw
				5/26/98 - Modified to handle doube-buffered OpenGL - dfw
*/
BOOL PlayEnhMetaFileAtOriginalSize( HDC hDC, HENHMETAFILE hMeta )
{
	BOOL			bFlag;
	ENHMETAHEADER	emh;
	RECT			Rect;
	float			PixelsX, PixelsY, MMX, MMY;

	// Do we have a metafile? If not, consider it drawn!
	if( hMeta == NULL )
		return TRUE;
	
	// Get the header from the enhanced metafile
	ZeroMemory( &emh, sizeof(ENHMETAHEADER) );
	emh.nSize = sizeof(ENHMETAHEADER);
	if( GetEnhMetaFileHeader( hMeta, sizeof( ENHMETAHEADER ), &emh ) == 0 )
		return FALSE;
	// Get the characteristics of the output device
	PixelsX = (float)GetDeviceCaps( hDC, HORZRES );
	PixelsY = (float)GetDeviceCaps( hDC, VERTRES );
	MMX = (float)GetDeviceCaps( hDC, HORZSIZE );
	MMY = (float)GetDeviceCaps( hDC, VERTSIZE );

	// Calculate the rect in which to draw the metafile based on the
	// intended size and the current output device resolution
	// Remember that the intended size is given in 0.01mm units, so
	// convert those to device units on the target device
	Rect.top = (int)((float)(emh.rclFrame.top) * PixelsY / (MMY*100.0f));
	Rect.left = (int)((float)(emh.rclFrame.left) * PixelsX / (MMX*100.0f));
	Rect.right = (int)((float)(emh.rclFrame.right) * PixelsX / (MMX*100.0f));
	Rect.bottom = (int)((float)(emh.rclFrame.bottom) * PixelsY / (MMY*100.0f));

	// If the enhanced metafile contains OpenGL records, attempt to set up target DC for OpenGL...
	if (emh.bOpenGL)
		PrepareOpenGL(hDC, hMeta);

	// Play the metafile in that rectangle
	bFlag = PlayEnhMetaFile( hDC, hMeta, &Rect );

	// Swap OpenGL buffers if OpenGL doublebuffered metafile...
	if ((emh.bOpenGL) && (bSwapAfterPlayback))
		SwapBuffers(hDC);

	// If the enhanced metafile contained OpenGL records, cleanup OpenGL...
	if (emh.bOpenGL)
		CleanupOpenGL(hDC, hMeta);

	return bFlag;

}
// End PlayEnhMetaFileAtOriginalSize
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : PlayEnhMetaFileInRect

  PARAMETERS : HDC          hDC    -  DC in which to draw
               HENHMETAFILE hMeta  -  Enhanced metafile to play
			   RECT	        Rect   -  Target Rect in which to play

  PURPOSE    : Plays the enhanced metafile on the target DC in the
               rectangle specified by the last parameter.

  RETURNS    : BOOL - TRUE for success, FALSE for failure

  COMMENTS   : 
 
  HISTORY    :	1/10/96 - Created
				10/7/97 - Modified to handle OpenGL - dfw
				5/26/98 - Modified to handle doube-buffered OpenGL - dfw
*/
BOOL PlayEnhMetaFileInRect( HDC hDC, HENHMETAFILE hMeta, RECT Rect )
{
	BOOL			bFlag;
	ENHMETAHEADER	emh;

	// Do we have a metafile? If not, consider it drawn!
	if( hMeta == NULL )
		return TRUE;

	// Get the header from the enhanced metafile
	ZeroMemory( &emh, sizeof(ENHMETAHEADER) );
	emh.nSize = sizeof(ENHMETAHEADER);
	if( GetEnhMetaFileHeader( hMeta, sizeof( ENHMETAHEADER ), &emh ) == 0 )
		return FALSE;

	// If the enhanced metafile contains OpenGL records, attempt to set up target DC for OpenGL...
	if (emh.bOpenGL)
		PrepareOpenGL(hDC, hMeta);

	// Play the metafile in that rectangle
	bFlag = PlayEnhMetaFile( hDC, hMeta, &Rect );

	// Swap OpenGL buffers if OpenGL doublebuffered metafile...
	if ((emh.bOpenGL) && (bSwapAfterPlayback))
		SwapBuffers(hDC);

	// If the enhanced metafile contained OpenGL records, cleanup OpenGL...
	if (emh.bOpenGL)
		CleanupOpenGL(hDC, hMeta);

	return bFlag;
}
// End PlayEnhMetaFileInRect
// ==========================================================================


// ==========================================================================
/*

  FUNCTION   : PrepareOpenGL

  PARAMETERS : HDC          hDC    -  DC which OpenGL is about to be used in
               HENHMETAFILE hMeta  -  Enhanced metafile about to be played

  PURPOSE    : Prepares the target DC for OpenGL rendering based on
			   the metafile's PIXELFORMATDESCRIPTOR.

  RETURNS    : BOOL - TRUE for success, FALSE for failure

  COMMENTS   : OpenGL rendering to enhanced metafiles works only on NT (as of 10/7/97).
 
  HISTORY    : 10/7/97 - Created (by dfw)

*/
BOOL PrepareOpenGL(HDC hDC, HENHMETAFILE hMeta)
{
	UINT iRet;
	BOOL bRet;
	int iPixelFormat;
	PIXELFORMATDESCRIPTOR pfd;
	HINSTANCE hGDI32 = NULL;
	FARPROC fnGetEnhMetaFilePixelFormat = NULL;
	FARPROC fnwglCreateContext = NULL;
	FARPROC fnwglMakeCurrent = NULL;

	/* Is OpenGL on this machine? */
	SetLastError(0);
	hOpengl32 = LoadLibrary("opengl32.dll");
	if (!hOpengl32)
	{
		DWORD dwErr;
		dwErr = GetLastError();
		return FALSE;
	}
	hGlu32 = LoadLibrary("glu32.dll");
	if (!hGlu32)
		return FALSE;

	/* Get addresses the OpenGL functions we'll need... */
    fnwglCreateContext = GetProcAddress(hOpengl32, "wglCreateContext");
	if (!fnwglCreateContext)
		return FALSE;
	fnwglMakeCurrent = GetProcAddress(hOpengl32, "wglMakeCurrent");
	if (!fnwglMakeCurrent)
		return FALSE;

	/* Can this OS handle OpenGL within metafiles? */
	hGDI32 = GetModuleHandle("gdi32.dll");
	if (!hGDI32)
		return FALSE;
    fnGetEnhMetaFilePixelFormat = GetProcAddress(hGDI32, "GetEnhMetaFilePixelFormat");
	if (!fnGetEnhMetaFilePixelFormat)
		return FALSE;

	/* Get the OpenGL pixel format from the metafile... */
	iRet = (int)(fnGetEnhMetaFilePixelFormat)(hMeta, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	if ((iRet == 0) || (iRet == GDI_ERROR))
		return FALSE;

	/* Set the OpenGL pixel format to the one specified in the metafile... */
	iPixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (iPixelFormat == 0)
		return FALSE;
	bRet = SetPixelFormat(hDC, iPixelFormat, &pfd);
	if (!bRet)
		return FALSE;

	/* We need to swap after playback if we're double-buffered... */
	bSwapAfterPlayback = FALSE;
	if (pfd.dwFlags |= PFD_DOUBLEBUFFER)
		bSwapAfterPlayback = TRUE;

	/* Create an OpenGL rendering context... */
	hRC = (HGLRC)((fnwglCreateContext)(hDC));
	if (!hRC)
		return FALSE;

	/* Make the OpenGL rendering context current... */
	bRet = (BOOL)((fnwglMakeCurrent)(hDC, hRC));
	if (!bRet)
		return FALSE;

	return TRUE;
}
// End PrepareOpenGL
// ==========================================================================


// ==========================================================================
/*

  FUNCTION   : CleanupOpenGL

  PARAMETERS :

  PURPOSE    : Makes the OpenGL rendering context non-current and deletes it.

  RETURNS    : BOOL - TRUE for success, FALSE for failure

  COMMENTS   : OpenGL rendering to enhanced metafiles works only on NT (as of 10/7/97).
 
  HISTORY    : 10/7/97 - Created (by dfw)

*/
BOOL CleanupOpenGL()
{
	FARPROC fnwglDeleteContext = NULL;
	FARPROC fnwglMakeCurrent = NULL;

	/* This handle should have been created in PrepareOpenGL...if it wasn't, OpenGL isn't installed... */
	if (!hOpengl32)
		return FALSE;

	/* Get addresses the OpenGL functions we'll need... */
	fnwglDeleteContext = GetProcAddress(hOpengl32, "wglDeleteContext");
	if (!fnwglDeleteContext)
		return FALSE;
	fnwglMakeCurrent = GetProcAddress(hOpengl32, "wglMakeCurrent");
	if (!fnwglMakeCurrent)
		return FALSE;

	/* Free the rendering context... */
	if (hRC)
	{
		(fnwglMakeCurrent)(NULL, NULL);
		(fnwglDeleteContext)(hRC);
		hRC = NULL;
	}

	/* Free the OpenGL libraries (created in PrepareOpenGL)... */
	if (hOpengl32)
	{
		FreeLibrary(hOpengl32);
		hOpengl32 = NULL;
	}
	if (hGlu32)
	{
		FreeLibrary(hGlu32);
		hGlu32 = NULL;
	}

	return TRUE;
}
// End CleanupOpenGL
// ==========================================================================


// ==========================================================================
/*

  FUNCTION   : GetEnhMetaFileFromClipboard

  PARAMETERS : HWND    hWnd    -  Parent window (to own clipboard)

  PURPOSE    : Extracts an enhanced metafile from the clipboard

  RETURNS    : HENHMETAFILE - the handle of the new enhanced metafile

  COMMENTS   : This function is called only if a CF_ENHMETAFILE is
               available on the clipboard. Disabling the menu item for
			   this function when one is not available prevents this
			   function from being called if one is not available.
 
  HISTORY    : 1/10/96 - Created

*/
HENHMETAFILE GetEnhMetaFileFromClipboard( HWND hWnd )
{
	HENHMETAFILE hTemp;

	// Open the clipboard
	OpenClipboard( hWnd );
	// Get the CF_ENHMETAFILE data
	hTemp = (HENHMETAFILE)GetClipboardData( CF_ENHMETAFILE );
	// Close the clipboard
	CloseClipboard();
	// If we failed to get CF_ENHMETAFILE data, return a NULL handle
	if( hTemp == NULL )
		return NULL;
	// Else make a copy and send it back 
	// (we don't own the original, the clipboard does)
	return CopyEnhMetaFile( hTemp, NULL );
}
// End GetEnhMetaFileFromClipboard
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : PutEnhMetaFileOnClipboard

  PARAMETERS : HWND         hWnd   -  Parent window (to own clipboard)
               HENHMETAFILE hMeta  -  Enhanced metafile to play

  PURPOSE    : Extracts an enhanced metafile from the clipboard

  RETURNS    : BOOL - TRUE for success, FALSE for failure

  COMMENTS   : This function copies the current enhanced metafile to the
               clipboard in CF_ENHMETAFILE format.
 
  HISTORY    : 1/10/96 - Created

*/
BOOL PutEnhMetaFileOnClipboard( HWND hWnd, HENHMETAFILE hMeta )
{
	// Open the clipboard
	OpenClipboard( hWnd );
	// Clear it out
	EmptyClipboard();
	// Send a copy of our current metafile
	SetClipboardData( CF_ENHMETAFILE, CopyEnhMetaFile( hMeta, NULL ) );
	// Close it
	CloseClipboard();
	return TRUE;
}
// End PutEnhMetaFileOnClipboard
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : GetPrinterDC

  PARAMETERS : void

  PURPOSE    : Uses Print Common Dialog to get a printer DC

  RETURNS    : HDC	- the printer DC. NULL on failure

  COMMENTS   : 
  
  HISTORY    : 1/10/96 - Created

*/
HDC GetPrinterDC( void )
{
	PRINTDLG pdlg;

	// Initialize the structure
	ZeroMemory( &pdlg, sizeof( PRINTDLG ) );
	pdlg.lStructSize = sizeof( PRINTDLG );
	pdlg.Flags = PD_RETURNDC;
	// Launch the common dialog
	if( PrintDlg( &pdlg ) )
		return pdlg.hDC;
	else
		return NULL;
}
// End GetPrinterDC
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : AbortProc

  PARAMETERS : HDC    hDC    - The DC for the print job
               int    Error  - An error code; 0 for no error

  PURPOSE    : The AbortProc callback used during printing

  RETURNS    : BOOL - TRUE to continue, FALSE to abort printing

  COMMENTS   : 
  
  HISTORY    : 1/10/96 - Created

*/
BOOL CALLBACK AbortProc( HDC hDC, int Error )
{
	MSG	msg;

	// Just do a simple message loop
	while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	return TRUE;
}
// End AbortProc
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : PrintEnhMetaFile

  PARAMETERS : HENHMETAFILE  hMeta              - The Enhanced Metafile to print
               BOOL          bPrintOriginalSize - TRUE = print at original size
			                                      FALSE = stretch to fit the page

  PURPOSE    : Print an enhanced metafile

  RETURNS    : BOOL - TRUE for success, FALSE for failure

  COMMENTS   : This function only prints one page. If the metafile is too large
               to fit on one page, it will need to be played on multiple pages.
  
  HISTORY    : 1/10/96 - Created

*/
BOOL PrintEnhMetaFile( HENHMETAFILE hMeta, BOOL bPrintOriginalSize )
{
	HDC	hDC;
	DOCINFO	di;
	
	// First we'll need a DC for the printer
	if( (hDC = GetPrinterDC()) == NULL )
		return FALSE;

	// Set the Abort Procedure
	SetAbortProc( hDC, AbortProc );
	// Zero then initialize the DOCINFO
	ZeroMemory( &di, sizeof( DOCINFO ) );
	di.cbSize = sizeof( DOCINFO );
	di.lpszDocName = "EnhMetaFile Document";
	// Start the document
	StartDoc( hDC, &di );
	// Start the page
	StartPage( hDC );
	
	// Is the global flag set to show the metafile at its intended size?
	if( bPrintOriginalSize )
	{
		// Yes, so play it at its intended size
		PlayEnhMetaFileAtOriginalSize( hDC, hEnhMeta );
	}
	else
	{
		RECT	Rect;

		// No, so scale it to fit the entire printed page
		SetRect( &Rect, 0, 0, GetDeviceCaps( hDC, HORZRES ), GetDeviceCaps( hDC, VERTRES ) );
		PlayEnhMetaFileInRect( hDC, hEnhMeta, Rect );
	}

	// Clean up
	EndPage( hDC );
	EndDoc( hDC );
	DeleteDC( hDC );
}
// End PrintEnhMetaFile
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : InfoDlgProc

  PARAMETERS : HWND hWnd        -  window handle
               UINT message     -  type of message
               WPARAM wParam    -  additional information
               LPARAM lParam    -  additional information

  PURPOSE    : Processes messages for the "Metafile Info" dialog.

  RETURNS    : BOOL - TRUE is Ok, FALSE is Cancel

  COMMENTS   : This dialog displays information found in the enhanced
               metafile header.
  
  HISTORY    : 1/10/96 - Created

*/
BOOL CALLBACK InfoDlgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	switch( Msg )
	{
		// Dialog is being initialized - fill in the texts
		case WM_INITDIALOG:
		{
			HENHMETAFILE	hMeta = (HENHMETAFILE)lParam;
			DWORD			dwNeeded;
			ENHMETAHEADER	emh;

			// Initialize the header
			ZeroMemory( &emh, sizeof(ENHMETAHEADER) );
			emh.nSize = sizeof(ENHMETAHEADER);
			// Get the header filled in
			GetEnhMetaFileHeader( hMeta, sizeof(ENHMETAHEADER), &emh );

			// Set the texts indicating the bounding rectangle
			SetDlgItemInt( hWnd, IDC_BOUNDTOP, emh.rclBounds.top, TRUE );
			SetDlgItemInt( hWnd, IDC_BOUNDLEFT, emh.rclBounds.left, TRUE );
			SetDlgItemInt( hWnd, IDC_BOUNDRIGHT, emh.rclBounds.right, TRUE );
			SetDlgItemInt( hWnd, IDC_BOUNDBOTTOM, emh.rclBounds.bottom, TRUE );

			// Set the texts indicating the "metafile device" rectangle
			SetDlgItemInt( hWnd, IDC_FRAMETOP, emh.rclFrame.top, TRUE );
			SetDlgItemInt( hWnd, IDC_FRAMELEFT, emh.rclFrame.left, TRUE );
			SetDlgItemInt( hWnd, IDC_FRAMERIGHT, emh.rclFrame.right, TRUE );
			SetDlgItemInt( hWnd, IDC_FRAMEBOTTOM, emh.rclFrame.bottom, TRUE );

			// Set the texts indicating the number of bytes, records, handles
			// and palette entries contained in the metafile
			SetDlgItemInt( hWnd, IDC_BYTES, emh.nBytes, TRUE );
			SetDlgItemInt( hWnd, IDC_RECORDS, emh.nRecords, TRUE );
			SetDlgItemInt( hWnd, IDC_HANDLES, emh.nHandles, TRUE );
			SetDlgItemInt( hWnd, IDC_PALENTRIES, emh.nPalEntries, TRUE );

			// Set the texts indicating the resolution and size of the reference device
			SetDlgItemInt( hWnd, IDC_PIXELSX, emh.szlDevice.cx, TRUE );
			SetDlgItemInt( hWnd, IDC_PIXELSY, emh.szlDevice.cy, TRUE );
			SetDlgItemInt( hWnd, IDC_MMX, emh.szlMillimeters.cx, TRUE );
			SetDlgItemInt( hWnd, IDC_MMY, emh.szlMillimeters.cy, TRUE );

			// Now, we'll ask for the description string for the enhanced metafile
			// How big is it?
			dwNeeded = GetEnhMetaFileDescription( hMeta, 0, NULL );
			if( dwNeeded > 0 )
			{
				TCHAR			*pDesc;

				// Allocate some memory for it
				pDesc = malloc( dwNeeded + 1 );
				// Ask to have that string filled in
				if( GetEnhMetaFileDescription( hMeta, dwNeeded, pDesc ) == 0 )
					lstrcpy( pDesc, "" );
				// Since the string is stored as two consecutive NULL-terminated
				// strings, we'll overwrite the first NULL so we can also see
				// the second string
				if( lstrlen( pDesc ) > 1 )
					pDesc[lstrlen(pDesc)] = '#';
				// Update the dialog item text
				SetDlgItemText( hWnd, IDC_DESCRIPTION, pDesc );
				// Free the memory
				free( pDesc );
			}
			else
			{
				// dwNeeded = 0, must be no description string
				SetDlgItemText( hWnd, IDC_DESCRIPTION, "<None>" );
			}
		}
		break;

		// Time to say "goodbye"
		case WM_CLOSE:
			PostMessage( hWnd, WM_COMMAND, IDCANCEL, 0l );
		break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				// User presses Cancel or Ok button
				case IDCANCEL:
				case IDOK:
					EndDialog( hWnd, TRUE );
				break;
			}
		break;
		default:
			return FALSE;
		break;
	}
	return TRUE;
}
// End InfoDlgProc
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : GetEnhMetaFileName

  PARAMETERS : HWND    hWndParent  -  Handle to Parent Window
               LPTSTR  szFileName  -  String in which to store the filename
			                          assumed to be MAX_PATH in size
               BOOL    bSaving     -  TRUE for Save dialog, FALSE for Open dialog

  PURPOSE    : Uses common dialog to prompt for a file name

  RETURNS    : BOOL - TRUE is Ok, FALSE is Cancel

  COMMENTS   : 
  
  HISTORY    : 1/10/96 - Created

*/
BOOL GetEnhMetaFileName( HWND hWndParent, LPTSTR szFileName, BOOL bSaving )
{
	OPENFILENAME	ofn;

	// Initialize the structure
	ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = hWndParent;
	ofn.lpstrFilter = "All Metafiles (*.?MF)\0*.?MF\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;

	if( bSaving )
	{
		// Set the flags and launch the Save common dialog
		ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
		return GetSaveFileName( &ofn );
	}
	else
	{
		// Set the flags and launch the Open common dialog
		ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		return GetOpenFileName( &ofn );
	}
	return TRUE;
}
// End GetEnhMetaFileName
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : CalculateAPMCheckSum

  PARAMETERS : APMFILEHEADER apmfh - A placeable header
 
  PURPOSE    : Calculates the checksum for a placeable header

  RETURNS    : WORD - the checksum

  COMMENTS   : The checksum is derived by XORing each word.

  HISTORY    : 4/17/95 - Created

*/
WORD CalculateAPMCheckSum( APMHEADER apmfh )
{
	LPWORD	lpWord;
	WORD	wResult, i;
	
	// Start with the first word
	wResult = *(lpWord = (LPWORD)(&apmfh));
	// XOR in each of the other 9 words
	for(i=1;i<=9;i++)
	{
		wResult ^= lpWord[i];
	}
	return wResult;
}
// End CalculateAPMCheckSum
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : WriteEnhMetaFileAsPlaceable

  PARAMETERS : HENHMETAFILE hMeta  -  Enhanced Metafile to write as aldus placeable
               LPTSTR  szFileName  -  Filename for the new metafile

  PURPOSE    : Write an enhanced metafile as a 16bit aldus placeable metafile

  RETURNS    : BOOL - TRUE for success, FALSE for failure

  COMMENTS   : 
  
  HISTORY    : 1/10/96 - Created

*/
BOOL WriteEnhMetaFileAsPlaceable( HENHMETAFILE hMeta, LPTSTR szFileName )
{
	APMHEADER		APMHeader;
	ENHMETAHEADER	emh;
	HDC				hDC;
	DWORD			dwSize, dwBytes;
	LPBYTE			pBits;
	HANDLE			hFile;

	// Initialize the header
	ZeroMemory( &emh, sizeof(ENHMETAHEADER) );
	emh.nSize = sizeof(ENHMETAHEADER);
	// Fill in the enhanced metafile header
	if( GetEnhMetaFileHeader( hMeta, sizeof( ENHMETAHEADER ), &emh ) == 0 )
		return FALSE;

	// Fill in the Aldus Placeable Header
	APMHeader.dwKey = 0x9ac6cdd7l;
	APMHeader.hmf = 0;
	APMHeader.bbox.Top = 1000 * emh.rclFrame.top/2540;
	APMHeader.bbox.Left = 1000 * emh.rclFrame.left/2540;
	APMHeader.bbox.Right = 1000 * emh.rclFrame.right/2540;
	APMHeader.bbox.Bottom = 1000 * emh.rclFrame.bottom/2540;
	APMHeader.wInch = 1000;
	APMHeader.dwReserved = 0;
	APMHeader.wCheckSum = CalculateAPMCheckSum( APMHeader );

	// We need a reference DC
	hDC = GetDC( NULL );
	// How big are the windows metafile bits going to be?
	dwSize = GetWinMetaFileBits( hMeta, 0, NULL, MM_ANISOTROPIC, hDC  );
	// Allocate that much memory
	pBits = malloc( dwSize );
	// Let Windows convert our enhanced metafile to a regular windows metafile
	GetWinMetaFileBits( hMeta, dwSize, pBits, MM_ANISOTROPIC, hDC );
	// Release the reference DC
	ReleaseDC( NULL, hDC );

	// Open the file
	hFile = CreateFile( szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	// Write the Aldus Placeable Header
	WriteFile( hFile, &APMHeader, sizeof(APMHEADER), &dwBytes, NULL );
	// Write the metafile bits
	WriteFile( hFile, pBits, dwSize, &dwBytes, NULL );

	// Clean up
	free( pBits );
	CloseHandle( hFile );

	return TRUE;
}
// End WriteEnhMetaFileAsPlaceable
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : WriteEnhMetaFileAsRegular

  PARAMETERS : HENHMETAFILE hMeta  -  Enhanced Metafile to write as windows wmf
               LPTSTR  szFileName  -  Filename for the new metafile

  PURPOSE    : Write an enhanced metafile as a 16bit windows metafile

  RETURNS    : BOOL - TRUE for success, FALSE for failure

  COMMENTS   : 
  
  HISTORY    : 1/10/96 - Created

*/
BOOL WriteEnhMetaFileAsRegular( HENHMETAFILE hMeta, LPTSTR szFileName )
{
	HDC				hDC;
	DWORD			dwSize, dwBytes;
	LPBYTE			pBits;
	HANDLE			hFile;

	// Need a reference DC
	hDC = GetDC( NULL );
	// How big will the metafile bits be?
	dwSize = GetWinMetaFileBits( hMeta, 0, NULL, MM_ANISOTROPIC, hDC  );
	// Allocate that much memory
	pBits = malloc( dwSize );
	// Let windows convert the enhanced metafile to a 16 windows metafile
	GetWinMetaFileBits( hMeta, dwSize, pBits, MM_ANISOTROPIC, hDC );
	// Release the reference DC
	ReleaseDC( NULL, hDC );

	// Open the file
	hFile = CreateFile( szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	// Write the metafile bits
	WriteFile( hFile, pBits, dwSize, &dwBytes, NULL );
	// Clean up
	free( pBits );
	CloseHandle( hFile );

	return TRUE;
}
// End WriteEnhMetaFileAsRegular
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : ReadEnhMetaFile

  PARAMETERS : LPTSTR  szFileName  -  Filename for the metafile to open
			   HWND    hWndParent  -  The parent window

  PURPOSE    : Read a metafile as an enhanced metafile

  RETURNS    : HENHMETAFILE - the handle of the new enhanced metafile

  COMMENTS   : This function will read either an enhanced metafile, or a
               regular 16bit windows metafile, or an Aldus Placeable metafile
			   and convert it into an enhanced metafile.
			   This function first tries to load the metafile as an enhanced
			   metafile. If that fails, it tries to load it as a 16bit windows
			   metafile. If that fails, it tries to load it as a 16bit Aldus
			   Placeable metafile. If all fail, NULL is returned.
  
  HISTORY    : 1/10/96 - Created

*/
HENHMETAFILE ReadEnhMetaFile( LPTSTR szFileName, HWND hWndParent )
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
			MessageBox( hWndParent, "Failed to Get MetaFile Bits Size", "Error Reading MetaFile", MB_OK );
			return NULL;
		}
		// Allocate that much memory
		if( (pBits = malloc( dwSize )) == NULL )
		{
			DeleteMetaFile( hOld );
			MessageBox( hWndParent, "Failed to Allocate Memory for Metafile Bits", "Error Reading MetaFile", MB_OK );
			return NULL;
		}
		// Get the metafile bits
		if( GetMetaFileBitsEx( hOld, dwSize, pBits ) == 0 )
		{
			free( pBits );
			DeleteMetaFile( hOld );
			MessageBox( hWndParent, "Failed to Get MetaFile Bits", "Error Reading MetaFile", MB_OK );
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
			MessageBox( hWndParent, "Failed to Create MetaFile from Bits", "Error Reading MetaFile", MB_OK );
		return hTemp;
	}
	// If we got here, the file of interest is neither an enhanced metafile nor a
	// 16bit windows metafile. Let's assume it's a placeable metafile and move on.
	// First, open the file for reading
	hFile = CreateFile( szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE )
	{
		MessageBox( hWndParent, "Error Opening File", szFileName, MB_OK );
		return NULL;
	}
	// How big is the file?
	dwSize = GetFileSize( hFile, NULL );
	// Allocate enough memory to hold it
	pBits = malloc( dwSize );
	// Read it in
	ReadFile( hFile, pBits, dwSize, &dwSize, NULL );
	// Close the file
	CloseHandle( hFile );
	// Is it a placeable metafile? (check the key)
	if( ((PAPMHEADER)pBits)->dwKey != 0x9ac6cdd7l )
	{
		// Not a metafile that we know how to recognise - bail out
		free( pBits );
		MessageBox( hWndParent, "Not a Valid Metafile", szFileName, MB_OK );
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
		MessageBox( hWndParent, "Failed to Create MetaFile from Bits", "Error Reading MetaFile", MB_OK );
	return hTemp;
}
// End ReadEnhMetaFile
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : AboutDlgProc

  PARAMETERS : HWND hWnd        -  window handle
               UINT message     -  type of message
               WPARAM wParam    -  additional information
               LPARAM lParam    -  additional information

  PURPOSE    : Processes messages for the About dialog.

  RETURNS    : BOOL - TRUE is Ok, FALSE is Cancel

  COMMENTS   : 
 
  HISTORY    : 4/17/95 - Created

*/
BOOL CALLBACK AboutDlgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	switch( Msg )
	{
		case WM_INITDIALOG:
		{
			TCHAR	szBuffer[512];

			// Load the string
			LoadString( hInst, IDS_ABOUTSTRING, szBuffer, 512 );
			// Set the text
			SetDlgItemText( hWnd, IDC_ABOUTTEXT, szBuffer );
		}
		break;

		// Time to say "goodbye"
		case WM_CLOSE:
			PostMessage( hWnd, WM_COMMAND, IDCANCEL, 0l );
		break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				// User has pressed Ok or Cancel button
				case IDCANCEL:
				case IDOK:
					EndDialog( hWnd, TRUE );
				break;
			}
		break;
		default:
			return FALSE;
		break;
	}
	return TRUE;
}
// End AboutDlgProc
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : RecordsDlgProc

  PARAMETERS : HWND hWnd        -  window handle
               UINT message     -  type of message
               WPARAM wParam    -  additional information
               LPARAM lParam    -  additional information

  PURPOSE    : Processes messages for the Metafile Records dialog.

  RETURNS    : BOOL - TRUE is Ok, FALSE is Cancel

  COMMENTS   : 
 
  HISTORY    : 4/17/95 - Created

*/
BOOL CALLBACK RecordsDlgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	switch( Msg )
	{
		// We're being sized! Tell the children!
		case WM_SIZE:
		{
			#define BORDER_SIZE		10
			RECT	ButtonRect, BoxRect, ClientRect;
			HWND	hButton, hBox;
			int		x, y, width, height;

			// Get a handle to the OK button
			hButton = GetDlgItem( hWnd, IDOK );
			// Get a handle to the ListBox
			hBox = GetDlgItem( hWnd, IDC_RECORDSLIST );
			// Get the new rectangles for each window
			GetClientRect( hWnd, &ClientRect );
			GetWindowRect( hButton, &ButtonRect );
			GetWindowRect( hBox, &BoxRect );

			// Adjust the Ok button to its new position/size
			width = ButtonRect.right - ButtonRect.left;
			height = ButtonRect.bottom - ButtonRect.top;
			x = ( ClientRect.right - ClientRect.left - width ) / 2;
			y =  ClientRect.bottom - BORDER_SIZE - height;
			MoveWindow( hButton, x, y, width, height, TRUE );
			
			// Adjust the list box to its new position/size
			height = y - ( 2 * BORDER_SIZE );
			width = ClientRect.right - ClientRect.left - ( 2 * BORDER_SIZE );
			x = ( y = BORDER_SIZE );
			MoveWindow( hBox, x, y, width, height, TRUE );

			#undef	BORDER_SIZE
		}
		break;

		// Init time - here we initialize the list box with the records
		case WM_INITDIALOG:
		{
			HENHMETAFILE	hMeta = (HENHMETAFILE)lParam;
			ENHMETAHEADER	emh;
			TCHAR			szBuffer[128];
			RECT			Rect;
			HWND			hWndListBox;

			// Get the list box window handle
			hWndListBox = GetDlgItem( hWnd, IDC_RECORDSLIST );
			// Enumerate the records, pass the window handle to the callback
			// The callback will add each record's text to the listbox
			EnumEnhMetaFile( NULL, hMeta, (ENHMFENUMPROC)EnumEnhMetafileProc, (LPVOID)&hWndListBox, &Rect );

			// Initialize the header
			ZeroMemory( &emh, sizeof(ENHMETAHEADER) );
			emh.nSize = sizeof(ENHMETAHEADER);
			// have the header filled in
			GetEnhMetaFileHeader( hMeta, sizeof(ENHMETAHEADER), &emh );
			// Set the title of the dialog to indicate how many records exist
			wsprintf( szBuffer, "%d Metafile Records", emh.nRecords );
			SetWindowText( hWnd, szBuffer );
		}
		break;

		// Time to say "goodbye"
		case WM_CLOSE:
			PostMessage( hWnd, WM_COMMAND, IDCANCEL, 0l );
		break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				// User has clicked on the Ok or Cancel button
				case IDCANCEL:
				case IDOK:
					EndDialog( hWnd, TRUE );
				break;
			}
		break;
		default:
			return FALSE;
		break;
	}
	return TRUE;
}
// End RecordsDlgProc
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : EnumEnhMetafileProc

  PARAMETERS : HDC            hDC        -  DC on which to draw
               HANDLETABLE    *lpHTable  -  Metafile's handle table
               ENHMETARECORD  *lpEMFR    -  This record
			   int            nObj       -  How many objects in handle table?
			   LPARAM         lpData     -  User defined (handle to listbox)

  PURPOSE    : Enumeration callback that puts text for each record into listbox

  RETURNS    : int - non-zero to keep enumerating

  COMMENTS   : 
 
  HISTORY    : 4/17/95 - Created

*/
int CALLBACK EnumEnhMetafileProc( HDC hDC, HANDLETABLE *lpHTable, ENHMETARECORD *lpEMFR, int nObj, LPARAM lpData )
{
	HWND	hWndList = *(HWND *)lpData;
	TCHAR	szBuffer[100];
	LPTSTR	pString;
	DWORD	i;
	SIZE	size;
	HDC		hTempDC;
	HFONT	hFont, hOldFont;
	TEXTMETRIC	tm;
	int		nOldExtent, nNewExtent;


	// If it's a known record type, get the string from the global string array
	if( (lpEMFR->iType>=EMR_MIN) && (lpEMFR->iType<=EMR_MAX) )
		wsprintf( szBuffer, "%010d Bytes : %s", lpEMFR->nSize, szRecordStrings[lpEMFR->iType - EMR_MIN] );
	else
		wsprintf( szBuffer, "%010d Bytes : %s", lpEMFR->nSize, "Unknown" );

	// Allocate a string big enough for the record text and the parameter bytes
	pString = malloc( lstrlen(szBuffer) + (lpEMFR->nSize * 12) );
	// Begin with the record string
	lstrcpy( pString, szBuffer );
	// For each parameter DWORD, add the string representation to the string
	for(i=0;i<(lpEMFR->nSize/sizeof(DWORD))-2;i++)
	{
		wsprintf( szBuffer, " %x", lpEMFR->dParm[i] );
		lstrcat( pString, szBuffer );
	}

	// Now we need to check for listbox horizontal scrolling
	// Get a temporary DC
	hTempDC = GetDC( NULL );
	// Get the font used in the listbox
	if( (hFont = (HFONT)SendMessage( hWndList, WM_GETFONT, 0, 0 )) != NULL )
		hOldFont = SelectObject( hTempDC, hFont );
	// How big is this text?
	GetTextExtentPoint32( hTempDC, pString, lstrlen(pString), &size );
	GetTextMetrics( hTempDC, &tm );
	if( hFont != NULL )
	{
		SelectObject( hTempDC, hOldFont );
		DeleteObject( hFont );
	}
	ReleaseDC( NULL, hTempDC );

	// Get the old widest string extent
	nOldExtent = SendMessage( hWndList, LB_GETHORIZONTALEXTENT, 0, 0 );
	// If the new one is bigger, set it
	if( (nNewExtent = size.cx + tm.tmAveCharWidth) > nOldExtent )
		SendMessage( hWndList, LB_SETHORIZONTALEXTENT, (WPARAM)nNewExtent, 0 );

	// Add the new string
	SendMessage( hWndList, LB_ADDSTRING, 0, (LPARAM)pString );
	free( pString );

	return 1;
}
// End EnumEnhMetafileProc
// ==========================================================================



// ==========================================================================
/*

  FUNCTION   : GetEnhancedMetafilePalette

  PARAMETERS : HENHMETAFILE hMeta - The metafile from which to extract a palette
 
  PURPOSE    : Get the palette from an enhanced metafile

  RETURNS    : HPALETTE - the palette used by the metafile, NULL if none

  COMMENTS   : This palette may contain more than 256 entries. For a complete
               solution, palettes with more than 256 entries should be reduced
			   to include only 256 entries. One way to do this is shown in the
			   following pseudocode:
			   while( have more colors than needed )
			       find two closest colors and discard one of them
			
 
  HISTORY    : 4/17/95 - Created

*/
HPALETTE GetEnhancedMetafilePalette( HENHMETAFILE hMeta )
{
	ENHMETAHEADER	emh;
	LOGPALETTE		*pLogPal = NULL;
	HPALETTE		hPal = NULL;

	// Initialize the enhanced metafile header
	ZeroMemory( &emh, sizeof(ENHMETAHEADER) );
	emh.nSize = sizeof(ENHMETAHEADER);
	// Fill in the header
	if( GetEnhMetaFileHeader( hMeta, sizeof( ENHMETAHEADER ), &emh ) == 0 )
		return NULL;
	
	// If this metafile has no palette, return NULL
	if( emh.nPalEntries == 0 )
		return NULL;

	// Allocate enough space for the LOGPALETTE and its entries
	if( (pLogPal = malloc( ( 2 * sizeof(WORD) ) + (emh.nPalEntries * sizeof( PALETTEENTRY )) )) == NULL )
		return NULL;
	pLogPal->palVersion = 0x300;
	pLogPal->palNumEntries = (WORD)emh.nPalEntries;
	// Get the entries from the metafile
	if( GetEnhMetaFilePaletteEntries( hMeta, emh.nPalEntries, pLogPal->palPalEntry ) == GDI_ERROR )
	{
		free( pLogPal );
		return NULL;
	}
	// Create a palette
	hPal = CreatePalette( pLogPal );
	free( pLogPal );
	return hPal;
}
// End GetEnhancedMetafilePalette
// ==========================================================================


//***************************************************************************
// End of file <EnhMeta.c>
//***************************************************************************
