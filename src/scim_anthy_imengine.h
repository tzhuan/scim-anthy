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

#ifndef __SCIM_ANTHY_IMENGINE_H__
#define __SCIM_ANTHY_IMENGINE_H__

#define Uses_SCIM_ICONV
#include <anthy/anthy.h>
#include <scim.h>
#include "scim_anthy_preedit.h"
using namespace scim;

class AnthyFactory : public IMEngineFactoryBase
{
    String m_uuid;

    IConvert m_iconv;
    friend class AnthyInstance;

    /* config */
    ConfigPointer  m_config;
    Connection     m_reload_signal_connection;

    /* for preferece */
    String       m_typing_method;
    String       m_period_style;
    String       m_space_type;
    bool         m_auto_convert;
    String       m_dict_admin_command;
    String       m_add_word_command;
    bool         m_show_input_mode_label;
    bool         m_show_typing_method_label;
    bool         m_show_period_style_label;
    bool         m_show_dict_label;
    bool         m_show_dict_admin_label;
    bool         m_show_add_word_label;

    /* for key bindings */
    KeyEventList m_commit_keys;
    KeyEventList m_convert_keys;
    KeyEventList m_cancel_keys;

    KeyEventList m_backspace_keys;
    KeyEventList m_delete_keys;

    KeyEventList m_move_caret_first_keys;
    KeyEventList m_move_caret_last_keys;
    KeyEventList m_move_caret_forward_keys;
    KeyEventList m_move_caret_backward_keys;

    KeyEventList m_select_first_segment_keys;
    KeyEventList m_select_last_segment_keys;
    KeyEventList m_select_next_segment_keys;
    KeyEventList m_select_prev_segment_keys;
    KeyEventList m_shrink_segment_keys;
    KeyEventList m_expand_segment_keys;
    KeyEventList m_commit_first_segment_keys;
    KeyEventList m_commit_selected_segment_keys;

    KeyEventList m_next_candidate_keys;
    KeyEventList m_prev_candidate_keys;
    KeyEventList m_candidates_page_up_keys;
    KeyEventList m_candidates_page_down_keys;

    KeyEventList m_conv_to_hiragana_keys;
    KeyEventList m_conv_to_katakana_keys;
    KeyEventList m_conv_to_half_katakana_keys;
    KeyEventList m_conv_to_latin_keys;
    KeyEventList m_conv_to_wide_latin_keys;

    KeyEventList m_latin_mode_keys;
    KeyEventList m_wide_latin_mode_keys;
    KeyEventList m_circle_kana_mode_keys;

public:
    AnthyFactory (const String &lang,
                  const String &uuid,
                  const ConfigPointer &config);
    virtual ~AnthyFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);

private:
    void reload_config (const ConfigPointer &config);
};

class AnthyInstance : public IMEngineInstanceBase
{
    AnthyFactory       *m_factory;

    KeyEvent            m_prev_key;

    /* for preedit */
    Preedit             m_preedit;

    /* for candidates window */
    CommonLookupTable   m_lookup_table;
    bool                m_show_lookup_table;

    /* for toggling latin and wide latin */
    InputMode           m_prev_input_mode;

    /* for toolbar */
    PropertyList        m_properties;

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
    void   install_properties                 (void);
    void   set_input_mode                     (InputMode mode);
    void   set_typing_method                  (TypingMethod method); /* FIXME! */
    void   set_period_style                   (PeriodStyle period,
                                               CommaStyle comma);
    bool   is_selecting_candidates            (void);
    bool   convert_kana                       (CandidateType type);

    /* processing key event */
    bool   process_key_event_lookup_keybind   (const KeyEvent &key);
    bool   process_key_event_without_preedit  (const KeyEvent &key);
    bool   process_key_event_with_preedit     (const KeyEvent &key);
    bool   process_key_event_with_candidate   (const KeyEvent &key);
    bool   process_remaining_key_event        (const KeyEvent &key);

    /* actinos */
    bool   action_convert                     (void);
    bool   action_revert                      (void);
    bool   action_commit                      (void);

    bool   action_move_caret_backward         (void);
    bool   action_move_caret_forward          (void);
    bool   action_move_caret_first            (void);
    bool   action_move_caret_last             (void);

    bool   action_back                        (void);
    bool   action_delete                      (void);

    bool   action_select_prev_segment         (void);
    bool   action_select_next_segment         (void);
    bool   action_select_first_segment        (void);
    bool   action_select_last_segment         (void);
    bool   action_shrink_segment              (void);
    bool   action_expand_segment              (void);
    bool   action_commit_first_segment        (void);
    bool   action_commit_selected_segment     (void);

    bool   action_select_next_candidate       (void);
    bool   action_select_prev_candidate       (void);
    bool   action_candidates_page_up          (void);
    bool   action_candidates_page_down        (void);

    bool   action_convert_to_hiragana         (void);
    bool   action_convert_to_katakana         (void);
    bool   action_convert_to_half_katakana    (void);
    bool   action_convert_to_latin            (void);
    bool   action_convert_to_wide_latin       (void);

    bool   action_circle_input_mode           (void);
    bool   action_circle_typing_method        (void);
    bool   action_circle_kana_mode            (void);
    bool   action_toggle_latin_mode           (void);
    bool   action_toggle_wide_latin_mode      (void);

    void   action_add_word                  (void);
    void   action_launch_dict_admin_tool      (void);
    /*
    void   actoin_regist_word                 (void);
    */

    /* utility */
    bool   match_key_event (const KeyEventList &keys, const KeyEvent &key) const;
};
#endif /* __SCIM_ANTHY_IMENGINE_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
