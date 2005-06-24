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
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <scim.h>
#include "scim_anthy_factory.h"
#include "scim_anthy_imengine.h"
#include "scim_anthy_prefs.h"
#include "scim_anthy_intl.h"

#define SCIM_PROP_PREFIX                     "/IMEngine/Anthy"
#define SCIM_PROP_INPUT_MODE                 "/IMEngine/Anthy/InputMode"
#define SCIM_PROP_INPUT_MODE_HIRAGANA        "/IMEngine/Anthy/InputMode/Hiragana"
#define SCIM_PROP_INPUT_MODE_KATAKANA        "/IMEngine/Anthy/InputMode/Katakana"
#define SCIM_PROP_INPUT_MODE_HALF_KATAKANA   "/IMEngine/Anthy/InputMode/HalfKatakana"
#define SCIM_PROP_INPUT_MODE_LATIN           "/IMEngine/Anthy/InputMode/Latin"
#define SCIM_PROP_INPUT_MODE_WIDE_LATIN      "/IMEngine/Anthy/InputMode/WideLatin"

#define SCIM_PROP_CONV_MODE                  "/IMEngine/ANthy/ConvMode"
#define SCIM_PROP_CONV_MODE_MULTI_SEG        "/IMEngine/ANthy/ConvMode/MultiSegment"
#define SCIM_PROP_CONV_MODE_SINGLE_SEG       "/IMEngine/ANthy/ConvMode/SingleSegment"
#define SCIM_PROP_CONV_MODE_MULTI_REAL_TIME  "/IMEngine/ANthy/ConvMode/MultiRealTime"
#define SCIM_PROP_CONV_MODE_SINGLE_REAL_TIME "/IMEngine/ANthy/ConvMode/SingleRealTime"

#define SCIM_PROP_TYPING_METHOD              "/IMEngine/Anthy/TypingMethod"
#define SCIM_PROP_TYPING_METHOD_ROMAJI       "/IMEngine/Anthy/TypingMethod/RomaKana"
#define SCIM_PROP_TYPING_METHOD_KANA         "/IMEngine/Anthy/TypingMethod/Kana"

#define SCIM_PROP_PERIOD_STYLE               "/IMEngine/Anthy/PeriodType"
#define SCIM_PROP_PERIOD_STYLE_JAPANESE      "/IMEngine/Anthy/PeriodType/Japanese"
#define SCIM_PROP_PERIOD_STYLE_WIDE_LATIN    "/IMEngine/Anthy/PeriodType/WideRatin"
#define SCIM_PROP_PERIOD_STYLE_LATIN         "/IMEngine/Anthy/PeriodType/Ratin"
#define SCIM_PROP_PERIOD_STYLE_WIDE_LATIN_JAPANESE \
                                             "/IMEngine/Anthy/PeriodType/WideRatin_Japanese"

#define SCIM_PROP_DICT                       "/IMEngine/Anthy/Dictionary"
#define SCIM_PROP_DICT_ADD_WORD              "/IMEngine/Anthy/Dictionary/AddWord"
#define SCIM_PROP_DICT_LAUNCH_ADMIN_TOOL     "/IMEngine/Anthy/Dictionary/LaunchAdminTool"

AnthyInstance::AnthyInstance (AnthyFactory   *factory,
                              const String   &encoding,
                              int             id)
    : IMEngineInstanceBase     (factory, encoding, id),
      m_factory                (factory),
      m_prev_key               (0,0),
      m_preedit                (m_key2kana_tables),
      m_preedit_string_visible (false),
      m_lookup_table_visible   (false),
      m_prev_input_mode        (SCIM_ANTHY_MODE_HIRAGANA),
      m_conv_mode              (SCIM_ANTHY_CONVERSION_MULTI_SEGMENT)
{
    SCIM_DEBUG_IMENGINE(1) << "Create Anthy Instance : ";

    reload_config (m_factory->m_config);
    m_factory->append_config_listener (this);
}

AnthyInstance::~AnthyInstance ()
{
    m_factory->remove_config_listener (this);
}

bool
AnthyInstance::process_key_event_lookup_keybind (const KeyEvent& key)
{
    std::vector<Action>::iterator it;
    for (it = m_factory->m_actions.begin();
         it != m_factory->m_actions.end();
         it++)
    {
        if (it->perform (this, key))
            return true;
    }

    return false;
}

bool
AnthyInstance::process_key_event_without_preedit (const KeyEvent& key)
{
    return process_remaining_key_event (key);
}

bool
AnthyInstance::process_key_event_with_preedit (const KeyEvent& key)
{
    return process_remaining_key_event (key);
}

bool
AnthyInstance::process_key_event_with_candidate (const KeyEvent &key)
{
    return process_remaining_key_event (key);
}

bool
AnthyInstance::process_remaining_key_event (const KeyEvent &key)
{
    if (m_preedit.can_process_key_event (key)) {
        if (m_preedit.is_converting ()) {
            if (is_realtime_conversion ())
                action_revert ();
            else
                action_commit (m_factory->m_learn_on_auto_commit);
        }

        bool need_commit = m_preedit.process_key_event (key);

        if (need_commit) {
            action_commit (m_factory->m_learn_on_auto_commit);
        } else {
            if (is_realtime_conversion ()) {
                m_preedit.convert (SCIM_ANTHY_CANDIDATE_NORMAL,
                                   is_single_segment ());
                m_preedit.select_segment (-1);
            }
            show_preedit_string ();
            m_preedit_string_visible = true;
            set_preedition ();
        }

        return true;
    }

    if (m_preedit.is_preediting ())
        return true;
    else
        return false;
}

