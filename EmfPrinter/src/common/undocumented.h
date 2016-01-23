/*
 * undocumented.h
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

#pragma warning( push )
#pragma warning( disable : 4200 )

// from http://www.undocprint.org/formats/winspool/spl

/*****************************************************************************
 * Terms of Use
 * Copyright © 2005 by Christoph Lindemann. All rights reserved.
 *
 * Permission is granted to copy, distribute and/or modify this document under
 * the terms of the GNU Free  Documentation License, Version 1.2 or any later 
 * version published by the Free Software Foundation; with no Invariant 
 * Sections, no Front-Cover Texts, and no Back-Cover Texts. A copy of the 
 * license is included in the section entitled "GNU Free Documentation 
 * License".
 *----------------------------------------------------------------------------
 * History:
 *  24-03-2004  Initial Document
 *     Christoph Lindemann, christoph@lindemann.nu
 *  
 *  24-03-2004  Release to public
 *     Christoph Lindemann, christoph@lindemann.nu
 *  
 *  10-06-2004  Added SPL_SMR_PADDING
 *     Thanks to Fabian Franz
 *  
 *  11-06-2004  CORRECTED FAULTY INFORMATIONS
 *     Christoph Lindemann, christoph@lindemann.nu
 *  
 *  14-06-2004  Added some text explaining the format.
 *     Christoph Lindemann, christoph@lindemann.nu
 *  
 *  19-05-2005  Corrected typos in SMREXT definition
 *     Thanks to Peter Wasser
 *  
 *  02-11-2005  Updated End-Of-Page records 0x0D and 0x0E
 *     Thanks to Krzys
 *
 *****************************************************************************/
 
//Spool Metafile constants
#define SPLMETA_SIGNATURE 0x00010000 //Version 1.0 
 
// Spool Metafile record types
#define SRT_RESERVED_WMF  0x00000001 /*  1 NT4 WMF Data ?NOT CONFIRMED?   */
#define SRT_FONT1         0x00000002 /*  2 Font Data                      */
#define SRT_DEVMODE       0x00000003 /*  3 DevMode                        */
#define SRT_FONT2         0x00000004 /*  4 Font Data                      */
#define SRT_RESERVED_5    0x00000005 /*  5                                */
#define SRT_FONT_MM       0x00000006 /*  6 Font Data (Multiple Master)    */
#define SRT_FONT_SUB1     0x00000007 /*  7 Font Data (SubsetFont 1)       */
#define SRT_FONT_SUB2     0x00000008 /*  8 Font Data (SubsetFont 2)       */
#define SRT_RESERVED_9    0x00000009 /*  9                                */
#define SRT_RESERVED_A    0x0000000A /* 10                                */ 
#define SRT_RESERVED_B    0x0000000B /* 11                                */
#define SRT_PAGE          0x0000000C /* 12 Enhanced Meta File (EMF)       */ 
#define SRT_EOPAGE_RASTER 0x0000000D /* 13 EndOfPage raster bitmap        */
#define SRT_EOPAGE_VECTOR 0x0000000E /* 14 EndOfPage GDI image data       */
#define SRT_EXT_FONT      0x0000000F /* 15 Ext Font Data                  */
#define SRT_EXT_FONT2     0x00000010 /* 16 Ext Font Data                  */
#define SRT_EXT_FONT_MM   0x00000011 /* 17 Ext Font Data (Multiple Master)*/
#define SRT_EXT_FONT_SUB1 0x00000012 /* 18 Ext Font Data (SubsetFont 1)   */
#define SRT_EXT_FONT_SUB2 0x00000013 /* 19 Ext Font Data (SubsetFont 2)   */
#define SRT_EXT_PAGE      0x00000014 /* 20 Enhanced Meta File?            */
 
/*****************************************************************************
 * SPL_HEADER
 *----------------------------------------------------------------------------
 * SPL file header for EMFSPL files
 *****************************************************************************/
typedef struct tagSPLHEADER { 
    DWORD SIGNATURE; 
    DWORD nSize;            // record size INCLUDING header 
    DWORD offDocumentName;  // offset of Job Title from start 
    DWORD offPort;          // offset of portname from start
    BYTE HeaderData[]; 
} SPL_HEADER, *PSPL_HEADER;
 
/*****************************************************************************
 * SMR - Base record
 *----------------------------------------------------------------------------
 * Base record type for the Spool Metafile.
 *****************************************************************************/
typedef struct tagSMR{ 
    DWORD iType; // Spool metafile record type 
    DWORD nSize; // length of the following data 
                 // NOT INCLUDING this header 
} SMR, *PSMR;
 
/*****************************************************************************
 * SMREXT - Extended record
 *----------------------------------------------------------------------------
 * Contains neg. distance to start of Data
 *****************************************************************************/
