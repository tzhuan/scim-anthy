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

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3. 
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#ifndef __SCIM_ANTHY_IMENGINE_FACTORY_H__
#define __SCIM_ANTHY_IMENGINE_FACTORY_H__

#include <anthy/anthy.h>
#include <scim.h>
#include "scim_anthy_action.h"

using namespace scim;
using namespace scim_anthy;

class AnthyFactory : public IMEngineFactoryBase
{
    String m_uuid;

    friend class AnthyInstance;

    /* config */
    ConfigPointer m_config;
    Connection    m_reload_signal_connection;

    /* for preferece */
    String        m_typing_method;
    String        m_period_style;
    String        m_space_type;
    String        m_ten_key_type;
    bool          m_auto_convert;
    bool          m_close_cand_win_on_select;
    bool          m_learn_on_manual_commit;
    bool          m_learn_on_auto_commit;
    bool          m_romaji_half_symbol;
    bool          m_romaji_half_number;
    String        m_dict_admin_command;
    String        m_add_word_command;
    bool          m_show_input_mode_label;
    bool          m_show_typing_method_label;
    bool          m_show_period_style_label;
    bool          m_show_dict_label;
    bool          m_show_dict_admin_label;
    bool          m_show_add_word_label;

    std::vector<AnthyInstance*> m_config_listeners;

    /* for key bindings */
    std::vector<Action> m_actions;

public:
    AnthyFactory (const String &lang,
                  const String &uuid,
                  const ConfigPointer &config);
    virtual ~AnthyFactory ();

    virtual WideString  get_name      () const;
    virtual WideString  get_authors   () const;
    virtual WideString  get_credits   () const;
    virtual WideString  get_help      () const;
    virtual String      get_uuid      () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding,
                                                     int id = -1);

    virtual void append_config_listener (AnthyInstance *listener);
    virtual void remove_config_listener (AnthyInstance *listener);

private:
    void reload_config (const ConfigPointer &config);
};

#endif /* __SCIM_ANTHY_IMENGINE_FACTORY_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
