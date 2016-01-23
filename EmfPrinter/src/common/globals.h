/*
 * globals.h
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

#ifndef GLOBALS_H
#define GLOBALS_H

// -----------------------------------------------------------------------------
// OUTPUT FORMATS
// -----------------------------------------------------------------------------

typedef struct tagOutputFormat
{
    LPCTSTR strName;
    LPCTSTR strDescription;
} OUTPUTFORMAT;


// (vector) formats natively supported by EmfPrinter
static OUTPUTFORMAT g_vectorFormats[] = 
{
    { TEXT("EMF"), TEXT("Enhanced MetaFile") },
    { TEXT("WMF"), TEXT("Windows MetaFile - old, use only if really needed") }
};

// raster formats supported through ImageMagick
static OUTPUTFORMAT g_rasterFormats[] = 
{
    { TEXT("AVS"), TEXT("AVS X image") },
    { TEXT("BMP"), TEXT("Microsoft Windows bitmap image") },
    { TEXT("BMP2"), TEXT("Microsoft Windows bitmap image v2") },
    { TEXT("BMP3"), TEXT("Microsoft Windows bitmap image v3") },
    { TEXT("CIN"), TEXT("Cineon Image File") },
    { TEXT("CIP"), TEXT("Cisco IP phone image format") },
    { TEXT("CLIP"), TEXT("Image Clip Mask") },
    { TEXT("CMYK"), TEXT(" cyan, magenta, yellow, and black samples") },
    { TEXT("CMYKA"), TEXT(" cyan, magenta, yellow, black, and opacity samples") },
    { TEXT("CUR"), TEXT("Microsoft icon") },
    { TEXT("DCX"), TEXT("ZSoft IBM PC multi page Paintbrush") },
    { TEXT("EPDF"), TEXT("Encapsulated Portable Document Format") },
    { TEXT("EPI"), TEXT("Encapsulated PostScript Interchange format") },
    { TEXT("EPS"), TEXT("Encapsulated PostScript") },
    { TEXT("EPS2"), TEXT("Level II Encapsulated PostScript") },
    { TEXT("EPS3"), TEXT("Level III Encapsulated PostScript") },
    { TEXT("EPSF"), TEXT("Encapsulated PostScript") },
    { TEXT("EPSI"), TEXT("Encapsulated PostScript Interchange format") },
    { TEXT("EPT"), TEXT("Encapsulated PostScript with TIFF preview") },
    { TEXT("EPT2"), TEXT("Encapsulated PostScript Level II with TIFF preview") },
    { TEXT("EPT3"), TEXT("Encapsulated PostScript Level III with TIFF preview") },
    { TEXT("FAX"), TEXT("Group 3 FAX") },
    { TEXT("FITS"), TEXT("Flexible Image Transport System") },
    { TEXT("FTS"), TEXT("Flexible Image Transport System") },
    { TEXT("G3"), TEXT("Group 3 FAX") },
    { TEXT("GIF"), TEXT("CompuServe graphics interchange format") },
    { TEXT("GIF87"), TEXT("CompuServe graphics interchange format (version 87a)") },
    { TEXT("HTM"), TEXT("Hypertext Markup Language and a client side image map") },
    { TEXT("HTML"), TEXT("Hypertext Markup Language and a client side image map") },
    { TEXT("ICB"), TEXT("Truevision Targa image") },
    { TEXT("ICO"), TEXT("Microsoft icon") },
    { TEXT("ICON"), TEXT("Microsoft icon") },
    { TEXT("INFO"), TEXT("The image format and characteristics") },
    { TEXT("IPL"), TEXT("IPL Image Sequence") },
    { TEXT("JNG"), TEXT("JPEG Network Graphics") },
    { TEXT("JP2"), TEXT("JPEG 2000 File Format Syntax") },
    { TEXT("JPC"), TEXT("JPEG 2000 Code Stream Syntax") },
    { TEXT("JPEG"), TEXT("Joint Photographic Experts Group JFIF format") },
    { TEXT("JPG"), TEXT("Joint Photographic Experts Group JFIF format") },
    { TEXT("JPX"), TEXT("JPEG 2000 File Format Syntax") },
    { TEXT("M2V"), TEXT("MPEG Video Stream") },
    { TEXT("MAP"), TEXT("Colormap intensities and indices") },
    { TEXT("MAT"), TEXT("MATLAB image format") },
    { TEXT("MATTE"), TEXT("MATTE format") },
    { TEXT("MIFF"), TEXT("Magick Image File Format") },
    { TEXT("MNG"), TEXT("Multiple image Network Graphics") },
    { TEXT("MPC"), TEXT("Magick Persistent Cache image format") },
    { TEXT("MPEG"), TEXT("MPEG Video Stream") },
    { TEXT("MPG"), TEXT("MPEG Video Stream") },
    { TEXT("MSL"), TEXT("Magick Scripting Language") },
    { TEXT("MSVG"), TEXT("ImageMagick's own SVG internal renderer") },
    { TEXT("MTV"), TEXT("MTV Raytracing image format") },
    { TEXT("MVG"), TEXT("Magick Vector Graphics") },
    { TEXT("OTB"), TEXT("On the air bitmap") },
    { TEXT("PAL"), TEXT("16bit/pixel interleaved YUV") },
    { TEXT("PALM"), TEXT("Palm pixmap") },
    { TEXT("PAM"), TEXT("Common 2 dimensional bitmap format") },
    { TEXT("PBM"), TEXT("Portable bitmap format (black and white)") },
    { TEXT("PCD"), TEXT("Photo CD") },
    { TEXT("PCDS"), TEXT("Photo CD") },
    { TEXT("PCL"), TEXT("Printer Control Language") },
    { TEXT("PCT"), TEXT("Apple Macintosh QuickD/PICT") },
    { TEXT("PCX"), TEXT("ZSoft IBM PC Paintbrush") },
    { TEXT("PDB"), TEXT("Palm Database ImageViewer Format") },
    { TEXT("PDF"), TEXT("Portable Document Format") },
    { TEXT("PFM"), TEXT("Portable float format") },
    { TEXT("PGM"), TEXT("Portable graymap format (gray scale)") },
    { TEXT("PICON"), TEXT("Personal Icon") },
    { TEXT("PICT"), TEXT("Apple Macintosh QuickD/PICT") },
    { TEXT("PJPEG"), TEXT("Progessive Joint Photographic Experts Group JFIF") },
    { TEXT("PNG"), TEXT("Portable Network Graphics") },
    { TEXT("PNG24"), TEXT("24 bit RGB PNG, opaque only") },
    { TEXT("PNG32"), TEXT("32 bit RGBA PNG, semitransparency OK") },
    { TEXT("PNG8"), TEXT("8 bit indexed PNG, binary transparency only") },
    { TEXT("PNM"), TEXT("Portable anymap") },
    { TEXT("PPM"), TEXT("Portable pixmap format (color)") },
    { TEXT("PS"), TEXT("PostScript") },
    { TEXT("PS2"), TEXT("Level II PostScript") },
    { TEXT("PS3"), TEXT("Level III PostScript") },
    { TEXT("PSD"), TEXT("Adobe Photoshop bitmap") },
    { TEXT("PTIF"), TEXT("Pyramid encoded TIFF") },
    { TEXT("RAS"), TEXT("SUN Rasterfile") },
    { TEXT("RGB"), TEXT(" red, green, and blue samples") },
    { TEXT("RGBA"), TEXT(" red, green, blue, and alpha samples") },
    { TEXT("RGBO"), TEXT(" red, green, blue, and opacity samples") },
    { TEXT("SGI"), TEXT("Irix RGB image") },
    { TEXT("SHTML"), TEXT("Hypertext Markup Language and a client side image map") },
    { TEXT("SUN"), TEXT("SUN Rasterfile") },
    { TEXT("SVG"), TEXT("Scalable Vector Graphics") },
    { TEXT("SVGZ"), TEXT("Compressed Scalable Vector Graphics") },
    { TEXT("TEXT"), TEXT("Text") },
    { TEXT("TGA"), TEXT("Truevision Targa image") },
    { TEXT("TIFF"), TEXT("Tagged Image File Format") },
    { TEXT("UIL"), TEXT("X Motif UIL table") },
    { TEXT("UYVY"), TEXT("16bit/pixel interleaved YUV") },
    { TEXT("VDA"), TEXT("Truevision Targa image") },
    { TEXT("VICAR"), TEXT("VICAR rasterfile format") },
    { TEXT("VID"), TEXT("Visual Image Directory") },
    { TEXT("VIFF"), TEXT("Khoros Visualization image") },
    { TEXT("VST"), TEXT("Truevision Targa image") },
    { TEXT("WBMP"), TEXT("Wireless Bitmap (level 0) image") },
    { TEXT("X"), TEXT("X Image") },
    { TEXT("XBM"), TEXT("X Windows system bitmap (black and white)") },
    { TEXT("XPM"), TEXT("X Windows system pixmap (color)") },
    { TEXT("XV"), TEXT("Khoros Visualization image") },
    { TEXT("XWD"), TEXT("X Windows system window dump (color)") },
    { TEXT("YCbCr"), TEXT(" Y, Cb, and Cr samples") },
    { TEXT("YCbCrA"), TEXT(" Y, Cb, Cr, and opacity samples") },
    { TEXT("YUV"), TEXT("CCIR 601 4:1:1 or 4:2:2") }
};



// -----------------------------------------------------------------------------
// DRIVER DATA
// -----------------------------------------------------------------------------

#define emfDRIVER_NAME                      TEXT("emfDriver")
#define emfDRIVER_FILENAME                  TEXT("emfprinter_drv.dll")
#define emfDRIVER_DEFAULT_DATATYPE          TEXT("")
/*
// our printer resolution in DPI
#define emfDRIVER_WIDTH_DPI                 600
#define emfDRIVER_HEIGHT_DPI                600
*/
#define emfDRIVER_WIDTH_DPI                 150
#define emfDRIVER_HEIGHT_DPI                150

