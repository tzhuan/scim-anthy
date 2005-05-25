/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "scim_anthy_preedit.h"

using namespace scim_anthy;

static void      convert_string_to_wide       (WideString       &wide,
                                               const String     &str);
static void      convert_hiragana_to_katakana (WideString       &kata,
                                               const WideString &hira,
                                               bool              half = false);
static ConvRule *get_period_rule              (TypingMethod method,
                                               PeriodStyle  period);
static ConvRule *get_comma_rule               (TypingMethod method,
                                               CommaStyle   period);



Preedit::Preedit (Key2KanaTableSet & tables)
    : m_key2kana_tables  (tables),
      m_reading          (tables, m_iconv),
      m_anthy_context    (anthy_create_context()),
      m_input_mode       (SCIM_ANTHY_MODE_HIRAGANA),
      m_auto_convert     (false),
      m_start_segment_id (0),
      m_cur_segment_id   (-1),
      m_cur_segment_pos  (0),
      m_kana_converting  (false)
{
#ifdef HAS_ANTHY_CONTEXT_SET_ENCODING
    anthy_context_set_encoding (m_anthy_context, ANTHY_EUC_JP_ENCODING);
#endif /* HAS_ANTHY_CONTEXT_SET_ENCODING */

    if (!m_iconv.set_encoding ("EUC-JP"))
        return;
}

Preedit::~Preedit ()
{
    anthy_release_context (m_anthy_context);
}


/*
 * getting status
 */
unsigned int
Preedit::get_length (void)
{
    if (is_converting ())
        return m_conv_string.length ();
    else
        return m_reading.get_length ();

    return 0;
}

