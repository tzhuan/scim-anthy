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


#if 1 // FIXME! it's ad-hoc.
static void      convert_string_to_wide       (const String     &str,
                                               WideString       &wide,
                                               SpaceType         space);;
static void      convert_hiragana_to_katakana (const WideString &hira,
                                               WideString       &kata,
                                               bool              half = false);
static ConvRule *get_period_rule              (TypingMethod      method,
                                               PeriodStyle       period);
static ConvRule *get_comma_rule               (TypingMethod      method,
                                               CommaStyle        period);
#endif // FIXME! it's ad-hoc.



AnthyPreeditChar::AnthyPreeditChar (void)
    : pending (true)
{
}

AnthyPreeditChar::~AnthyPreeditChar ()
{
}



AnthyPreedit::AnthyPreedit ()
    : m_anthy_context (anthy_create_context()),
      m_romaji_half_symbol (false),
      m_romaji_half_number (false),
      m_input_mode (MODE_HIRAGANA),
      m_typing_method (METHOD_ROMAKANA),
      m_period_style (PERIOD_JAPANESE),
      m_comma_style (COMMA_JAPANESE),
      m_space_type (SPACE_WIDE),
      m_ten_key_type (TEN_KEY_WIDE),
      m_auto_convert (false),
      m_start_char (0),
      m_char_caret (0),
      m_caret (0),
      m_start_segment_id (0),
      m_start_segment_pos (0),
      m_selected_segment_id (-1),
      m_selected_segment_pos (0),
      m_kana_converting (false)
{
#ifdef HAS_ANTHY_CONTEXT_SET_ENCODING
    anthy_context_set_encoding (m_anthy_context, ANTHY_EUC_JP_ENCODING);
#endif /* HAS_ANTHY_CONTEXT_SET_ENCODING */

    if (!m_iconv.set_encoding ("EUC-JP"))
        return;

    set_table (m_romaji_half_symbol, m_romaji_half_number,
               m_typing_method, m_period_style, m_comma_style, m_space_type);
}

AnthyPreedit::~AnthyPreedit ()
{
    anthy_release_context (m_anthy_context);
}


/*
 * getting status
 */
unsigned int
AnthyPreedit::get_length (AnthyPreeditStringType type)
{
    if (type == PREEDIT_CURRENT) {
        if (is_converting ())
            type = PREEDIT_CONVERSION;
        else
            type = PREEDIT_NO_CONVERSION;
    }

    switch (type) {
    case PREEDIT_RAW_KEY:
    {
#if 0 /* FIXME! */
        unsigned int len = 0;
        for (unsigned int i = 0; i < m_char_list.size (); i++)
            len += m_char_list[i].key.length();
        return len;
#else
        return get_string (PREEDIT_RAW_KEY).length ();
#endif
    }
    case PREEDIT_NO_CONVERSION:
    case PREEDIT_NO_CONVERSION_HIRAGANA:
    {
        unsigned int len = 0;
        for (unsigned int i = 0; i < m_char_list.size (); i++)
            len += m_char_list[i].kana.length();
        return len - m_start_segment_pos;
    }
    case PREEDIT_CONVERSION:
        return m_conv_string.length ();
    default:
        break;
    }

    return 0;
}

WideString
AnthyPreedit::get_string (AnthyPreeditStringType type)
{
    if (type == PREEDIT_CURRENT) {
        if (is_converting ())
            type = PREEDIT_CONVERSION;
        else
            type = PREEDIT_NO_CONVERSION;
    }

    switch (type) {
    case PREEDIT_RAW_KEY:
    {
        unsigned int len = 0;
        for (unsigned int i = 0; i < m_char_list.size (); i++)
            len += m_char_list[i].kana.length();
        WideString str;
        get_kana_substr (str, m_start_segment_pos, len, CANDIDATE_LATIN);
        return str;
    }
    case PREEDIT_NO_CONVERSION:
        return get_preedit_string ();
    case PREEDIT_NO_CONVERSION_HIRAGANA:
        return get_preedit_string_as_hiragana ();
    case PREEDIT_CONVERSION:
        return m_conv_string;
    default:
        break;
    }

    return WideString ();
}

unsigned int
AnthyPreedit::get_preedit_length (void)
{
    unsigned int len = 0;
    for (unsigned int i = 0; i < m_char_list.size (); i++)
        len += m_char_list[i].kana.length();
    return len - m_start_segment_pos;
}