// A4 paper is 210x297 mm
#define A4_WIDTH_MM                         210
#define A4_HEIGHT_MM                        297

// an inch is 25.4 mm
#define A4_WIDTH_INCH                       (A4_WIDTH_MM/25.4)
#define A4_HEIGHT_INCH                      (A4_HEIGHT_MM/25.4)

// the size of the internal ding surface 
#define emfDRIVER_WIDTH_PIXEL               ((int)(A4_WIDTH_INCH*emfDRIVER_WIDTH_DPI)+1)
#define emfDRIVER_HEIGHT_PIXEL              ((int)(A4_HEIGHT_INCH*emfDRIVER_HEIGHT_DPI)+1)

// This constant is used only in AddPrinterDriver and its value indicates
// for which OS was this driver designed:
// 2 = win95/98/me, 3 = nt4, 4 = win2000/xp
// see DRIVER_INFO_3 structure for more info; for max compatibility we use 3 for this value.
#define emfDRIVER_VERSION                   3

// This is the driver version returned by DrvDeviceCapabilities and used for ours
// PDEVMODE structures and also in the GDIINFO structure by DrvEnablePDEV.
//
// From MSDN:
// The high-order 16 bits must be set to zero. Bits 8 through 15 specify the 
// version number of the Microsoft operating system for which the driver is designed. 
// The high-order 4 bits of this range specify the major number of the version, 
// the low-order 4 bits contain the minor number of the version. 
// The low-order 8 bits of ulVersion specify the version number of the display driver; 
// this value should be incremented for each release of the display driver binary file
#define emfDRIVER_REAL_VERSION              ((WORD)(0x0500))


