/*
 * buffer.h
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

#ifndef BUFFER_H
#define BUFFER_H

#include "stdheaderwrapper.h"


class Buffer
{
public:
    enum errcode { eAllocateFail };

    Buffer()
    {
        m_pBuff = NULL;
        m_dwSize = 0;
    }

    Buffer(DWORD dwSize) throw(.../*errcode*/)
    {
        m_pBuff = MyRealloc(NULL, dwSize);
        m_dwSize = dwSize;
    }

    Buffer(const Buffer& tocopy) throw(.../*errcode*/)
    {
        m_pBuff = MyRealloc(NULL, tocopy.m_dwSize);
        m_dwSize = tocopy.m_dwSize;

        memcpy(m_pBuff, tocopy.m_pBuff, m_dwSize);
    }

    ~Buffer() 
        { MyFree(m_pBuff); }

    VOID RequireMinSize(DWORD dwNewSize)
    {
        if (m_dwSize < dwNewSize) {
            m_pBuff = MyRealloc(m_pBuff, dwNewSize);
            m_dwSize = dwNewSize;
        }
    }

    PVOID GetPtr() const { return m_pBuff; }
    DWORD GetSize() const { return m_dwSize; }

private:
    static PVOID MyRealloc(PVOID pvOld, DWORD dwNewSize) throw(.../*errcode*/) 
    {
        PVOID pvRetValue = NULL;
        if (dwNewSize != 0) {
            if (pvOld != NULL) {
                pvRetValue = realloc(pvOld, dwNewSize);
            } else {
                pvRetValue = malloc(dwNewSize);
            }
            if (pvRetValue == NULL) {
                throw(eAllocateFail);
            }
        } else {
            MyFree(pvOld);
        }
        return pvRetValue;
    }

    static VOID MyFree(PVOID pv)
    {
        if (pv != NULL) {
            free(pv);
        }
    }
            
    PVOID m_pBuff;
    DWORD m_dwSize;
};

class ItemBuffer
{
public:
    ItemBuffer(DWORD dwItemCount, DWORD dwItemSize) throw(.../*Buffer::errcode*/) 
        : m_dwItemSize(dwItemSize),
          m_buff(dwItemCount * dwItemSize)
    { 
        m_dwItemCount = dwItemCount;
    }

    ~ItemBuffer() {}

    VOID RequireMinCount(DWORD dwNewItemCount) throw(.../*Buffer::errcode*/)
    { 
        m_buff.RequireMinSize(dwNewItemCount * m_dwItemSize);
        m_dwItemCount = dwNewItemCount;
    }

    PVOID GetPtr(VOID) const { return m_buff.GetPtr(); }
    DWORD GetCount(VOID) const { return m_dwItemCount; }

private:
    const DWORD m_dwItemSize;
    DWORD m_dwItemCount;
    Buffer m_buff;
};

#endif