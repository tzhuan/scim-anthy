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

#include <scim_anthy_reading.h>

using namespace scim_anthy;

ReadingSegment::ReadingSegment ()
{
}

ReadingSegment::~ReadingSegment ()
{
}

static const char *
find_romaji (WideString c)
{
    ConvRule *table = scim_anthy_romaji_typing_rule;

    for (unsigned int i = 0; table[i].string; i++) {
        WideString kana = utf8_mbstowcs (table[i].result);
        if (c == kana)
            return table[i].string;
    }

    return "";
}

static void
to_half (String &dest, WideString &src)
{
    WideRule *table = scim_anthy_wide_table;
    
    for (unsigned int i = 0; i < src.size (); i++) {
        bool found = false;
        WideString kana1 = src.substr (i, 1);
        for (unsigned int i = 0; table[i].code; i++) {
            WideString kana2 = utf8_mbstowcs (table[i].wide);
            if (kana1 == kana2) {
                dest += table[i].code;
                found = true;
                break;
            }
        }
        if (!found)
            dest += utf8_wcstombs (kana1);
    }
}

// Only a romaji string can be splited with raw key string.
// Other typing method aren't supported splitting raw key string.
void
ReadingSegment::split (ReadingSegments &segments)
{
    if (kana.length () <= 1)
        segments.push_back (*this);

    String half;
    to_half (half, kana);
    bool same_with_raw = half == raw;

    WideString::iterator it;
    for (unsigned int i = 0; i < kana.size (); i++) {
        WideString c = kana.substr (i, 1);
        ReadingSegment seg;
        seg.kana = c;
        if (same_with_raw)
            to_half (seg.raw, c);
        else
            seg.raw = find_romaji (c);
        segments.push_back (seg);
    }
}



Reading::Reading (Key2KanaTableSet & tables, IConvert & iconv)
    : m_key2kana_tables (tables),
      m_key2kana        (m_key2kana_tables),
      m_iconv           (iconv),
      m_segment_pos     (0),
      m_caret_offset    (0),
      m_ten_key_type    (SCIM_ANTHY_TEN_KEY_FOLLOW_MODE)
{
}

Reading::~Reading ()
{
}

bool
Reading::can_process_key_event (const KeyEvent & key)
{
    // ignore short cut keys of apllication.
    if (key.mask & SCIM_KEY_ControlMask ||
        key.mask & SCIM_KEY_AltMask)
    {
        return false;
    }

    if (isprint(key.get_ascii_code ()) && !isspace(key.get_ascii_code ()))
        return true;

    if (key.code >= SCIM_KEY_KP_0 && key.code <= SCIM_KEY_KP_9)
        return true;

    if (key.code >= SCIM_KEY_KP_Multiply && key.code <= SCIM_KEY_KP_Divide)
        return true;

    if (key.code == SCIM_KEY_KP_Equal)
        return true;

    return false;
}

bool
Reading::process_key_event (const KeyEvent & key)
{
    bool is_ten_key = true;

    if (!can_process_key_event (key))
        return false;

    char str[2];

    switch (key.code) {
    case SCIM_KEY_KP_Equal:
        str[0] = '=';
        break;

    case SCIM_KEY_KP_Multiply:
        str[0] = '*';
        break;

    case SCIM_KEY_KP_Add:
        str[0] = '+';
        break;

    case SCIM_KEY_KP_Separator:
        str[0] = ',';
        break;

    case SCIM_KEY_KP_Subtract:
        str[0] = '-';
        break;

    case SCIM_KEY_KP_Decimal:
        str[0] = '.';
        break;

    case SCIM_KEY_KP_Divide:
        str[0] = '/';
        break;

    case SCIM_KEY_KP_0:
    case SCIM_KEY_KP_1:
    case SCIM_KEY_KP_2:
    case SCIM_KEY_KP_3:
    case SCIM_KEY_KP_4:
    case SCIM_KEY_KP_5:
    case SCIM_KEY_KP_6:
    case SCIM_KEY_KP_7:
    case SCIM_KEY_KP_8:
    case SCIM_KEY_KP_9:
        str[0] = '0' + key.code - SCIM_KEY_KP_0;
        break;

    default:
        is_ten_key = false;
        str[0] = key.code;
        break;
    }

    str[1] = '\0';

    bool half = true;
    bool prev_symbol = m_key2kana_tables.symbol_is_half ();
    bool prev_number = m_key2kana_tables.number_is_half ();

    if (is_ten_key && m_ten_key_type != SCIM_ANTHY_TEN_KEY_FOLLOW_MODE) {
        if (m_ten_key_type == SCIM_ANTHY_TEN_KEY_HALF)
            half = true;
        else if (m_ten_key_type == SCIM_ANTHY_TEN_KEY_WIDE)
            half = false;

        m_key2kana_tables.set_symbol_width (half);
        m_key2kana_tables.set_number_width (half);
    }

    bool retval = append_str (String (str));

    if (is_ten_key && m_ten_key_type != SCIM_ANTHY_TEN_KEY_FOLLOW_MODE) {
        m_key2kana_tables.set_symbol_width (prev_symbol);
        m_key2kana_tables.set_number_width (prev_number);
    }

    return retval;
}

