/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include "scim_anthy_imengine.h"
#include "scim_anthy_prefs.h"

#ifdef HAVE_GETTEXT
  #include <libintl.h>
  #define _(String) dgettext(GETTEXT_PACKAGE,String)
  #define N_(String) (String)
#else
  #define _(String) (String)
  #define N_(String) (String)
  #define bindtextdomain(Package,Directory)
  #define textdomain(domain)
  #define bind_textdomain_codeset(domain,codeset)
#endif

#define scim_module_init anthy_LTX_scim_module_init
#define scim_module_exit anthy_LTX_scim_module_exit
#define scim_imengine_module_init anthy_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory anthy_LTX_scim_imengine_module_create_factory

#define SCIM_CONFIG_IMENGINE_ANTHY_UUID     "/IMEngine/Anthy/UUID-"

#define SCIM_PROP_PREFIX                    "/IMEngine/Anthy"
#define SCIM_PROP_INPUT_MODE                "/IMEngine/Anthy/InputMode"
#define SCIM_PROP_INPUT_MODE_HIRAGANA       "/IMEngine/Anthy/InputMode/Hiragana"
#define SCIM_PROP_INPUT_MODE_KATAKANA       "/IMEngine/Anthy/InputMode/Katakana"
#define SCIM_PROP_INPUT_MODE_HALF_KATAKANA  "/IMEngine/Anthy/InputMode/HalfKatakana"
#define SCIM_PROP_INPUT_MODE_LATIN          "/IMEngine/Anthy/InputMode/Latin"
#define SCIM_PROP_INPUT_MODE_WIDE_LATIN     "/IMEngine/Anthy/InputMode/WideLatin"

#define SCIM_PROP_TYPING_METHOD             "/IMEngine/Anthy/TypingMethod"
#define SCIM_PROP_TYPING_METHOD_ROMAKANA    "/IMEngine/Anthy/TypingMethod/RomaKana"
#define SCIM_PROP_TYPING_METHOD_KANA        "/IMEngine/Anthy/TypingMethod/Kana"

#define SCIM_PROP_PERIOD_STYLE              "/IMEngine/Anthy/PeriodType"
#define SCIM_PROP_PERIOD_STYLE_JAPANESE     "/IMEngine/Anthy/PeriodType/Japanese"
#define SCIM_PROP_PERIOD_STYLE_WIDE_LATIN   "/IMEngine/Anthy/PeriodType/WideRatin"
#define SCIM_PROP_PERIOD_STYLE_LATIN        "/IMEngine/Anthy/PeriodType/Ratin"

#ifndef SCIM_ANTHY_ICON_FILE
    #define SCIM_ANTHY_ICON_FILE           (SCIM_ICONDIR"/scim-anthy.png")
#endif

