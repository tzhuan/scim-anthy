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

#ifndef __SCIM_ANTHY_PREFS_H__
#define __SCIM_ANTHY_PREFS_H__

/* config keys */
#define SCIM_ANTHY_CONFIG_ON_KEY                      "/IMEngine/Anthy/OnKey"
#define SCIM_ANTHY_CONFIG_USE_KANA                    "/IMEngine/Anthy/UseKana"
#define SCIM_ANTHY_CONFIG_TYPING_METHOD               "/IMEngine/Anthy/TypingMethod"
#define SCIM_ANTHY_CONFIG_PERIOD_STYLE                "/IMEngine/Anthy/PeriodStyle"
#define SCIM_ANTHY_CONFIG_SPACE_TYPE                  "/IMEngine/Anthy/SpaceType"
#define SCIM_ANTHY_CONFIG_TEN_KEY_TYPE                "/IMEngine/Anthy/TenKeyType"
#define SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD      "/IMEngine/Anthy/AutoConvertOnPeriod"
#define SCIM_ANTHY_CONFIG_CLOSE_CAND_WIN_ON_SELECT    "/IMEngine/Anthy/CloseCandWinOnSelect"
#define SCIM_ANTHY_CONFIG_LEARN_ON_MANUAL_COMMIT      "/IMEngine/Anthy/LearnOnManualCommit"
#define SCIM_ANTHY_CONFIG_LEARN_ON_AUTO_COMMIT        "/IMEngine/Anthy/LearnOnAutoCommit"
#define SCIM_ANTHY_CONFIG_ROMAJI_HALF_SYMBOL          "/IMEngine/Anthy/RomajiHalfSymbol"
#define SCIM_ANTHY_CONFIG_ROMAJI_HALF_NUMBER          "/IMEngine/Anthy/RomajiHalfNumber"
#define SCIM_ANTHY_CONFIG_ROMAJI_ALLOW_SPLIT          "/IMEngine/Anthy/RomajiAllowSplit"
#define SCIM_ANTHY_CONFIG_DICT_ADMIN_COMMAND          "/IMEngine/Anthy/DictAdminCommand"
#define SCIM_ANTHY_CONFIG_ADD_WORD_COMMAND            "/IMEngine/Anthy/AddWordCommand"

#define SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL       "/IMEngine/Anthy/ShowInputModeLabel"
#define SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL    "/IMEngine/Anthy/ShowTypingMethodLabel"
#define SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL     "/IMEngine/Anthy/ShowPeriodStyleLabel"
#define SCIM_ANTHY_CONFIG_SHOW_DICT_LABEL             "/IMEngine/Anthy/ShowDictLabel"
#define SCIM_ANTHY_CONFIG_SHOW_DICT_ADMIN_LABEL       "/IMEngine/Anthy/ShowDictAdminLabel"
#define SCIM_ANTHY_CONFIG_SHOW_ADD_WORD_LABEL         "/IMEngine/Anthy/ShowAddWordLabel"

#define SCIM_ANTHY_CONFIG_KEY_THEME                   "/IMEngine/Anthy/KeyTheme"
#define SCIM_ANTHY_CONFIG_ROMAJI_THEME                "/IMEngine/Anthy/RomajiTheme"
#define SCIM_ANTHY_CONFIG_COLOR_THEME                 "/IMEngine/Anthy/ColorTheme"

#define SCIM_ANTHY_CONFIG_COMMIT_KEY                  "/IMEngine/Anthy/CommitKey"
#define SCIM_ANTHY_CONFIG_COMMIT_REVERSE_LEARN_KEY    "/IMEngine/Anthy/CommitReverseLearnKey"
#define SCIM_ANTHY_CONFIG_CONVERT_KEY                 "/IMEngine/Anthy/ConvertKey"
#define SCIM_ANTHY_CONFIG_CANCEL_KEY                  "/IMEngine/Anthy/CancelKey"

#define SCIM_ANTHY_CONFIG_BACKSPACE_KEY               "/IMEngine/Anthy/BackSpaceKey"
#define SCIM_ANTHY_CONFIG_DELETE_KEY                  "/IMEngine/Anthy/DeleteKey"