bool
Reading::append_str (const String & str)
{
    if (str.length () <= 0)
        return false;

    bool was_pending = m_key2kana.is_pending ();

    WideString result, pending;
    bool need_commiting;
    need_commiting = m_key2kana.append (str, result, pending);

    ReadingSegments::iterator begin = m_segments.begin ();

    // fix previous segment and prepare next segment if needed
    if (!was_pending ||  // previous segment was already fixed
        need_commiting)  // previous segment has been fixed
    {
        ReadingSegment c;
        m_segments.insert (begin + m_segment_pos, c);
        m_segment_pos++;
    }

    // fill segment
    if (result.length() > 0 && pending.length () > 0) {
        m_segments[m_segment_pos - 1].kana = result;

        ReadingSegment c;
        c.raw += str;
        c.kana = pending;
        m_segments.insert (begin + m_segment_pos, c);
        m_segment_pos++;

    } else if (result.length () > 0) {
        m_segments[m_segment_pos - 1].raw += str;
        m_segments[m_segment_pos - 1].kana = result;

    } else if (pending.length () > 0) {
        m_segments[m_segment_pos - 1].raw += str;
        m_segments[m_segment_pos - 1].kana = pending;

    } else {
        // error
    }

    return false;
}

void
Reading::commit (void)
{
    if (!m_key2kana.is_pending ()) return;

    WideString result;
    result = m_key2kana.flush_pending ();
    if (result.length () > 0)
        m_segments[m_segment_pos - 1].kana = result;
}

void
Reading::clear (void)
{
    m_key2kana.clear ();
    m_segments.clear ();
    m_segment_pos = 0;
    m_caret_offset = 0;
}

WideString
Reading::get (unsigned int start, int end)
{
    WideString str;
    get (str, start, end);
    return str;
}

void
Reading::get (WideString & str, unsigned int start, int len)
{
    unsigned int pos = 0, end = len > 0 ? start + len : get_length () - start;
    String raw;

    if (start >= end) return;
    if (start >= get_length ()) return;

    for (unsigned int i = 0; i < m_segments.size (); i++) {
        if (pos >= start || pos + m_segments[i].kana.length () > start) {
            unsigned int startstart = 0, len;

            if (pos >= start)
                startstart = 0;
            else
                startstart = pos - start;

            if (pos + m_segments[i].kana.length () > end)
                len = end - start;
            else
                len = m_segments[i].kana.length ();

            str += m_segments[i].kana.substr (startstart, len);
        }

        pos += m_segments[i].kana.length ();
        if (pos >= end)
            break;
    }
}

String
Reading::get_raw (unsigned int start, int end)
{
    String str;
    get_raw (str, start, end);
    return str;
}

void
Reading::get_raw (String & str, unsigned int start, int len)
{
    unsigned int pos = 0, end = len > 0 ? start + len : get_length () - start;

    if (start >= end) return;

    for (unsigned int i = 0; i < m_segments.size (); i++) {
        if (pos >= start || pos + m_segments[i].kana.length () > start) {
            // FIXME!
            str += m_segments[i].raw;
        }

        pos += m_segments[i].kana.length ();

        if (pos >= end)
            break;
    }
}

