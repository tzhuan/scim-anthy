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

#include "scim_anthy_style_file.h"

const int MAX_LINE_LENGTH = 4096;

AnthyStyleLine::AnthyStyleLine (const char *line)
    : m_line (line),
      m_type (ANTHY_STYLE_LINE_UNKNOWN)
{
}

AnthyStyleLine::~AnthyStyleLine ()
{
}

AnthyStyleLineType
AnthyStyleLine::get_type (void)
{
    if (m_type != ANTHY_STYLE_LINE_UNKNOWN)
        return m_type;

    unsigned int spos, epos;
    for (spos = 0; spos < m_line.length () && isspace (m_line[spos]); spos++);
    if (m_line.length() > 0)
        for (epos = m_line.length () - 1; epos >= 0 && isspace (m_line[epos]); epos--);
    else
        epos = 0;

    if (m_line.length() == 0 || spos >= m_line.length()) {
        m_type = ANTHY_STYLE_LINE_SPACE;
        return m_type;

    } else if (m_line[spos] == '#' || m_line [spos] == ';') {
        m_type = ANTHY_STYLE_LINE_COMMENT;
        return m_type;

    } else if (m_line[spos] == '[' && m_line[epos] == ']') {
        m_type = ANTHY_STYLE_LINE_SECTION;
        return m_type;
    }

    m_type = ANTHY_STYLE_LINE_KEY;
    return m_type;
}

bool
AnthyStyleLine::get_section (String &section)
{
    if (m_type != ANTHY_STYLE_LINE_SECTION)
        return false;

    unsigned int spos, epos;
    for (spos = 0; spos < m_line.length () && isspace (m_line[spos]); spos++);
    for (epos = m_line.length () - 1; epos >= 0 && isspace (m_line[epos]); epos--);
    spos++;

    if (spos < epos)
        section = m_line.substr (spos, epos - spos);
    else
        section = String ();

    return true;
}

bool
AnthyStyleLine::get_key (String &key)
{
    if (m_type != ANTHY_STYLE_LINE_KEY)
        return false;

    unsigned int spos, epos;
    for (spos = 0; spos < m_line.length () && isspace (m_line[spos]); spos++);
    for (epos = m_line.length () - 1; epos >= spos && m_line[epos] != '='; epos--);
    if (epos <= spos)
        epos = m_line.length ();
    for (--epos; epos >= spos && isspace (m_line[epos]); epos--);
    if (!isspace(m_line[epos]))
        epos++;

    if (spos < epos)
        key = m_line.substr (spos, epos - spos);
    else
        key = String ();

    return true;
}

bool
AnthyStyleLine::get_value (String &value)
{
    if (m_type != ANTHY_STYLE_LINE_KEY)
        return false;

    unsigned int spos, epos;
    for (spos = 0; spos < m_line.length () && m_line[spos] != '='; spos++);
    if (spos >= m_line.length ())
        spos = 0;
    else
        spos++;

    for (; spos < m_line.length () && isspace(m_line[spos]); spos++);
    for (epos = m_line.length (); epos >= spos && isspace (m_line[epos]); epos--);
    if (!isspace(m_line[epos]))
        epos++;

    value = m_line.substr (spos, epos - spos);

    return true;
}

void
AnthyStyleLine::set_value (String value)
{
    String key;
    get_key (key);
    m_line = key + String ("=") + value;
}

AnthyStyleFile::AnthyStyleFile ()
{
}

AnthyStyleFile::~AnthyStyleFile ()
{
}

bool
AnthyStyleFile::load (const char *filename)
{
    std::ifstream in_file (filename);
    if (!in_file)
        return false;

    m_sections.clear ();

    m_sections.push_back (AnthyStyleLines ());
    AnthyStyleLines *section = &m_sections[0];

    char buf[MAX_LINE_LENGTH];
    while (!in_file.eof ()) {
        in_file.getline (buf, MAX_LINE_LENGTH);

        AnthyStyleLine line (buf);
        AnthyStyleLineType type = line.get_type ();

        if (type == ANTHY_STYLE_LINE_SECTION) {
            m_sections.push_back (AnthyStyleLines ());
            section = &*(m_sections.end() - 1);
        }

        section->push_back (line);
    }

    in_file.close ();

    return true;
}

bool
AnthyStyleFile::save (const char *filename)
{
    std::ofstream out_file (filename);
    if (!out_file)
        return false;

    AnthyStyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        AnthyStyleLines::iterator lit;
        for (lit = it->begin (); lit != it->end (); lit++) {
            String line;
            lit->get_line (line);
            out_file << line.c_str () << std::endl;
        }
    }

    out_file.close ();

    return true;
}

bool
AnthyStyleFile::get_string (String &value, String section, String key)
{
    AnthyStyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        String s, k;
        (*it)[0].get_section (s);

        if (s != section)
            continue;

        AnthyStyleLines::iterator lit;
        for (lit = it->begin (); lit != it->end (); lit++) {
            lit->get_key (k);
            if (k == key) {
                lit->get_value (value);
                return true;
            }
        }
    }

    return false;
}

void
AnthyStyleFile::set_string (String section, String key, String value)
{
    // find section
    AnthyStyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        String s, k;
        (*it)[0].get_section (s);

        if (s != section)
            continue;

        // find entry
        AnthyStyleLines::iterator lit, last = it->begin () + 1;
        for (lit = last; lit != it->end (); lit++) {
            AnthyStyleLineType type = lit->get_type ();
            if (type != ANTHY_STYLE_LINE_SPACE)
                last = lit;
            lit->get_key (k);

            // replace existing entry
            if (k == key) {
                lit->set_value (value);
                return;
            }
        }

        // append new entry if no mathced entry exists.
        String str = String (key) + String ("=") + String(value);
        it->insert (last + 1, AnthyStyleLine (str.c_str ()));
        return;
    }

    // append new section
    //String str = String ("[") + String (section) + String ("]");
    //mandokuse-
}

bool
AnthyStyleFile::get_entry_list (AnthyStyleLines &lines, String section)
{
    AnthyStyleSections::iterator it;
    for (it = m_sections.begin (); it != m_sections.end (); it++) {
        if (it->size () <= 0)
            continue;

        AnthyStyleLines::iterator lit;
        String s;
        (*it)[0].get_section (s);
        if (s == section) {
            lines = (*it);
            return true;
        }
    }

    return false;
}
