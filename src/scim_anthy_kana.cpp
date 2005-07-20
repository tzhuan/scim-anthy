/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "scim_anthy_kana.h"

using namespace scim_anthy;

KanaConvertor::KanaConvertor ()
    : m_ten_key_type    (SCIM_ANTHY_TEN_KEY_FOLLOW_MODE)
{
}

KanaConvertor::~KanaConvertor ()
{
}

bool
KanaConvertor::can_append (const KeyEvent & key)
{
    // ignore key release.
    if (key.is_key_release ())
        return false;

    // ignore short cut keys of apllication.
    if (key.mask & SCIM_KEY_ControlMask ||
        key.mask & SCIM_KEY_AltMask)
    {
        return false;
    }

    if (key.code == SCIM_KEY_overline ||
        (key.code >= SCIM_KEY_kana_fullstop &&
         key.code <= SCIM_KEY_semivoicedsound))
    {
        return true;
    }

    return false;
}

bool
KanaConvertor::append (const KeyEvent & key,
                       WideString & result,
                       WideString & pending,
                       String &raw)
{
    KanaRule *table = scim_anthy_kana_table;

    for (unsigned int i = 0; table[i].code; i++) {
        if (table[i].code == key.code) {
            result = utf8_mbstowcs (table[i].kana);
            raw    = key.get_ascii_code ();
            return false;
        }
    }

    String s;
    s += key.get_ascii_code ();
    result = utf8_mbstowcs (s);
    raw    = s;

    return false;
}

bool
KanaConvertor::append (const String   & str,
                       WideString     & result,
                       WideString     & pending)
{
    return false;
}

void
KanaConvertor::clear (void)
{
}

bool
KanaConvertor::is_pending (void)
{
    return false;
}

WideString
KanaConvertor::get_pending (void)
{
    return WideString ();
}

WideString
KanaConvertor::flush_pending (void)
{
    return WideString ();
}

void
KanaConvertor::set_case_sensitive (bool sens)
{
}

bool
KanaConvertor::get_case_sensitive (void)
{
    return false;
}

void
KanaConvertor::set_ten_key_type (TenKeyType type)
{
    m_ten_key_type = type;
}

TenKeyType
KanaConvertor::get_ten_key_type (void)
{
    return m_ten_key_type;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