#define SCIM_ANTHY_CONFIG_INSERT_SPACE_KEY            "/IMEngine/Anthy/InsertSpaceKey"
#define SCIM_ANTHY_CONFIG_INSERT_ALT_SPACE_KEY        "/IMEngine/Anthy/InsertAltSpaceKey"
#define SCIM_ANTHY_CONFIG_INSERT_HALF_SPACE_KEY       "/IMEngine/Anthy/InsertHalfSpaceKey"
#define SCIM_ANTHY_CONFIG_INSERT_WIDE_SPACE_KEY       "/IMEngine/Anthy/InsertWideSpaceKey"

#define SCIM_ANTHY_CONFIG_MOVE_CARET_FIRST_KEY        "/IMEngine/Anthy/MoveCaretFirstKey"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_LAST_KEY         "/IMEngine/Anthy/MoveCaretLastKey"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_FORWARD_KEY      "/IMEngine/Anthy/MoveCaretForwardKey"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_BACKWARD_KEY     "/IMEngine/Anthy/MoveCaretBackwardKey"

#define SCIM_ANTHY_CONFIG_SELECT_FIRST_SEGMENT_KEY    "/IMEngine/Anthy/SelectFirstSegmentKey"
#define SCIM_ANTHY_CONFIG_SELECT_LAST_SEGMENT_KEY     "/IMEngine/Anthy/SelectLastSegmentKey"
#define SCIM_ANTHY_CONFIG_SELECT_NEXT_SEGMENT_KEY     "/IMEngine/Anthy/SelectNextSegmentKey"
#define SCIM_ANTHY_CONFIG_SELECT_PREV_SEGMENT_KEY     "/IMEngine/Anthy/SelectPrevSegmentKey"
#define SCIM_ANTHY_CONFIG_SHRINK_SEGMENT_KEY          "/IMEngine/Anthy/ShrinkSegmentKey"
#define SCIM_ANTHY_CONFIG_EXPAND_SEGMENT_KEY          "/IMEngine/Anthy/ExpandSegmentKey"
#define SCIM_ANTHY_CONFIG_COMMIT_FIRST_SEGMENT_KEY    "/IMEngine/Anthy/CommitFirstSegmentKey"
#define SCIM_ANTHY_CONFIG_COMMIT_SELECTED_SEGMENT_KEY "/IMEngine/Anthy/CommitSelectedSegmentKey"
#define SCIM_ANTHY_CONFIG_COMMIT_FIRST_SEGMENT_REVERSE_LEARN_KEY    "/IMEngine/Anthy/CommitFirstSegmentReverseLearnKey"
#define SCIM_ANTHY_CONFIG_COMMIT_SELECTED_SEGMENT_REVERSE_LEARN_KEY "/IMEngine/Anthy/CommitSelectedSegmentReverseLearnKey"

#define SCIM_ANTHY_CONFIG_SELECT_FIRST_CANDIDATE_KEY  "/IMEngine/Anthy/SelectFirstCandidateKey"
#define SCIM_ANTHY_CONFIG_SELECT_LAST_CANDIDATE_KEY   "/IMEngine/Anthy/SelectLastCandidateKey"
#define SCIM_ANTHY_CONFIG_SELECT_NEXT_CANDIDATE_KEY   "/IMEngine/Anthy/SelectNextCandidateKey"
#define SCIM_ANTHY_CONFIG_SELECT_PREV_CANDIDATE_KEY   "/IMEngine/Anthy/SelectPrevCandidateKey"
#define SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_UP_KEY      "/IMEngine/Anthy/CandidatesPageUpKey"
#define SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_DOWN_KEY    "/IMEngine/Anthy/CandidatesPageDownKey"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_1_KEY      "/IMEngine/Anthy/SelectCandidates1Key"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_2_KEY      "/IMEngine/Anthy/SelectCandidates2Key"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_3_KEY      "/IMEngine/Anthy/SelectCandidates3Key"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_4_KEY      "/IMEngine/Anthy/SelectCandidates4Key"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_5_KEY      "/IMEngine/Anthy/SelectCandidates5Key"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_6_KEY      "/IMEngine/Anthy/SelectCandidates6Key"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_7_KEY      "/IMEngine/Anthy/SelectCandidates7Key"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_8_KEY      "/IMEngine/Anthy/SelectCandidates8Key"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_9_KEY      "/IMEngine/Anthy/SelectCandidates9Key"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_10_KEY     "/IMEngine/Anthy/SelectCandidates10Key"

