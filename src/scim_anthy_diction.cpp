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
#define Uses_STL_MAP
#define Uses_STL_VECTOR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <scim.h>
#include "scim_anthy_diction.h"
#include "scim_anthy_prefs.h"

#define CONJUGATION_FILE SCIM_ANTHY_DATADIR"/conjugation"

#define READING_BASE_STATE 1
#define READING_POS_STATE 2
#define READING_DICTION_STATE 3

#define WRONG_SYNTAX -1
#define REACHING_EOF -2

static void
read_conjugation_file ()
{
    char line[256];
    char element[256];

    FILE *f = fopen (CONJUGATION_FILE, "r");
    if (f == NULL)
        return;
//ToDo: read csv file
/*
    char *p, *p2;
    int i = 0;
    while (fgets (line, 256, f) != NULL)
    {
        len = strlen (line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        int j = 0;
        p = line;
        while (*p != '\0')
        {
            p2 = strchr (p, ',');
            if (p2 != NULL)
            {
                p2 = strnchr(element, p, p2 - p);
                element[p2 - p] = '\0';
                p = ++p2;
            }
            else
            {
                strcpy (element, p);
                *p = '\0';
            }
            j++;
        }
        i++;
    }
*/
    fclose (f);
}

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
    m_diction = a.m_diction;
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
    m_diction_file_mtime        (0)
{
    reload_config (config);
}

AnthyDictionService::~AnthyDictionService ()
{
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

    if (m_enable_diction)
        load_diction_file ();
    else
        reset ();
}

void
AnthyDictionService::get_dictions (std::vector< WideString > &segments,
                                   std::vector< AnthyDiction > &dictions)
{
    dictions.clear ();

    if (m_enable_diction == false)
        return;

    if (is_diction_file_modified ())
        load_diction_file (); // reload the diction file

    FILE *f = open_diction_file ();

    if (f == NULL)
    {
        // failed to open the diction file
        reset ();
        m_enable_diction = false;
        return;
    }

    std::vector< WideString >::iterator seg = segments.begin ();
    WideString base;
    WideString pos;
    WideString diction;

    while (seg != segments.end ())
    {
        for (int i = 1; i <= segments.size (); i++)
        {
            WideString key = seg->substr (0, i);
            std::map< WideString, long >::iterator p = m_hash.find (key);
            if (p != m_hash.end ())
            {
                // ToDo: cache
                read_one_chunk (f, p->second, base, pos, diction);
                dictions.push_back (AnthyDiction (base, pos, diction));
                break;
            }
        }
        seg++;
    }

    close_diction_file (f);
}

void
AnthyDictionService::reset ()
{
    m_diction_file_mtime = 0;
    m_hash.clear ();
}

void
AnthyDictionService::load_diction_file ()
{
    FILE *f;

    // clear previously loaded data
    reset ();

    f = open_diction_file ();
    if (f == NULL)
    {
        m_enable_diction = false;
        return; // failed to open diction file
    }

    long position = 0;
    long next_position;
    WideString base;
    WideString pos;
    WideString diction;

    while ((next_position = read_one_chunk (f, position, base, pos, diction))
           >= 0)
    {
        append_word (base, pos, position);
        position = next_position;
    }

    if (next_position == WRONG_SYNTAX)
    {
        close_diction_file (f);
        reset ();
        m_enable_diction = false;
        return;
    }

    close_diction_file (f);
}

// return
//   >=0: means next word position
//   -1:  means wrong syntax (WRONG_SYNTAX)
//   -2:  means reaching EOF (REACHING_EOF)
long
AnthyDictionService::read_one_chunk (FILE *f,
                                     long position,
                                     WideString &base,
                                     WideString &pos,
                                     WideString &diction)
{
    int state = READING_BASE_STATE;
    int i;
    char c;
    String base_buffer = String ();
    String pos_buffer = String ();
    String diction_buffer = String ();

    fseek (f, position, SEEK_SET);

    while ((i = fgetc (f)) != EOF)
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

                return ftell (f);
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
AnthyDictionService::close_diction_file (FILE *f)
{
    if (f)
    {
        fclose (f);
    }
}

FILE *
AnthyDictionService::open_diction_file ()
{
    FILE *f =fopen (m_diction_file.c_str (), "r");

    if (f == NULL)
        return NULL;

    struct stat buf;
    int fd = fileno (f);

    // lock and get status
    if (flock (fd, LOCK_SH | LOCK_NB) == -1 ||
        fstat (fd, &buf) == -1)
    {
        close_diction_file (f);
        return NULL;
    }
    m_diction_file_mtime = buf.st_mtime;

    return f;
}

// returns true if the diction file is modified since the last time the
// file is modified
bool
AnthyDictionService::is_diction_file_modified ()
{
    struct stat buf;
    if (stat (m_diction_file.c_str (), &buf) == -1)
        return true;

    return (buf.st_mtime > m_diction_file_mtime);
}

/*
  vi:ts=4:nowrap:ai:expandtab
*/