WideString
Preedit::get_string (void)
{
    if (is_converting ()) {
        return m_conv_string;
    } else {
        WideString widestr;
        switch (m_input_mode) {
        case SCIM_ANTHY_MODE_KATAKANA:
            convert_hiragana_to_katakana (widestr, m_reading.get ());
            return widestr;

        case SCIM_ANTHY_MODE_HALF_KATAKANA:
            convert_hiragana_to_katakana (widestr, m_reading.get (), true);
            return widestr;

        case SCIM_ANTHY_MODE_LATIN:
            return utf8_mbstowcs (m_reading.get_raw ());

        case SCIM_ANTHY_MODE_WIDE_LATIN:
            convert_string_to_wide (widestr, m_reading.get_raw ());
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
        return m_conv_attrs;
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
    if (m_conv_string.length () > 0)
        return true;
    else
        return false;
}

bool
Preedit::is_kana_converting (void)
{
    return m_kana_converting;
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
    int pos = m_reading.get_caret_pos ();
    if (backward)
        pos--;
    if (pos < 0)
        return;
    m_reading.erase (pos, 1);
}

void
Preedit::flush_pending (void)
{
    m_reading.commit ();
}


/*
 * manipulating conversion string
 */
void
Preedit::create_conversion_string (void)
{
    m_conv_string.clear ();
    m_conv_attrs.clear ();

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return;
    if (m_start_segment_id < 0 || m_start_segment_id >= conv_stat.nr_segment)
        return; /* error */

    for (int i = m_start_segment_id; i < conv_stat.nr_segment; i++) {
        int seg = i - m_start_segment_id;

        // get string of this segment
        WideString segment_str = get_segment_string (seg);

        // set caret
        if (seg == m_cur_segment_id)
            m_cur_segment_pos = m_conv_string.length ();

        // create attribute for this segment
        Attribute attr (m_conv_string.length (), segment_str.length (),
                        SCIM_ATTR_DECORATE);
        if (seg == m_cur_segment_id)
            attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
        else
            attr.set_value (SCIM_ATTR_DECORATE_UNDERLINE);

        // join the string to the whole conversion string
        if (segment_str.length () > 0) {
            m_conv_string += segment_str;
            m_conv_attrs.push_back (attr);
        }
    }
}

void
Preedit::get_reading_substr (WideString & substr,
                             unsigned int start, unsigned int len,
                             CandidateType type)
{
    WideString kana;
    String raw;

    switch (type) {
    case SCIM_ANTHY_CANDIDATE_LATIN:
        m_reading.get_raw (raw, start, len);
        substr = utf8_mbstowcs (raw);
        break;

    case SCIM_ANTHY_CANDIDATE_WIDE_LATIN:
        m_reading.get_raw (raw, start, len);
        convert_string_to_wide (substr, raw);
        break;

    case SCIM_ANTHY_CANDIDATE_HIRAGANA:
        m_reading.get (substr, start, len);
        break;

    case SCIM_ANTHY_CANDIDATE_KATAKANA:
        m_reading.get (kana, start, len);
        convert_hiragana_to_katakana (substr, kana);
        break;

    case SCIM_ANTHY_CANDIDATE_HALF_KATAKANA:
        m_reading.get (kana, start, len);
        convert_hiragana_to_katakana (substr, kana, true);
        break;

    default:
        return;
        break;
    }
}

void
Preedit::convert (CandidateType type)
{
    if (type != SCIM_ANTHY_CANDIDATE_NORMAL) {
        convert_kana (type);
        return;
    }

    if (is_converting()) {
        // FIXME!
    } else {
        String dest;

        // convert
        struct anthy_conv_stat conv_stat;
        anthy_get_stat (m_anthy_context, &conv_stat);
        if (conv_stat.nr_segment <= 0) {
            m_iconv.convert (dest, m_reading.get ());
            anthy_set_string (m_anthy_context, dest.c_str ());
        }

        /* get information about conversion string */
        //struct anthy_conv_stat conv_stat;
        anthy_get_stat (m_anthy_context, &conv_stat);
        if (conv_stat.nr_segment <= 0)
            return;

        // select first segment
        m_cur_segment_id = 0;
        m_cur_segment_pos = 0;

        // select first candidates for all segment
        m_candidates.clear ();
        for (int i = m_start_segment_id; i < conv_stat.nr_segment; i++)
            m_candidates.push_back (0);

        // create whole string
        create_conversion_string ();
    }
}

#if 1 // FIXME! it's ad-hoc
void
Preedit::convert_kana (CandidateType type)
{
    String str;
    WideString wide;

    m_conv_string.clear ();
    m_conv_attrs.clear ();
    m_candidates.clear ();
    m_cur_segment_id = 0;
    m_cur_segment_pos = 0;
    m_kana_converting = true;

    switch (type) {
    case SCIM_ANTHY_CANDIDATE_LATIN:
        m_conv_string = utf8_mbstowcs (m_reading.get_raw ());
        break;

    case SCIM_ANTHY_CANDIDATE_WIDE_LATIN:
        convert_string_to_wide (m_conv_string, m_reading.get_raw ());
        break;

    case SCIM_ANTHY_CANDIDATE_HIRAGANA:
        m_conv_string = m_reading.get ();
        break;

    case SCIM_ANTHY_CANDIDATE_KATAKANA:
        convert_hiragana_to_katakana (m_conv_string, m_reading.get ());
        break;

    case SCIM_ANTHY_CANDIDATE_HALF_KATAKANA:
        convert_hiragana_to_katakana (m_conv_string, m_reading.get (), true);
        break;

    default:
        // error
        return;
        break;
    }

    // set candidate type
    m_candidates.push_back (type);

    // create attribute for this segment
    Attribute attr (0, m_conv_string.length (),
                    SCIM_ATTR_DECORATE);
    attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
    m_conv_attrs.push_back (attr);
}
#endif // FIXME!

void
Preedit::revert (void)
{
    anthy_reset_context (m_anthy_context);

    m_conv_string.clear ();
    m_conv_attrs.clear ();
    m_candidates.clear ();
    m_start_segment_id = 0;
    m_cur_segment_id = -1;
    m_cur_segment_pos = 0;
    m_kana_converting = false;
}

void
Preedit::commit (int segment_id, bool learn)
{
    if (m_kana_converting) return;
    if (m_candidates.size () <= 0) return;

    for (unsigned int i = m_start_segment_id;
         i < m_candidates.size () &&
             (segment_id < 0 || (int) i <= segment_id);
         i++)
    {
        if (m_candidates[i] >= 0 && learn)
            anthy_commit_segment (m_anthy_context, i, m_candidates[i]);
    }


    if (segment_id >= 0 &&
        segment_id + 1 < (int) m_candidates.size ())
    {
        // remove commited segments
        std::vector<int>::iterator it = m_candidates.begin ();
        m_candidates.erase(it, it + segment_id + 1);

        // adjust selected segment
        int new_start_segment_id = m_start_segment_id + segment_id + 1;
        m_cur_segment_id -= new_start_segment_id - m_start_segment_id;
        if (m_cur_segment_id < 0)
            m_cur_segment_id = 0;

        // adjust offset
        unsigned int commited_len = 0;
        for (int i = m_start_segment_id; i < new_start_segment_id; i++) {
            struct anthy_segment_stat seg_stat;
            anthy_get_segment_stat (m_anthy_context, i, &seg_stat);
            commited_len += seg_stat.seg_len;
        }
        m_reading.erase (0, commited_len);
        m_start_segment_id = new_start_segment_id;

        // recreate conversion string
        create_conversion_string ();

    } else if (segment_id < 0 ||
               segment_id >= (int) m_candidates.size () - 1)
    {
        clear ();
    }
}

int
Preedit::get_nr_segments (void)
{
    if (!is_converting ()) return 0;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    return conv_stat.nr_segment - m_start_segment_id;
}

WideString
Preedit::get_segment_string (int segment_id)
{
    if (segment_id < 0)
        segment_id = m_cur_segment_id;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return WideString ();

    if (m_start_segment_id < 0 ||
        m_start_segment_id >= conv_stat.nr_segment)
    {
        return WideString (); // error
    }

    if (segment_id < 0 ||
        segment_id + m_start_segment_id >= conv_stat.nr_segment)
    {
        return WideString (); //error
    }

    // character position of the head of segment.
    unsigned int real_seg_start = 0;
    for (int i = 0; i < m_start_segment_id + segment_id; i++) {
        struct anthy_segment_stat seg_stat;
        anthy_get_segment_stat (m_anthy_context, i, &seg_stat);
        real_seg_start += seg_stat.seg_len;
    }

    int real_seg = segment_id + m_start_segment_id;
    int cand = m_candidates[segment_id];

    // get information of this segment
    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_seg, &seg_stat);

    // get string of this segment
    WideString segment_str;
    if (cand < 0) {
        get_reading_substr (segment_str,
                            real_seg_start, seg_stat.seg_len,
                            (CandidateType) cand);
    } else {
        int len = anthy_get_segment (m_anthy_context, real_seg, cand, NULL, 0);
        char buf[len + 1];
        anthy_get_segment (m_anthy_context, real_seg, cand, buf, len + 1);
        buf[len] = '\0';
        m_iconv.convert (segment_str, buf, len);
    }

    return segment_str;
}

int
Preedit::get_selected_segment (void)
{
    return m_cur_segment_id;
}

void
Preedit::select_segment (int segment_id)
{
    if (!is_converting ()) return;

    if (segment_id < 0) return;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    int real_segment_id = segment_id + m_start_segment_id;

    if (segment_id >= 0 && real_segment_id < conv_stat.nr_segment) {
        m_cur_segment_id = segment_id;
        create_conversion_string ();
    }
}

int
Preedit::get_segment_size (int segment_id)
{
    if (!is_converting ()) return -1;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (segment_id < 0)
        segment_id = m_cur_segment_id;
    int real_segment_id = segment_id + m_start_segment_id;

    if (real_segment_id >= conv_stat.nr_segment)
        return -1;

    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_segment_id, &seg_stat);

    return seg_stat.seg_len;
}