// first = name, second = lang
static KeyEvent __anthy_on_key;

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
      m_config (0),
      m_typing_method (SCIM_ANTHY_CONFIG_TYPING_METHOD_DEFAULT),
      m_period_style (SCIM_ANTHY_CONFIG_PERIOD_STYLE_DEFAULT),
      m_auto_convert (SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD_DEFAULT),
      m_show_input_mode_label (SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL_DEFAULT),
      m_show_typing_method_label (SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL_DEFAULT),
      m_show_period_style_label (SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL_DEFAULT)
{
    SCIM_DEBUG_IMENGINE(1) << "Create Anthy Factory :\n";
    SCIM_DEBUG_IMENGINE(1) << "  Lang : " << lang << "\n";
    SCIM_DEBUG_IMENGINE(1) << "  UUID : " << uuid << "\n";

    if (lang.length () >= 2)
        set_languages (lang);

    if (!m_iconv.set_encoding ("EUC-JP"))
        return;

    /* config */
    reload_config (config);
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

AnthyInstance::AnthyInstance (AnthyFactory   *factory,
                              const String   &encoding,
                              int             id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory(factory),
      m_prev_key (0,0),
      m_show_lookup_table (false),
      m_prev_input_mode (MODE_HIRAGANA)
{
    SCIM_DEBUG_IMENGINE(1) << "Create Anthy Instance : ";

    // set input mode
    if (factory->m_typing_method == "Kana")
        m_preedit.set_typing_method (METHOD_KANA);
    else
        m_preedit.set_typing_method (METHOD_ROMAKANA);

    // set typing method
    if (factory->m_period_style == "WideLatin")
        m_preedit.set_period_style (PERIOD_WIDE_LATIN);
    else if (factory->m_period_style == "Latin")
        m_preedit.set_period_style (PERIOD_LATIN);
    else
        m_preedit.set_period_style (PERIOD_JAPANESE);

    // set auto convert
    m_preedit.set_auto_convert (factory->m_auto_convert);
}

void
AnthyFactory::reload_config (const ConfigPointer &config)
{
    m_reload_signal_connection.disconnect ();

    if (config) {
        String str;

#if 1 // FIXME!
        String on_key
            = config->read (SCIM_ANTHY_CONFIG_ON_KEY,
                            String ("Shift+space"));
        if (!scim_string_to_key (__anthy_on_key, on_key))
            __anthy_on_key = KeyEvent (SCIM_KEY_space, SCIM_KEY_ShiftMask);
#endif

        m_typing_method
            = config->read (SCIM_ANTHY_CONFIG_TYPING_METHOD,
                            m_typing_method);
        m_period_style
            = config->read (SCIM_ANTHY_CONFIG_PERIOD_STYLE,
                            m_period_style);
        m_auto_convert
            = config->read (SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD,
                            m_auto_convert);
        m_show_input_mode_label
            = config->read (SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL,
                            m_show_input_mode_label);
        m_show_typing_method_label
            = config->read (SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL,
                            m_show_typing_method_label);
        m_show_period_style_label
            = config->read (SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL,
                            m_show_period_style_label);

        // edit keys
        str = config->read (String (SCIM_ANTHY_CONFIG_COMMIT_KEY),
                            String (SCIM_ANTHY_CONFIG_COMMIT_KEY_DEFAULT));
        scim_string_to_key_list (m_commit_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_CONVERT_KEY),
                            String (SCIM_ANTHY_CONFIG_CONVERT_KEY_DEFAULT));
        scim_string_to_key_list (m_convert_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_CANCEL_KEY),
                            String (SCIM_ANTHY_CONFIG_CANCEL_KEY_DEFAULT));
        scim_string_to_key_list (m_cancel_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_BACKSPACE_KEY),
                            String (SCIM_ANTHY_CONFIG_BACKSPACE_KEY_DEFAULT));
        scim_string_to_key_list (m_backspace_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_DELETE_KEY),
                            String (SCIM_ANTHY_CONFIG_DELETE_KEY_DEFAULT));
        scim_string_to_key_list (m_delete_keys, str);

        // caret keys
        str = config->read (String (SCIM_ANTHY_CONFIG_MOVE_CARET_FIRST_KEY),
                            String (SCIM_ANTHY_CONFIG_MOVE_CARET_FIRST_KEY_DEFAULT));
        scim_string_to_key_list (m_move_caret_first_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_MOVE_CARET_LAST_KEY),
                            String (SCIM_ANTHY_CONFIG_MOVE_CARET_LAST_KEY_DEFAULT));
        scim_string_to_key_list (m_move_caret_last_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_MOVE_CARET_FORWARD_KEY),
                            String (SCIM_ANTHY_CONFIG_MOVE_CARET_FORWARD_KEY_DEFAULT));
        scim_string_to_key_list (m_move_caret_forward_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_MOVE_CARET_BACKWARD_KEY),
                            String (SCIM_ANTHY_CONFIG_MOVE_CARET_BACKWARD_KEY_DEFAULT));
        scim_string_to_key_list (m_move_caret_backward_keys, str);

        // segment keys
        str = config->read (String (SCIM_ANTHY_CONFIG_SELECT_FIRST_SEGMENT_KEY),
                            String (SCIM_ANTHY_CONFIG_SELECT_FIRST_SEGMENT_KEY_DEFAULT));
        scim_string_to_key_list (m_select_first_segment_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_SELECT_LAST_SEGMENT_KEY),
                            String (SCIM_ANTHY_CONFIG_SELECT_LAST_SEGMENT_KEY_DEFAULT));
        scim_string_to_key_list (m_select_last_segment_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_SELECT_NEXT_SEGMENT_KEY),
                            String (SCIM_ANTHY_CONFIG_SELECT_NEXT_SEGMENT_KEY_DEFAULT));
        scim_string_to_key_list (m_select_next_segment_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_SELECT_PREV_SEGMENT_KEY),
                            String (SCIM_ANTHY_CONFIG_SELECT_PREV_SEGMENT_KEY_DEFAULT));
        scim_string_to_key_list (m_select_prev_segment_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_SHRINK_SEGMENT_KEY),
                            String (SCIM_ANTHY_CONFIG_SHRINK_SEGMENT_KEY_DEFAULT));
        scim_string_to_key_list (m_shrink_segment_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_EXPAND_SEGMENT_KEY),
                            String (SCIM_ANTHY_CONFIG_EXPAND_SEGMENT_KEY_DEFAULT));
        scim_string_to_key_list (m_expand_segment_keys, str);

        // candidates keys
        str = config->read (String (SCIM_ANTHY_CONFIG_SELECT_NEXT_CANDIDATE_KEY),
                            String (SCIM_ANTHY_CONFIG_SELECT_NEXT_CANDIDATE_KEY_DEFAULT));
        scim_string_to_key_list (m_next_candidate_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_SELECT_PREV_CANDIDATE_KEY),
                            String (SCIM_ANTHY_CONFIG_SELECT_PREV_CANDIDATE_KEY_DEFAULT));
        scim_string_to_key_list (m_prev_candidate_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_UP_KEY),
                            String (SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_UP_KEY_DEFAULT));
        scim_string_to_key_list (m_candidates_page_up_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_DOWN_KEY),
                            String (SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_DOWN_KEY_DEFAULT));
        scim_string_to_key_list (m_candidates_page_down_keys, str);

        // convert keys
        str = config->read (String (SCIM_ANTHY_CONFIG_CONV_TO_HIRAGANA_KEY),
                            String (SCIM_ANTHY_CONFIG_CONV_TO_HIRAGANA_KEY_DEFAULT));
        scim_string_to_key_list (m_conv_to_hiragana_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_CONV_TO_KATAKANA_KEY),
                            String (SCIM_ANTHY_CONFIG_CONV_TO_KATAKANA_KEY_DEFAULT));
        scim_string_to_key_list (m_conv_to_katakana_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_CONV_TO_HALF_KATAKANA_KEY),
                            String (SCIM_ANTHY_CONFIG_CONV_TO_HALF_KATAKANA_KEY_DEFAULT));
        scim_string_to_key_list (m_conv_to_half_katakana_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_CONV_TO_LATIN_KEY),
                            String (SCIM_ANTHY_CONFIG_CONV_TO_LATIN_KEY_DEFAULT));
        scim_string_to_key_list (m_conv_to_latin_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_CONV_TO_WIDE_LATIN_KEY),
                            String (SCIM_ANTHY_CONFIG_CONV_TO_WIDE_LATIN_KEY_DEFAULT));
        scim_string_to_key_list (m_conv_to_wide_latin_keys, str);

        // mode keys
        str = config->read (String (SCIM_ANTHY_CONFIG_LATIN_MODE_KEY),
                            String (SCIM_ANTHY_CONFIG_LATIN_MODE_KEY_DEFAULT));
        scim_string_to_key_list (m_latin_mode_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_WIDE_LATIN_MODE_KEY),
                            String (SCIM_ANTHY_CONFIG_WIDE_LATIN_MODE_KEY_DEFAULT));
        scim_string_to_key_list (m_wide_latin_mode_keys, str);

        str = config->read (String (SCIM_ANTHY_CONFIG_CIRCLE_KANA_MODE_KEY),
                            String (SCIM_ANTHY_CONFIG_CIRCLE_KANA_MODE_KEY_DEFAULT));
        scim_string_to_key_list (m_circle_kana_mode_keys, str);
    }

    m_config = config;
    m_reload_signal_connection = m_config->signal_connect_reload (slot (this, &AnthyFactory::reload_config));
}

