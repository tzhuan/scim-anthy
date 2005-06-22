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
#include "scim_anthy_key2kana_table.h"

using namespace scim;
using namespace scim_anthy;

class AnthyInstance : public IMEngineInstanceBase
{
public:
    AnthyInstance (AnthyFactory   *factory,
                   const String   &encoding,
                   int             id = -1);
    virtual ~AnthyInstance ();

    virtual bool process_key_event            (const KeyEvent& key);
    virtual void move_preedit_caret           (unsigned int pos);
    virtual void select_candidate             (unsigned int item);
    virtual void update_lookup_table_page_size(unsigned int page_size);
    virtual void lookup_table_page_up         (void);
    virtual void lookup_table_page_down       (void);
    virtual void reset                        (void);
    virtual void focus_in                     (void);
    virtual void focus_out                    (void);
    virtual void trigger_property             (const String &property);

    virtual void reload_config                (const ConfigPointer &config);

public:
    /* actinos */
    bool   action_convert                     (void);
    bool   action_revert                      (void);
    bool   action_commit_follow_preference    (void);
    bool   action_commit_reverse_preference   (void);
    bool   action_commit_first_segment        (void);
    bool   action_commit_selected_segment     (void);
    bool   action_commit_first_segment_reverse_preference
                                              (void);
    bool   action_commit_selected_segment_reverse_preference
                                              (void);
    bool   action_back                        (void);
    bool   action_delete                      (void);
    bool   action_insert_space                (void);
    bool   action_insert_alternative_space    (void);
    bool   action_insert_half_space           (void);
    bool   action_insert_wide_space           (void);

    bool   action_move_caret_backward         (void);
    bool   action_move_caret_forward          (void);
    bool   action_move_caret_first            (void);
    bool   action_move_caret_last             (void);

    bool   action_select_prev_segment         (void);
    bool   action_select_next_segment         (void);
    bool   action_select_first_segment        (void);
    bool   action_select_last_segment         (void);
    bool   action_shrink_segment              (void);
    bool   action_expand_segment              (void);

    bool   action_select_first_candidate      (void);
    bool   action_select_last_candidate       (void);
    bool   action_select_next_candidate       (void);
    bool   action_select_prev_candidate       (void);
    bool   action_candidates_page_up          (void);
    bool   action_candidates_page_down        (void);

    bool   action_select_candidate_1          (void);
    bool   action_select_candidate_2          (void);
    bool   action_select_candidate_3          (void);
    bool   action_select_candidate_4          (void);
    bool   action_select_candidate_5          (void);
    bool   action_select_candidate_6          (void);
    bool   action_select_candidate_7          (void);
    bool   action_select_candidate_8          (void);
    bool   action_select_candidate_9          (void);
    bool   action_select_candidate_10         (void);

    bool   action_convert_to_hiragana         (void);
    bool   action_convert_to_katakana         (void);
    bool   action_convert_to_half             (void);
    bool   action_convert_to_half_katakana    (void);
    bool   action_convert_to_latin            (void);
    bool   action_convert_to_wide_latin       (void);

    bool   action_circle_input_mode           (void);
    bool   action_circle_typing_method        (void);
    bool   action_circle_kana_mode            (void);
    bool   action_toggle_latin_mode           (void);
    bool   action_toggle_wide_latin_mode      (void);
    bool   action_hiragana_mode               (void);
    bool   action_katakana_mode               (void);

    bool   action_add_word                    (void);
    bool   action_launch_dict_admin_tool      (void);
    /*
    void   actoin_register_word               (void);
    */

private:
    /* processing key event */
    bool   process_key_event_lookup_keybind   (const KeyEvent &key);
    bool   process_key_event_without_preedit  (const KeyEvent &key);
    bool   process_key_event_with_preedit     (const KeyEvent &key);
    bool   process_key_event_with_candidate   (const KeyEvent &key);
    bool   process_remaining_key_event        (const KeyEvent &key);

    /* utility */
    void   set_preedition                     (void);
    void   install_properties                 (void);
    void   set_input_mode                     (InputMode mode);
    void   set_typing_method                  (TypingMethod method);
    void   set_period_style                   (PeriodStyle period,
                                               CommaStyle comma);
    bool   is_selecting_candidates            (void);
    void   select_candidate_no_direct         (unsigned int item);
    bool   convert_kana                       (CandidateType type);

    bool   action_commit                      (bool learn);
    bool   action_select_candidate            (unsigned int i);

private:
    AnthyFactory         *m_factory;

    KeyEvent              m_prev_key;

    /* for preedit */
    Key2KanaTableSet      m_key2kana_tables;
    Preedit               m_preedit;
    bool                  m_preedit_string_visible;

    /* for candidates window */
    CommonLookupTable     m_lookup_table;
    bool                  m_lookup_table_visible;

    /* for toggling latin and wide latin */
    InputMode             m_prev_input_mode;

    /* for toolbar */
    PropertyList          m_properties;
};
#endif /* __SCIM_ANTHY_IMENGINE_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
