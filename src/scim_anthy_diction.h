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
    AnthyDiction (const ConfigPointer &config);
    ~AnthyDiction();
    
    void reload_config             (const ConfigPointer &config);
    WideString get_diction         (const WideString &word);
private:
    String m_diction_file;
    bool   m_enable_diction;
    FILE  *m_diction_file_ptr;
    std::map< WideString, WideString > m_hash;
    // ToDo: use buffered cache to reduce memory usage

private:
    void reload_diction_file ();
};

#endif /* __SCIM_ANTHY_DICTION_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