typedef struct tagSMREXT{ 
    SMR smr; 
    DWORD DistanceLow; 
    DWORD DistanceHigh;
} SMREXT, *PSMREXT;
 
/*****************************************************************************
 * SMR_PAGE - EMF/Page data
 *----------------------------------------------------------------------------
 * EMF/Page data
 *****************************************************************************/
typedef struct tagSMRPAGE{ 
    SMR smr; 
    BYTE EMFData[];
} SMR_PAGE, *PSMR_PAGE;
 
/*****************************************************************************
 * SMR_DEVMODE - DEVMODE data
 *----------------------------------------------------------------------------
 * DEVMODE data
 *****************************************************************************/
typedef struct tagSMRDEVMODE{ 
    SMR smr; 
    BYTE DEVMODEData[];
} SMR_DEVMODE, *PSMR_DEVMODE;
 
/*****************************************************************************
 * SMR_FONT - FONT data
 *****************************************************************************/
typedef struct tagSMRFONT{ 
    SMR smr; 
    BYTE FONTData[];
} SMR_FONT, *PSMR_FONT;
 
/*****************************************************************************
 * SMR_EXTFONT - Extended Font Data
 *----------------------------------------------------------------------------
 * Contains neg. distance to start of
 * Font Data
 * Font data is typically embedded as
 * GDICOMMENT in the prev EMF data
 *****************************************************************************/
typedef struct tagEXTFONT{ 
    SMREXT smrext;
} SMR_EXTFONT, *PSMR_EXTFONT;
 
/*****************************************************************************
 * SMR_EOPAGE - End of Page
 *----------------------------------------------------------------------------
 * Contains neg. distance to
 * start of page record
 *****************************************************************************/
typedef struct tagSMREOPAGE{ 
    SMREXT smrext; 
} SMR_EOPAGE, *PSMR_EOPAGE;


// from http://undocprint.printassociates.com/formats/winspool/emf


/* WINVER >= #0500 (Windows 2000) */
//EMR_DRAWESCAPE            105 (#69) 
//EMR_EXTESCAPE             106 (#6A) 
//EMR_STARTDOC              107 (#6B) 
//EMR_SMALLTEXTOUT          108 (#6C) 
//EMR_FORCEUFIMAPPING       109 (#6D) 
//EMR_NAMEDESCAPE           110 (#6E) 
//    
//EMR_TRANSPARENTDIB        117 (#75) 
//    
//EMR_SETLINKEDUFIS         119 (#77) 
//EMR_SETTEXTJUSTIFICATION  120 (#78) 

// conversion is:

#define EMR_DRAWESCAPE            105
#define EMR_EXTESCAPE             106
#define EMR_STARTDOC              107
#define EMR_SMALLTEXTOUT          108
#define EMR_FORCEUFIMAPPING       109
#define EMR_NAMEDESCAPE           110
#define EMR_TRANSPARENTDIB        117
#define EMR_SETLINKEDUFIS         119
#define EMR_SETTEXTJUSTIFICATION  120

/*
  //EMR_EXTESCAPE = RECORD
  //  emr     : EMR;
  //  nEscape : DWORD;
  //  nSize   : DWORD; // == sizeof(DRAWPATRECT)
  //  pattern : DRAWPATRECT;
  //END;

// conversion is 

typedef struct tagEMREXTESCAPE {
	EMR emr;
	DWORD nEscape;
	DWORD nSize;
	DRAWPATRECT pattern;
} EMR_EXTESCAPE;*/

//EMR_SMALLTEXTOUT
//The EMR record type 108 can be 2 different records. EMR_SMALLTEXTOUT or EMR_SMALLTEXTOUTCLIP. Look at the flags in fOptions, to see what type is used. The fOptions field also specifies if the ANSI or UNICODE version is used. 
//
//The interpretation of the reference point depends on the current text-alignment mode. An application can retrieve this mode by calling the getTextAlign function; an application can alter this mode by calling the setTextAlign function. 
//
//By default, the current position is not used or updated by this function. However, an application can call the setTextAlign function with the fMode parameter set to TA_UPDATECP to permit the system to use and update the current position each time the application calls textOut for a specified device context. When this flag is set, the system ignores the nXStart and nYStart parameters on subsequent textOut calls. 
//

//CONST
//  SMALLTEXT_TYPE_WITHOUT_CLIP = $100; 
//      // if set use EMR_SMALLTEXTOUT else use EMR_SMALLTEXTOUTCLIP
//  SMALLTEXT_TYPE_ANSI         = $200; 
//      // if set use ANSI version else UNICODE

// conversion:

#define SMALLTEXT_TYPE_WITHOUT_CLIP		0x100
#define SMALLTEXT_TYPE_ANSI				0x200


#pragma warning( pop )
