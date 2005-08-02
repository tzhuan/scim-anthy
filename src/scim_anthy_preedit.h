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

class AnthyInstance;

namespace scim_anthy {

typedef enum {
    SCIM_ANTHY_MODE_HIRAGANA,
    SCIM_ANTHY_MODE_KATAKANA,
    SCIM_ANTHY_MODE_HALF_KATAKANA,
    SCIM_ANTHY_MODE_LATIN,
    SCIM_ANTHY_MODE_WIDE_LATIN,
} InputMode;

typedef enum {
    SCIM_ANTHY_NONE_ON_PERIOD,
    SCIM_ANTHY_CONVERT_ON_PERIOD,
    SCIM_ANTHY_COMMIT_ON_PERIOD,
} BehaviorOnPeriod;

class Preedit
{
public:
    Preedit (AnthyInstance &anthy, Key2KanaTableSet & tables);
    virtual ~Preedit ();

    // getting status
    virtual unsigned int  get_length             (void);
    virtual WideString    get_string             (void);
    virtual AttributeList get_attribute_list     (void);
    virtual Reading      &get_reading            (void);

    virtual bool          is_preediting          (void);
    virtual bool          is_converting          (void);

    // manipulating the preedit string
    virtual bool          can_process_key_event  (const KeyEvent & key);
    // return true if commiting is needed.
    virtual bool          process_key_event      (const KeyEvent & key);
    virtual void          erase                  (bool backward = true);
    virtual void          finish                 (void);

    // manipulating the conversion string
    virtual void          convert                (CandidateType type
                                                  = SCIM_ANTHY_CANDIDATE_NORMAL,
                                                  bool single_segment = false);
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
    virtual void          set_typing_method      (TypingMethod method,
                                                  Key2KanaTable *fund_table
                                                  = NULL);
    virtual TypingMethod  get_typing_method      (void);

#if 1
    // FIMXE! Read from config directly
    virtual void          set_ten_key_type       (TenKeyType type);
    virtual TenKeyType    get_ten_key_type       (void);
    virtual void          set_behavior_on_period (BehaviorOnPeriod behavior);
    virtual BehaviorOnPeriod
                          get_behavior_on_period (void);
    virtual void          set_allow_split_romaji (bool allow);
    virtual bool          get_allow_split_romaji (void);
#endif

private:
    void                  get_reading_substr     (WideString & substr,
                                                  unsigned int start,
                                                  unsigned int len,
                                                  CandidateType type);
    bool                  is_comma_or_period     (const String & str);

private:
    AnthyInstance    &m_anthy;

    // converter objects
    Key2KanaTableSet &m_key2kana_tables;
    Reading           m_reading;
    Conversion        m_conversion;

    // mode flags
    InputMode         m_input_mode;
    BehaviorOnPeriod  m_behavior_on_period;
    bool              m_romaji_allow_split;

    unsigned int      m_preedit_fg_color;
    unsigned int      m_preedit_bg_color;
};

}

#endif /* __SCIM_ANTHY_PREEDIT_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