#define SCIM_ANTHY_CONFIG_CONV_TO_HIRAGANA_KEY        "/IMEngine/Anthy/ConvertToHiraganaKey"
#define SCIM_ANTHY_CONFIG_CONV_TO_KATAKANA_KEY        "/IMEngine/Anthy/ConvertToKatakanaKey"
#define SCIM_ANTHY_CONFIG_CONV_TO_HALF_KEY            "/IMEngine/Anthy/ConvertToHalfKey"
#define SCIM_ANTHY_CONFIG_CONV_TO_HALF_KATAKANA_KEY   "/IMEngine/Anthy/ConvertToHalfKatakanaKey"
#define SCIM_ANTHY_CONFIG_CONV_TO_LATIN_KEY           "/IMEngine/Anthy/ConvertToLatinKey"
#define SCIM_ANTHY_CONFIG_CONV_TO_WIDE_LATIN_KEY      "/IMEngine/Anthy/ConvertToWideLatinKey"

#define SCIM_ANTHY_CONFIG_LATIN_MODE_KEY              "/IMEngine/Anthy/LatinModeKey"
#define SCIM_ANTHY_CONFIG_WIDE_LATIN_MODE_KEY         "/IMEngine/Anthy/WideLatinModeKey"
#define SCIM_ANTHY_CONFIG_CIRCLE_KANA_MODE_KEY        "/IMEngine/Anthy/CircleKanaModeKey"
#define SCIM_ANTHY_CONFIG_CIRCLE_TYPING_METHOD_KEY    "/IMEngine/Anthy/CircleTypingMethodKey"
#define SCIM_ANTHY_CONFIG_HIRAGANA_MODE_KEY           "/IMEngine/Anthy/HiraganaModeKey"
#define SCIM_ANTHY_CONFIG_KATAKANA_MODE_KEY           "/IMEngine/Anthy/KatakanaModeKey"

#define SCIM_ANTHY_CONFIG_DICT_ADMIN_KEY              "/IMEngine/Anthy/DictAdminKey"
#define SCIM_ANTHY_CONFIG_ADD_WORD_KEY                "/IMEngine/Anthy/AddWordKey"


/* default config values */
#define SCIM_ANTHY_CONFIG_KEY_THEME_DEFAULT                   "Default"
#define SCIM_ANTHY_CONFIG_ROMAJI_THEME_DEFAULT                "Default"
#define SCIM_ANTHY_CONFIG_COLOR_THEME_DEFAULT                 "Default"

#define SCIM_ANTHY_CONFIG_TYPING_METHOD_DEFAULT               "Roma"
#define SCIM_ANTHY_CONFIG_PERIOD_STYLE_DEFAULT                "Japanese"
#define SCIM_ANTHY_CONFIG_SPACE_TYPE_DEFAULT                  "FollowMode"
#define SCIM_ANTHY_CONFIG_TEN_KEY_TYPE_DEFAULT                "FollowMode"
#define SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD_DEFAULT      false
#define SCIM_ANTHY_CONFIG_CLOSE_CAND_WIN_ON_SELECT_DEFAULT    false
#define SCIM_ANTHY_CONFIG_LEARN_ON_MANUAL_COMMIT_DEFAULT      true
#define SCIM_ANTHY_CONFIG_LEARN_ON_AUTO_COMMIT_DEFAULT        true
#define SCIM_ANTHY_CONFIG_ROMAJI_HALF_SYMBOL_DEFAULT          false
#define SCIM_ANTHY_CONFIG_ROMAJI_HALF_NUMBER_DEFAULT          false
#define SCIM_ANTHY_CONFIG_ROMAJI_ALLOW_SPLIT_DEFAULT          false
#define SCIM_ANTHY_CONFIG_DICT_ADMIN_COMMAND_DEFAULT          "kasumi"
#define SCIM_ANTHY_CONFIG_ADD_WORD_COMMAND_DEFAULT            "kasumi --add"