AnthyInstance::~AnthyInstance ()
{
}

bool
AnthyInstance::process_key_event_lookup_keybind (const KeyEvent& key)
{
    // edit
    if (match_key_event (m_factory->m_commit_keys, key) &&
        action_commit ())
        return true;

    if (match_key_event (m_factory->m_convert_keys, key) &&
        action_convert ())
        return true;

    if (match_key_event (m_factory->m_cancel_keys, key) &&
        action_revert ())
        return true;

    if (match_key_event (m_factory->m_backspace_keys, key) &&
        action_back ())
        return true;

    if (match_key_event (m_factory->m_delete_keys, key) &&
        action_delete ())
        return true;

    // caret
    if (match_key_event (m_factory->m_move_caret_first_keys, key) &&
        action_move_caret_first ())
        return true;

    if (match_key_event (m_factory->m_move_caret_last_keys, key) &&
        action_move_caret_last ())
        return true;

    if (match_key_event (m_factory->m_move_caret_forward_keys, key) &&
        action_move_caret_forward ())
        return true;

    if (match_key_event (m_factory->m_move_caret_backward_keys, key) &&
        action_move_caret_backward ())
        return true;

    // segment
    if (match_key_event (m_factory->m_select_first_segment_keys, key) &&
        action_select_first_segment ())
        return true;

    if (match_key_event (m_factory->m_select_last_segment_keys, key) &&
        action_select_last_segment ())
        return true;

    if (match_key_event (m_factory->m_select_next_segment_keys, key) &&
        action_select_next_segment ())
        return true;

    if (match_key_event (m_factory->m_select_prev_segment_keys, key) &&
        action_select_prev_segment ())
        return true;

    if (match_key_event (m_factory->m_shrink_segment_keys, key) &&
        action_shrink_segment ())
        return true;

    if (match_key_event (m_factory->m_expand_segment_keys, key) &&
        action_expand_segment ())
        return true;

    // candidate
    if (match_key_event (m_factory->m_next_candidate_keys, key) &&
        action_select_next_candidate ())
        return true;

    if (match_key_event (m_factory->m_prev_candidate_keys, key) &&
        action_select_prev_candidate ())
        return true;

    if (match_key_event (m_factory->m_candidates_page_up_keys, key) &&
        action_candidates_page_up ())
        return true;

    if (match_key_event (m_factory->m_candidates_page_down_keys, key) &&
        action_candidates_page_down ())
        return true;

    // convert
    if (match_key_event (m_factory->m_conv_to_hiragana_keys, key) &&
        action_convert_to_hiragana ())
        return true;

    if (match_key_event (m_factory->m_conv_to_katakana_keys, key) &&
        action_convert_to_katakana ())
        return true;

    if (match_key_event (m_factory->m_conv_to_half_katakana_keys, key) &&
        action_convert_to_half_katakana ())
        return true;

    if (match_key_event (m_factory->m_conv_to_latin_keys, key) &&
        action_convert_to_latin ())
        return true;

    if (match_key_event (m_factory->m_conv_to_wide_latin_keys, key) &&
        action_convert_to_wide_latin ())
        return true;

    // mode
    if (match_key_event (m_factory->m_circle_kana_mode_keys, key) &&
        action_circle_kana_mode ())
        return true;

    if (match_key_event (m_factory->m_latin_mode_keys, key) &&
        action_toggle_latin_mode ())
        return true;

    if (match_key_event (m_factory->m_wide_latin_mode_keys, key) &&
        action_toggle_wide_latin_mode ())
        return true;

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
    switch (key.code) {
    case SCIM_KEY_1: 
    case SCIM_KEY_2: 
    case SCIM_KEY_3: 
    case SCIM_KEY_4: 
    case SCIM_KEY_5: 
    case SCIM_KEY_6: 
    case SCIM_KEY_7: 
    case SCIM_KEY_8: 
    case SCIM_KEY_9: 
        select_candidate (key.code - SCIM_KEY_1);
        return true;

    default:
        break;
    }

    return process_remaining_key_event (key);
}