void
Reading::erase (unsigned int start, int len, bool allow_split)
{
    if (m_segments.size () <= 0)
        return;

    if (get_length () < start)
        return;

    if (len < 0)
        len = get_length () - start;

    // erase
    unsigned int pos = 0;
    for (int i = 0; i <= (int) m_segments.size (); i++) {
        if (pos < start) {
            if (i == (int) m_segments.size ())
                break;

            // we have not yet reached start position.
            pos += m_segments[i].kana.length ();

        } else if (pos == start) {
            if (i == (int) m_segments.size ())
                break;

            if (allow_split &&
                pos + m_segments[i].kana.length () > start + len)
            {
                // we have overshooted the end position!
                // we have to split this segment
                ReadingSegments segments;
                m_segments[i].split (segments);
                m_segments.erase (m_segments.begin () + i);
                for (int j = segments.size () - 1; j >= 0; j--) {
                    m_segments.insert (m_segments.begin () + i, segments[j]);
                    if ((int) m_segment_pos > i)
                        m_segment_pos++;
                }

            } else {
                // This segment is completely in the rage, erase it!
                len -= m_segments[i].kana.length ();
                m_segments.erase (m_segments.begin () + i);
                if ((int) m_segment_pos > i)
                    m_segment_pos--;
            }

            // retry from the same position
            i--;

        } else {
            if (allow_split) {
                // we have overshooted the start position!
                // we have to split the previous segment
                ReadingSegments segments;
                m_segments[i - 1].split (segments);
                pos -= m_segments[i - 1].kana.length ();
                m_segments.erase (m_segments.begin () + i - 1);
                for (int j = segments.size () - 1; j >= 0; j--) {
                    m_segments.insert (m_segments.begin () + i - 1, segments[j]);
                    if ((int) m_segment_pos > i - 1)
                        m_segment_pos++;
                }

                // retry from the previous position
                i -= 2;

            } else {
                // we have overshooted the start position, but have not been
                // allowed to split the segment.
                // So remove all string of previous segment.
                len -= pos - start;
                pos -= m_segments[i - 1].kana.length ();
                m_segments.erase (m_segments.begin () + i - 1);
                if ((int) m_segment_pos > i - 1)
                    m_segment_pos--;

                // retry from the previous position
                i -= 2;
            }
        }

        // Now all strings in the range is removed.
        // Exit the loop.
        if (len <= 0)
            break;
    }

    // reset values
    if (m_segments.size () <= 0) {
        clear ();
    } else {
        reset_pending ();
    }
}

void
Reading::reset_pending (void)
{
    if (m_key2kana.is_pending ())
        m_key2kana.clear ();

    if (m_segment_pos <= 0)
        return;

    for (unsigned int i = 0;
         i < m_segments[m_segment_pos - 1].raw.length ();
         i++)
    {
        WideString result, pending;
        m_key2kana.append (m_segments[m_segment_pos - 1].raw.substr(i, 1),
                           result, pending);
    }
}

unsigned int
Reading::get_length (void)
{
    unsigned int len = 0;
    for (unsigned int i = 0; i < m_segments.size (); i++)
        len += m_segments[i].kana.length();
    return len;
}

unsigned int
Reading::get_caret_pos (void)
{
    unsigned int pos = 0;

    for (unsigned int i = 0;
         i < m_segment_pos && i < m_segments.size ();
         i++)
    {
        pos += m_segments[i].kana.length();
    }

    pos += m_caret_offset;

    return pos;
}

void
Reading::set_caret_pos (unsigned int pos)
{
    if (pos == get_caret_pos ())
        return;

    m_key2kana.clear ();

    if (pos >= get_length ()) {
        m_segment_pos = m_segments.size ();

    } else if (pos == 0 ||  m_segments.size () <= 0) {
        m_segment_pos = 0;

    } else {
        unsigned int i, tmp_pos = 0;

        for (i = 0; tmp_pos <= pos; i++)
            tmp_pos += m_segments[i].kana.length();

        if (tmp_pos == pos) {
            m_segment_pos = i + 1;
        } else if (tmp_pos < get_caret_pos ()) {
            m_segment_pos = i;
        } else if (tmp_pos > get_caret_pos ()) {
            m_segment_pos = i + 1;
        }
    }

    reset_pending ();
}

void
Reading::move_caret (int step, bool allow_split)
{
    if (step == 0)
        return;

    if (m_key2kana.is_pending ()) {
        m_key2kana.clear ();
    }

    if (allow_split) {
        unsigned int pos = get_caret_pos ();
        if (step < 0 && pos < abs (step)) {
            // lower limit
            m_segment_pos = 0;

        } else if (step > 0 && pos + step > get_length ()) {
            // upper limit
            m_segment_pos = m_segments.size ();

        } else {
            unsigned int new_pos = pos + step;
            ReadingSegments::iterator it;
            pos = 0;
            m_segment_pos = 0;
            m_caret_offset = 0;
            for (it = m_segments.begin (); pos < new_pos; it++) {
                if (pos + it->kana.length () > new_pos) {
                    m_caret_offset = new_pos - pos;
                    break;
                } else {
                    m_segment_pos++;
                    pos += it->kana.length ();
                }
            }
        }

    } else {
        if (step < 0 && m_segment_pos < abs (step)) {
            // lower limit
            m_segment_pos = 0;

        } else if (step > 0 && m_segment_pos + step > m_segments.size ()) {
            // upper limit
            m_segment_pos = m_segments.size ();

        } else {
            // other
            m_segment_pos += step;
        }
    }

    reset_pending ();
}

void
Reading::set_ten_key_type (TenKeyType type)
{
    m_ten_key_type = type;
}

TenKeyType
Reading::get_ten_key_type (void)
{
    return m_ten_key_type;
}