WideString
AnthyPreedit::get_preedit_string_as_hiragana (void)
{
    WideString widestr;
    unsigned int len = 0;
    for (unsigned int i = 0; i < m_char_list.size (); i++)
        len += m_char_list[i].kana.length();
    get_kana_substr (widestr, m_start_segment_pos, len, CANDIDATE_HIRAGANA);
    return widestr;
}

WideString
AnthyPreedit::get_preedit_string (void)
{
    WideString tmpwidestr, widestr;
    String str;
    unsigned int i;

    switch (m_input_mode) {
    case MODE_KATAKANA:
        convert_hiragana_to_katakana (get_preedit_string_as_hiragana (),
                                      widestr);
        return widestr;
    case MODE_HALF_KATAKANA:
        convert_hiragana_to_katakana (get_preedit_string_as_hiragana (),
                                      widestr, true);
        return widestr;
    case MODE_LATIN:
        for (i = 0; i < m_char_list.size (); i++)
            str += m_char_list[i].key;
        return utf8_mbstowcs (str);
    case MODE_WIDE_LATIN:
        for (i = 0; i < m_char_list.size (); i++)
            str += m_char_list[i].key;
        convert_string_to_wide (str, widestr, m_space_type);
        return widestr;
    case MODE_HIRAGANA:
    default:
        return get_preedit_string_as_hiragana ();
    }
}

AttributeList
AnthyPreedit::get_attribute_list (AnthyPreeditStringType type)
{
    /* FIXME! */
    return m_conv_attrs;
}

bool
AnthyPreedit::is_preediting (void)
{
    if (m_char_list.size () <= 0)
        return false;
    else
        return true;
}

bool
AnthyPreedit::is_converting (void)
{
    if (m_conv_string.length () > 0)
        return true;
    else
        return false;
}

bool
AnthyPreedit::is_kana_converting (void)
{
    return m_kana_converting;
}


/*
 * manipulating the preedit string
 */