bool
AnthyInstance::process_remaining_key_event (const KeyEvent &key)
{
    if (key.mask & SCIM_KEY_ControlMask ||
        key.mask & SCIM_KEY_Mod1Mask ||
        key.mask & SCIM_KEY_Mod2Mask ||
        key.mask & SCIM_KEY_Mod3Mask ||
        key.mask & SCIM_KEY_Mod4Mask ||
        key.mask & SCIM_KEY_Mod5Mask)
    {
        return false;
    }

    // FIXME!
    if (isprint(key.code) && !isspace(key.code)) {
        // commit old conversion string before update preedit string
        if (m_preedit.is_converting ())
            action_commit ();

        bool need_convert = m_preedit.append (key);

        if (m_preedit.get_input_mode () == MODE_LATIN ||
            m_preedit.get_input_mode () == MODE_WIDE_LATIN)
        {
            action_commit ();
            m_preedit.clear ();
        } else {
            if (need_convert)
                action_convert ();
            show_preedit_string ();
            update_preedit_string (m_preedit.get_string (),
                                   m_preedit.get_attribute_list ());
            update_preedit_caret (m_preedit.get_caret_pos());
        }

        return true;
    }

    return false;
}

bool
AnthyInstance::process_key_event (const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE(2) << "process_key_event.\n";
    KeyEvent newkey;

    newkey.code = key.code;
    newkey.mask = key.mask & (SCIM_KEY_ShiftMask | SCIM_KEY_ControlMask | SCIM_KEY_AltMask | SCIM_KEY_ReleaseMask);

#if 0
    // change input mode
    if (__anthy_on_key.is_key_press ()) {
        trigger_property (SCIM_PROP_INPUT_MODE);
        return true;
    }
#endif

    m_prev_key = key;

    // ignore key release.
    if (key.is_key_release ())
        return false;

    // ignore modifier keys
    if (key.code == SCIM_KEY_Shift_L || key.code == SCIM_KEY_Shift_R ||
        key.code == SCIM_KEY_Control_L || key.code == SCIM_KEY_Control_R ||
        key.code == SCIM_KEY_Alt_L || key.code == SCIM_KEY_Alt_R)
        return false;

    // lookup user defined key binds
    if (process_key_event_lookup_keybind (key))
        return true;

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
AnthyInstance::select_candidate (unsigned int item)
{
    if (!is_selecting_candidates ()) return;

    SCIM_DEBUG_IMENGINE(2) << "select_candidate.\n";
 
    // update lookup table
    m_lookup_table.set_cursor_pos_in_current_page (item);
    update_lookup_table (m_lookup_table);

    // update preedit
    m_preedit.select_candidate(m_lookup_table.get_cursor_pos ());
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos ());
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
    if (!is_selecting_candidates () || !m_lookup_table.get_current_page_start ()) return;

    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_up.\n";

    m_lookup_table.page_up ();

    update_lookup_table (m_lookup_table);
}

