/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
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

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3. 
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#ifndef __SCIM_ANTHY_IMENGINE_H__
#define __SCIM_ANTHY_IMENGINE_H__

#define Uses_SCIM_ICONV
#include <scim.h>
using namespace scim;

class AnthyFactory : public IMEngineFactoryBase
{
    String m_uuid;

    IConvert m_iconv;
    friend class AnthyInstance;

public:
    AnthyFactory (const String &lang,
                  const String &uuid);

    virtual ~AnthyFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);
};

class AnthyInstance : public IMEngineInstanceBase
{
    AnthyFactory     *m_factory;

    WideString        m_preedit_string;
    AttributeList     m_preedit_attrs;
    int               m_preedit_caret;

    CommonLookupTable m_lookup_table;

    bool              m_show_lookup_table;

    int               m_input_mode;
    int               m_output_mode;
    
    PropertyList      m_properties;

public:
    AnthyInstance (AnthyFactory   *factory,
                   const String   &encoding,
                   int             id = -1);

    virtual ~AnthyInstance ();

    virtual bool process_key_event             (const KeyEvent& key);
    virtual void move_preedit_caret            (unsigned int pos);
    virtual void select_candidate              (unsigned int item);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up          (void);
    virtual void lookup_table_page_down        (void);
    virtual void reset                         (void);
    virtual void focus_in                      (void);
    virtual void focus_out                     (void);
    virtual void trigger_property              (const String &property);

private:
    /* candidate keys */
    bool   candidate_key_event (const KeyEvent &key);

};
#endif /* __SCIM_ANTHY_IMENGINE_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