// -----------------------------------------------------------------------------
// PORT DATA
// -----------------------------------------------------------------------------

// NOTE: don't terminate it with a semicolon!
#define emfPORT_NAME                        TEXT("EmfVirtualPrinterPort")

// DIDN'T SEE THIS MAGIC NAME REALLY DOCUMENTED ANYWHERE ... 
// WEB RESEARCH SHOWS IT SEEMS TO BE THE RIGHT "PORT MONITOR" NAME TO USE
#define emfPORT_MONITOR_NAME                TEXT("Local Port")  


// -----------------------------------------------------------------------------
// PRINTER DATA
// -----------------------------------------------------------------------------

// default printer name
#define emfPRINTER_NAME                     TEXT("EmfVirtualPrinter")

// we _do_ need at least a datafile for the installation to work!
#define emfLICENSE_FILENAME                 TEXT("gnugpl.txt")

#define emfPRINTER_DEFAULT_DATATYPE         TEXT("NT EMF 1.006")


// -----------------------------------------------------------------------------
// MISC
// -----------------------------------------------------------------------------

#define emfUIDLL_FILENAME                   TEXT("emfprinter_ui.dll")

#define emfMAX_CCH_FILENAME                 32
#define emfMAX_ALTERNATES                   10

typedef TCHAR emfFILENAME_BUFF[emfMAX_CCH_FILENAME + 1];
typedef emfFILENAME_BUFF emfFILENAME_ALTERNATES[emfMAX_ALTERNATES];

#define emfVERSION_MAJOR                    0
#define emfVERSION_MINOR                    3
#define emfVERSION_RELEASE                  1


#endif  // GLOBALS_H