void
AnthyInstance::lookup_table_page_down ()
{
    if (!is_selecting_candidates () ||
        m_lookup_table.get_current_page_start () + m_lookup_table.get_current_page_size () >=
        (int) m_lookup_table.number_of_candidates ())
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
}

void
AnthyInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";

    hide_aux_string ();

    if (m_show_lookup_table && is_selecting_candidates ()) {
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
                             _("Half width katakana"), String (""), _("Half width katakana"));
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

            prop = Property (SCIM_PROP_TYPING_METHOD_ROMAKANA,
                             _("Roma"), String (""), _("Roma"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_TYPING_METHOD_KANA,
                             _("Kana"), String (""), _("Kana"));
            m_properties.push_back (prop);
        }

        if (m_factory->m_show_period_style_label) {
            prop = Property (SCIM_PROP_PERIOD_STYLE,
                             "\xE3\x80\x81\xE3\x80\x82", String (""), _("Period style"));
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_PERIOD_STYLE_JAPANESE,
                             "\xE3\x80\x81\xE3\x80\x82", String (""), "\xE3\x80\x81\xE3\x80\x82");
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_PERIOD_STYLE_WIDE_LATIN,
                             "\xEF\xBC\x8C\xEF\xBC\x8E", String (""), "\xEF\xBC\x8C\xEF\xBC\x8E");
            m_properties.push_back (prop);

            prop = Property (SCIM_PROP_PERIOD_STYLE_LATIN,
                             ",.", String (""), ",.");
            m_properties.push_back (prop);
        }
    }

    set_input_mode (m_preedit.get_input_mode ());
    set_typing_method (m_preedit.get_typing_method ());
    set_period_style (m_preedit.get_period_style ());

    register_properties (m_properties);
}

