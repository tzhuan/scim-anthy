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

#ifndef __SCIM_ANTHY_READING_H__
#define __SCIM_ANTHY_READING_H__

#define Uses_SCIM_ICONV
#define Uses_SCIM_EVENT
#include <scim.h>
#include "scim_anthy_key2kana.h"

using namespace scim;

namespace scim_anthy {

typedef std::vector<KeyEvent> KeyEvents;

typedef enum {
    SCIM_ANTHY_STRING_LATIN,
    SCIM_ANTHY_STRING_WIDE_LATIN,
    SCIM_ANTHY_STRING_HIRAGANA,
    SCIM_ANTHY_STRING_KATAKANA,
    SCIM_ANTHY_STRING_HALF_KATAKANA,
} StringType;

typedef enum {
    SCIM_ANTHY_TEN_KEY_HALF,
    SCIM_ANTHY_TEN_KEY_WIDE,
    SCIM_ANTHY_TEN_KEY_FOLLOW_MODE,
} TenKeyType;

class Reading;
class ReadingSegment;
typedef std::vector<ReadingSegment> ReadingSegments;

class ReadingSegment
{
    friend class Reading;

public:
    ReadingSegment (void);
    virtual ~ReadingSegment ();

    const WideString & get     (void) { return kana; }
    const String     & get_raw (void) { return raw; }

    void split (ReadingSegments &segments);

private:
//    KeyEvents  keys;
    String     raw;
    WideString kana;

};

class Reading
{
public:
    Reading (Key2KanaTableSet & tables);
    virtual ~Reading ();

    bool         can_process_key_event (const KeyEvent & key);
    bool         process_key_event     (const KeyEvent & key);
    void         commit                (void);
    void         clear                 (void);

    WideString   get                   (unsigned int start  = 0,
                                        int          length = -1,
                                        StringType   type
                                        = SCIM_ANTHY_STRING_HIRAGANA);
    void         get                   (WideString & string,
                                        unsigned int start  = 0,
                                        int          length = -1,
                                        StringType   type
                                        = SCIM_ANTHY_STRING_HIRAGANA);
    String       get_raw               (unsigned int start  = 0,
                                        int          length = -1);
    void         get_raw               (String     & string,
                                        unsigned int start  = 0,
                                        int          length = -1);
#if 0
    void         get_key_events        (KeyEvents  & events,
                                        unsigned int start  = 0,
                                        int          length = -1);
#endif
    void         erase                 (unsigned int start  = 0,
                                        int          length = -1,
                                        bool         allow_split = false);

    unsigned int get_length            (void);
    unsigned int get_caret_pos         (void);
    void         set_caret_pos         (unsigned int pos);
    void         move_caret            (int          step,
                                        bool         allow_split = false);

    void         set_ten_key_type      (TenKeyType type);
    TenKeyType   get_ten_key_type      (void);

private:
    bool         append_str            (const String & str);
    void         reset_pending         (void);
    void         split_segment         (unsigned int seg_id);

private:
    // convertors
    Key2KanaTableSet  &m_key2kana_tables;
    Key2KanaConvertor  m_key2kana;

    // state
    ReadingSegments    m_segments;
    unsigned int       m_segment_pos;
    unsigned int       m_caret_offset;

    // mode
    TenKeyType         m_ten_key_type;
};

}

#endif /* __SCIM_ANTHY_READING_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
