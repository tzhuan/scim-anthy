/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Takashi Nakamoto
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

#ifndef __SCIM_ANTHY_DICTION_H__
#define __SCIM_ANTHY_DICTION_H__

#define Uses_SCIM_CONFIG_BASE

#include <scim.h>

using namespace scim;

class AnthyDiction
{
public:
    AnthyDiction (const WideString &base,
                  const WideString &pos,
                  const WideString &diction);
    AnthyDiction (const AnthyDiction &a);
    ~AnthyDiction ();

    WideString get_base ();
    WideString get_pos ();
    WideString get_end_form ();
    WideString get_diction ();
    bool       has_diction ();
private:
    WideString m_base;
    WideString m_pos;
    WideString m_ending;
    WideString m_diction;
};

class AnthyDictionService
{
public:
    AnthyDictionService (const ConfigPointer &config);
    ~AnthyDictionService();
    
    void reload_config             (const ConfigPointer &config);
    AnthyDiction get_diction       (const WideString &segment);
private:
    String                       m_diction_file;
    bool                         m_enable_diction;
    FILE                        *m_diction_file_ptr;
    std::map< WideString, long > m_hash;

private:
    void load_diction_file ();

    long read_one_chunk (WideString &word,
                         WideString &pos,
                         WideString &diction,
                         long position);
    void append_word    (const WideString &word,
                         const WideString &pos,
                         long position);

    void close_diction_file ();
    bool open_diction_file ();
};

#endif /* __SCIM_ANTHY_DICTION_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