bool
AnthyPreedit::can_process (const KeyEvent & key)
{
    if (isprint(key.get_ascii_code ()))
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
AnthyPreedit::append (const KeyEvent & key)
{
    bool is_ten_key = true;

    if (!can_process (key))
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
    bool prev_symbol = m_romaji_half_symbol;
    bool prev_number = m_romaji_half_number;

    if (is_ten_key && m_ten_key_type != TEN_KEY_FOLLOW_MODE) {
        if (m_ten_key_type == TEN_KEY_HALF)
            half = true;
        else if (m_ten_key_type == TEN_KEY_WIDE)
            half = false;

        set_table (half, half,
                   m_typing_method, m_period_style,
                   m_comma_style, m_space_type);
    }

    bool retval = append_str (String (str));

    if (is_ten_key && m_ten_key_type != TEN_KEY_FOLLOW_MODE) {
        set_table (prev_symbol, prev_number,
                   m_typing_method, m_period_style,
                   m_comma_style, m_space_type);
    }

    return retval;
}

bool
AnthyPreedit::append_str (const String & str)
{
    if (str.length () <= 0)
        return false;

#if 1 /* for reseting partial commit */
    anthy_reset_context (m_anthy_context);
    m_selected_candidates.clear ();
    m_start_segment_id = 0;
#endif

    bool was_pending = m_key2kana.is_pending ();

    WideString result, pending;
    bool commit_pending;
    commit_pending = m_key2kana.append (str, result, pending);

    std::vector<AnthyPreeditChar>::iterator begin = m_char_list.begin ();

    if (commit_pending)
        m_char_list[m_char_caret - 1].pending = true;

    if (!was_pending || commit_pending) {
        AnthyPreeditChar c;
        m_char_list.insert (begin + m_char_caret, c);
        m_char_caret++;
    }

    /*
     * FIXME! It's not consider about failure of matching on pending state.
     * Automaton::append() should have more detailed return value.
     */
    if (result.length() > 0 && pending.length () > 0) {
        m_char_list[m_char_caret - 1].kana = result;
        m_char_list[m_char_caret - 1].pending = false; // FIXME!

        AnthyPreeditChar c;
        c.key += str;
        c.kana = pending;
        c.pending = true;
        m_char_list.insert (begin + m_char_caret, c);

        m_char_caret++;
    } else if (result.length () > 0) {
        m_char_list[m_char_caret - 1].key += str;
        m_char_list[m_char_caret - 1].kana = result;
        m_char_list[m_char_caret - 1].pending = false; // FIXME!
    } else if (pending.length () > 0) {
        m_char_list[m_char_caret - 1].key += str;
        m_char_list[m_char_caret - 1].kana = pending;
        m_char_list[m_char_caret - 1].pending = true;
    } else {
        // error
    }

    m_caret = 0;
    for (unsigned int i = 0; i < m_char_caret; i++)
        m_caret += m_char_list[i].kana.length();
    m_caret -= m_start_segment_pos;

    if (m_input_mode == MODE_LATIN ||
        m_input_mode == MODE_WIDE_LATIN)
        return true;

#if 1 // FIXME!
    if (str.length () == 1 && isspace(*(str.c_str())))
        return true;
#endif

    if (is_comma_or_period (m_char_list[m_char_caret - 1].key) &&
        m_auto_convert)
    {
        convert ();
    }

    return false;
}

void
AnthyPreedit::erase (bool backward)
{
    if (m_char_list.size () <= 0)
        return;

#if 1
    anthy_reset_context (m_anthy_context);
    m_selected_candidates.clear ();
    m_start_segment_id = 0;
    //m_start_segment_pos = 0;
#endif

    // erase string
    if (backward && m_char_caret > 0) {
        unsigned int key_len = m_char_list[m_char_caret - 1].key.length ();

        if (m_key2kana.is_pending () && key_len > 1) {
            String key_str = m_char_list[m_char_caret - 1].key.substr (0, key_len - 1);
            m_char_list[m_char_caret - 1].key = key_str;
            unsigned int kana_len = m_char_list[m_char_caret - 1].kana.length();
            WideString kana_str = m_char_list[m_char_caret - 1].kana.substr (0, kana_len - 1);
            m_char_list[m_char_caret - 1].kana = kana_str;
        } else {
            std::vector<AnthyPreeditChar>::iterator end = m_char_list.begin () + m_char_caret;
            std::vector<AnthyPreeditChar>::iterator begin = end - 1;
            m_char_list.erase(begin, end);
            m_char_caret--;
        }

    } else if (!backward && m_char_caret < m_char_list.size ()) {
        std::vector<AnthyPreeditChar>::iterator begin, end;
        begin = m_char_list.begin () + m_char_caret;
        end   = begin + 1;
        m_char_list.erase(begin, end);
    }

    // reset values
    if (m_char_list.size () <= 0) {
        clear ();
    } else {
        reset_pending ();

        // reset caret position
        m_caret = 0;
        for (unsigned int i = 0; i < m_char_caret; i++)
            m_caret += m_char_list[i].kana.length();
        m_caret -= m_start_segment_pos;
    }
}

void
AnthyPreedit::flush_pending (void)
{
    if (!m_key2kana.is_pending ()) return;

    WideString result;
    result = m_key2kana.flush_pending ();
    if (result.length () > 0)
        m_char_list[m_char_caret - 1].kana = result;
}


/*
 * manipulating conversion string
 */
void
AnthyPreedit::create_conversion_string (void)
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
        if (seg == m_selected_segment_id)
            m_selected_segment_pos = m_conv_string.length ();

        // create attribute for this segment
        Attribute attr (m_conv_string.length (), segment_str.length (),
                        SCIM_ATTR_DECORATE);
        if (seg == m_selected_segment_id)
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
AnthyPreedit::get_kana_substr (WideString & substr,
                               unsigned int start, unsigned int end,
                               CandidateType type)
{
    unsigned int pos = 0, i = 0;
    WideString kana;
    String raw;

    if (start >= end) return;

    do {
        if (pos >= start || pos + m_char_list[i].kana.length () > start) {
            if (type == CANDIDATE_LATIN || type == CANDIDATE_WIDE_LATIN) {
                // FIXME!
                raw += m_char_list[i].key;
            } else {
                unsigned int startstart = 0, endend;
                if (pos >= start)
                    startstart = 0;
                else
                    startstart = pos - start;
                if (pos + m_char_list[i].kana.length () > end)
                    endend = end - start;
                else
                    endend = m_char_list[i].kana.length ();
                kana += m_char_list[i].kana.substr (startstart, endend);
            }
        }
        pos += m_char_list[i].kana.length ();
        if (pos >= end)
            break;
        i++;
    } while (i < m_char_list.size ());

    switch (type) {
    case CANDIDATE_LATIN:
        substr = utf8_mbstowcs (raw);
        break;
    case CANDIDATE_WIDE_LATIN:
        convert_string_to_wide (raw, substr, m_space_type);
        break;
    case CANDIDATE_HIRAGANA:
        substr = kana;
        break;

    case CANDIDATE_KATAKANA:
        convert_hiragana_to_katakana (kana, substr);
        break;

    case CANDIDATE_HALF_KATAKANA:
        convert_hiragana_to_katakana (kana, substr, true);
        break;

    default:
        return;
        break;
    }
}

void
AnthyPreedit::convert (CandidateType type)
{
    if (type != CANDIDATE_NORMAL) {
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
            m_iconv.convert (dest, get_preedit_string_as_hiragana ());
            anthy_set_string (m_anthy_context, dest.c_str ());
        }

        /* get information about conversion string */
        //struct anthy_conv_stat conv_stat;
        anthy_get_stat (m_anthy_context, &conv_stat);
        if (conv_stat.nr_segment <= 0)
            return;

        // select first segment
        m_selected_segment_id = 0;
        m_selected_segment_pos = 0;

        // select first candidates for all segment
        m_selected_candidates.clear ();
        for (int i = m_start_segment_id; i < conv_stat.nr_segment; i++)
            m_selected_candidates.push_back (0);

        // create whole string
        create_conversion_string ();
    }
}

