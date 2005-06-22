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

#include "scim_anthy_conversion.h"
#include "scim_anthy_utils.h"

using namespace scim_anthy;

ConversionSegment::ConversionSegment ()
{
}

ConversionSegment::~ConversionSegment ()
{
}



Conversion::Conversion (Reading &reading)
    : m_reading          (reading),
      m_anthy_context    (anthy_create_context()),
      m_start_id         (0),
      m_cur_segment      (-1),
      m_cur_pos          (0),
      m_kana_converting  (false)
{
#ifdef HAS_ANTHY_CONTEXT_SET_ENCODING
    anthy_context_set_encoding (m_anthy_context, ANTHY_EUC_JP_ENCODING);
#endif /* HAS_ANTHY_CONTEXT_SET_ENCODING */

    if (!m_iconv.set_encoding ("EUC-JP")) {
        // error
    }
}

Conversion::~Conversion ()
{
    anthy_release_context (m_anthy_context);
}

void
Conversion::start (CandidateType ctype)
{
    if (ctype < SCIM_ANTHY_CANDIDATE_NORMAL) {
        convert_kana (ctype);
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
        anthy_get_stat (m_anthy_context, &conv_stat);
        if (conv_stat.nr_segment <= 0)
            return;

        // select first segment
        m_cur_segment = 0;
        m_cur_pos = 0;

        // select first candidates for all segment
        m_candidates.clear ();
        for (int i = m_start_id; i < conv_stat.nr_segment; i++)
            m_candidates.push_back (0);

        // create whole string
        create_string ();
    }
}

void
Conversion::convert_kana (CandidateType ctype)
{
    String str;
    WideString wide;

    m_string.clear ();
    m_attrs.clear ();
    m_cur_segment = 0;
    m_cur_pos = 0;
    m_kana_converting = true;

    switch (ctype) {
    case SCIM_ANTHY_CANDIDATE_LATIN:
        m_string = utf8_mbstowcs (m_reading.get_raw ());
        break;

    case SCIM_ANTHY_CANDIDATE_WIDE_LATIN:
        convert_to_wide (m_string, m_reading.get_raw ());
        break;

    case SCIM_ANTHY_CANDIDATE_HIRAGANA:
        m_string = m_reading.get ();
        break;

    case SCIM_ANTHY_CANDIDATE_KATAKANA:
        convert_to_katakana (m_string, m_reading.get ());
        break;

    case SCIM_ANTHY_CANDIDATE_HALF_KATAKANA:
        convert_to_katakana (m_string, m_reading.get (), true);
        break;

    case SCIM_ANTHY_CANDIDATE_HALF:
        if (m_candidates.empty ()) {
            convert_to_katakana (m_string, m_reading.get (), true);
            ctype = SCIM_ANTHY_CANDIDATE_HALF_KATAKANA;
        } else {
            switch (m_candidates[0]) {
            case SCIM_ANTHY_CANDIDATE_LATIN:
            case SCIM_ANTHY_CANDIDATE_WIDE_LATIN:
                m_string = utf8_mbstowcs (m_reading.get_raw ());
                ctype = SCIM_ANTHY_CANDIDATE_LATIN;
                break;
            default:
                convert_to_katakana (m_string, m_reading.get (), true);
                ctype = SCIM_ANTHY_CANDIDATE_HALF_KATAKANA;
                break;
            }
        }
        break;

    default:
        // error
        return;
        break;
    }

    // set candidate type
    m_candidates.clear ();
    m_candidates.push_back (ctype);

    // create attribute for this segment
    Attribute attr (0, m_string.length (),
                    SCIM_ATTR_DECORATE);
    attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
    m_attrs.push_back (attr);
}

void
Conversion::clear (void)
{
    anthy_reset_context (m_anthy_context);

    m_string.clear ();
    m_attrs.clear ();
    m_candidates.clear ();
    
    m_start_id        = 0;
    m_cur_segment     = -1;
    m_cur_pos         = 0;
    m_kana_converting = false;
}