#define SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL_DEFAULT       true
#define SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL_DEFAULT    false
#define SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL_DEFAULT     false
#define SCIM_ANTHY_CONFIG_SHOW_DICT_LABEL_DEFAULT             false
#define SCIM_ANTHY_CONFIG_SHOW_DICT_ADMIN_LABEL_DEFAULT       true
#define SCIM_ANTHY_CONFIG_SHOW_ADD_WORD_LABEL_DEFAULT         true

#define SCIM_ANTHY_CONFIG_COMMIT_KEY_DEFAULT                  "Return,KP_Enter,Control+j,Control+J,Control+m,Control+M"
#define SCIM_ANTHY_CONFIG_COMMIT_REVERSE_LEARN_KEY_DEFAULT    "Shift+Return"
#define SCIM_ANTHY_CONFIG_CONVERT_KEY_DEFAULT                 "space"
#define SCIM_ANTHY_CONFIG_CANCEL_KEY_DEFAULT                  "Escape,Control+g,Control+G"

#define SCIM_ANTHY_CONFIG_BACKSPACE_KEY_DEFAULT               "BackSpace,Control+h,Control+H"
#define SCIM_ANTHY_CONFIG_DELETE_KEY_DEFAULT                  "Delete,Control+d,Control+D"

#define SCIM_ANTHY_CONFIG_INSERT_SPACE_KEY_DEFAULT            "space"
#define SCIM_ANTHY_CONFIG_INSERT_ALT_SPACE_KEY_DEFAULT        "Shift+space"
#define SCIM_ANTHY_CONFIG_INSERT_HALF_SPACE_KEY_DEFAULT       " "
#define SCIM_ANTHY_CONFIG_INSERT_WIDE_SPACE_KEY_DEFAULT       " "

#define SCIM_ANTHY_CONFIG_MOVE_CARET_FIRST_KEY_DEFAULT        "Control+a,Control+A,Home"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_LAST_KEY_DEFAULT         "Control+e,Control+E,End"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_FORWARD_KEY_DEFAULT      "Right,Control+f,Control+F"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_BACKWARD_KEY_DEFAULT     "Left,Control+b,Control+B"

#define SCIM_ANTHY_CONFIG_SELECT_FIRST_SEGMENT_KEY_DEFAULT    "Control+a,Control+A,Home"
#define SCIM_ANTHY_CONFIG_SELECT_LAST_SEGMENT_KEY_DEFAULT     "Control+e,Control+E,End"
#define SCIM_ANTHY_CONFIG_SELECT_NEXT_SEGMENT_KEY_DEFAULT     "Right,Control+f,Control+F"
#define SCIM_ANTHY_CONFIG_SELECT_PREV_SEGMENT_KEY_DEFAULT     "Left,Control+b,Control+B"
#define SCIM_ANTHY_CONFIG_SHRINK_SEGMENT_KEY_DEFAULT          "Shift+Left,Control+i,Control+I"
#define SCIM_ANTHY_CONFIG_EXPAND_SEGMENT_KEY_DEFAULT          "Shift+Right,Control+o,Control+O"
#define SCIM_ANTHY_CONFIG_COMMIT_FIRST_SEGMENT_KEY_DEFAULT    "Shift+Down"
#define SCIM_ANTHY_CONFIG_COMMIT_SELECTED_SEGMENT_KEY_DEFAULT "Control+Down"
#define SCIM_ANTHY_CONFIG_COMMIT_FIRST_SEGMENT_REVERSE_LEARN_KEY_DEFAULT    ""
#define SCIM_ANTHY_CONFIG_COMMIT_SELECTED_SEGMENT_REVERSE_LEARN_KEY_DEFAULT ""

