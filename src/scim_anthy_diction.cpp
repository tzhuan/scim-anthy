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
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <scim.h>
#include "scim_anthy_diction.h"
#include "scim_anthy_prefs.h"

#define SYSTEM_DICTION_FILE     SCIM_ANTHY_DATADIR"/diction"
#define SYSTEM_CONJUGATION_FILE SCIM_ANTHY_DATADIR"/conjugation"

#define READING_BASE_STATE 1
#define READING_POS_STATE 2
#define READING_DICTION_STATE 3

#define WRONG_SYNTAX -1
#define REACHING_EOF -2

static std::map< WideString, AnthyConjugation > conjugations;

AnthyConjugation::AnthyConjugation (const WideString &pos,
                                    const WideString &end_form_ending,
                                    const std::vector <WideString > endings)
{
    m_pos = pos;
    m_end_form_ending = end_form_ending;
    m_endings = endings;
}

WideString
AnthyConjugation::get_end_form_ending ()
{
    return m_end_form_ending;
}

std::vector< WideString >::iterator AnthyConjugation::begin_endings ()
{
    return m_endings.begin ();
}

std::vector< WideString >::iterator AnthyConjugation::end_endings ()
{
    return m_endings.end ();
}

AnthyDiction::AnthyDiction (const WideString &base,
                            const WideString &pos,
                            const WideString &end_form_ending,
                            const WideString &diction)
{
    m_base = base;
    m_pos = pos;
    m_end_form_ending = end_form_ending;
    m_diction = diction;

}

AnthyDiction::AnthyDiction (const AnthyDiction &a)
{
    m_base = a.m_base;
    m_end_form_ending = a.m_end_form_ending;
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
    return m_base + m_end_form_ending;
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
  : m_diction_file              (String(SYSTEM_DICTION_FILE)),
    m_enable_diction            (false),
    m_diction_file_mtime        (0)
{
    String user_diction = scim_get_user_data_dir() + String("/Anthy/diction");
    if (access (user_diction.c_str(), R_OK) == 0)
        m_diction_file = user_diction;

    reload_config (config);
    load_conjugation_file ();
}

AnthyDictionService::~AnthyDictionService ()
{
}

void
AnthyDictionService::reload_config (const ConfigPointer &config)
{
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
    {
        load_diction_file (); // reload the diction file
        if (m_enable_diction == false)
            return;
    }

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
        for (unsigned int i = 1; i <= seg->size (); i++)
        {
            WideString key = seg->substr (0, i);
            std::map< WideString, long >::iterator p = m_hash.find (key);
            if (p != m_hash.end ())
            {
                // ToDo: cache
                read_one_chunk (f, p->second, base, pos, diction);

                // get end form ending
                WideString end_form_ending;
                std::map< WideString, AnthyConjugation >::iterator q = conjugations.find (pos);
                if (q != conjugations.end ())
                {
                    end_form_ending = q->second.get_end_form_ending ();
                }

                // ToDo: don't add the same diction
                dictions.push_back (AnthyDiction (base, pos, end_form_ending, diction));

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

    // comment lines
    char buf[256];
    do
    {
        if( fgets(buf, 256, f) == NULL )
        {
            // failed to find "EOC" line and actual contents
            close_diction_file (f);
            reset ();
            m_enable_diction = false;
            return;
        }
    }while( strcmp(buf, "EOC") == 0);

    long position = ftell(f);
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

                // strip the last line feed code
                diction_buffer[diction_buffer.size() - 1] = '\0';
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
    std::map< WideString, AnthyConjugation >::iterator p = conjugations.find (pos);
    if (p != conjugations.end ())
    {
        // declinable word
        std::vector< WideString >::iterator q = p->second.begin_endings ();
        while (q != p->second.end_endings ())
        {
            m_hash.insert (std::make_pair (base + (*q), position));
            q++;
        }
    }
    else
    {
        // indeclinable word
        m_hash.insert (std::make_pair (base, position));
    }
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

void
AnthyDictionService::load_conjugation_file ()
{
    int len;
    char line[256];
    char *p, *p2;

    conjugations.clear ();

    FILE *f = fopen (SYSTEM_CONJUGATION_FILE, "r");
    if (f == NULL)
        return;

    while (fgets (line, 256, f) != NULL)
    {
        len = strlen (line);
        if (len == 0 ||       // empty line
            line[0] == '#')   // comment line
        {
            continue;
        }
        else
        {
            WideString pos;
            WideString end_form;
            std::vector< WideString > endings;

            p = line;

            // read part of speech
            p2 = strchr (p, ',');
            if (p2 == NULL)
                continue; // invalid line
            pos = utf8_mbstowcs (p, p2 - p);

            // read the end form
            p = p2 + 1;
            p2 = strchr (p, ',');
            if (p2 == NULL)
                continue; // invalid line
            end_form = utf8_mbstowcs (p, p2 - p);
            
            // read endings
            p = p2 + 1;
            while ( (p2 = strchr (p, ',')) != NULL)
            {
                endings.push_back (utf8_mbstowcs (p, p2 - p));
                p = p2 + 1;
            }

            AnthyConjugation conj (pos, end_form, endings);
            conjugations.insert (std::make_pair (pos, conj));
        }
    }

    fclose (f);
}

/*
  vi:ts=4:nowrap:ai:expandtab
*/
