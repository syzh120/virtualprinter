/*
 * stdheaderwrapper.h
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

#ifndef STDHEADERS_H
#define STDHEADERS_H

#define STRSAFE_NO_DEPRECATE
    // otherwise we get a sprintf_instead_use_StringCbPrintfA_or_StringCchPrintfA
    // error from locale.h or 'anachronism' errors or other weird ones!
	
#if !defined(USERMODE_DRIVER) && !defined(BUILDING_INSTALLER)
	// we need to define USERMODE_DRIVER _before_ including winddi.h to correctly
	// build the driver as user-mode driver!
	#error The symbol USERMODE_DRIVER should have been defined!
#endif

#include <windows.h>
#include <wchar.h>
#include <winspool.h>

#ifndef BUILDING_INSTALLER
#include <winsplp.h>
#include <winddi.h>
#include <winddiui.h>
extern "C" {
#include <winppi.h>
}
#endif		// BUILDING_INSTALLER

#include <tchar.h>
#include <strsafe.h>
#include <malloc.h>
#include <excpt.h>

#endif  // STDHEADERS_H