void
AnthyInstance::set_input_mode (InputMode mode)
{
    const char *label = "";

    switch (mode) {
    case MODE_HIRAGANA:
        label = "\xE3\x81\x82";
        break;
    case MODE_KATAKANA:
        label = "\xE3\x82\xA2";
        break;
    case MODE_HALF_KATAKANA:
        label = "\xEF\xBD\xB1";
        break;
    case MODE_LATIN:
        label = "a";
        break;
    case MODE_WIDE_LATIN:
        label = "\xEF\xBD\x81";
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

    if (mode != m_preedit.get_input_mode ())
        m_preedit.set_input_mode (mode);
}

void
AnthyInstance::set_typing_method (TypingMethod method)
{
    const char *label = "";

    switch (method) {
    case METHOD_ROMAKANA:
        label = "\xEF\xBC\xB2";
        break;
    case METHOD_KANA:
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

    if (method != m_preedit.get_typing_method ())
        m_preedit.set_typing_method (method);
}

void
AnthyInstance::set_period_style (PeriodStyle style)
{
    const char *label = "";

    switch (style) {
    case PERIOD_JAPANESE:
        label = "\xE3\x80\x81\xE3\x80\x82";
        break;
    case PERIOD_WIDE_LATIN:
        label = "\xEF\xBC\x8C\xEF\xBC\x8E";
        break;
    case PERIOD_LATIN:
        label = ",.";
        break;
    default:
        break;
    }

    if (label && *label /*&& m_factory->m_show_typing_method_label*/) {
        PropertyList::iterator it = std::find (m_properties.begin (),
                                               m_properties.end (),
                                               SCIM_PROP_PERIOD_STYLE);
        if (it != m_properties.end ()) {
            it->set_label (label);
            update_property (*it);
        }
    }

    if (style != m_preedit.get_period_style ())
        m_preedit.set_period_style (style);
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
    if (m_preedit.is_kana_converting ()) {
        m_preedit.revert ();
        //return false;
    }

    if (!m_preedit.is_preediting ())
        return false;

    if (!m_preedit.is_converting ()) {
        // show conversion string
        m_preedit.flush_pending ();
        m_preedit.convert ();
        update_preedit_string (m_preedit.get_string (),
                               m_preedit.get_attribute_list ());
        update_preedit_caret (m_preedit.get_caret_pos());
    } else {
        // show candidates window
        // FIXME!
        m_preedit.setup_lookup_table(m_lookup_table);
        int page = m_preedit.get_selected_candidate () / m_lookup_table.get_page_size ();
        int pos  = m_preedit.get_selected_candidate () % m_lookup_table.get_page_size ();
        for (int i = 0; i < page; i++)
            m_lookup_table.page_down ();
        if (pos + 1 >= m_lookup_table.get_page_size ()) {
            m_lookup_table.page_down ();
            select_candidate (0);
        } else {
            select_candidate (pos + 1);
        }
        show_lookup_table ();
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

    m_lookup_table.clear ();
    hide_lookup_table ();

    m_preedit.revert ();
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos ());

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
    update_preedit_string (m_preedit.get_string(),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos());

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
    update_preedit_string (m_preedit.get_string(),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos());

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
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos ());

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
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos ());

    return true;
}

bool
AnthyInstance::action_commit (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (m_preedit.is_converting ()) {
        m_preedit.commit ();
        commit_string (m_preedit.get_string ());
    } else {
        m_preedit.flush_pending ();
        commit_string (m_preedit.get_string ());
    }
    m_lookup_table.clear ();
    m_preedit.clear ();
    hide_lookup_table ();
    hide_preedit_string ();

    return true;
}

bool
AnthyInstance::action_back (void)
{
    if (!m_preedit.is_preediting ())
        return false;

    if (m_preedit.is_converting ()) {
        action_revert ();
        return true;
    }

    m_preedit.erase ();

    if (m_preedit.get_length () > 0) {
        update_preedit_string (m_preedit.get_string (),
                               m_preedit.get_attribute_list ());
        update_preedit_caret (m_preedit.get_caret_pos ());
    } else {
        m_preedit.clear();
        m_lookup_table.clear ();
        hide_preedit_string ();
        hide_lookup_table ();
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
        return true;
    }

    m_preedit.erase (false);

    if (m_preedit.get_length () > 0) {
        update_preedit_string (m_preedit.get_string (),
                               m_preedit.get_attribute_list ());
        update_preedit_caret (m_preedit.get_caret_pos ());
    } else {
        m_preedit.clear();
        m_lookup_table.clear ();
        hide_preedit_string ();
        hide_lookup_table ();
    }

    return true;
}