void
Conversion::commit (int segment_id, bool learn)
{
    if (m_kana_converting) return;
    if (m_candidates.size () <= 0) return;

    // learn
    for (unsigned int i = m_start_id;
         learn &&
             i < m_candidates.size () &&
             (segment_id < 0 || (int) i <= segment_id);
         i++)
    {
        if (m_candidates[i] >= 0)
            anthy_commit_segment (m_anthy_context, i, m_candidates[i]);
    }


    if (segment_id >= 0 &&
        segment_id + 1 < (int) m_candidates.size ())
    {
        // partial commit

        // remove commited segments
        std::vector<int>::iterator it = m_candidates.begin ();
        m_candidates.erase(it, it + segment_id + 1);

        // adjust selected segment
        int new_start_segment_id = m_start_id + segment_id + 1;
        m_cur_segment -= new_start_segment_id - m_start_id;
        if (m_cur_segment < 0)
            m_cur_segment = 0;

        // adjust offset
        unsigned int commited_len = 0;
        for (int i = m_start_id; i < new_start_segment_id; i++) {
            struct anthy_segment_stat seg_stat;
            anthy_get_segment_stat (m_anthy_context, i, &seg_stat);
            commited_len += seg_stat.seg_len;
        }
        m_reading.erase (0, commited_len, true);
        m_start_id = new_start_segment_id;

        // recreate conversion string
        create_string ();

    } else {
        // commit all
        clear ();
    }
}

void
Conversion::create_string (void)
{
    m_string.clear ();
    m_attrs.clear ();

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return;
    if (m_start_id < 0 || m_start_id >= conv_stat.nr_segment)
        return; /* error */

    for (int i = m_start_id; i < conv_stat.nr_segment; i++) {
        int seg = i - m_start_id;

        // get string of this segment
        WideString segment_str = get_segment_string (seg);

        // set caret
        if (seg == m_cur_segment)
            m_cur_pos = m_string.length ();

        // create attribute for this segment
        Attribute attr (m_string.length (), segment_str.length (),
                        SCIM_ATTR_DECORATE);
        if (seg == m_cur_segment)
            attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
        else
            attr.set_value (SCIM_ATTR_DECORATE_UNDERLINE);

        // join the string to the whole conversion string
        if (segment_str.length () > 0) {
            m_string += segment_str;
            m_attrs.push_back (attr);
        }
    }
}

WideString
Conversion::get_segment_string (int segment_id)
{
    if (segment_id < 0)
        segment_id = m_cur_segment;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return WideString ();

    if (m_start_id < 0 ||
        m_start_id >= conv_stat.nr_segment)
    {
        return WideString (); // error
    }

    if (segment_id < 0 ||
        segment_id + m_start_id >= conv_stat.nr_segment)
    {
        return WideString (); //error
    }

    // character position of the head of segment.
    unsigned int real_seg_start = 0;
    for (int i = 0; i < m_start_id + segment_id; i++) {
        struct anthy_segment_stat seg_stat;
        anthy_get_segment_stat (m_anthy_context, i, &seg_stat);
        real_seg_start += seg_stat.seg_len;
    }

    int real_seg = segment_id + m_start_id;
    int cand = m_candidates[segment_id];

    // get information of this segment
    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_seg, &seg_stat);

    // get string of this segment
    WideString segment_str;
    if (cand < 0) {
        StringType type;
        switch ((CandidateType) cand) {
        case SCIM_ANTHY_CANDIDATE_LATIN:
            type = SCIM_ANTHY_STRING_LATIN;
            break;
        case SCIM_ANTHY_CANDIDATE_WIDE_LATIN:
            type = SCIM_ANTHY_STRING_WIDE_LATIN;
            break;
        case SCIM_ANTHY_CANDIDATE_KATAKANA:
            type = SCIM_ANTHY_STRING_KATAKANA;
            break;
        case SCIM_ANTHY_CANDIDATE_HALF_KATAKANA:
            type = SCIM_ANTHY_STRING_HALF_KATAKANA;
            break;
        case SCIM_ANTHY_CANDIDATE_HALF:
            // shouldn't reach to this entry
            type = SCIM_ANTHY_STRING_HALF_KATAKANA;
            break;
        case SCIM_ANTHY_CANDIDATE_HIRAGANA:
        default:
            type = SCIM_ANTHY_STRING_HIRAGANA;
            break;
        }
        m_reading.get (segment_str,
                       real_seg_start, seg_stat.seg_len,
                       type);
    } else {
        int len = anthy_get_segment (m_anthy_context, real_seg, cand, NULL, 0);
        char buf[len + 1];
        anthy_get_segment (m_anthy_context, real_seg, cand, buf, len + 1);
        buf[len] = '\0';
        m_iconv.convert (segment_str, buf, len);
    }

    return segment_str;
}

