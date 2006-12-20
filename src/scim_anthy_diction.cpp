/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Takashi Nakamoto <bluedwarf@bpost.plala.or.jp>
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

#define Uses_SCIM_UTILITY

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/file.h>

#include <scim.h>
#include <map>
#include "scim_anthy_diction.h"
#include "scim_anthy_prefs.h"

#define READING_BASE_STATE 1
#define READING_POS_STATE 2
#define READING_DICTION_STATE 3

#define WRONG_SYNTAX -1
#define REACHING_EOF -2

AnthyDiction::AnthyDiction (const WideString &base,
                            const WideString &pos,
                            const WideString &diction)
{
    m_base = base;
    m_ending = WideString (); // ToDo: specify ending from pos
    m_pos = pos;
    m_diction = diction;

}

AnthyDiction::AnthyDiction (const AnthyDiction &a)
{
    m_base = a.m_base;
    m_ending = a.m_ending;
    m_pos = a.m_pos;
    m_diction = m_diction;
}

AnthyDiction::~AnthyDiction ()
{
}

WideString
AnthyDiction::get_base ()
{
    return m_base;
}

WideString
AnthyDiction::get_pos ()
{
    return m_pos;
}

WideString
AnthyDiction::get_end_form ()
{
    return m_base + m_ending;
}

WideString
AnthyDiction::get_diction ()
{
    return m_diction;
}

bool
AnthyDiction::has_diction ()
{
    return (m_diction.size () > 0);
}

AnthyDictionService::AnthyDictionService (const ConfigPointer &config)
  : m_diction_file              (String("")),
    m_enable_diction            (false),
    m_diction_file_ptr          (NULL)
{
    reload_config (config);
}

AnthyDictionService::~AnthyDictionService ()
{
    m_hash.clear ();
    close_diction_file ();
}

void
AnthyDictionService::reload_config (const ConfigPointer &config)
{
    String tmp;

    tmp = config->read (String (SCIM_ANTHY_CONFIG_DICTION_FILE),
                        String (SCIM_ANTHY_CONFIG_DICTION_FILE_DEFAULT));
    m_diction_file = tmp;

    m_enable_diction = config->read (String (SCIM_ANTHY_CONFIG_ENABLE_DICTION),
                                     SCIM_ANTHY_CONFIG_ENABLE_DICTION_DEFAULT);

    // close previously loaded file
    close_diction_file ();
    m_hash.clear ();

    if (m_enable_diction)
        load_diction_file ();
}


AnthyDiction
AnthyDictionService::get_diction (const WideString &segment)
{
    if (m_enable_diction == false ||
        m_diction_file_ptr == NULL)
        return AnthyDiction (WideString (), WideString (), WideString ());

    WideString base;
    WideString pos;
    WideString diction;

    // prefix search?
    for(int i = 1; i <= segment.size (); i++)
    {
        WideString key = segment.substr (0, i);
        
        std::map< WideString, long >::iterator p = m_hash.find (key);
        if (p != m_hash.end ())
        {
            // ToDo: chache
            read_one_chunk (base, pos, diction, p->second);
            return AnthyDiction (base, pos, diction);
        }
    }

    return AnthyDiction (WideString (), WideString (), WideString ());
}

void
AnthyDictionService::load_diction_file ()
{
    int fd;

    m_enable_diction = open_diction_file ();

    if (m_enable_diction == false)
        return; // failed to open diction file

    long position = 0;
    long next_position;
    WideString base;
    WideString pos;
    WideString diction;

    while ((next_position = read_one_chunk (base, pos, diction, position)) >= 0)
    {
        append_word (base, pos, position);
        position = next_position;
    }

    if (position == WRONG_SYNTAX)
    {
        close_diction_file ();
        m_hash.clear ();
        m_enable_diction = false;
        return;
    }
}

// return
//   >=0: means next word position
//   -1:  means wrong syntax
//   -2:  means reaching EOF
long
AnthyDictionService::read_one_chunk (WideString &base,
                                     WideString &pos,
                                     WideString &diction,
                                     long position)
{
    int state = READING_BASE_STATE;
    int i;
    char c;
    String base_buffer = String ();
    String pos_buffer = String ();
    String diction_buffer = String ();

    fseek (m_diction_file_ptr, position, SEEK_SET);

    while ((i = fgetc (m_diction_file_ptr)) != EOF)
    {
        c = (i >= 0x80) ? (i - 0x100) : i;
        switch(state)
        {
        case READING_BASE_STATE:
        {
            if (c == '\n')
            {
                if (base_buffer.size () == 0)
                    return WRONG_SYNTAX;

                state = READING_POS_STATE;
            }
            else
            {
                base_buffer += c;
            }
            break;
        }
        case READING_POS_STATE:
        {
            if (c == '\n')
            {
                if (pos_buffer.size () == 0)
                    return WRONG_SYNTAX;

                state = READING_DICTION_STATE;
            }
            else
            {
                pos_buffer += c;
            }
            break;
        }
        case READING_DICTION_STATE:
        {
            if (c == '\n' &&
                diction_buffer[diction_buffer.size () - 1] == '\n')
            {
                base.clear();
                base.append (utf8_mbstowcs (base_buffer));
                pos.clear ();
                pos.append (utf8_mbstowcs (pos_buffer));
                diction.clear ();
                diction.append (utf8_mbstowcs (diction_buffer));

                return ftell (m_diction_file_ptr);
            }
            else
            {
                diction_buffer += c;
            }
            break;
        }
        }
    }

    return REACHING_EOF;
}

void
AnthyDictionService::append_word (const WideString &base,
                                  const WideString &pos,
                                  const long position)
{
	m_hash.insert (std::make_pair (base, position));
}


void
AnthyDictionService::close_diction_file ()
{
    if (m_diction_file_ptr)
    {
        int fd = fileno (m_diction_file_ptr);
        flock (fd, LOCK_UN);
        fclose (m_diction_file_ptr);

        m_diction_file_ptr = NULL;
    }


}

bool
AnthyDictionService::open_diction_file ()
{
    m_diction_file_ptr = fopen (m_diction_file.c_str (), "r");

    if (m_diction_file_ptr == NULL)
        return false;
    else
    {
        int fd = fileno (m_diction_file_ptr);
//        flock (fd, LOCK_EX);

        return true;
    }
}

/*
  vi:ts=4:nowrap:ai:expandtab
*/
