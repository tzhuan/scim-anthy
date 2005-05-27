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

#ifndef __SCIM_ANTHY_KEY2KANA_H__
#define __SCIM_ANTHY_KEY2KANA_H__

#define Uses_SCIM_ICONV
#include <scim.h>

#include "scim_anthy_default_tables.h"
#include "scim_anthy_key2kana_table.h"

using namespace scim;

namespace scim_anthy {

typedef enum {
    SCIM_ANTHY_KEY2KANA_NEED_COMMIT,
} Key2KanaResult;

class Key2KanaConvertor
{
    Key2KanaTableSet  &m_tables;
    WideString         m_pending;
    ConvRule          *m_exact_match;

public:
    Key2KanaConvertor (Key2KanaTableSet & tables);
    virtual ~Key2KanaConvertor ();

    bool       append        (const String & str,
                              WideString   & result,
                              WideString   & pending);
    void       clear         (void);

    bool       is_pending    (void);
    WideString get_pending   (void);
    WideString flush_pending (void);
};

}
#endif /* __SCIM_ANTHY_KEY2KANA_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