#if 1 // FIXME! it's ad-hoc
void
AnthyPreedit::convert_kana (CandidateType type)
{
    String str;
    WideString wide;

    m_conv_string.clear ();
    m_conv_attrs.clear ();
    m_selected_candidates.clear ();
    m_selected_segment_id = 0;
    m_selected_segment_pos = 0;
    m_kana_converting = true;

    switch (type) {
    case CANDIDATE_LATIN:
    case CANDIDATE_WIDE_LATIN:
    {
        unsigned int len = 0;
        for (unsigned int i = 0; i < m_char_list.size (); i++)
            len += m_char_list[i].kana.length();

        if (type == CANDIDATE_LATIN)
            get_kana_substr (m_conv_string, m_start_segment_pos, len,
                             CANDIDATE_LATIN);
        else if (type == CANDIDATE_WIDE_LATIN)
            get_kana_substr (m_conv_string, m_start_segment_pos, len,
                             CANDIDATE_WIDE_LATIN);
        break;
    }
    case CANDIDATE_HIRAGANA:
        m_conv_string = get_preedit_string_as_hiragana ();
        break;

    case CANDIDATE_KATAKANA:
        convert_hiragana_to_katakana (get_preedit_string_as_hiragana (),
                                      m_conv_string);
        break;

    case CANDIDATE_HALF_KATAKANA:
        convert_hiragana_to_katakana (get_preedit_string_as_hiragana (),
                                      m_conv_string, true);
        break;

    default:
        // error
        return;
        break;
    }

    // set candidate type
    m_selected_candidates.push_back (type);

    // create attribute for this segment
    Attribute attr (0, m_conv_string.length (),
                    SCIM_ATTR_DECORATE);
    attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
    m_conv_attrs.push_back (attr);
}
#endif // FIXME!

void
AnthyPreedit::revert (void)
{
    m_conv_string.clear ();
    m_conv_attrs.clear ();
    m_selected_segment_id = -1;
    m_selected_segment_pos = 0;
    m_kana_converting = false;
}

