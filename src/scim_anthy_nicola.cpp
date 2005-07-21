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
    : //m_tables             (tables),
      m_case_sensitive     (false),
      m_ten_key_type       (SCIM_ANTHY_TEN_KEY_FOLLOW_MODE),
      m_nicola_time        (200000),
      m_has_pressed_key    (false),
      m_shift_type         (SCIM_ANTHY_NICOLA_SHIFT_NONE),
      m_repeat_thumb_shift (SCIM_ANTHY_NICOLA_SHIFT_NONE),
      m_is_repeating       (false)
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
NicolaConvertor::is_thumb_shift_key (const KeyEvent key)
{
    if (is_left_thumb_shift_key (key) || is_right_thumb_shift_key (key))
        return true;

    return false;
}

bool
NicolaConvertor::is_left_thumb_shift_key (const KeyEvent key)
{
    if (key.code == SCIM_KEY_Muhenkan)
        return true;

    return false;
}

bool
NicolaConvertor::is_right_thumb_shift_key (const KeyEvent key)
{
    if (key.code == SCIM_KEY_Henkan)
        return true;

    return false;
}

NicolaShiftType
NicolaConvertor::get_thumb_shift_key_type (const KeyEvent key)
{
    if (is_left_thumb_shift_key (key))
        return SCIM_ANTHY_NICOLA_SHIFT_LEFT;
    else if (is_right_thumb_shift_key (key))
        return SCIM_ANTHY_NICOLA_SHIFT_RIGHT;
    else
        return SCIM_ANTHY_NICOLA_SHIFT_NONE;
}

void
NicolaConvertor::handle_repeat (const KeyEvent   key,
                                WideString     & result,
                                String         & raw)
{
    if (key.is_key_release ()) {
        m_repeat_thumb_shift = SCIM_ANTHY_NICOLA_SHIFT_NONE;
        m_is_repeating = false;
        m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;
        m_has_pressed_key = false;

    } else if ((key == m_repeat_key) ||
               (m_repeat_thumb_shift == get_thumb_shift_key_type (key)))
    {
        if (m_has_pressed_key)
            search (m_prev_pressed_key, m_shift_type, result, raw);
        else
            ; // FIXME! emmit key event and through it


    } else if (!is_thumb_shift_key (key) && key != m_repeat_key) {
        m_repeat_thumb_shift = SCIM_ANTHY_NICOLA_SHIFT_NONE;
        m_is_repeating = false;

        m_prev_pressed_key = key;
        m_has_pressed_key = true;
        m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;

    } else if (m_shift_type == get_thumb_shift_key_type (key)) {
        m_repeat_thumb_shift = SCIM_ANTHY_NICOLA_SHIFT_NONE;
        m_is_repeating = false;

        m_shift_type = get_thumb_shift_key_type (key);
        m_has_pressed_key = false;

    } else {
        m_repeat_thumb_shift = SCIM_ANTHY_NICOLA_SHIFT_NONE;
        m_is_repeating = false;
        m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;
        m_has_pressed_key = false;
    }
}

void
NicolaConvertor::handle_both_key_pressed (const KeyEvent key,
                                          WideString & result,
                                          String &raw)
{
    struct timeval cur_time;
    long diff1, diff2;
    gettimeofday (&cur_time, NULL);

    diff1 = m_time_thumb.tv_usec - m_time.tv_usec;
    diff2 = cur_time.tv_usec - m_time_thumb.tv_usec;

    if (key.is_key_press () && m_shift_type == get_thumb_shift_key_type (key)) {
        search (m_prev_pressed_key, m_shift_type, result, raw);
        m_prev_pressed_key = key;
        m_shift_type = get_thumb_shift_key_type (key);
        m_is_repeating = true;

    } else if (!is_thumb_shift_key (key) && isprint (key.get_ascii_code ())) {
        if (key.is_key_press ()) {
            if (diff2 < diff1) {
                WideString result1, result2;
                String raw1, raw2;
                search (m_prev_pressed_key,
                        SCIM_ANTHY_NICOLA_SHIFT_NONE,
                        result1, raw1);
                search (key, m_shift_type, result2, raw2);
                result = result1 + result2;
                raw = raw1 + raw2;

                /*
                m_has_pressed_key = false;
                m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;
                */
                m_repeat_key = key;
                m_repeat_thumb_shift = get_thumb_shift_key_type (key);
                m_is_repeating = true;
                

            } else {
                search (m_prev_pressed_key, m_shift_type, result, raw);
                m_prev_pressed_key = key;
                m_has_pressed_key = true;
                m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;
            }

        } else {
            if (diff2 < m_nicola_time && diff1 > diff2) {
                search (m_prev_pressed_key,
                        SCIM_ANTHY_NICOLA_SHIFT_NONE,
                        result, raw);
                m_has_pressed_key = false;
            } else {
                search (m_prev_pressed_key, m_shift_type, result, raw);
                m_has_pressed_key = false;
                m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;
            }
        }
    } else if (is_thumb_shift_key (key)) {
        if (key.is_key_press ()) {
            search (m_prev_pressed_key, m_shift_type, result, raw);
            m_has_pressed_key = false;
            m_shift_type = get_thumb_shift_key_type (key);
            gettimeofday (&m_time_thumb, NULL);
        } else {
            search (m_prev_pressed_key, m_shift_type, result, raw);
            m_has_pressed_key = false;
            m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;
        }
    } else {
        search (m_prev_pressed_key, m_shift_type, result, raw);
        m_has_pressed_key = false;
        m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;
    }
}

