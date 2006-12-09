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
#include <map.h>
#include "scim_anthy_diction.h"
#include "scim_anthy_prefs.h"

AnthyDiction::AnthyDiction (const ConfigPointer &config)
    : m_diction_file              (String("")),
    m_enable_diction            (false),
    m_diction_file_ptr          (NULL)
{
    reload_config (config);
}

AnthyDiction::~AnthyDiction ()
{
    // close previously loaded file
    if (m_diction_file_ptr)
    {
        int fd = fileno (m_diction_file_ptr);
        flock (fd, LOCK_UN);
        fclose (m_diction_file_ptr);
    }
}

void
AnthyDiction::reload_config (const ConfigPointer &config)
{
    String tmp;

    tmp = config->read (String (SCIM_ANTHY_CONFIG_DICTION_FILE),
                        String (SCIM_ANTHY_CONFIG_DICTION_FILE_DEFAULT));
    m_diction_file = tmp;

    m_enable_diction = config->read (String (SCIM_ANTHY_CONFIG_ENABLE_DICTION),
                                     SCIM_ANTHY_CONFIG_ENABLE_DICTION_DEFAULT);

    reload_diction_file ();
}

WideString
AnthyDiction::get_diction (const WideString &word)
{
    if (m_enable_diction == false)
        return WideString ();

    std::map< WideString, WideString >::iterator p;
    p = m_hash.find (word);
    if (p == m_hash.end ())
    {
        return WideString ();
    }

    return p->second;
}

void
AnthyDiction::reload_diction_file ()
{
    int fd;

    // close previously loaded file
    if (m_diction_file_ptr)
    {
        fd = fileno (m_diction_file_ptr);
        flock (fd, LOCK_UN);
        fclose (m_diction_file_ptr);
    }
    m_hash.clear ();

    // reload
    m_diction_file_ptr = fopen (m_diction_file.c_str (), "r");
    if (m_diction_file_ptr == NULL)
    {
        // failed to load the specified diction file
        m_enable_diction = false;
        return;
    }
//    fd = fileno (m_diction_file_ptr);
//    flock (fd, LOCK_EX);

#define READING_WORD_STATE 1
#define READING_DICTION_STATE 2
    int state = READING_WORD_STATE;
    int i;
    char c;
    String word_buffer;
    String diction_buffer;

    while ((i = fgetc (m_diction_file_ptr)) != EOF)
    {
        c = (i >= 0x80) ? (i - 0x100) : i;
        switch(state)
        {
        case READING_WORD_STATE:
        {
            if (c == '\n')
            {
                if (word_buffer.size () == 0)
                {
                    m_enable_diction = false;
                    return; // wrong syntax
                }
                state = READING_DICTION_STATE;
            }
            else
            {
                word_buffer += c;
            }
            break;
        }
        case READING_DICTION_STATE:
        {
            if (c == '\n' &&
                diction_buffer[diction_buffer.size () - 1] == '\n')
            {
                state = READING_WORD_STATE;
                m_hash.insert (
                    std::make_pair (utf8_mbstowcs (word_buffer),
                                    utf8_mbstowcs (diction_buffer)));

                word_buffer = String ("");
                diction_buffer = String ("");
            }
            else
            {
                diction_buffer += c;
            }
            break;
        }
        }
    }

    if (word_buffer.size () != 0 &&
        diction_buffer.size () != 0)
        m_hash.insert (
            std::make_pair (utf8_mbstowcs (word_buffer),
                            utf8_mbstowcs (diction_buffer)));
}

/*
  vi:ts=4:nowrap:ai:expandtab
*/