bool
AnthyInstance::process_key_event (const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE(2) << "process_key_event.\n";
    KeyEvent newkey;

    // ignore key release.
    if (key.is_key_release ())
        return true;

    // ignore modifier keys
    if (key.code == SCIM_KEY_Shift_L || key.code == SCIM_KEY_Shift_R ||
        key.code == SCIM_KEY_Control_L || key.code == SCIM_KEY_Control_R ||
        key.code == SCIM_KEY_Alt_L || key.code == SCIM_KEY_Alt_R)
        return false;

    // lookup user defined key bindings
    if (process_key_event_lookup_keybind (key))
        return true;

    if (m_preedit.get_input_mode () == SCIM_ANTHY_MODE_LATIN)
        return false;

    // process hard coded keys
    if (is_selecting_candidates ())
        return process_key_event_with_candidate (key);
    else if (m_preedit.is_preediting ())
        return process_key_event_with_preedit(key);
    else
        return process_key_event_without_preedit(key);
}

void
AnthyInstance::move_preedit_caret (unsigned int pos)
{
    m_preedit.set_caret_pos (pos);
    update_preedit_caret (m_preedit.get_caret_pos());
}

void
AnthyInstance::select_candidate_no_direct (unsigned int item)
{
    if (!is_selecting_candidates ()) return;

    SCIM_DEBUG_IMENGINE(2) << "select_candidate_no_direct.\n";
 
    // update lookup table
    m_lookup_table.set_cursor_pos_in_current_page (item);
    update_lookup_table (m_lookup_table);

    // update preedit
    m_preedit.select_candidate (m_lookup_table.get_cursor_pos ());
    set_preedition ();
}

void
AnthyInstance::select_candidate (unsigned int item)
{
    SCIM_DEBUG_IMENGINE(2) << "select_candidate.\n";

    select_candidate_no_direct (item);

    if (m_factory->m_close_cand_win_on_select) {
        m_lookup_table.clear ();
        hide_lookup_table ();
        m_lookup_table_visible = false;
        action_select_next_segment();
    }
}

void
AnthyInstance::update_lookup_table_page_size (unsigned int page_size)
{
    SCIM_DEBUG_IMENGINE(2) << "update_lookup_table_page_size.\n";

    m_lookup_table.set_page_size (page_size);
}

void
AnthyInstance::lookup_table_page_up ()
{
    if (!is_selecting_candidates () ||
        !m_lookup_table.get_current_page_start ())
    {
        return;
    }

    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_up.\n";

    m_lookup_table.page_up ();

    update_lookup_table (m_lookup_table);
}

void
AnthyInstance::lookup_table_page_down ()
{
    int page_start = m_lookup_table.get_current_page_start ();
    int page_size = m_lookup_table.get_current_page_size ();
    int num = m_lookup_table.number_of_candidates ();

    if (!is_selecting_candidates () || page_start + page_size >= num) 
        return;

    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_down.\n";

    m_lookup_table.page_down ();

    update_lookup_table (m_lookup_table);
}

void
AnthyInstance::reset ()
{
    SCIM_DEBUG_IMENGINE(2) << "reset.\n";

    m_preedit.clear ();
    m_lookup_table.clear ();

    hide_lookup_table ();
    hide_preedit_string ();

    m_preedit_string_visible = false;
    m_lookup_table_visible   = false;

    set_preedition ();
}

void
AnthyInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";

    hide_aux_string ();

    if (m_preedit_string_visible) {
        set_preedition ();
        show_preedit_string ();
    } else {
        hide_preedit_string ();
    }

    if (m_lookup_table_visible && is_selecting_candidates ()) {
        update_lookup_table (m_lookup_table);
        show_lookup_table ();
    } else {
        hide_lookup_table ();
    }

    install_properties ();
}

void
AnthyInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_out.\n";
}

void
AnthyInstance::set_preedition (void)
{
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos());
}