void
Preedit::resize_segment (int relative_size, int segment_id)
{
    if (!is_converting ()) return;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    int real_segment_id;

    if (segment_id < 0) {
        segment_id = m_cur_segment_id;
        real_segment_id = segment_id + m_start_segment_id;
    } else {
        real_segment_id = segment_id + m_start_segment_id;
        if (m_cur_segment_id > segment_id)
            m_cur_segment_id = segment_id;
    }

    if (real_segment_id >= conv_stat.nr_segment)
        return;

    // do resize
    anthy_resize_segment (m_anthy_context, real_segment_id, relative_size);

    // reset candidate of trailing segments
    anthy_get_stat (m_anthy_context, &conv_stat);
    std::vector<int>::iterator start_iter = m_candidates.begin();
    std::vector<int>::iterator end_iter   = m_candidates.end();
    m_candidates.erase (start_iter + segment_id, end_iter);
    for (int i = real_segment_id; i < conv_stat.nr_segment; i++)
        m_candidates.push_back (0);

    // recreate conversion string
    create_conversion_string ();
}


/*
 * candidates for a segment
 */
void
Preedit::get_candidates (CommonLookupTable &table, int segment_id)
{
    table.clear ();

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return;

    if (segment_id < 0)
        segment_id = m_cur_segment_id;
    int real_segment_id = segment_id + m_start_segment_id;

    if (real_segment_id >= conv_stat.nr_segment)
        return;

    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_segment_id, &seg_stat);

    for (int i = 0; i < seg_stat.nr_candidate; i++) {
        int len = anthy_get_segment (m_anthy_context, real_segment_id, i,
                                     NULL, 0);
        char *buf = (char *) malloc (len + 1);
        anthy_get_segment (m_anthy_context, real_segment_id, i, buf, len + 1);

        WideString cand_wide;
        m_iconv.convert (cand_wide, buf, len);

        table.append_candidate (cand_wide);

        free (buf);
    }
}

