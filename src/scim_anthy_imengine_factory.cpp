/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2004 Takuro Ashie <ashie@homa.ne.jp>
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

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <scim.h>
#include "scim_anthy_imengine_factory.h"
#include "scim_anthy_imengine.h"
#include "scim_anthy_prefs.h"
#include "intl.h"

#define scim_module_init anthy_LTX_scim_module_init
#define scim_module_exit anthy_LTX_scim_module_exit
#define scim_imengine_module_init anthy_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory anthy_LTX_scim_imengine_module_create_factory

#define SCIM_CONFIG_IMENGINE_ANTHY_UUID     "/IMEngine/Anthy/UUID-"

#ifndef SCIM_ANTHY_ICON_FILE
    #define SCIM_ANTHY_ICON_FILE           (SCIM_ICONDIR"/scim-anthy.png")
#endif

static ConfigPointer _scim_config (0);

extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_ANTHY_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
        anthy_quit ();
        _scim_config.reset ();
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        SCIM_DEBUG_IMENGINE(1) << "Initialize Anthy Engine.\n";

        _scim_config = config;

        if (anthy_init ()) {
            SCIM_DEBUG_IMENGINE(1) << "Failed to initialize Anthy Library!\n";
            return 0;
        }

        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        AnthyFactory *factory = 0;

        try {
            factory = new AnthyFactory (String ("ja_JP"),
                                        String ("fffb6633-7041-428e-9dfc-139117a71b6e"),
                                        _scim_config);
        } catch (...) {
            delete factory;
            factory = 0;
        }

        return factory;
    }
}

AnthyFactory::AnthyFactory (const String &lang,
                            const String &uuid,
                            const ConfigPointer &config)
    : m_uuid (uuid),
      m_config (config),
      m_typing_method (SCIM_ANTHY_CONFIG_TYPING_METHOD_DEFAULT),
      m_period_style (SCIM_ANTHY_CONFIG_PERIOD_STYLE_DEFAULT),
      m_space_type (SCIM_ANTHY_CONFIG_SPACE_TYPE_DEFAULT),
      m_ten_key_type (SCIM_ANTHY_CONFIG_TEN_KEY_TYPE_DEFAULT),
      m_auto_convert (SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD_DEFAULT),
      m_close_cand_win_on_select (SCIM_ANTHY_CONFIG_CLOSE_CAND_WIN_ON_SELECT_DEFAULT),
      m_learn_on_manual_commit (SCIM_ANTHY_CONFIG_LEARN_ON_MANUAL_COMMIT_DEFAULT),
      m_learn_on_auto_commit (SCIM_ANTHY_CONFIG_LEARN_ON_AUTO_COMMIT_DEFAULT),
      m_romaji_half_symbol (SCIM_ANTHY_CONFIG_ROMAJI_HALF_SYMBOL_DEFAULT),
      m_romaji_half_number (SCIM_ANTHY_CONFIG_ROMAJI_HALF_NUMBER_DEFAULT),
      m_dict_admin_command (SCIM_ANTHY_CONFIG_DICT_ADMIN_COMMAND_DEFAULT),
      m_add_word_command (SCIM_ANTHY_CONFIG_ADD_WORD_COMMAND_DEFAULT),
      m_show_input_mode_label (SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL_DEFAULT),
      m_show_typing_method_label (SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL_DEFAULT),
      m_show_period_style_label (SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL_DEFAULT),
      m_show_dict_label (SCIM_ANTHY_CONFIG_SHOW_DICT_LABEL_DEFAULT),
      m_show_dict_admin_label (SCIM_ANTHY_CONFIG_SHOW_DICT_ADMIN_LABEL_DEFAULT),
      m_show_add_word_label (SCIM_ANTHY_CONFIG_SHOW_ADD_WORD_LABEL_DEFAULT)
{
    SCIM_DEBUG_IMENGINE(1) << "Create Anthy Factory :\n";
    SCIM_DEBUG_IMENGINE(1) << "  Lang : " << lang << "\n";
    SCIM_DEBUG_IMENGINE(1) << "  UUID : " << uuid << "\n";

    if (lang.length () >= 2)
        set_languages (lang);

    /* config */
    reload_config (m_config);
    m_reload_signal_connection = m_config->signal_connect_reload (slot (this, &AnthyFactory::reload_config));
}

AnthyFactory::~AnthyFactory ()
{
    m_reload_signal_connection.disconnect ();
}

WideString
AnthyFactory::get_name () const
{
    return utf8_mbstowcs (String ("Anthy"));
}

WideString
AnthyFactory::get_authors () const
{
    return WideString ();
}

WideString
AnthyFactory::get_credits () const
{
    return WideString ();
}