void
AnthyInstance::install_properties (void)
{
    if (m_properties.size () <= 0) {
        Property prop;

        if (m_factory->m_show_input_mode_label) {
            prop = Property (SCIM_PROP_INPUT_MODE,
                             "\xE3\x81\x82", String (""), _("Input mode"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_INPUT_MODE_HIRAGANA,
                             _("Hiragana"), String (""), _("Hiragana"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_INPUT_MODE_KATAKANA,
                             _("Katakana"), String (""), _("Katakana"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_INPUT_MODE_HALF_KATAKANA,
                             _("Half width katakana"), String (""),
                             _("Half width katakana"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_INPUT_MODE_LATIN,
                             _("Latin"), String (""), _("Direct input"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_INPUT_MODE_WIDE_LATIN,
                             _("Wide latin"), String (""), _("Wide latin"));
            m_properties.push_back (prop);
        }

        if (m_factory->m_show_typing_method_label) {
            prop = Property (SCIM_PROP_TYPING_METHOD,
                             "\xEF\xBC\xB2", String (""), _("Typing method"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_TYPING_METHOD_ROMAJI,
                             _("Romaji"), String (""), _("Romaji"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_TYPING_METHOD_KANA,
                             _("Kana"), String (""), _("Kana"));
            m_properties.push_back (prop);
        }

        if (m_factory->m_show_conv_mode_label) {
            prop = Property (SCIM_PROP_CONV_MODE,
                             "\xE9\x80\xA3", String (""),
                             _("Conversion mode"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_CONV_MODE_MULTI_SEG,
                             _("Multi segment"), String (""),
                             _("Multi segment"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_CONV_MODE_SINGLE_SEG,
                             _("Single segment"), String (""),
                             _("Single segment"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_CONV_MODE_MULTI_REAL_TIME,
                             _("Convert as you type (Multi segment)"),
                             String (""),
                             _("Convert as you type (Multi segment)"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_CONV_MODE_SINGLE_REAL_TIME,
                             _("Convert as you type (Single segment)"),
                             String (""),
                             _("Convert as you type (Single segment)"));
            m_properties.push_back (prop);
        }

        if (m_factory->m_show_period_style_label) {
            prop = Property (SCIM_PROP_PERIOD_STYLE,
                             "\xE3\x80\x81\xE3\x80\x82", String (""),
                             _("Period style"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_PERIOD_STYLE_JAPANESE,
                             "\xE3\x80\x81\xE3\x80\x82", String (""),
                             "\xE3\x80\x81\xE3\x80\x82");
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_PERIOD_STYLE_WIDE_LATIN_JAPANESE,
                             "\xEF\xBC\x8C\xE3\x80\x82", String (""),
                             "\xEF\xBC\x8C\xE3\x80\x82");
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_PERIOD_STYLE_WIDE_LATIN,
                             "\xEF\xBC\x8C\xEF\xBC\x8E", String (""),
                             "\xEF\xBC\x8C\xEF\xBC\x8E");
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_PERIOD_STYLE_LATIN,
                             ",.", String (""), ",.");
            m_properties.push_back (prop);
        }

        if (m_factory->m_show_dict_label) {
            prop = Property (SCIM_PROP_DICT,
                             String(""), //_("Dictionary"),
                             String (SCIM_ICONDIR "/" "scim-anthy-dict.png"),
                             _("Dictionary menu"));
            m_properties.push_back (prop);

            if (m_factory->m_show_dict_admin_label) {
                prop = Property (SCIM_PROP_DICT_LAUNCH_ADMIN_TOOL,
                                 _("Edit the dictionary"),
                                 String (SCIM_ICONDIR "/" "scim-anthy-dict.png"),
                                 _("Launch the dictionary administration tool."));
                m_properties.push_back (prop);
            }

            if (m_factory->m_show_add_word_label) {
                prop = Property (SCIM_PROP_DICT_ADD_WORD,
                                 _("Add a word"),
                                 String (SCIM_ICONDIR "/" "scim-anthy-dict.png"),
                                 _("Add a word to the dictorinay."));
                m_properties.push_back (prop);
            }
        }
    }

    set_input_mode(m_preedit.get_input_mode ());
    set_conversion_mode (m_conv_mode);
    set_typing_method (m_key2kana_tables.get_typing_method ());
    set_period_style (m_key2kana_tables.get_period_style (),
                      m_key2kana_tables.get_comma_style ());

    register_properties (m_properties);
}

void
AnthyInstance::set_input_mode (InputMode mode)
{
    const char *label = "";

    switch (mode) {
    case SCIM_ANTHY_MODE_HIRAGANA:
        label = "\xE3\x81\x82";
        break;
    case SCIM_ANTHY_MODE_KATAKANA:
        label = "\xE3\x82\xA2";
        break;
    case SCIM_ANTHY_MODE_HALF_KATAKANA:
        label = "_\xEF\xBD\xB1";
        break;
    case SCIM_ANTHY_MODE_LATIN:
        label = "_A";
        break;
    case SCIM_ANTHY_MODE_WIDE_LATIN:
        label = "\xEF\xBC\xA1";
        break;
    default:
        break; 
    }

    if (label && *label && m_factory->m_show_input_mode_label) {
        PropertyList::iterator it = std::find (m_properties.begin (),
                                               m_properties.end (),
                                               SCIM_PROP_INPUT_MODE);
        if (it != m_properties.end ()) {
            it->set_label (label);
            update_property (*it);
        }
    }

    if (mode != m_preedit.get_input_mode ()) {
        m_preedit.set_input_mode (mode);
        set_preedition ();
    }
}

void
AnthyInstance::set_conversion_mode (ConversionMode mode)
{
    const char *label = "";

    switch (mode) {
    case SCIM_ANTHY_CONVERSION_MULTI_SEGMENT:
        label = "\xE9\x80\xA3";
        break;
    case SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT:
        label = "\xE5\x8D\x98";
        break;
    case SCIM_ANTHY_CONVERSION_MULTI_SEGMENT_IMMEDIATE:
        label = "\xE9\x80\x90 \xE9\x80\xA3";
        break;
    case SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE:
        label = "\xE9\x80\x90 \xE5\x8D\x98";
        break;
    default:
        break; 
    }

    if (label && *label /*&& m_factory->m_show_input_mode_label*/) {
        PropertyList::iterator it = std::find (m_properties.begin (),
                                               m_properties.end (),
                                               SCIM_PROP_CONV_MODE);
        if (it != m_properties.end ()) {
            it->set_label (label);
            update_property (*it);
        }
    }

    m_conv_mode = mode;
}

void
AnthyInstance::set_typing_method (TypingMethod method)
{
    const char *label = "";

    switch (method) {
    case SCIM_ANTHY_TYPING_METHOD_ROMAJI:
        label = "\xEF\xBC\xB2";
        break;
    case SCIM_ANTHY_TYPING_METHOD_KANA:
        label = "\xE3\x81\x8B";
        break;
    default:
        break;
    }

    if (label && *label && m_factory->m_show_typing_method_label) {
        PropertyList::iterator it = std::find (m_properties.begin (),
                                               m_properties.end (),
                                               SCIM_PROP_TYPING_METHOD);
        if (it != m_properties.end ()) {
            it->set_label (label);
            update_property (*it);
        }
    }

    if (method != m_key2kana_tables.get_typing_method ()) {
        Key2KanaTable *fundamental_table = NULL;
        if (method == SCIM_ANTHY_TYPING_METHOD_ROMAJI)
            fundamental_table = m_factory->m_custom_romaji_table;
        m_key2kana_tables.set_typing_method
            (method, fundamental_table);
    }
}

void
AnthyInstance::set_period_style (PeriodStyle period,
                                 CommaStyle  comma)
{
    String label;

    switch (comma) {
    case SCIM_ANTHY_COMMA_JAPANESE:
        label = "\xE3\x80\x81";
        break;
    case SCIM_ANTHY_COMMA_WIDE:
        label = "\xEF\xBC\x8C";
        break;
    case SCIM_ANTHY_COMMA_HALF:
        label = ",";
        break;
    default:
        break;
    }

    switch (period) {
    case SCIM_ANTHY_PERIOD_JAPANESE:
        label += "\xE3\x80\x82";
        break;
    case SCIM_ANTHY_PERIOD_WIDE:
        label += "\xEF\xBC\x8E";
        break;
    case SCIM_ANTHY_PERIOD_HALF:
        label += ".";
        break;
    default:
        break;
    }

    if (label.length () > 0) {
        PropertyList::iterator it = std::find (m_properties.begin (),
                                               m_properties.end (),
                                               SCIM_PROP_PERIOD_STYLE);
        if (it != m_properties.end ()) {
            it->set_label (label.c_str ());
            update_property (*it);
        }
    }

    if (period != m_key2kana_tables.get_period_style ())
        m_key2kana_tables.set_period_style (period);
    if (comma != m_key2kana_tables.get_comma_style ())
        m_key2kana_tables.set_comma_style (comma);
}

bool
AnthyInstance::is_selecting_candidates (void)
{
    if (m_lookup_table.number_of_candidates ())
        return true;
    else
        return false;
}

bool
AnthyInstance::action_convert (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (!m_preedit.is_converting ()) {
        // show conversion string
        m_preedit.convert (SCIM_ANTHY_CANDIDATE_NORMAL,
                           is_single_segment ());
        set_preedition ();
    } else {
        if (is_realtime_conversion () &&
            m_preedit.get_selected_segment () < 0)
        {
            int n = m_preedit.get_nr_segments ();
            if (n < 1)
                return false;
            m_preedit.select_segment (n - 1);
        }

        // show candidates window
        m_preedit.get_candidates (m_lookup_table);

        if (m_lookup_table.number_of_candidates () == 0)
            return false;

        int selected = m_preedit.get_selected_candidate ();
        int page_size = m_lookup_table.get_page_size ();
        int end = m_lookup_table.number_of_candidates () - 1;

        int page = selected / page_size;
        int pos  = selected % page_size;

        if (m_preedit.get_selected_candidate () >= end) {
            m_lookup_table.set_cursor_pos (0);
            select_candidate_no_direct (0);

        } else {
            for (int i = 0; i < page; i++)
                m_lookup_table.page_down ();

            if (pos + 1 >= m_lookup_table.get_page_size ()) {
                m_lookup_table.page_down ();
                select_candidate_no_direct (0);

            } else {
                select_candidate_no_direct (pos + 1);
            }
        }
        show_lookup_table ();
        m_lookup_table_visible = true;
    }
    
    return true;
}

bool
AnthyInstance::action_revert (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (!m_preedit.is_converting ()) {
        reset ();
        return true;
    }

    if (is_selecting_candidates ()) {
        m_lookup_table.clear ();
        hide_lookup_table ();
        m_lookup_table_visible = false;
        return true;
    }

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    m_preedit.revert ();
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_commit (bool learn)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (m_preedit.is_converting ()) {
        commit_string (m_preedit.get_string ());
        if (learn)
            m_preedit.commit ();
    } else {
        m_preedit.finish ();
        commit_string (m_preedit.get_string ());
    }

    reset ();

    return true;
}

bool
AnthyInstance::action_commit_follow_preference (void)
{
    return action_commit (m_factory->m_learn_on_manual_commit);
}

bool
AnthyInstance::action_commit_reverse_preference (void)
{
    return action_commit (!m_factory->m_learn_on_manual_commit);
}

bool
AnthyInstance::action_back (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (m_preedit.is_converting ()) {
        action_revert ();
        if (!is_realtime_conversion ())
            return true;
    }

    m_preedit.erase ();

    if (m_preedit.get_length () > 0) {
        if (is_realtime_conversion ()) {
            m_preedit.convert (SCIM_ANTHY_CANDIDATE_NORMAL,
                               is_single_segment ());
            m_preedit.select_segment (-1);
        }
        set_preedition ();
    } else {
        reset ();
    }

    return true;
}

bool
AnthyInstance::action_delete (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (m_preedit.is_converting ()) {
        action_revert ();
        if (!is_realtime_conversion ())
            return true;
    }

    m_preedit.erase (false);

    if (m_preedit.get_length () > 0) {
        if (is_realtime_conversion ()) {
            m_preedit.convert (SCIM_ANTHY_CANDIDATE_NORMAL,
                               is_single_segment ());
            m_preedit.select_segment (-1);
        }
        set_preedition ();
    } else {
        reset ();
    }

    return true;
}

bool
AnthyInstance::action_insert_space (void)
{
    if (m_preedit.is_preediting ())
        return false;

    bool is_wide = false;

    if (m_factory->m_space_type == "FollowMode") {
        InputMode mode = m_preedit.get_input_mode ();
        if (mode == SCIM_ANTHY_MODE_LATIN ||
            mode == SCIM_ANTHY_MODE_HALF_KATAKANA)
        {
            is_wide = false;
        } else {
            is_wide = true;
        }
    } else if (m_factory->m_space_type == "Wide") {
        is_wide = true;
    }

    if (is_wide)
        commit_string (utf8_mbstowcs ("\xE3\x80\x80"));
    else
        commit_string (utf8_mbstowcs (" "));

    return true;
}

bool 
AnthyInstance::action_insert_alternative_space (void)
{
    if (m_preedit.is_preediting ())
        return false;

    bool is_wide = false;

    if (m_factory->m_space_type == "FollowMode") {
        InputMode mode = m_preedit.get_input_mode ();
        if (mode == SCIM_ANTHY_MODE_LATIN ||
            mode == SCIM_ANTHY_MODE_HALF_KATAKANA)
        {
            is_wide = true;
        } else {
            is_wide = false;
        }
    } else if (m_factory->m_space_type != "Wide") {
        is_wide = true;
    }

    if (is_wide)
        commit_string (utf8_mbstowcs ("\xE3\x80\x80"));
    else
        commit_string (utf8_mbstowcs (" "));

    return true;
}

bool
AnthyInstance::action_insert_half_space (void)
{
    if (m_preedit.is_preediting ())
        return false;

    commit_string (utf8_mbstowcs (" "));

    return true;
}

bool
AnthyInstance::action_insert_wide_space (void)
{
    if (m_preedit.is_preediting ())
        return false;

    commit_string (utf8_mbstowcs ("\xE3\x80\x80"));

    return true;
}

bool
AnthyInstance::action_move_caret_backward (void)
{
    if (!m_preedit.is_preediting ())
        return false;
    if (m_preedit.is_converting ())
        return false;

    m_preedit.move_caret(-1);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_move_caret_forward (void)
{
    if (!m_preedit.is_preediting ())
        return false;
    if (m_preedit.is_converting ())
        return false;

    m_preedit.move_caret(1);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_move_caret_first (void)
{
    if (!m_preedit.is_preediting ())
        return false;
    if (m_preedit.is_converting ())
        return false;

    m_preedit.set_caret_pos (0);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_move_caret_last (void)
{
    if (!m_preedit.is_preediting ())
        return false;
    if (m_preedit.is_converting ())
        return false;

    m_preedit.set_caret_pos (m_preedit.get_length ());
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_select_prev_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    int idx = m_preedit.get_selected_segment ();
    if (idx - 1 < 0) {
        int n = m_preedit.get_nr_segments ();
        if (n <= 0) return false;
        m_preedit.select_segment (n - 1);
    } else {
        m_preedit.select_segment (idx - 1);
    }
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_select_next_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    int idx = m_preedit.get_selected_segment ();
    if (idx < 0) {
        m_preedit.select_segment(0);
    } else {
        int n = m_preedit.get_nr_segments ();
        if (n <= 0)
            return false;
        if (idx + 1 >= n)
            m_preedit.select_segment(0);
        else
            m_preedit.select_segment(idx + 1);
    }
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_select_first_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    m_preedit.select_segment(0);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_select_last_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    int n = m_preedit.get_nr_segments ();
    if (n <= 0) return false;

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    m_preedit.select_segment(n - 1);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_shrink_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    m_preedit.resize_segment (-1);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_expand_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    m_preedit.resize_segment (1);
    set_preedition ();

    return true;
}

bool
AnthyInstance::action_commit_first_segment (void)
{
    if (!m_preedit.is_converting ()) {
        if (m_preedit.is_preediting ()) {
            return action_commit (m_factory->m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    commit_string (m_preedit.get_segment_string (0));
    if (m_factory->m_learn_on_manual_commit)
        m_preedit.commit (0);

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_commit_selected_segment (void)
{
    if (!m_preedit.is_converting ()) {
        if (m_preedit.is_preediting ()) {
            return action_commit (m_factory->m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    for (int i = 0; i <= m_preedit.get_selected_segment (); i++)
        commit_string (m_preedit.get_segment_string (i));
    if (m_factory->m_learn_on_manual_commit)
        m_preedit.commit (m_preedit.get_selected_segment ());

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_commit_first_segment_reverse_preference (void)
{
    if (!m_preedit.is_converting ()) {
        if (m_preedit.is_preediting ()) {
            return action_commit (!m_factory->m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    commit_string (m_preedit.get_segment_string (0));
    if (!m_factory->m_learn_on_manual_commit)
        m_preedit.commit (0);

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_commit_selected_segment_reverse_preference (void)
{
    if (!m_preedit.is_converting ()) {
        if (m_preedit.is_preediting ()) {
            return action_commit (!m_factory->m_learn_on_manual_commit);
        } else {
            return false;
        }
    }

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    for (int i = 0; i <= m_preedit.get_selected_segment (); i++)
        commit_string (m_preedit.get_segment_string (i));
    if (!m_factory->m_learn_on_manual_commit)
        m_preedit.commit (m_preedit.get_selected_segment ());

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_select_next_candidate (void)
{
    if (!m_preedit.is_converting ())
        return false;

    if (!is_selecting_candidates ()) {
        action_convert ();
        return true;
    }

    int end = m_lookup_table.number_of_candidates () - 1;
    if (m_lookup_table.get_cursor_pos () == end)
    {
        m_lookup_table.set_cursor_pos (0);
    } else {
        m_lookup_table.cursor_down ();
    }

    int pos_in_page = m_lookup_table.get_cursor_pos_in_current_page ();
    select_candidate_no_direct (pos_in_page);

    return true;
}

bool
AnthyInstance::action_select_prev_candidate (void)
{
    if (!m_preedit.is_converting ()) return false;

    if (!is_selecting_candidates ())
        action_convert ();

    int end = m_lookup_table.number_of_candidates () - 1;
    if (m_lookup_table.get_cursor_pos () == 0)
        m_lookup_table.set_cursor_pos (end);
    else
        m_lookup_table.cursor_up ();

    int pos_in_page = m_lookup_table.get_cursor_pos_in_current_page ();
    select_candidate_no_direct (pos_in_page);

    return true;
}

bool
AnthyInstance::action_select_first_candidate (void)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;

    m_lookup_table.set_cursor_pos (0);

    int pos_in_page = m_lookup_table.get_cursor_pos_in_current_page ();
    select_candidate_no_direct (pos_in_page);

    return true;
}

bool
AnthyInstance::action_select_last_candidate (void)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;

    int end = m_lookup_table.number_of_candidates () - 1;
    m_lookup_table.set_cursor_pos (end);

    int pos_in_page = m_lookup_table.get_cursor_pos_in_current_page ();
    select_candidate_no_direct (pos_in_page);

    return true;
}

bool
AnthyInstance::action_candidates_page_up(void)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;

    m_lookup_table.page_up ();

    int pos_in_page = m_lookup_table.get_cursor_pos_in_current_page ();
    select_candidate_no_direct (pos_in_page);

    return true;
}

bool
AnthyInstance::action_candidates_page_down (void)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;

    m_lookup_table.page_down ();

    int pos_in_page = m_lookup_table.get_cursor_pos_in_current_page ();
    select_candidate_no_direct (pos_in_page);

    return true;
}

bool
AnthyInstance::action_select_candidate (unsigned int i)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;

    select_candidate (i);

    return true;
}

bool
AnthyInstance::action_select_candidate_1 (void)
{
    return action_select_candidate (0);
}

bool
AnthyInstance::action_select_candidate_2 (void)
{
    return action_select_candidate (1);
}

bool
AnthyInstance::action_select_candidate_3 (void)
{
    return action_select_candidate (2);
}

bool
AnthyInstance::action_select_candidate_4 (void)
{
    return action_select_candidate (3);
}

bool
AnthyInstance::action_select_candidate_5 (void)
{
    return action_select_candidate (4);
}

bool
AnthyInstance::action_select_candidate_6 (void)
{
    return action_select_candidate (5);
}

bool
AnthyInstance::action_select_candidate_7 (void)
{
    return action_select_candidate (6);
}


bool
AnthyInstance::action_select_candidate_8 (void)
{
    return action_select_candidate (7);
}

bool
AnthyInstance::action_select_candidate_9 (void)
{
    return action_select_candidate (8);
}

bool
AnthyInstance::action_select_candidate_10 (void)
{
    return action_select_candidate (9);
}

bool
AnthyInstance::action_circle_input_mode (void)
{
    InputMode mode = SCIM_ANTHY_MODE_HIRAGANA;

    switch (mode) {
    case SCIM_ANTHY_MODE_HIRAGANA:
        mode = SCIM_ANTHY_MODE_KATAKANA;
        break;
    case SCIM_ANTHY_MODE_KATAKANA:
        mode = SCIM_ANTHY_MODE_HALF_KATAKANA;
        break;
    case SCIM_ANTHY_MODE_HALF_KATAKANA:
        mode = SCIM_ANTHY_MODE_LATIN;
        break;
    case SCIM_ANTHY_MODE_LATIN:
        mode = SCIM_ANTHY_MODE_WIDE_LATIN;
        break;
    case SCIM_ANTHY_MODE_WIDE_LATIN:
        mode = SCIM_ANTHY_MODE_HIRAGANA;
        break;
    default:
        mode = SCIM_ANTHY_MODE_HIRAGANA;
        break; 
    }

    set_input_mode (mode);

    return true;
}

bool
AnthyInstance::action_circle_typing_method (void)
{
    TypingMethod method;

    method = m_key2kana_tables.get_typing_method ();
    if (method == SCIM_ANTHY_TYPING_METHOD_KANA)
        method = SCIM_ANTHY_TYPING_METHOD_ROMAJI;
    else
        method = SCIM_ANTHY_TYPING_METHOD_KANA;

    set_typing_method (method);

    return true;
}

bool
AnthyInstance::action_circle_kana_mode (void)
{
    InputMode mode;

    if (m_preedit.get_input_mode () == SCIM_ANTHY_MODE_LATIN ||
        m_preedit.get_input_mode () == SCIM_ANTHY_MODE_WIDE_LATIN)
    {
        mode = SCIM_ANTHY_MODE_HIRAGANA;
    } else {
        switch (m_preedit.get_input_mode ()) {
        case SCIM_ANTHY_MODE_HIRAGANA:
            mode = SCIM_ANTHY_MODE_KATAKANA;
            break;
        case SCIM_ANTHY_MODE_KATAKANA:
            mode = SCIM_ANTHY_MODE_HALF_KATAKANA;
            break;
        case SCIM_ANTHY_MODE_HALF_KATAKANA:
        default:
            mode = SCIM_ANTHY_MODE_HIRAGANA;
            break;
        }
    }

    set_input_mode (mode);

    return true;
}

bool
AnthyInstance::action_toggle_latin_mode (void)
{
    if (m_preedit.get_input_mode () == SCIM_ANTHY_MODE_LATIN ||
        m_preedit.get_input_mode () == SCIM_ANTHY_MODE_WIDE_LATIN)
    {
        set_input_mode (m_prev_input_mode);
        m_preedit.set_input_mode (m_prev_input_mode);
    } else {
        m_prev_input_mode = m_preedit.get_input_mode ();
        set_input_mode (SCIM_ANTHY_MODE_LATIN);
        m_preedit.set_input_mode (SCIM_ANTHY_MODE_LATIN);
    }

    return true;
}

bool
AnthyInstance::action_toggle_wide_latin_mode (void)
{
    if (m_preedit.get_input_mode () == SCIM_ANTHY_MODE_LATIN ||
        m_preedit.get_input_mode () == SCIM_ANTHY_MODE_WIDE_LATIN)
    {
        set_input_mode (m_prev_input_mode);
    } else {
        m_prev_input_mode = m_preedit.get_input_mode ();
        set_input_mode (SCIM_ANTHY_MODE_WIDE_LATIN);
    }

    return true;
}

bool
AnthyInstance::action_hiragana_mode (void)
{
    set_input_mode (SCIM_ANTHY_MODE_HIRAGANA);
    return true;
}

bool
AnthyInstance::action_katakana_mode (void)
{
    set_input_mode (SCIM_ANTHY_MODE_KATAKANA);
    return true;
}

bool
AnthyInstance::convert_kana (CandidateType type)
{
    if (!m_preedit.is_preediting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_lookup_table_visible = false;

    if (m_preedit.is_converting ()) {
        int idx = m_preedit.get_selected_segment ();
        if (idx < 0) {
            action_revert ();
            m_preedit.convert (type, true);
        } else {
            m_preedit.select_candidate (type);
        }
    } else {
        m_preedit.convert (type, true);
    }

    set_preedition ();

    return true;
}

bool
AnthyInstance::action_convert_to_hiragana (void)
{
    return convert_kana (SCIM_ANTHY_CANDIDATE_HIRAGANA);
}

bool
AnthyInstance::action_convert_to_katakana (void)
{
    return convert_kana (SCIM_ANTHY_CANDIDATE_KATAKANA);
}

bool
AnthyInstance::action_convert_to_half (void)
{
    return convert_kana (SCIM_ANTHY_CANDIDATE_HALF);
}

bool
AnthyInstance::action_convert_to_half_katakana (void)
{
    return convert_kana (SCIM_ANTHY_CANDIDATE_HALF_KATAKANA);
}

bool
AnthyInstance::action_convert_to_latin (void)
{
    return convert_kana (SCIM_ANTHY_CANDIDATE_LATIN);
}

bool
AnthyInstance::action_convert_to_wide_latin (void)
{
    return convert_kana (SCIM_ANTHY_CANDIDATE_WIDE_LATIN);
}

static void
launch_program (const char *command)
{
    if (!command) return;

    /* split string */
    unsigned int len = strlen (command);
    char tmp[len + 1];
    strncpy (tmp, command, len);
    tmp[len] = '\0';

    char *str = tmp;
    std::vector<char *> array;

    for (unsigned int i = 0; i < len + 1; i++) {
        if (!tmp[i] || isspace (tmp[i])) {
            if (*str) {
                tmp[i] = '\0';
                array.push_back (str);
                printf("str: %s\n", str);
            }
            str = tmp + i + 1;
        }
    }

    if (array.size () <= 0) return;
    array.push_back (NULL);

    char *args[array.size()];
    for (unsigned int i = 0; i < array.size (); i++)
        args[i] = array[i];


    /* exec command */
	pid_t child_pid;

	child_pid = fork();
	if (child_pid < 0) {
		perror("fork");
	} else if (child_pid == 0) {		 /* child process  */
		pid_t grandchild_pid;

		grandchild_pid = fork();
		if (grandchild_pid < 0) {
			perror("fork");
			_exit(1);
		} else if (grandchild_pid == 0) { /* grandchild process  */
			execvp(args[0], args);
			perror("execvp");
			_exit(1);
		} else {
			_exit(0);
		}
	} else {                              /* parent process */
		int status;
		waitpid(child_pid, &status, 0);
	}
}

bool
AnthyInstance::action_add_word (void)
{
    launch_program (m_factory->m_add_word_command.c_str ());

    return true;
}

bool
AnthyInstance::action_launch_dict_admin_tool (void)
{
    launch_program (m_factory->m_dict_admin_command.c_str ());

    return true;
}

#if 0
void
AnthyInstance::action_regist_word (void)
{
}
#endif

void
AnthyInstance::trigger_property (const String &property)
{
    String anthy_prop = property.substr (property.find_last_of ('/') + 1);

    SCIM_DEBUG_IMENGINE(2)
        << "trigger_property : " << property << " - " << anthy_prop << "\n";

    // input mode
    if (property == SCIM_PROP_INPUT_MODE_HIRAGANA) {
        set_input_mode (SCIM_ANTHY_MODE_HIRAGANA);
    } else if (property == SCIM_PROP_INPUT_MODE_KATAKANA) {
        set_input_mode (SCIM_ANTHY_MODE_KATAKANA);
    } else if (property == SCIM_PROP_INPUT_MODE_HALF_KATAKANA) {
        set_input_mode (SCIM_ANTHY_MODE_HALF_KATAKANA);
    } else if (property == SCIM_PROP_INPUT_MODE_LATIN) {
        set_input_mode (SCIM_ANTHY_MODE_LATIN);
    } else if (property == SCIM_PROP_INPUT_MODE_WIDE_LATIN) {
        set_input_mode (SCIM_ANTHY_MODE_WIDE_LATIN);

    // conversion mode
    } else if (property == SCIM_PROP_CONV_MODE_MULTI_SEG) {
        set_conversion_mode (SCIM_ANTHY_CONVERSION_MULTI_SEGMENT);
    } else if (property == SCIM_PROP_CONV_MODE_SINGLE_SEG) {
        set_conversion_mode (SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT);
    } else if (property == SCIM_PROP_CONV_MODE_MULTI_REAL_TIME) {
        set_conversion_mode (SCIM_ANTHY_CONVERSION_MULTI_SEGMENT_IMMEDIATE);
    } else if (property == SCIM_PROP_CONV_MODE_SINGLE_REAL_TIME) {
        set_conversion_mode (SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE);

    // typing method
    } else if (property == SCIM_PROP_TYPING_METHOD_ROMAJI) {
        set_typing_method (SCIM_ANTHY_TYPING_METHOD_ROMAJI);
    } else if (property == SCIM_PROP_TYPING_METHOD_KANA) {
        set_typing_method (SCIM_ANTHY_TYPING_METHOD_KANA);

    // period type
    } else if (property == SCIM_PROP_PERIOD_STYLE_JAPANESE) {
        set_period_style (SCIM_ANTHY_PERIOD_JAPANESE,
                          SCIM_ANTHY_COMMA_JAPANESE);
    } else if (property == SCIM_PROP_PERIOD_STYLE_WIDE_LATIN_JAPANESE) {
        set_period_style (SCIM_ANTHY_PERIOD_JAPANESE,
                          SCIM_ANTHY_COMMA_WIDE);
    } else if (property == SCIM_PROP_PERIOD_STYLE_WIDE_LATIN) {
        set_period_style (SCIM_ANTHY_PERIOD_WIDE,
                          SCIM_ANTHY_COMMA_WIDE);
    } else if (property == SCIM_PROP_PERIOD_STYLE_LATIN) {
        set_period_style (SCIM_ANTHY_PERIOD_HALF,
                          SCIM_ANTHY_COMMA_HALF);

    // dictionary
    } else if (property == SCIM_PROP_DICT_ADD_WORD) {
        action_add_word ();
    } else if (property == SCIM_PROP_DICT_LAUNCH_ADMIN_TOOL) {
        action_launch_dict_admin_tool ();
    }
}

void
AnthyInstance::reload_config (const ConfigPointer &config)
{
    // set romaji settings
    m_key2kana_tables.set_symbol_width (m_factory->m_romaji_half_symbol);
    m_key2kana_tables.set_number_width (m_factory->m_romaji_half_number);
    m_preedit.set_allow_split_romaji (m_factory->m_romaji_allow_split);

    // set input mode
    if (m_factory->m_input_mode == "Hiragana")
        m_preedit.set_input_mode (SCIM_ANTHY_MODE_HIRAGANA);
    else if (m_factory->m_input_mode == "Katakana")
        m_preedit.set_input_mode (SCIM_ANTHY_MODE_KATAKANA);
    else if (m_factory->m_input_mode == "HalfKatakana")
        m_preedit.set_input_mode (SCIM_ANTHY_MODE_HALF_KATAKANA);
    else if (m_factory->m_input_mode == "Latin")
        m_preedit.set_input_mode (SCIM_ANTHY_MODE_LATIN);
    else if (m_factory->m_input_mode == "WideLatin")
        m_preedit.set_input_mode (SCIM_ANTHY_MODE_WIDE_LATIN);

    // set typing method
    if (m_factory->m_typing_method == "Kana")
        m_key2kana_tables.set_typing_method (SCIM_ANTHY_TYPING_METHOD_KANA,
                                             NULL);
    else if (m_factory->m_typing_method == "Roma")
        m_key2kana_tables.set_typing_method (SCIM_ANTHY_TYPING_METHOD_ROMAJI,
                                             m_factory->m_custom_romaji_table);
    else
        m_key2kana_tables.set_typing_method (SCIM_ANTHY_TYPING_METHOD_ROMAJI,
                                             m_factory->m_custom_romaji_table);

    // set conversion mode
    if (m_factory->m_conversion_mode == "MultiSeg")
        m_conv_mode = SCIM_ANTHY_CONVERSION_MULTI_SEGMENT;
    else if (m_factory->m_conversion_mode == "SingleSeg")
        m_conv_mode = SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT;
    else if (m_factory->m_conversion_mode == "CAYT_MultiSeg")
        m_conv_mode = SCIM_ANTHY_CONVERSION_MULTI_SEGMENT_IMMEDIATE;
    else if (m_factory->m_conversion_mode == "CAYT_SingleSeg")
        m_conv_mode = SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE;

    // set period style
    if (m_factory->m_period_style == "WideLatin") {
        m_key2kana_tables.set_comma_style  (SCIM_ANTHY_COMMA_WIDE);
        m_key2kana_tables.set_period_style (SCIM_ANTHY_PERIOD_WIDE);
    } else if (m_factory->m_period_style == "Latin") {
        m_key2kana_tables.set_comma_style  (SCIM_ANTHY_COMMA_HALF);
        m_key2kana_tables.set_period_style (SCIM_ANTHY_PERIOD_HALF);
    } else if (m_factory->m_period_style == "Japanese") {
        m_key2kana_tables.set_comma_style  (SCIM_ANTHY_COMMA_JAPANESE);
        m_key2kana_tables.set_period_style (SCIM_ANTHY_PERIOD_JAPANESE);
    } else if (m_factory->m_period_style == "WideLatin_Japanese") {
        m_key2kana_tables.set_comma_style  (SCIM_ANTHY_COMMA_WIDE);
        m_key2kana_tables.set_period_style (SCIM_ANTHY_PERIOD_JAPANESE);
    } else {
        m_key2kana_tables.set_comma_style  (SCIM_ANTHY_COMMA_JAPANESE);
        m_key2kana_tables.set_period_style (SCIM_ANTHY_PERIOD_JAPANESE);
    }

    // set ten key type
    if (m_factory->m_ten_key_type == "Half")
        m_preedit.set_ten_key_type (SCIM_ANTHY_TEN_KEY_HALF);
    else if (m_factory->m_ten_key_type == "Wide")
        m_preedit.set_ten_key_type (SCIM_ANTHY_TEN_KEY_WIDE);
    else 
        m_preedit.set_ten_key_type (SCIM_ANTHY_TEN_KEY_FOLLOW_MODE);

    // set auto convert
    m_preedit.set_auto_convert (m_factory->m_auto_convert);

    // setup toolbar
    m_properties.clear ();
    install_properties ();
}

bool
AnthyInstance::is_single_segment (void)
{
    if (m_conv_mode == SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT ||
        m_conv_mode == SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE)
        return true;
    else
        return false;
}

bool
AnthyInstance::is_realtime_conversion (void)
{
    if (m_conv_mode == SCIM_ANTHY_CONVERSION_MULTI_SEGMENT_IMMEDIATE ||
        m_conv_mode == SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE)
        return true;
    else
        return false;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