void
AnthyPreedit::commit (int segment_id)
{
    if (m_kana_converting) return;
    if (m_selected_candidates.size () <= 0) return;

    for (unsigned int i = m_start_segment_id;
         i < m_selected_candidates.size () && (segment_id < 0 || (int) i <= segment_id);
         i++)
    {
        if (m_selected_candidates[i] >= 0)
            anthy_commit_segment (m_anthy_context, i, m_selected_candidates[i]);
    }


    if (segment_id >= 0 && segment_id + 1 < (int) m_selected_candidates.size ()) {
        // remove commited segments
        std::vector<int>::iterator it = m_selected_candidates.begin ();
        m_selected_candidates.erase(it, it + segment_id + 1);

        // adjust selected segment
        int new_start_segment_id = m_start_segment_id + segment_id + 1;
        m_selected_segment_id -= new_start_segment_id - m_start_segment_id;
        if (m_selected_segment_id < 0)
            m_selected_segment_id = 0;

        // adjust offset
        for (int i = m_start_segment_id; i < new_start_segment_id; i++) {
            struct anthy_segment_stat seg_stat;
            anthy_get_segment_stat (m_anthy_context, i, &seg_stat);
            m_start_segment_pos += seg_stat.seg_len;
        }
        m_start_segment_id = new_start_segment_id;

        // recreate conversion string
        create_conversion_string ();
    } else if (segment_id < 0 || segment_id >= (int) m_selected_candidates.size () - 1) {
        clear ();
    }
}

int
AnthyPreedit::get_nr_segments (void)
{
    if (!is_converting ()) return 0;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    return conv_stat.nr_segment - m_start_segment_id;
}

WideString
AnthyPreedit::get_segment_string (int segment_id)
{
    if (segment_id < 0)
        segment_id = m_selected_segment_id;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return WideString ();

    if (m_start_segment_id < 0 || m_start_segment_id >= conv_stat.nr_segment)
        return WideString (); // error

    if (segment_id < 0 || segment_id + m_start_segment_id >= conv_stat.nr_segment)
        return WideString (); //error

    // character position of the head of segment.
    unsigned int real_seg_start = 0;
    for (int i = 0; i < m_start_segment_id + segment_id; i++) {
        struct anthy_segment_stat seg_stat;
        anthy_get_segment_stat (m_anthy_context, i, &seg_stat);
        real_seg_start += seg_stat.seg_len;
    }

    int real_seg = segment_id + m_start_segment_id;
    int cand = m_selected_candidates[segment_id];

    // get information of this segment
    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_seg, &seg_stat);

    // get string of this segment
    WideString segment_str;
    if (cand < 0) {
        get_kana_substr (segment_str,
                         real_seg_start, real_seg_start + seg_stat.seg_len,
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
AnthyPreedit::get_selected_segment (void)
{
    return m_selected_segment_id;
}

void
AnthyPreedit::select_segment (int segment_id)
{
    if (!is_converting ()) return;

    if (segment_id < 0) return;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    int real_segment_id = segment_id + m_start_segment_id;

    if (segment_id >= 0 && real_segment_id < conv_stat.nr_segment) {
        m_selected_segment_id = segment_id;
        create_conversion_string ();
    }
}

int
AnthyPreedit::get_segment_size (int segment_id)
{
    if (!is_converting ()) return -1;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (segment_id < 0)
        segment_id = m_selected_segment_id;
    int real_segment_id = segment_id + m_start_segment_id;

    if (real_segment_id >= conv_stat.nr_segment)
        return -1;

    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_segment_id, &seg_stat);

    return seg_stat.seg_len;
}

void
AnthyPreedit::resize_segment (int relative_size, int segment_id)
{
    if (!is_converting ()) return;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    int real_segment_id;

    if (segment_id < 0) {
        segment_id = m_selected_segment_id;
        real_segment_id = segment_id + m_start_segment_id;
    } else {
        real_segment_id = segment_id + m_start_segment_id;
        if (m_selected_segment_id > segment_id)
            m_selected_segment_id = segment_id;
    }

    if (real_segment_id >= conv_stat.nr_segment)
        return;

    // do resize
    anthy_resize_segment (m_anthy_context, real_segment_id, relative_size);

    // reset candidate of trailing segments
    anthy_get_stat (m_anthy_context, &conv_stat);
    std::vector<int>::iterator start_iter = m_selected_candidates.begin();
    std::vector<int>::iterator end_iter   = m_selected_candidates.end();
    m_selected_candidates.erase (start_iter + segment_id, end_iter);
    for (int i = real_segment_id; i < conv_stat.nr_segment; i++)
        m_selected_candidates.push_back (0);

    // recreate conversion string
    create_conversion_string ();
}


/*
 * candidates for a segment
 */
void
AnthyPreedit::setup_lookup_table (CommonLookupTable &table, int segment_id)
{
    table.clear ();

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return;

    if (segment_id < 0)
        segment_id = m_selected_segment_id;
    int real_segment_id = segment_id + m_start_segment_id;

    if (real_segment_id >= conv_stat.nr_segment)
        return;

    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_segment_id, &seg_stat);

    for (int i = 0; i < seg_stat.nr_candidate; i++) {
        int len = anthy_get_segment (m_anthy_context, real_segment_id, i, NULL, 0);
        char *buf = (char *) malloc (len + 1);
        anthy_get_segment (m_anthy_context, real_segment_id, i, buf, len + 1);

        WideString cand_wide;
        m_iconv.convert (cand_wide, buf, len);

        table.append_candidate (cand_wide);

        free (buf);
    }
}

