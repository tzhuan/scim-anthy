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

#ifndef __SCIM_ANTHY_KEY2KANA_TABLE_H__
#define __SCIM_ANTHY_KEY2KANA_TABLE_H__

#define Uses_SCIM_ICONV
#include <scim.h>
#include "scim_anthy_key2kana_table_default.h"

using namespace scim;


typedef enum {
    SCIM_ANTHY_PERIOD_JAPANESE,
    SCIM_ANTHY_PERIOD_WIDE,
    SCIM_ANTHY_PERIOD_HALF,
} SCIMAnthyPeriodStyle;

typedef enum {
    SCIM_ANTHY_COMMA_JAPANESE,
    SCIM_ANTHY_COMMA_WIDE,
    SCIM_ANTHY_COMMA_HALF,
} SCIMAnthyCommaStyle;

typedef enum {
    SCIM_ANTHY_TYPING_METHOD_ROMAJI,
    SCIM_ANTHY_TYPING_METHOD_KANA,
    SCIM_ANTHY_TYPING_METHOD_CUSTOM,
} SCIMAnthyTypingMethod;


class AnthyKey2KanaTable
{
public:
    AnthyKey2KanaTable (WideString name, ConvRule *table);
    virtual ~AnthyKey2KanaTable ();

    ConvRule * get_table (void) { return m_table; }

private:
    WideString  m_name;
    ConvRule   *m_table;
};


class AnthyKey2KanaTableSet
{
public:
    AnthyKey2KanaTableSet ();
    virtual ~AnthyKey2KanaTableSet ();

    std::vector<AnthyKey2KanaTable*> &
         get_tables (void) { return m_all_tables; };

    void set_typing_method       (SCIMAnthyTypingMethod method);
    void set_symbol_width        (bool half);
    void set_number_width        (bool half);
    void set_period_style        (SCIMAnthyPeriodStyle style);
    void set_comma_style         (SCIMAnthyCommaStyle  style);

    SCIMAnthyTypingMethod
         get_typing_method       (void) { return m_typing_method; }
    bool symbol_is_half          (void) { return m_use_half_symbol; }
    bool number_is_half          (void) { return m_use_half_number;}
    SCIMAnthyPeriodStyle
         get_period_style        (void) { return m_period_style; }
    SCIMAnthyCommaStyle
         get_comma_style         (void) { return m_comma_style; }

#if 0
    // void set_use_consonant_table (bool use);
    void set_use_symbol_table    (bool use);
    void set_use_number_table    (bool use);
    void set_use_period_table    (bool use);
    void set_use_comma_table     (bool use);

    bool get_use_consonant_table (void);
    bool get_use_symbol_table    (void);
    bool get_use_number_table    (void);
    bool get_use_period_table    (void);
    bool get_use_comma_table     (void);
#endif

private:
    void reset_tables            (void);

private:
    WideString m_name;

    // tables
    std::vector<AnthyKey2KanaTable>  m_tables;
    std::vector<AnthyKey2KanaTable*> m_all_tables;

    // flags
    SCIMAnthyTypingMethod m_typing_method;
    SCIMAnthyPeriodStyle  m_period_style;
    SCIMAnthyCommaStyle   m_comma_style;
    bool                  m_use_half_symbol;
    bool                  m_use_half_number;
};

#endif /* __SCIM_ANTHY_KEY2KANA_TABLE_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
