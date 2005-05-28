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

#ifndef __SCIM_ANTHY_STYLE_FILE_H__
#define __SCIM_ANTHY_STYLE_FILE_H__

#include <scim.h>
#include <scim_event.h>

using namespace scim;

namespace scim_anthy {

class StyleLine;
class StyleSection;
class StyleFile;

typedef std::vector<StyleLine>  StyleLines;
typedef std::vector<StyleLines> StyleSections;
typedef std::vector<StyleFile>  StyleFiles;

typedef enum {
    SCIM_ANTHY_STYLE_LINE_UNKNOWN,
    SCIM_ANTHY_STYLE_LINE_SPACE,
    SCIM_ANTHY_STYLE_LINE_COMMENT,
    SCIM_ANTHY_STYLE_LINE_SECTION,
    SCIM_ANTHY_STYLE_LINE_KEY,
} StyleLineType;

class StyleLine
{
public:
    StyleLine (const char *line);
    ~StyleLine ();

public:
    StyleLineType get_type    (void);
    void          get_line    (String &line) { line = m_line; }
    bool          get_section (String &section);
    bool          get_key     (String &key);
    bool          get_value   (String &value);
    void          set_value   (String value);

private:
    String        m_line;
    StyleLineType m_type;
};

class StyleFile
{
public:
    StyleFile ();
    ~StyleFile ();

public:
    bool   load (const char *filename);
    bool   save (const char *filename);

    String get_encoding     (void);
    String get_title        (void);

    bool   get_string       (String        &value,
                             String         section,
                             String         key);
    void   set_string       (String         section,
                             String         key,
                             String         value);
    bool   get_section_list (StyleSections &sections);
    bool   get_entry_list   (StyleLines    &lines,
                             String         section);

private:

private:
    String        m_filename;
    String        m_encoding;
    String        m_title;
    StyleSections m_sections;
};

}

#endif /* __SCIM_ANTHY_STYLE_FILE_H__ */