int
AnthyPreedit::get_selected_candidate (int segment_id)
{
    if (!is_converting ()) return -1;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return -1;

    if (segment_id < 0)
        segment_id = m_selected_segment_id;
    else if (segment_id >= conv_stat.nr_segment)
        return -1;

    return m_selected_candidates[segment_id];
}

void
AnthyPreedit::select_candidate (int candidate_id, int segment_id)
{
    if (!is_converting ()) return;

    if (candidate_id <= LAST_SPECIAL_CANDIDATE) return;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return;

    if (segment_id < 0)
        segment_id = m_selected_segment_id;
    int real_segment_id = segment_id + m_start_segment_id;

    if (segment_id >= conv_stat.nr_segment)
        return;

    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_segment_id, &seg_stat);

    if (candidate_id < seg_stat.nr_candidate) {
        m_selected_candidates[segment_id] = candidate_id;
        create_conversion_string ();
    }
}


/*
 * manipulating the caret
 */
unsigned int
AnthyPreedit::get_caret_pos (void)
{
    if (is_converting ())
        return m_selected_segment_pos;
    else
        return m_caret;
}

// CHECKME!
void
AnthyPreedit::set_caret_pos (unsigned int pos)
{
    if (is_converting ())
        return;

    if (pos == m_caret)
        return;

    m_key2kana.clear ();

    if (pos >= get_preedit_length ()) {
        m_caret = get_preedit_length ();
        m_char_caret = m_char_list.size ();
    } else if (pos == 0 ||  m_char_list.size () <= 0) {
        m_caret = 0;
        m_char_caret = 0;
    } else {
        unsigned int i, tmp_pos = 0;

        for (i = 0; tmp_pos <= pos; i++)
            tmp_pos += m_char_list[i].kana.length();

        if (tmp_pos == pos) {
            m_caret = pos;
            m_char_caret = i + 1;
        } else if (tmp_pos < m_caret) {
            m_char_caret = i;
            m_caret = tmp_pos - m_char_list[i].kana.length();
        } else if (tmp_pos > m_caret) {
            m_caret = tmp_pos;
            m_char_caret = i + 1;
        }
    }

    reset_pending ();
}

void
AnthyPreedit::move_caret (int step)
{
    if (is_converting ())
        return;

    if (step == 0)
        return;

    if (m_key2kana.is_pending ()) {
        m_key2kana.clear ();
    }

    if (step < 0 && m_char_caret < abs (step)) {
        m_char_caret = 0;
    } else if (step > 0 && m_char_caret + step > m_char_list.size ()) {
        m_char_caret = m_char_list.size ();
    } else {
        m_char_caret += step;
    }

    m_caret = 0;
    for (unsigned int i = 0; i < m_char_caret; i++)
        m_caret += m_char_list[i].kana.length();
    m_caret -= m_start_segment_pos;

    reset_pending ();
}

void
AnthyPreedit::reset_pending (void)
{
    if (m_key2kana.is_pending ())
        m_key2kana.clear ();

    // FIXME! should we revert from more previous characters?
    if (m_char_caret > 0 /*&& m_char_list[m_char_caret - 1].pending*/) {
        for (unsigned int i = 0;
             i < m_char_list[m_char_caret - 1].key.length ();
             i++)
        {
            WideString result, pending;
            m_key2kana.append (m_char_list[m_char_caret - 1].key.substr(i, 1),
                               result, pending);
        }
    }
}


/*
 * clear all string
 */
