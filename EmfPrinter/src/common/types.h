/*
 * types.h
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

#ifndef TYPES_H
#define TYPES_H

#include "stdheaderwrapper.h"

typedef unsigned int                uint;
typedef unsigned long               ulong;
typedef const BYTE		            *PCBYTE;
typedef const DEVMODE               *PCDEVMODE;
typedef const DWORD		            *PCDWORD;

struct RECTD
{
	double left;
	double top;
	double right;
	double bottom;
};

struct SIZED
{
	double cx;
	double cy;
};

struct POINTD
{
	double x;
	double y;
};

typedef BOOL (CALLBACK* EXECPROC)(LPVOID param);

#endif