bool
AnthyInstance::action_select_prev_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();

    m_preedit.select_segment(m_preedit.get_selected_segment () - 1);
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos ());

    return true;
}

bool
AnthyInstance::action_select_next_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();

    m_preedit.select_segment(m_preedit.get_selected_segment () + 1);
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos ());

    return true;
}

bool
AnthyInstance::action_select_first_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();

    m_preedit.select_segment(0);
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos ());

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

    m_preedit.select_segment(n - 1);
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos ());

    return true;
}

bool
AnthyInstance::action_shrink_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();

    m_preedit.resize_segment (-1);
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos ());

    return true;
}

bool
AnthyInstance::action_expand_segment (void)
{
    if (!m_preedit.is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();

    m_preedit.resize_segment (1);
    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos ());

    return true;
}

bool
AnthyInstance::action_select_next_candidate (void)
{
    if (!m_preedit.is_converting ())
        return false;

    if (!is_selecting_candidates ())
        action_convert ();

    m_lookup_table.cursor_down ();
    select_candidate (m_lookup_table.get_cursor_pos_in_current_page ());

    return true;
}

bool
AnthyInstance::action_select_prev_candidate (void)
{
    if (!m_preedit.is_converting ()) return false;

    if (!is_selecting_candidates ())
        action_convert ();

    m_lookup_table.cursor_up ();
    select_candidate (m_lookup_table.get_cursor_pos_in_current_page ());

    return true;
}

bool
AnthyInstance::action_candidates_page_up(void)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;

    m_lookup_table.page_up ();
    select_candidate (m_lookup_table.get_cursor_pos_in_current_page ());

    return true;
}

bool
AnthyInstance::action_candidates_page_down (void)
{
    if (!m_preedit.is_converting ()) return false;
    if (!is_selecting_candidates ()) return false;

    m_lookup_table.page_down ();
    select_candidate (m_lookup_table.get_cursor_pos_in_current_page ());

    return true;
}

bool
AnthyInstance::action_circle_input_mode (void)
{
    InputMode mode = MODE_HIRAGANA;

    switch (mode) {
    case MODE_HIRAGANA:
        mode = MODE_KATAKANA;
        break;
    case MODE_KATAKANA:
        mode = MODE_HALF_KATAKANA;
        break;
    case MODE_HALF_KATAKANA:
        mode = MODE_LATIN;
        break;
    case MODE_LATIN:
        mode = MODE_WIDE_LATIN;
        break;
    case MODE_WIDE_LATIN:
        mode = MODE_HIRAGANA;
        break;
    default:
        mode = MODE_HIRAGANA;
        break; 
    }

    set_input_mode (mode);

    return true;
}

bool
AnthyInstance::action_circle_typing_method (void)
{
    TypingMethod method;

    method = m_preedit.get_typing_method();
    if (method == METHOD_KANA)
        method = METHOD_ROMAKANA;
    else
        method = METHOD_KANA;

    set_typing_method (method);

    return true;
}

bool
AnthyInstance::action_circle_kana_mode (void)
{
    InputMode mode;

    if (m_preedit.get_input_mode () == MODE_LATIN ||
        m_preedit.get_input_mode () == MODE_WIDE_LATIN)
    {
        mode = MODE_HIRAGANA;
    } else {
        switch (m_preedit.get_input_mode ()) {
        case MODE_HIRAGANA:
            mode = MODE_KATAKANA;
            break;
        case MODE_KATAKANA:
            mode = MODE_HALF_KATAKANA;
            break;
        case MODE_HALF_KATAKANA:
        default:
            mode = MODE_HIRAGANA;
            break;
        }
    }

    set_input_mode (mode);

    return true;
}