void
AnthyPreedit::clear (void)
{
    m_char_list.clear ();
    m_start_char = 0;
    m_char_caret = 0;
    m_conv_string.clear ();
    m_conv_attrs.clear ();
    anthy_reset_context (m_anthy_context);
    m_key2kana.clear();
    m_caret = 0;
    m_selected_candidates.clear ();
    m_start_segment_id = 0;
    m_start_segment_pos = 0;
    m_selected_segment_id = -1;
    m_selected_segment_pos = 0;
    m_kana_converting = false;
}


/*
 * preference
 */
void
AnthyPreedit::set_symbol_width (bool half)
{
    set_table (half, m_romaji_half_number,
               m_typing_method, m_period_style, m_comma_style, m_space_type);
}

bool
AnthyPreedit::symbol_is_half_width (void)
{
    return m_romaji_half_symbol;
}

void
AnthyPreedit::set_number_width (bool half)
{
    set_table (m_romaji_half_symbol, half,
               m_typing_method, m_period_style, m_comma_style, m_space_type);
}

bool
AnthyPreedit::number_is_half_width (void)
{
    return m_romaji_half_symbol;
}

void
AnthyPreedit::set_input_mode (InputMode mode)
{
    m_input_mode = mode;
}

InputMode
AnthyPreedit::get_input_mode (void)
{
    return m_input_mode;
}

void
AnthyPreedit::set_typing_method (TypingMethod method)
{
    set_table (m_romaji_half_symbol, m_romaji_half_number,
               method, m_period_style, m_comma_style, m_space_type);
}

TypingMethod
AnthyPreedit::get_typing_method (void)
{
    return m_typing_method;
}

void
AnthyPreedit::set_period_style (PeriodStyle style)
{
    set_table (m_romaji_half_symbol, m_romaji_half_number,
               m_typing_method, style, m_comma_style, m_space_type);
}

PeriodStyle
AnthyPreedit::get_period_style (void)
{
    return m_period_style;
}

void
AnthyPreedit::set_comma_style (CommaStyle style)
{
    set_table (m_romaji_half_symbol, m_romaji_half_number,
               m_typing_method, m_period_style, style, m_space_type);
}

CommaStyle
AnthyPreedit::get_comma_style (void)
{
    return m_comma_style;
}

void
AnthyPreedit::set_space_type (SpaceType type)
{
    set_table (m_romaji_half_symbol, m_romaji_half_number,
               m_typing_method, m_period_style, m_comma_style, type);
}

SpaceType
AnthyPreedit::get_space_type (void)
{
    return m_space_type;
}

void
AnthyPreedit::set_ten_key_type (TenKeyType type)
{
    m_ten_key_type = type;
}

TenKeyType
AnthyPreedit::get_ten_key_type (void)
{
    return m_ten_key_type;
}

void
AnthyPreedit::set_auto_convert (bool autoconv)
{
    m_auto_convert = autoconv;
}

bool
AnthyPreedit::get_auto_convert (void)
{
    return m_auto_convert;
}

void
AnthyPreedit::set_table (bool romaji_half_symbol,
                         bool romaji_half_number,
                         TypingMethod method,
                         PeriodStyle period,
                         CommaStyle comma,
                         SpaceType space)
{
    switch (method) {
    case METHOD_KANA:
        m_key2kana.set_table (scim_anthy_kana_typing_rule);
        break;
    case METHOD_ROMAKANA:
    default:
        m_key2kana.set_table (scim_anthy_romakana_typing_rule);

        if (romaji_half_symbol)
            m_key2kana.append_table (scim_anthy_romakana_symbol_rule);
        else
            m_key2kana.append_table (scim_anthy_romakana_wide_symbol_rule);

        if (romaji_half_number)
            m_key2kana.append_table (scim_anthy_romakana_number_rule);
        else
            m_key2kana.append_table (scim_anthy_romakana_wide_number_rule);

        break;
    };

    ConvRule *period_rule = get_period_rule (method, period);
    if (period_rule)
        m_key2kana.append_table (period_rule);

    ConvRule *comma_rule  = get_comma_rule  (method, comma);
    if (comma_rule)
        m_key2kana.append_table (comma_rule);

    switch (space) {
    case SPACE_NORMAL:
        m_key2kana.append_table (scim_anthy_space_rule);
        break;
    case SPACE_WIDE:
    default:
        m_key2kana.append_table (scim_anthy_wide_space_rule);
        break;
    };

    m_romaji_half_symbol = romaji_half_symbol;
    m_romaji_half_number = romaji_half_number;
    m_typing_method      = method;
    m_period_style       = period;
    m_comma_style        = comma;
    m_space_type         = space;
}