WideString
AnthyFactory::get_help () const
{
    return WideString ();
}

String
AnthyFactory::get_uuid () const
{
    return m_uuid;
}

String
AnthyFactory::get_icon_file () const
{
    return String (SCIM_ANTHY_ICON_FILE);
}

IMEngineInstancePointer
AnthyFactory::create_instance (const String &encoding, int id)
{
    return new AnthyInstance (this, encoding, id);
}

void
AnthyFactory::append_config_listener (AnthyInstance *listener)
{
    bool found = false;
    std::vector<AnthyInstance*>::iterator it;
    for (it = m_config_listeners.begin();
         it != m_config_listeners.end();
         it++)
    {
        if (*it == listener) {
            found = true;
            break;
        }
    }

    if (!found)
        m_config_listeners.push_back (listener);
}

void
AnthyFactory::remove_config_listener (AnthyInstance *listener)
{
    std::vector<AnthyInstance*>::iterator it;
    for (it = m_config_listeners.begin();
         it != m_config_listeners.end();
         it++)
    {
        if (*it == listener) {
            m_config_listeners.erase (it);
            break;
        }
    }
}


#if 0
#define APPEND_ACTION(key, func)                                               \
{                                                                              \
    String name = "func", str;                                                 \
    str = config->read (String (SCIM_ANTHY_CONFIG_##key##_KEY),                \
                        String (SCIM_ANTHY_CONFIG_##key##_KEY_DEFAULT));       \
    m_actions.push_back (AnthyAction (name, str, &AnthyInstance::func));       \
}
#else
#define APPEND_ACTION(key, func)                                               \
{                                                                              \
    String name = "func", str;                                                 \
    str = config->read (String (SCIM_ANTHY_CONFIG_##key##_KEY),                \
                        String (SCIM_ANTHY_CONFIG_##key##_KEY_DEFAULT));       \
    m_actions.push_back (AnthyAction (name, str, func));                       \
}
#endif

#define ANTHY_DEFINE_ACTION(func) \
static bool                       \
func (AnthyInstance *anthy)       \
{                                 \
    return anthy->func ();        \
}

ANTHY_DEFINE_ACTION (action_commit_follow_preference);
ANTHY_DEFINE_ACTION (action_commit_reverse_preference);
ANTHY_DEFINE_ACTION (action_convert);
ANTHY_DEFINE_ACTION (action_revert);
ANTHY_DEFINE_ACTION (action_back);
ANTHY_DEFINE_ACTION (action_delete);
ANTHY_DEFINE_ACTION (action_insert_space);
ANTHY_DEFINE_ACTION (action_insert_alternative_space);
ANTHY_DEFINE_ACTION (action_insert_half_space);
ANTHY_DEFINE_ACTION (action_insert_wide_space);
ANTHY_DEFINE_ACTION (action_move_caret_first);
ANTHY_DEFINE_ACTION (action_move_caret_last);
ANTHY_DEFINE_ACTION (action_move_caret_forward);
ANTHY_DEFINE_ACTION (action_move_caret_backward);
ANTHY_DEFINE_ACTION (action_select_first_segment);
ANTHY_DEFINE_ACTION (action_select_last_segment);
ANTHY_DEFINE_ACTION (action_select_next_segment);
ANTHY_DEFINE_ACTION (action_select_prev_segment);
ANTHY_DEFINE_ACTION (action_shrink_segment);
ANTHY_DEFINE_ACTION (action_expand_segment);
ANTHY_DEFINE_ACTION (action_commit_first_segment);
ANTHY_DEFINE_ACTION (action_commit_selected_segment);
ANTHY_DEFINE_ACTION (action_commit_first_segment_reverse_preference);
ANTHY_DEFINE_ACTION (action_commit_selected_segment_reverse_preference);
ANTHY_DEFINE_ACTION (action_select_first_candidate);
ANTHY_DEFINE_ACTION (action_select_last_candidate);
ANTHY_DEFINE_ACTION (action_select_next_candidate);
ANTHY_DEFINE_ACTION (action_select_prev_candidate);
ANTHY_DEFINE_ACTION (action_candidates_page_up);
ANTHY_DEFINE_ACTION (action_candidates_page_down);
ANTHY_DEFINE_ACTION (action_select_candidate_1);
ANTHY_DEFINE_ACTION (action_select_candidate_2);
ANTHY_DEFINE_ACTION (action_select_candidate_3);
ANTHY_DEFINE_ACTION (action_select_candidate_4);
ANTHY_DEFINE_ACTION (action_select_candidate_5);
ANTHY_DEFINE_ACTION (action_select_candidate_6);
ANTHY_DEFINE_ACTION (action_select_candidate_7);
ANTHY_DEFINE_ACTION (action_select_candidate_8);
ANTHY_DEFINE_ACTION (action_select_candidate_9);
ANTHY_DEFINE_ACTION (action_select_candidate_10);
ANTHY_DEFINE_ACTION (action_convert_to_hiragana);
ANTHY_DEFINE_ACTION (action_convert_to_katakana);
ANTHY_DEFINE_ACTION (action_convert_to_half_katakana);
ANTHY_DEFINE_ACTION (action_convert_to_latin);
ANTHY_DEFINE_ACTION (action_convert_to_wide_latin);
ANTHY_DEFINE_ACTION (action_toggle_latin_mode);
ANTHY_DEFINE_ACTION (action_toggle_wide_latin_mode);
ANTHY_DEFINE_ACTION (action_circle_kana_mode);
ANTHY_DEFINE_ACTION (action_circle_typing_method);
ANTHY_DEFINE_ACTION (action_hiragana_mode);
ANTHY_DEFINE_ACTION (action_katakana_mode);
ANTHY_DEFINE_ACTION (action_launch_dict_admin_tool);
ANTHY_DEFINE_ACTION (action_add_word);

void
AnthyFactory::reload_config (const ConfigPointer &config)
{
    if (config) {
        String str;

        m_typing_method
            = config->read (SCIM_ANTHY_CONFIG_TYPING_METHOD,
                            m_typing_method);
        m_period_style
            = config->read (SCIM_ANTHY_CONFIG_PERIOD_STYLE,
                            m_period_style);
        m_space_type
            = config->read (SCIM_ANTHY_CONFIG_SPACE_TYPE,
                            m_space_type);
        m_ten_key_type
            = config->read (SCIM_ANTHY_CONFIG_TEN_KEY_TYPE,
                            m_ten_key_type);
        m_auto_convert
            = config->read (SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD,
                            m_auto_convert);
        m_close_cand_win_on_select
            = config->read (SCIM_ANTHY_CONFIG_CLOSE_CAND_WIN_ON_SELECT,
                            m_close_cand_win_on_select);
        m_learn_on_manual_commit
            = config->read (SCIM_ANTHY_CONFIG_LEARN_ON_MANUAL_COMMIT,
                            m_learn_on_manual_commit);
        m_learn_on_auto_commit
            = config->read (SCIM_ANTHY_CONFIG_LEARN_ON_AUTO_COMMIT,
                            m_learn_on_auto_commit);
        m_romaji_half_symbol
            = config->read (SCIM_ANTHY_CONFIG_ROMAJI_HALF_SYMBOL,
                            m_romaji_half_symbol);
        m_romaji_half_number
            = config->read (SCIM_ANTHY_CONFIG_ROMAJI_HALF_NUMBER,
                            m_romaji_half_number);
        m_dict_admin_command
            = config->read (SCIM_ANTHY_CONFIG_DICT_ADMIN_COMMAND,
                            m_dict_admin_command);
        m_add_word_command
            = config->read (SCIM_ANTHY_CONFIG_ADD_WORD_COMMAND,
                            m_add_word_command);
        m_show_input_mode_label
            = config->read (SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL,
                            m_show_input_mode_label);
        m_show_typing_method_label
            = config->read (SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL,
                            m_show_typing_method_label);
        m_show_period_style_label
            = config->read (SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL,
                            m_show_period_style_label);
        m_show_dict_label
            = config->read (SCIM_ANTHY_CONFIG_SHOW_DICT_LABEL,
                            m_show_dict_label);
        m_show_dict_admin_label
            = config->read (SCIM_ANTHY_CONFIG_SHOW_DICT_ADMIN_LABEL,
                            m_show_dict_admin_label);
        m_show_add_word_label
            = config->read (SCIM_ANTHY_CONFIG_SHOW_ADD_WORD_LABEL,
                            m_show_add_word_label);

        // clear old actions
        m_actions.clear ();

        // candidates keys
        APPEND_ACTION (SELECT_FIRST_CANDIDATE,  action_select_first_candidate);
        APPEND_ACTION (SELECT_LAST_CANDIDATE,   action_select_last_candidate);
        APPEND_ACTION (SELECT_NEXT_CANDIDATE,   action_select_next_candidate);
        APPEND_ACTION (SELECT_PREV_CANDIDATE,   action_select_prev_candidate);
        APPEND_ACTION (CANDIDATES_PAGE_UP,      action_candidates_page_up);
        APPEND_ACTION (CANDIDATES_PAGE_DOWN,    action_candidates_page_down);
        APPEND_ACTION (SELECT_CANDIDATE_1,      action_select_candidate_1);
        APPEND_ACTION (SELECT_CANDIDATE_2,      action_select_candidate_2);
        APPEND_ACTION (SELECT_CANDIDATE_3,      action_select_candidate_3);
        APPEND_ACTION (SELECT_CANDIDATE_4,      action_select_candidate_4);
        APPEND_ACTION (SELECT_CANDIDATE_5,      action_select_candidate_5);
        APPEND_ACTION (SELECT_CANDIDATE_6,      action_select_candidate_6);
        APPEND_ACTION (SELECT_CANDIDATE_7,      action_select_candidate_7);
        APPEND_ACTION (SELECT_CANDIDATE_8,      action_select_candidate_8);
        APPEND_ACTION (SELECT_CANDIDATE_9,      action_select_candidate_9);
        APPEND_ACTION (SELECT_CANDIDATE_10,     action_select_candidate_10);

        // segment keys
        APPEND_ACTION (SELECT_FIRST_SEGMENT,    action_select_first_segment);
        APPEND_ACTION (SELECT_LAST_SEGMENT,     action_select_last_segment);
        APPEND_ACTION (SELECT_NEXT_SEGMENT,     action_select_next_segment);
        APPEND_ACTION (SELECT_PREV_SEGMENT,     action_select_prev_segment);
        APPEND_ACTION (SHRINK_SEGMENT,          action_shrink_segment);
        APPEND_ACTION (EXPAND_SEGMENT,          action_expand_segment);
        APPEND_ACTION (COMMIT_FIRST_SEGMENT,    action_commit_first_segment);
        APPEND_ACTION (COMMIT_SELECTED_SEGMENT, action_commit_selected_segment);
        APPEND_ACTION (COMMIT_FIRST_SEGMENT_REVERSE_LEARN,
                       action_commit_first_segment_reverse_preference);
        APPEND_ACTION (COMMIT_SELECTED_SEGMENT_REVERSE_LEARN,
                       action_commit_selected_segment_reverse_preference);

        // convert keys
        APPEND_ACTION (CONV_TO_HIRAGANA,        action_convert_to_hiragana);
        APPEND_ACTION (CONV_TO_KATAKANA,        action_convert_to_katakana);
        APPEND_ACTION (CONV_TO_HALF_KATAKANA,   action_convert_to_half_katakana);
        APPEND_ACTION (CONV_TO_LATIN,           action_convert_to_latin);
        APPEND_ACTION (CONV_TO_WIDE_LATIN,      action_convert_to_wide_latin);

        // caret keys
        APPEND_ACTION (MOVE_CARET_FIRST,        action_move_caret_first);
        APPEND_ACTION (MOVE_CARET_LAST,         action_move_caret_last);
        APPEND_ACTION (MOVE_CARET_FORWARD,      action_move_caret_forward);
        APPEND_ACTION (MOVE_CARET_BACKWARD,     action_move_caret_backward);

        // edit keys
        APPEND_ACTION (BACKSPACE,               action_back);
        APPEND_ACTION (DELETE,                  action_delete);
        APPEND_ACTION (COMMIT,                  action_commit_follow_preference);
        APPEND_ACTION (COMMIT_REVERSE_LEARN,    action_commit_reverse_preference);
        APPEND_ACTION (CONVERT,                 action_convert);
        APPEND_ACTION (CANCEL,                  action_revert);
        APPEND_ACTION (INSERT_SPACE,            action_insert_space);
        APPEND_ACTION (INSERT_ALT_SPACE,        action_insert_alternative_space);
        APPEND_ACTION (INSERT_HALF_SPACE,       action_insert_half_space);
        APPEND_ACTION (INSERT_WIDE_SPACE,       action_insert_wide_space);

        // mode keys
        APPEND_ACTION (LATIN_MODE,              action_toggle_latin_mode);
        APPEND_ACTION (WIDE_LATIN_MODE,         action_toggle_wide_latin_mode);
        APPEND_ACTION (CIRCLE_KANA_MODE,        action_circle_kana_mode);
        APPEND_ACTION (CIRCLE_TYPING_METHOD,    action_circle_typing_method);
        APPEND_ACTION (HIRAGANA_MODE,           action_hiragana_mode);
        APPEND_ACTION (KATAKANA_MODE,           action_katakana_mode);

        // dict keys
        APPEND_ACTION (DICT_ADMIN,              action_launch_dict_admin_tool);
        APPEND_ACTION (ADD_WORD,                action_add_word);

        std::vector<AnthyInstance*>::iterator it;
        for (it = m_config_listeners.begin();
             it != m_config_listeners.end();
             it++)
        {
            (*it)->reload_config (config);
        }
    }
}
