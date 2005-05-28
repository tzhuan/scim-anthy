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

#ifndef __SCIM_ANTHY_PREEDIT_H__
#define __SCIM_ANTHY_PREEDIT_H__

#define Uses_SCIM_ICONV
#define Uses_SCIM_EVENT
#define Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_LOOKUP_TABLE
#include <anthy/anthy.h>
#include <scim.h>
#include "scim_anthy_reading.h"
#include "scim_anthy_conversion.h"

using namespace scim;

namespace scim_anthy {

typedef enum {
    SCIM_ANTHY_MODE_HIRAGANA,
    SCIM_ANTHY_MODE_KATAKANA,
    SCIM_ANTHY_MODE_HALF_KATAKANA,
    SCIM_ANTHY_MODE_LATIN,
    SCIM_ANTHY_MODE_WIDE_LATIN,
} InputMode;

#if 0
typedef enum {
    SCIM_ANTHY_CANDIDATE_NORMAL        = 0,
    SCIM_ANTHY_CANDIDATE_LATIN         = -1,
    SCIM_ANTHY_CANDIDATE_WIDE_LATIN    = -2,
    SCIM_ANTHY_CANDIDATE_HIRAGANA      = -3,
    SCIM_ANTHY_CANDIDATE_KATAKANA      = -4,
    SCIM_ANTHY_CANDIDATE_HALF_KATAKANA = -5,
    SCIM_ANTHY_LAST_SPECIAL_CANDIDATE  = -6,
} CandidateType;
#endif

class Preedit
{
public:
    Preedit (Key2KanaTableSet & tables);
    virtual ~Preedit ();

    // getting status
    virtual unsigned int  get_length             (void);
    virtual WideString    get_string             (void);
    virtual AttributeList get_attribute_list     (void);

    virtual bool          is_preediting          (void);
    virtual bool          is_converting          (void);
    virtual bool          is_kana_converting     (void);

    // manipulating the preedit string
    virtual bool          can_process_key_event  (const KeyEvent & key);
    // return true if commiting is needed.
    virtual bool          process_key_event      (const KeyEvent & key);
    virtual void          erase                  (bool backward = true);
    virtual void          flush_pending          (void);

    // manipulating the conversion string
    virtual void          convert                (CandidateType type
                                                  = SCIM_ANTHY_CANDIDATE_NORMAL);
    virtual void          revert                 (void);
    virtual void          commit                 (int  segment_id = -1,
                                                  bool lean       = true);

    // segments of the converted sentence
    virtual int           get_nr_segments        (void);
    virtual WideString    get_segment_string     (int segment_id = -1);
    virtual int           get_selected_segment   (void);
    virtual void          select_segment         (int segment_id);
    virtual int           get_segment_size       (int segment_id = -1);
    virtual void          resize_segment         (int relative_size,
                                                  int segment_id = -1);

    // candidates for a segment
    virtual void          get_candidates         (CommonLookupTable &table,
                                                  int segment_id = -1);
    virtual int           get_selected_candidate (int segment_id = -1);
    virtual void          select_candidate       (int candidate_id,
                                                  int segment_id = -1);

    // manipulating the caret
    virtual unsigned int  get_caret_pos          (void);
    virtual void          set_caret_pos          (unsigned int pos);
    virtual void          move_caret             (int len);

    // clear all string
    virtual void          clear                  (void);

    // preference
    virtual void          set_input_mode         (InputMode mode);
    virtual InputMode     get_input_mode         (void);
    virtual void          set_ten_key_type       (TenKeyType type);
    virtual TenKeyType    get_ten_key_type       (void);
    virtual void          set_auto_convert       (bool autoconv);
    virtual bool          get_auto_convert       (void);
    virtual void          set_allow_split_romaji (bool allow);
    virtual bool          get_allow_split_romaji (void);

private:
    void          get_reading_substr             (WideString & substr,
                                                  unsigned int start,
                                                  unsigned int len,
                                                  CandidateType type);
    bool          is_comma_or_period             (const String & str);

private:
    // converter objects
    Key2KanaTableSet &m_key2kana_tables;
    Reading           m_reading;
    Conversion        m_conversion;

    // mode flags
    InputMode         m_input_mode;
    bool              m_auto_convert;
    bool              m_romaji_allow_split;
};

}

#endif /* __SCIM_ANTHY_PREEDIT_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
