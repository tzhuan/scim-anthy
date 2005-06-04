/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004-2005 Takuro Ashie
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

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "scim_anthy_preedit.h"
#include "scim_anthy_utils.h"

using namespace scim_anthy;

static ConvRule *get_period_rule              (TypingMethod method,
                                               PeriodStyle  period);
static ConvRule *get_comma_rule               (TypingMethod method,
                                               CommaStyle   period);

Preedit::Preedit (Key2KanaTableSet & tables)
    : m_key2kana_tables  (tables),
      m_reading          (tables),
      m_conversion       (m_reading),
      m_input_mode       (SCIM_ANTHY_MODE_HIRAGANA),
      m_auto_convert     (false)
{
}

Preedit::~Preedit ()
{
}


/*
 * getting status
 */
unsigned int
Preedit::get_length (void)
{
    if (is_converting ())
        return m_conversion.get_length ();
    else
        return m_reading.get_length ();

    return 0;
}

WideString
Preedit::get_string (void)
{
    if (is_converting ()) {
        return m_conversion.get ();
    } else {
        WideString widestr;
        switch (m_input_mode) {
        case SCIM_ANTHY_MODE_KATAKANA:
            convert_to_katakana (widestr, m_reading.get ());
            return widestr;

        case SCIM_ANTHY_MODE_HALF_KATAKANA:
            convert_to_katakana (widestr, m_reading.get (), true);
            return widestr;

        case SCIM_ANTHY_MODE_LATIN:
            return utf8_mbstowcs (m_reading.get_raw ());

        case SCIM_ANTHY_MODE_WIDE_LATIN:
            convert_to_wide (widestr, m_reading.get_raw ());
            return widestr;

        case SCIM_ANTHY_MODE_HIRAGANA:
        default:
            return m_reading.get ();
        }
    }

    return WideString ();
}

AttributeList
Preedit::get_attribute_list (void)
{
    if (is_converting ())
        return m_conversion.get_attribute_list ();
    else
        return AttributeList ();
}

bool
Preedit::is_preediting (void)
{
    if (m_reading.get_length () > 0)
        return true;
    else
        return false;
}

bool
Preedit::is_converting (void)
{
    return m_conversion.is_converting ();
}

bool
Preedit::is_kana_converting (void)
{
    return m_conversion.is_kana_converting ();
}


/*
 * manipulating the preedit string
 */
bool
Preedit::can_process_key_event (const KeyEvent & key)
{
    return m_reading.can_process_key_event (key);
}

bool
Preedit::process_key_event (const KeyEvent & key)
{
    if (!m_reading.can_process_key_event (key))
        return false;

    bool retval = m_reading.process_key_event (key);

    if (m_input_mode == SCIM_ANTHY_MODE_LATIN ||
        m_input_mode == SCIM_ANTHY_MODE_WIDE_LATIN)
    {
        return true;
    }

    // auto convert
    unsigned int len = m_reading.get_length ();
    if (len > 0) {
        String str;
        m_reading.get_raw (str, len - 1, 1);
        if (is_comma_or_period (str) && m_auto_convert)
            convert ();
    }

    return retval;
}

void
Preedit::erase (bool backward)
{
    if (m_reading.get_length () <= 0)
        return;

    // cancel conversion
    revert ();

    // erase
    TypingMethod method = m_key2kana_tables.get_typing_method ();
    bool allow_split
        = (method == SCIM_ANTHY_TYPING_METHOD_ROMAJI) && m_romaji_allow_split;
    if (backward && m_reading.get_caret_pos () == 0)
        return;
    if (!backward && m_reading.get_caret_pos () >= m_reading.get_length ())
        return;
    if (backward)
        m_reading.move_caret (-1, allow_split);
    m_reading.erase (m_reading.get_caret_pos (), 1, allow_split);
}

void
Preedit::finish (void)
{
    m_reading.finish ();
}


/*
 * manipulating conversion string
 */
void
Preedit::convert (CandidateType type)
{
    m_conversion.start (type);
}

void
Preedit::revert (void)
{
    m_conversion.clear ();
}

void
Preedit::commit (int segment_id, bool learn)
{
    if (m_conversion.is_converting ())
        m_conversion.commit (segment_id, learn);
    if (!m_conversion.is_converting ())
        clear ();
}

int
Preedit::get_nr_segments (void)
{
    return m_conversion.get_nr_segments ();
}

WideString
Preedit::get_segment_string (int segment_id)
{
    return m_conversion.get_segment_string (segment_id);
}

int
Preedit::get_selected_segment (void)
{
    return m_conversion.get_selected_segment ();
}

void
Preedit::select_segment (int segment_id)
{
    m_conversion.select_segment (segment_id);
}

int
Preedit::get_segment_size (int segment_id)
{
    return m_conversion.get_segment_size (segment_id);
}

void
Preedit::resize_segment (int relative_size, int segment_id)
{
    m_conversion.resize_segment (relative_size, segment_id);
}


/*
 * candidates for a segment
 */
void
Preedit::get_candidates (CommonLookupTable &table, int segment_id)
{
    m_conversion.get_candidates (table, segment_id);
}

int
Preedit::get_selected_candidate (int segment_id)
{
    return m_conversion.get_selected_candidate (segment_id);
}