void
NicolaConvertor::handle_thumb_key_pressed (const KeyEvent key,
                                           WideString & result,
                                           String &raw)
{
    if (key.is_key_press () && m_shift_type == get_thumb_shift_key_type (key)) {
        m_repeat_thumb_shift = get_thumb_shift_key_type (key);
        m_is_repeating = true;

    } else if (is_thumb_shift_key (key) && key.is_key_release ()) {
        // FIXME! emmit key press event, and through it.
        m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;

    } else if (is_thumb_shift_key (key) && key.is_key_press ()) {
        m_shift_type = get_thumb_shift_key_type (key);
        gettimeofday (&m_time_thumb, NULL);

    } else if (isprint (key.get_ascii_code ()) && key.is_key_press ()) {
        m_prev_pressed_key = key;
        m_has_pressed_key  = true;
        gettimeofday (&m_time, NULL);

        search (m_prev_pressed_key, m_shift_type, result, raw);

        // repeat
        m_repeat_key = key;
        m_repeat_thumb_shift = get_thumb_shift_key_type (key);
        m_is_repeating = true;

    } else {
        m_shift_type = SCIM_ANTHY_NICOLA_SHIFT_NONE;
    }
}

void
NicolaConvertor::handle_char_key_pressed (const KeyEvent key,
                                          WideString & result,
                                          String &raw)
{
    if (key.is_key_press () && key.code == m_prev_pressed_key.code) {
        search (m_prev_pressed_key,
                SCIM_ANTHY_NICOLA_SHIFT_NONE,
                result, raw);
        m_prev_pressed_key = key;
        m_is_repeating = true;

    } else if (!is_thumb_shift_key (key) &&
               isprint (key.get_ascii_code ()) && key.is_key_press ())
    {
        search (m_prev_pressed_key,
                SCIM_ANTHY_NICOLA_SHIFT_NONE,
                result, raw);
        m_prev_pressed_key = key;
        m_has_pressed_key  = true;
        gettimeofday (&m_time, NULL);

    } else if (is_thumb_shift_key (key) && key.is_key_press ()) {
        m_shift_type = get_thumb_shift_key_type (key);
        gettimeofday (&m_time_thumb, NULL);

    } else if (key.is_key_release () && key.code == m_prev_pressed_key.code) {
        search (m_prev_pressed_key,
                SCIM_ANTHY_NICOLA_SHIFT_NONE,
                result, raw);
        m_has_pressed_key  = false;
    } else {
        search (m_prev_pressed_key,
                SCIM_ANTHY_NICOLA_SHIFT_NONE,
                result, raw);
        m_has_pressed_key  = false;
    }
}

void
NicolaConvertor::handle_no_key_pressed (const KeyEvent key)
{
    if (key.is_key_release ())
        return;

    if (!is_thumb_shift_key (key) && isprint (key.get_ascii_code ())) {
        m_prev_pressed_key = key;
        m_has_pressed_key = true;
        gettimeofday (&m_time, NULL);
    } else if (is_thumb_shift_key (key)) {
        m_shift_type = get_thumb_shift_key_type (key);
        gettimeofday (&m_time_thumb, NULL);
    }
}

bool
NicolaConvertor::append (const KeyEvent & key,
                         WideString & result,
                         WideString & pending,
                         String &raw)
{
    if (m_is_repeating) {
        handle_repeat (key, result, raw);
    } else if (m_shift_type != SCIM_ANTHY_NICOLA_SHIFT_NONE &&
               m_has_pressed_key)
    {
        handle_both_key_pressed (key, result, raw);
    } else if (m_shift_type != SCIM_ANTHY_NICOLA_SHIFT_NONE) {
        handle_thumb_key_pressed (key, result, raw);
    } else if (m_has_pressed_key) {
        handle_char_key_pressed (key, result, raw);
    } else {
        handle_no_key_pressed (key);
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