int
Preedit::get_selected_candidate (int segment_id)
{
    if (!is_converting ()) return -1;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return -1;

    if (segment_id < 0)
        segment_id = m_cur_segment_id;
    else if (segment_id >= conv_stat.nr_segment)
        return -1;

    return m_candidates[segment_id];
}

void
Preedit::select_candidate (int candidate_id, int segment_id)
{
    if (!is_converting ()) return;

    if (candidate_id <= SCIM_ANTHY_LAST_SPECIAL_CANDIDATE) return;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return;

    if (segment_id < 0)
        segment_id = m_cur_segment_id;
    int real_segment_id = segment_id + m_start_segment_id;

    if (segment_id >= conv_stat.nr_segment)
        return;

    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_segment_id, &seg_stat);

    if (candidate_id < seg_stat.nr_candidate) {
        m_candidates[segment_id] = candidate_id;
        create_conversion_string ();
    }
}


/*
 * manipulating the caret
 */
unsigned int
Preedit::get_caret_pos (void)
{
    if (is_converting ())
        return m_cur_segment_pos;
    else
        return m_reading.get_caret_pos ();
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

    m_reading.move_caret (step);
}


/*
 * clear all string
 */
void
Preedit::clear (void)
{
    m_reading.clear ();
    m_conv_string.clear ();
    m_conv_attrs.clear ();
    anthy_reset_context (m_anthy_context);
    m_candidates.clear ();
    m_start_segment_id = 0;
    m_cur_segment_id = -1;
    m_cur_segment_pos = 0;
    m_kana_converting = false;
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
static void
convert_string_to_wide (WideString & wide, const String & str)
{
    if (str.length () < 0)
        return;

    for (unsigned int i = 0; i < str.length (); i++) {
        int c = str[i];
        char cc[2]; cc[0] = c; cc[1] = '\0';
        bool found = false;

        for (unsigned int j = 0; scim_anthy_wide_table[j].code; j++) {
            if ( scim_anthy_wide_table[j].code &&
                *scim_anthy_wide_table[j].code == c)
            {
                wide += utf8_mbstowcs (scim_anthy_wide_table[j].wide);
                found = true;
                break;
            }
        }

        if (!found)
            wide += utf8_mbstowcs (cc);
    }
}

static void
convert_hiragana_to_katakana (WideString & kata, const WideString & hira,
                              bool half)
{
    if (hira.length () < 0)
        return;

    for (unsigned int i = 0; i < hira.length (); i++) {
        WideString tmpwide;
        bool found = false;

        HiraganaKatakanaRule *table = scim_anthy_hiragana_katakana_table;

        for (unsigned int j = 0; table[j].hiragana; j++) {
            tmpwide = utf8_mbstowcs (table[j].hiragana);
            if (hira.substr(i, 1) == tmpwide) {
                if (half)
                    kata += utf8_mbstowcs (table[j].half_katakana);
                else
                    kata += utf8_mbstowcs (table[j].katakana);
                found = true;
                break;
            }
        }

        if (!found)
            kata += hira.substr(i, 1);
    }
}

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
