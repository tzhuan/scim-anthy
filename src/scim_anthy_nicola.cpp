/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 Takuro Ashie
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

#include "scim_anthy_nicola.h"

using namespace scim_anthy;

NicolaConvertor::NicolaConvertor ()
    : //m_tables         (tables),
      m_case_sensitive  (false),
      m_ten_key_type    (SCIM_ANTHY_TEN_KEY_FOLLOW_MODE),
      m_has_pressed_key (false),
      m_shift_type      (SCIM_ANTHY_NICOLA_SHIFT_NONE)
{
}

NicolaConvertor::~NicolaConvertor ()
{
}

bool
NicolaConvertor::can_append (const KeyEvent & key)
{
    if (isprint (key.get_ascii_code ()) &&
        !isspace (key.get_ascii_code ()))
    {
        return true;
    }

    if (key.code == SCIM_KEY_Henkan || key.code == SCIM_KEY_Muhenkan)
        return true;

    return false;
}

void
NicolaConvertor::search (const KeyEvent key,
                         NicolaShiftType shift_type,
                         WideString &result,
                         String &raw)
{
    NicolaRule *table = scim_anthy_nicola_table;

    raw = key.get_ascii_code ();

    String str1;
    if (m_case_sensitive)
        str1 = raw;
    else
        str1 = tolower (key.get_ascii_code ());

    for (unsigned int i = 0; table[i].key; i++) {
        String str2 = table[i].key;
        for (unsigned int j = 0; !m_case_sensitive && j < str2.length (); j++)
            str2[j] = tolower (str2[j]);

        if (str1 == str2) {
            switch (shift_type) {
            case SCIM_ANTHY_NICOLA_SHIFT_RIGHT:
                result = utf8_mbstowcs (table[i].right_shift);
                break;
            case SCIM_ANTHY_NICOLA_SHIFT_LEFT:
                result = utf8_mbstowcs (table[i].left_shift);
                break;
            default:
                result = utf8_mbstowcs (table[i].single);
                break;
            }
            break;
        }
    }

    if (result.empty ())
        result = utf8_mbstowcs (raw);
}

bool
NicolaConvertor::append (const KeyEvent & key,
                         WideString & result,
                         WideString & pending,
                         String &raw)
{
    if (isprint (key.get_ascii_code ())) {
        if (key.is_key_press ()) {
            if (m_shift_type == SCIM_ANTHY_NICOLA_SHIFT_RIGHT) {
                search (key, SCIM_ANTHY_NICOLA_SHIFT_RIGHT, result, raw);
                m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;
            } else if (m_shift_type == SCIM_ANTHY_NICOLA_SHIFT_LEFT) {
                search (key, SCIM_ANTHY_NICOLA_SHIFT_LEFT, result, raw);
                m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;
            } else {
                if (m_has_pressed_key) {
                    search (m_prev_pressed_key,
                            SCIM_ANTHY_NICOLA_SHIFT_NONE,
                            result, raw);
                    m_has_pressed_key = false;
                }

                m_prev_pressed_key = key;
                m_has_pressed_key = true;
            }

        } else if (m_has_pressed_key /*&& m_prev_pressed_key.code == key.code*/) {
            search (m_prev_pressed_key,
                    SCIM_ANTHY_NICOLA_SHIFT_NONE,
                    result, raw);
            m_has_pressed_key = false;
        }

    } else if (key.code == SCIM_KEY_Henkan) {
        if (key.is_key_release ()) {
            m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;

        } else {
            if (m_has_pressed_key) {
                search (m_prev_pressed_key,
                        SCIM_ANTHY_NICOLA_SHIFT_RIGHT,
                        result, raw);
                m_has_pressed_key = false;
                m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;

            } else {
                m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_RIGHT;
            }
        }

    } else if (key.code == SCIM_KEY_Muhenkan) {
        if (key.is_key_release ()) {
            m_shift_type  = SCIM_ANTHY_NICOLA_SHIFT_NONE;

        } else {
            if (m_has_pressed_key) {
                search (m_prev_pressed_key,
                        SCIM_ANTHY_NICOLA_SHIFT_LEFT,
                        result, raw);
                m_has_pressed_key = false;
                m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;

            } else {
                m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_LEFT;
            }
        }
    }

    return false;
}

bool
NicolaConvertor::append (const String   & str,
                         WideString     & result,
                         WideString     & pending)
{
    return false;
}

void
NicolaConvertor::clear (void)
{
}

bool
NicolaConvertor::is_pending (void)
{
    return false;
}

WideString
NicolaConvertor::get_pending (void)
{
    return WideString ();
}

WideString
NicolaConvertor::flush_pending (void)
{
    return WideString ();
}

void
NicolaConvertor::set_case_sensitive (bool sens)
{
    m_case_sensitive = sens;
}

bool
NicolaConvertor::get_case_sensitive (void)
{
    return m_case_sensitive;
}

void
NicolaConvertor::set_ten_key_type (TenKeyType type)
{
    m_ten_key_type = type;
}

TenKeyType
NicolaConvertor::get_ten_key_type (void)
{
    return m_ten_key_type;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
