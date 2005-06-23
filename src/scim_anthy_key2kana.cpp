/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
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

#include "scim_anthy_key2kana.h"

using namespace scim_anthy;

Key2KanaConvertor::Key2KanaConvertor (Key2KanaTableSet & tables)
    : m_tables         (tables),
      m_case_sensitive (false)
{
}

Key2KanaConvertor::~Key2KanaConvertor ()
{
}

bool
Key2KanaConvertor::append (const String & str,
                           WideString & result, WideString & pending)
{
    WideString widestr = utf8_mbstowcs (str);
    WideString matching_str = m_pending + widestr;
    Key2KanaRule exact_match;
    bool has_partial_match = false;
    bool retval = false;

    if (!m_case_sensitive) {
        String half = utf8_wcstombs (matching_str);
        for (unsigned int i = 0; i < half.length (); i++)
            half[i] = tolower (half[i]);
        matching_str = utf8_mbstowcs (half);
    }

    /* find matched table */
    std::vector<Key2KanaTable*> &tables = m_tables.get_tables();
    for (unsigned int j = 0; j < tables.size(); j++) {
        if (!tables[j])
            continue;

        Key2KanaRules &rules = tables[j]->get_table ();

        for (unsigned int i = 0; i < rules.size(); i++) {
            /* matching */
            String seq = rules[i].get_sequence ();
            if (!m_case_sensitive) {
                for (unsigned int i = 0; i < seq.length (); i++)
                    seq[i] = tolower (seq[i]);
            }
            WideString romaji = utf8_mbstowcs(seq);
            if (romaji.find (matching_str) == 0) {
                if (romaji.length () == matching_str.length ()) {
                    /* exact match */
                    exact_match = rules[i];
                } else {
                    /* partial match */
                    has_partial_match = true;
                }
            }
        }
    }

    /* return result */
    if (has_partial_match) {
        m_exact_match = exact_match;
        result.clear ();
        m_pending += widestr;
        pending   =  m_pending;

    } else if (!exact_match.is_empty()) {
        if (!exact_match.get_continue_string().empty())
            m_exact_match = exact_match;
        else
            m_exact_match.clear ();
        m_pending = utf8_mbstowcs (exact_match.get_continue_string ());
        result    = utf8_mbstowcs (exact_match.get_result ());
        pending   = m_pending;

    } else {
        if (!m_exact_match.is_empty()) {
            if (!m_exact_match.get_result().empty() &&
                m_exact_match.get_continue_string().empty())
            {
                result = utf8_mbstowcs (m_exact_match.get_result());
            } else {
                retval = true; /* commit prev pending */
            }
            m_pending.clear ();
            m_exact_match.clear ();

            WideString tmp_result;
            append(str, tmp_result, pending);
            result += tmp_result;

        } else {
            if (m_pending.length () > 0) {
                retval     = true; /* commit prev pending */
                m_pending  = widestr;
                pending    = m_pending;

            } else {
                result     = widestr;
                pending.clear();
                m_pending.clear ();
            }
        }
    }

    return retval;
}

void
Key2KanaConvertor::clear (void)
{
    m_pending.clear ();
    m_exact_match.clear ();
}

bool
Key2KanaConvertor::is_pending (void)
{
    if (m_pending.length () > 0)
        return true;
    else
        return false;
}

WideString
Key2KanaConvertor::get_pending (void)
{
    return m_pending;
}

WideString
Key2KanaConvertor::flush_pending (void)
{
    WideString result;
    if (!m_exact_match.is_empty ()) {
        if (!m_exact_match.get_result().empty() &&
            m_exact_match.get_continue_string().empty())
        {
            result = utf8_mbstowcs (m_exact_match.get_result());
        } else if (!m_exact_match.get_continue_string().empty()) {
            result += utf8_mbstowcs (m_exact_match.get_continue_string());
        } else if (m_pending.length () > 0) {
            result += m_pending;
        }
    }
    clear ();
    return result;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