bool
AnthyPreedit::is_comma_or_period (const String & str)
{
    ConvRule *period_rule = get_period_rule (m_typing_method, m_period_style);
    ConvRule *comma_rule  = get_comma_rule  (m_typing_method, m_comma_style);

    for (unsigned int i = 0; period_rule && period_rule[i].string; i++) {
        if (period_rule[i].string && !strcmp (period_rule[i].string, str.c_str ()))
            return true;
    }
    for (unsigned int i = 0; comma_rule && comma_rule[i].string; i++) {
        if (comma_rule[i].string && !strcmp (comma_rule[i].string, str.c_str ()))
            return true;
    }

    return false;
}




#if 1 // FIXME! it's ad-hoc.  linear search kayo!
static void
convert_string_to_wide (const String & str, WideString & wide, SpaceType space)
{
    if (str.length () < 0)
        return;

    for (unsigned int i = 0; i < str.length (); i++) {
        int c = str[i];
        char cc[2]; cc[0] = c; cc[1] = '\0';
        bool found = false;

        for (unsigned int j = 0; scim_anthy_wide_table[j].code; j++) {
            if (scim_anthy_wide_table[j].code && *scim_anthy_wide_table[j].code == c) {
                wide += utf8_mbstowcs (scim_anthy_wide_table[j].wide);
                found = true;
                break;
            }
        }

        if (!found && space == SPACE_WIDE && c == ' ') {
            wide += utf8_mbstowcs ("\xE3\x80\x80");
            found = true;
        }

        if (!found)
            wide += utf8_mbstowcs (cc);
    }
}

static void
convert_hiragana_to_katakana (const WideString & hira, WideString & kata,
                              bool half)
{
    if (hira.length () < 0)
        return;

    for (unsigned int i = 0; i < hira.length (); i++) {
        WideString tmpwide;
        bool found = false;

        for (unsigned int j = 0; scim_anthy_hiragana_katakana_table[j].hiragana; j++) {
            tmpwide = utf8_mbstowcs (scim_anthy_hiragana_katakana_table[j].hiragana);
            if (hira.substr(i, 1) == tmpwide) {
                if (half)
                    kata += utf8_mbstowcs (scim_anthy_hiragana_katakana_table[j].half_katakana);
                else
                    kata += utf8_mbstowcs (scim_anthy_hiragana_katakana_table[j].katakana);
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
    case METHOD_KANA:
        switch (period) {
        case PERIOD_WIDE_LATIN:
            return scim_anthy_kana_wide_latin_period_rule;
        case PERIOD_LATIN:
            return scim_anthy_kana_latin_period_rule;
        case PERIOD_JAPANESE:
        default:
            return scim_anthy_kana_ja_period_rule;
        };
        break;
    case METHOD_ROMAKANA:
    default:
        switch (period) {
        case PERIOD_WIDE_LATIN:
            return scim_anthy_romakana_wide_latin_period_rule;
        case PERIOD_LATIN:
            return scim_anthy_romakana_latin_period_rule;
        case PERIOD_JAPANESE:
        default:
            return scim_anthy_romakana_ja_period_rule;
        };
        break;
    };

    return NULL;
}

static ConvRule *
get_comma_rule (TypingMethod method, CommaStyle period)
{
    switch (method) {
    case METHOD_KANA:
        switch (period) {
        case PERIOD_WIDE_LATIN:
            return scim_anthy_kana_wide_latin_comma_rule;
        case PERIOD_LATIN:
            return scim_anthy_kana_latin_comma_rule;
        case PERIOD_JAPANESE:
        default:
            return scim_anthy_kana_ja_comma_rule;
        };
        break;
    case METHOD_ROMAKANA:
    default:
        switch (period) {
        case PERIOD_WIDE_LATIN:
            return scim_anthy_romakana_wide_latin_comma_rule;
        case PERIOD_LATIN:
            return scim_anthy_romakana_latin_comma_rule;
        case PERIOD_JAPANESE:
        default:
            return scim_anthy_romakana_ja_comma_rule;
        };
        break;
    };

    return NULL;
}
#endif // FIXME!
/*
vi:ts=4:nowrap:ai:expandtab
*/