bool
AnthyInstance::action_toggle_latin_mode (void)
{
    if (m_preedit.get_input_mode () == MODE_LATIN ||
        m_preedit.get_input_mode () == MODE_WIDE_LATIN)
    {
        set_input_mode (m_prev_input_mode);
        m_preedit.set_input_mode (m_prev_input_mode);
    } else {
        m_prev_input_mode = m_preedit.get_input_mode ();
        set_input_mode (MODE_LATIN);
        m_preedit.set_input_mode (MODE_LATIN);
    }

    return true;
}

bool
AnthyInstance::action_toggle_wide_latin_mode (void)
{
    if (m_preedit.get_input_mode () == MODE_LATIN ||
        m_preedit.get_input_mode () == MODE_WIDE_LATIN)
    {
        set_input_mode (m_prev_input_mode);
    } else {
        m_prev_input_mode = m_preedit.get_input_mode ();
        set_input_mode (MODE_WIDE_LATIN);
    }

    return true;
}

bool
AnthyInstance::convert_kana (SpecialCandidate type)
{
    if (!m_preedit.is_preediting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();

    if (m_preedit.is_kana_converting ()) {
        m_preedit.convert (type);
    } else if (m_preedit.is_converting ()) {
        m_preedit.select_candidate (type);
    } else {
        m_preedit.convert (type);
    }

    update_preedit_string (m_preedit.get_string (),
                           m_preedit.get_attribute_list ());
    update_preedit_caret (m_preedit.get_caret_pos());

    return true;
}

bool
AnthyInstance::action_convert_to_hiragana (void)
{
    return convert_kana (CANDIDATE_HIRAGANA);
}

bool
AnthyInstance::action_convert_to_katakana (void)
{
    return convert_kana (CANDIDATE_KATAKANA);
}

bool
AnthyInstance::action_convert_to_half_katakana (void)
{
    return convert_kana (CANDIDATE_HALF_KATAKANA);
}

bool
AnthyInstance::action_convert_to_latin (void)
{
    return convert_kana (CANDIDATE_LATIN);
}

bool
AnthyInstance::action_convert_to_wide_latin (void)
{
    return convert_kana (CANDIDATE_WIDE_LATIN);
}

#if 0
void
AnthyInstance::action_regist_word (void)
{
}

void
AnthyInstance::action_launch_dict_admin (void)
{
}
#endif

void
AnthyInstance::trigger_property (const String &property)
{
    String anthy_prop = property.substr (property.find_last_of ('/') + 1);

    SCIM_DEBUG_IMENGINE(2) << "trigger_property : " << property << " - " << anthy_prop << "\n";

    if (property == SCIM_PROP_INPUT_MODE_HIRAGANA) {
        set_input_mode (MODE_HIRAGANA);
    } else if (property == SCIM_PROP_INPUT_MODE_KATAKANA) {
        set_input_mode (MODE_KATAKANA);
    } else if (property == SCIM_PROP_INPUT_MODE_HALF_KATAKANA) {
        set_input_mode (MODE_HALF_KATAKANA);
    } else if (property == SCIM_PROP_INPUT_MODE_LATIN) {
        set_input_mode (MODE_LATIN);
    } else if (property == SCIM_PROP_INPUT_MODE_WIDE_LATIN) {
        set_input_mode (MODE_WIDE_LATIN);
    } else if (property == SCIM_PROP_TYPING_METHOD_ROMAKANA) {
        set_typing_method (METHOD_ROMAKANA);
    } else if (property == SCIM_PROP_TYPING_METHOD_KANA) {
        set_typing_method (METHOD_KANA);
    } else if (property == SCIM_PROP_PERIOD_STYLE_JAPANESE) {
        set_period_style (PERIOD_JAPANESE);
    } else if (property == SCIM_PROP_PERIOD_STYLE_WIDE_LATIN) {
        set_period_style (PERIOD_WIDE_LATIN);
    } else if (property == SCIM_PROP_PERIOD_STYLE_LATIN) {
        set_period_style (PERIOD_LATIN);
    }
}

bool
AnthyInstance::match_key_event (const KeyEventList &keys, const KeyEvent &key) const
{
    KeyEventList::const_iterator kit;

    for (kit = keys.begin (); kit != keys.end (); ++kit) {
        if (key.code == kit->code && key.mask == kit->mask)
            if (!(key.mask & SCIM_KEY_ReleaseMask) ||
                m_prev_key.code == key.code)
            {
                return true;
            }
    }
    return false;
}