bool
Conversion::is_converting (void)
{
    if (m_string.length () > 0)
        return true;
    else
        return false;
}

bool
Conversion::is_kana_converting (void)
{
    return m_kana_converting;
}

WideString
Conversion::get (void)
{
    return m_string;
}

unsigned int
Conversion::get_length (void)
{
    return m_string.length ();
}

AttributeList
Conversion::get_attribute_list (void)
{
    return m_attrs;
}

int
Conversion::get_nr_segments (void)
{
    if (!is_converting ()) return 0;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    return conv_stat.nr_segment - m_start_id;
}

int
Conversion::get_selected_segment (void)
{
    return m_cur_segment;
}

void
Conversion::select_segment (int segment_id)
{
    if (!is_converting ()) return;

    if (segment_id < 0) return;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    int real_segment_id = segment_id + m_start_id;

    if (segment_id >= 0 && real_segment_id < conv_stat.nr_segment) {
        m_cur_segment = segment_id;
        create_string ();
    }
}

int
Conversion::get_segment_size (int segment_id)
{
    if (!is_converting ()) return -1;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (segment_id < 0)
        segment_id = m_cur_segment;
    int real_segment_id = segment_id + m_start_id;

    if (real_segment_id >= conv_stat.nr_segment)
        return -1;

    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_segment_id, &seg_stat);

    return seg_stat.seg_len;
}

void
Conversion::resize_segment (int relative_size, int segment_id)
{
    if (!is_converting ()) return;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    int real_segment_id;

    if (segment_id < 0) {
        segment_id = m_cur_segment;
        real_segment_id = segment_id + m_start_id;
    } else {
        real_segment_id = segment_id + m_start_id;
        if (m_cur_segment > segment_id)
            m_cur_segment = segment_id;
    }

    if (real_segment_id >= conv_stat.nr_segment)
        return;

    // do resize
    anthy_resize_segment (m_anthy_context, real_segment_id, relative_size);

    // reset candidates of trailing segments
    anthy_get_stat (m_anthy_context, &conv_stat);
    std::vector<int>::iterator start_iter = m_candidates.begin();
    std::vector<int>::iterator end_iter   = m_candidates.end();
    m_candidates.erase (start_iter + segment_id, end_iter);
    for (int i = real_segment_id; i < conv_stat.nr_segment; i++)
        m_candidates.push_back (0);

    // recreate conversion string
    create_string ();
}

unsigned int
Conversion::get_segment_position (int segment_id)
{
    //FIXME!!!
    if (segment_id < 0)
        return m_cur_pos;
    return 0;
}



/*
 * candidates for a segment
 */
void
Conversion::get_candidates (CommonLookupTable &table, int segment_id)
{
    table.clear ();

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return;

    if (segment_id < 0)
        segment_id = m_cur_segment;
    int real_segment_id = segment_id + m_start_id;

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
Conversion::get_selected_candidate (int segment_id)
{
    if (!is_converting ()) return -1;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return -1;

    if (segment_id < 0)
        segment_id = m_cur_segment;
    else if (segment_id >= conv_stat.nr_segment)
        return -1;

    return m_candidates[segment_id];
}

void
Conversion::select_candidate (int candidate_id, int segment_id)
{
    if (!is_converting ()) return;

    if (candidate_id <= SCIM_ANTHY_LAST_SPECIAL_CANDIDATE) return;

    struct anthy_conv_stat conv_stat;
    anthy_get_stat (m_anthy_context, &conv_stat);

    if (conv_stat.nr_segment <= 0)
        return;

    if (segment_id < 0)
        segment_id = m_cur_segment;
    int real_segment_id = segment_id + m_start_id;

    if (segment_id >= conv_stat.nr_segment)
        return;

    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat (m_anthy_context, real_segment_id, &seg_stat);

    if (candidate_id == SCIM_ANTHY_CANDIDATE_HALF) {
        switch (m_candidates[segment_id]) {
        case SCIM_ANTHY_CANDIDATE_LATIN:
        case SCIM_ANTHY_CANDIDATE_WIDE_LATIN:
            candidate_id = SCIM_ANTHY_CANDIDATE_LATIN;
            break;
        default:
            candidate_id = SCIM_ANTHY_CANDIDATE_HALF_KATAKANA;
            break;
        }
    }

    if (candidate_id < seg_stat.nr_candidate) {
        m_candidates[segment_id] = candidate_id;
        create_string ();
    }
}
