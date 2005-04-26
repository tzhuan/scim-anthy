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

class AnthyStyleFile
{
public:
    AnthyStyleFile ();
    ~AnthyStyleFile ();

public:
    bool load (const char *filename);
    bool save (const char *filename);

#if 0
    bool get_romaji_table  (AnthyRomajiTable  &table);
    bool get_key_bindings  (AnthyActions      &actions);
    bool get_preedit_style (AnthyPreeditStyle &style);
#endif

private:

private:
    String              m_filename;
    std::vector<String> m_lines;
};

#endif /* __SCIM_ANTHY_STYLE_FILE_H__ */
