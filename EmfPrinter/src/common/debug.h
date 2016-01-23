/*
 * debug.h
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

#ifndef DEBLOG_H
#define DEBLOG_H

#if DBG     // the driver is being built in debug mode
    #define ENABLE_DEBUGGING    1
#else
    #define ENABLE_DEBUGGING    0
#endif


#define DEBUGLOGFILE        "C:\\EMFPRINTER_DEBUGLOG.TXT"
#define APP_NAME            "EMFPR"     // keep it short for better DebugView readability

// -----------------------------------------------------------------------------
// MACROS
// -----------------------------------------------------------------------------

#if ENABLE_DEBUGGING
    #define SHOULD(b)       { if (!(b)) DEBUGMESSAGE(("FAILURE CONDITION ENCOUNTERED")); }
    #define ASSERT(b)       { if (!(b)) DebugBreak(); }
    #define ASSERTFAIL()    { ASSERT(0); }

    #define MAX_ERROR_CHARS     1024
    extern char g_strError[MAX_ERROR_CHARS];

    // internal private macro: don't use this directly!
    #define _DEBUG_MESSAGE(args)                                        \
        {                                                               \
            g_strError[0] = '\0';                                       \
            AppendDebugStr(APP_NAME);                                   \
            AppendDebugStr(" [%s] [%s, %d] ", __TIME__,                 \
                           strstr(__FILE__, "src"), __LINE__);          \
            AppendDebugStr args;                                        \
            AppendDebugStr("\r\n");                                     \
        }

    // use this macro as:
    //
    //   TCHAR arg[] = TEXT("format");
    //   DEBUGMESSAGE(("This is the %d %ws string", 1, arg));
    //
    // i.e.
    //  1) always use double start/end parentheses
    //  2) don't use TEXT() macro for the format string
    //  3) use %ws (or %ls) for embedding wide strings and %s for ANSI strings
    //
    // NOTE: very very important, we need to do a _single_ call to
    //       OutputDebugString() in the same DEBUGMESSAGE() macro
    //       otherwise the output may appear in DebugView screwed up
    //       because multiple debug messages (coming from different DLLs)
    //       may overlap!
    //
    #define DEBUGMESSAGE(args)                                          \
        _DEBUG_MESSAGE(args)                                            \
        SendDebugMessage(g_strError)

    // just like DEBUGMESSAGE but includes the description of the
    // last error as retrieved by the Windows API GetErrorDesc()
    #define DEBUGMESSAGE_LASTERROR(args)                                \
        {                                                               \
            /* save immediately the last err code */                    \
            DWORD last = GetLastError();                                \
            _DEBUG_MESSAGE(args)                                        \
            AppendDebugStr("Last error was: ");                         \
            AppendDebugStr("%ws", GetErrorDesc(last));                  \
            AppendDebugStr("\r\n");                                     \
            SendDebugMessage(g_strError);                               \
        }


    #ifdef __cplusplus
    extern "C"
    {
    #endif

    void SendDebugMessage(const char *err);
    void AppendDebugStr(const char *pszFormat, ...);
    void ErrorExit(const char *lpszFunction);

    #ifdef __cplusplus
    }
    #endif

#else

    #define SHOULD(b)       /* empty value */
    #define ASSERT(b)       /* empty value */
    #define ASSERTFAIL()    /* empty value */
    #define DEBUGMESSAGE(args)   /* empty value */
    #define DEBUGMESSAGE_LASTERROR(args)  /* empty value */

#endif


#endif      // DEBLOG_H