void
Preedit::select_candidate (int candidate_id, int segment_id)
{
    m_conversion.select_candidate (candidate_id, segment_id);
}


/*
 * manipulating the caret
 */
unsigned int
Preedit::get_caret_pos (void)
{
    if (is_converting ()) {
        return m_conversion.get_segment_position ();
    } else {
        if (get_input_mode () == SCIM_ANTHY_MODE_HALF_KATAKANA) {
            // FIXME! It's ad-hoc
            WideString substr;
            m_reading.get (substr, 0, m_reading.get_caret_pos (),
                           SCIM_ANTHY_STRING_HALF_KATAKANA);
            return substr.length ();
        } else {
            return m_reading.get_caret_pos ();
        }
    }
}

void
Preedit::set_caret_pos (unsigned int pos)
{
    if (is_converting ())
        return;

    m_reading.set_caret_pos (pos);
}

void
Preedit::move_caret (int step)
{
    if (is_converting ())
        return;

    TypingMethod method = m_key2kana_tables.get_typing_method ();
    bool allow_split
        = (method == SCIM_ANTHY_TYPING_METHOD_ROMAJI) && m_romaji_allow_split;

    m_reading.move_caret (step, allow_split);
}


/*
 * clear all string
 */
void
Preedit::clear (void)
{
    m_reading.clear ();
    m_conversion.clear ();
}


/*
 * preference
 */
void
Preedit::set_input_mode (InputMode mode)
{
    m_input_mode = mode;
}

InputMode
Preedit::get_input_mode (void)
{
    return m_input_mode;
}

void
Preedit::set_ten_key_type (TenKeyType type)
{
    m_reading.set_ten_key_type (type);
}

TenKeyType
Preedit::get_ten_key_type (void)
{
    return m_reading.get_ten_key_type ();
}

void
Preedit::set_auto_convert (bool autoconv)
{
    m_auto_convert = autoconv;
}

bool
Preedit::get_auto_convert (void)
{
    return m_auto_convert;
}

void
Preedit::set_allow_split_romaji (bool allow)
{
    m_romaji_allow_split = allow;
}

bool
Preedit::get_allow_split_romaji (void)
{
    return m_romaji_allow_split;
}

bool
Preedit::is_comma_or_period (const String & str)
{
    TypingMethod typing = m_key2kana_tables.get_typing_method ();
    PeriodStyle  period = m_key2kana_tables.get_period_style ();
    CommaStyle   comma  = m_key2kana_tables.get_comma_style ();

    ConvRule *period_rule = get_period_rule (typing, period);
    ConvRule *comma_rule  = get_comma_rule  (typing, comma);

    for (unsigned int i = 0; period_rule && period_rule[i].string; i++) {
        if (period_rule[i].string &&
            !strcmp (period_rule[i].string, str.c_str ()))
        {
            return true;
        }
    }
    for (unsigned int i = 0; comma_rule && comma_rule[i].string; i++) {
        if (comma_rule[i].string &&
            !strcmp (comma_rule[i].string, str.c_str ()))
        {
            return true;
        }
    }

    return false;
}



/*
 * utilities
 */
static ConvRule *
get_period_rule (TypingMethod method, PeriodStyle period)
{
    switch (method) {
    case SCIM_ANTHY_TYPING_METHOD_KANA:
        switch (period) {
        case SCIM_ANTHY_PERIOD_WIDE:
            return scim_anthy_kana_wide_period_rule;
        case SCIM_ANTHY_PERIOD_HALF:
            return scim_anthy_kana_half_period_rule;
        case SCIM_ANTHY_PERIOD_JAPANESE:
        default:
            return scim_anthy_kana_ja_period_rule;
        };
        break;

    case SCIM_ANTHY_TYPING_METHOD_ROMAJI:
    default:
        switch (period) {
        case SCIM_ANTHY_PERIOD_WIDE:
            return scim_anthy_romaji_wide_period_rule;
        case SCIM_ANTHY_PERIOD_HALF:
            return scim_anthy_romaji_half_period_rule;
        case SCIM_ANTHY_PERIOD_JAPANESE:
        default:
            return scim_anthy_romaji_ja_period_rule;
        };
        break;
    };

    return NULL;
}

static ConvRule *
get_comma_rule (TypingMethod method, CommaStyle period)
{
    switch (method) {
    case SCIM_ANTHY_TYPING_METHOD_KANA:
        switch (period) {
        case SCIM_ANTHY_PERIOD_WIDE:
            return scim_anthy_kana_wide_comma_rule;
        case SCIM_ANTHY_PERIOD_HALF:
            return scim_anthy_kana_half_comma_rule;
        case SCIM_ANTHY_PERIOD_JAPANESE:
        default:
            return scim_anthy_kana_ja_comma_rule;
        };
        break;

    case SCIM_ANTHY_TYPING_METHOD_ROMAJI:
    default:
        switch (period) {
        case SCIM_ANTHY_PERIOD_WIDE:
            return scim_anthy_romaji_wide_comma_rule;
        case SCIM_ANTHY_PERIOD_HALF:
            return scim_anthy_romaji_half_comma_rule;
        case SCIM_ANTHY_PERIOD_JAPANESE:
        default:
            return scim_anthy_romaji_ja_comma_rule;
        };
        break;
    };

    return NULL;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