#define SCIM_ANTHY_CONFIG_SELECT_FIRST_CANDIDATE_KEY_DEFAULT  "Home"
#define SCIM_ANTHY_CONFIG_SELECT_LAST_CANDIDATE_KEY_DEFAULT   "End"
#define SCIM_ANTHY_CONFIG_SELECT_NEXT_CANDIDATE_KEY_DEFAULT   "space,KP_Space,Down,KP_Add,Control+n,Control+N"
#define SCIM_ANTHY_CONFIG_SELECT_PREV_CANDIDATE_KEY_DEFAULT   "Up,KP_Subtract,Control+p,Control+P"
#define SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_UP_KEY_DEFAULT      "Page_Up"
#define SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_DOWN_KEY_DEFAULT    "Page_Down,KP_Tab"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_1_KEY_DEFAULT      "1"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_2_KEY_DEFAULT      "2"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_3_KEY_DEFAULT      "3"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_4_KEY_DEFAULT      "4"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_5_KEY_DEFAULT      "5"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_6_KEY_DEFAULT      "6"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_7_KEY_DEFAULT      "7"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_8_KEY_DEFAULT      "8"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_9_KEY_DEFAULT      "9"
#define SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_10_KEY_DEFAULT     "0"

#define SCIM_ANTHY_CONFIG_CONV_TO_HIRAGANA_KEY_DEFAULT        "F6"
#define SCIM_ANTHY_CONFIG_CONV_TO_KATAKANA_KEY_DEFAULT        "F7"
#define SCIM_ANTHY_CONFIG_CONV_TO_HALF_KEY_DEFAULT            "F8"
#define SCIM_ANTHY_CONFIG_CONV_TO_HALF_KATAKANA_KEY_DEFAULT   "Shift+F8"
#define SCIM_ANTHY_CONFIG_CONV_TO_LATIN_KEY_DEFAULT           "F10"
#define SCIM_ANTHY_CONFIG_CONV_TO_WIDE_LATIN_KEY_DEFAULT      "F9"

#define SCIM_ANTHY_CONFIG_DICT_ADMIN_KEY_DEFAULT              "F11"
#define SCIM_ANTHY_CONFIG_ADD_WORD_KEY_DEFAULT                "F12"

#define SCIM_ANTHY_CONFIG_LATIN_MODE_KEY_DEFAULT              "Zenkaku_Hankaku,Control+j,Control+J,Control+comma,Control+less,Muhenkan,Henkan"
#define SCIM_ANTHY_CONFIG_WIDE_LATIN_MODE_KEY_DEFAULT         ""
#define SCIM_ANTHY_CONFIG_CIRCLE_KANA_MODE_KEY_DEFAULT        "Control+period,Control+greater,Hiragana_Katakana"
#define SCIM_ANTHY_CONFIG_CIRCLE_TYPING_METHOD_KEY_DEFAULT    "Alt+Romaji"
#define SCIM_ANTHY_CONFIG_HIRAGANA_MODE_KEY_DEFAULT           "Hiragana_Katakana"
#define SCIM_ANTHY_CONFIG_KATAKANA_MODE_KEY_DEFAULT           "Shift+Hiragana_Katakana"


#ifdef SCIM_ANTHY_USE_GTK

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif /* HAVE_CONFIG_H */

#include <scim.h>

using namespace scim;

namespace scim_anthy {
struct BoolConfigData
{
    const char *key;
    bool        value;
    bool        default_value;
    const char *label;
    const char *title;
    const char *tooltip;
    void       *widget;
    bool        changed;
};

struct StringConfigData
{
    const char *key;
    String      value;
    String      default_value;
    const char *label;
    const char *title;
    const char *tooltip;
    void       *widget;
    bool        changed;
};

extern BoolConfigData   config_bool_common [];
extern StringConfigData config_string_common [];
extern StringConfigData config_keyboards_edit [];
extern StringConfigData config_keyboards_caret [];
extern StringConfigData config_keyboards_segments [];
extern StringConfigData config_keyboards_candidates [];
extern StringConfigData config_keyboards_direct_select [];
extern StringConfigData config_keyboards_converting [];
extern StringConfigData config_keyboards_mode [];
extern StringConfigData config_keyboards_dict [];
extern StringConfigData config_keyboards_reverse_learning [];

}

#endif /* SCIM_ANTHY_USE_GTK */


#endif /* __SCIM_ANTHY_PREFS_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
