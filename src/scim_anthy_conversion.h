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

#ifndef __SCIM_ANTHY_CONVERSION_H__
#define __SCIM_ANTHY_CONVERSION_H__

#define Uses_SCIM_ICONV
#define Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_LOOKUP_TABLE
#include <scim.h>
#include <anthy/anthy.h>

#include "scim_anthy_reading.h"

using namespace scim;

namespace scim_anthy {

typedef enum {
    SCIM_ANTHY_CANDIDATE_NORMAL        = 0,
    SCIM_ANTHY_CANDIDATE_LATIN         = -1,
    SCIM_ANTHY_CANDIDATE_WIDE_LATIN    = -2,
    SCIM_ANTHY_CANDIDATE_HIRAGANA      = -3,
    SCIM_ANTHY_CANDIDATE_KATAKANA      = -4,
    SCIM_ANTHY_CANDIDATE_HALF_KATAKANA = -5,
    SCIM_ANTHY_LAST_SPECIAL_CANDIDATE  = -6,
} CandidateType;

class ConversionSegment
{
public:
    ConversionSegment ();
    virtual ~ConversionSegment ();

private:
};

class Conversion
{
public:
    Conversion (Reading &reading);
    virtual ~Conversion ();

    void          start                  (CandidateType type);
    void          clear                  (void);
    void          commit                 (int  segment_id = -1,
                                          bool lean       = true);

    bool          is_converting          (void);
    bool          is_kana_converting     (void);

    WideString    get                    (void);
    unsigned int  get_length             (void);
    AttributeList get_attribute_list     (void);

    // segments of the converted sentence
    int           get_nr_segments        (void);
    WideString    get_segment_string     (int segment_id = -1);
    int           get_selected_segment   (void);
    void          select_segment         (int segment_id);
    int           get_segment_size       (int segment_id = -1);
    void          resize_segment         (int relative_size,
                                          int segment_id = -1);
    unsigned int  get_segment_position   (int segment_id = -1);

    // candidates for a segment
    void          get_candidates         (CommonLookupTable &table,
                                          int segment_id = -1);
    int           get_selected_candidate (int segment_id = -1);
    void          select_candidate       (int candidate_id,
                                          int segment_id = -1);
private:
    void          create_string          (void);
    void          convert_kana           (CandidateType type);

private:
    // convertors
    IConvert          m_iconv;
    Reading          &m_reading;
    anthy_context_t   m_anthy_context;

    // status variables
    WideString        m_string;
    AttributeList     m_attrs;
    std::vector<int>  m_candidates;
    int               m_start_id;    // number of commited segments
    int               m_cur_segment; // relative position from m_start_id
    int               m_cur_pos;     // caret position
    bool              m_kana_converting;
};

}

#endif /* __SCIM_ANTHY_READING_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
