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

#define SCIM_ANTHY_CONFIG_ON_KEY                      "/IMEngine/Anthy/OnKey"
#define SCIM_ANTHY_CONFIG_USE_KANA                    "/IMEngine/Anthy/UseKana"
#define SCIM_ANTHY_CONFIG_TYPING_METHOD               "/IMEngine/Anthy/TypingMethod"
#define SCIM_ANTHY_CONFIG_PERIOD_STYLE                "/IMEngine/Anthy/PeriodStyle"
#define SCIM_ANTHY_CONFIG_SPACE_TYPE                  "/IMEngine/Anthy/SpaceType"
#define SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD      "/IMEngine/Anthy/AutoConvertOnPeriod"
#define SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL       "/IMEngine/Anthy/ShowInputModeLabel"
#define SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL    "/IMEngine/Anthy/ShowTypingMethodLabel"
#define SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL     "/IMEngine/Anthy/ShowPeriodStyleLabel"

#define SCIM_ANTHY_CONFIG_COMMIT_KEY                  "/IMEngine/Anthy/CommitKey"
#define SCIM_ANTHY_CONFIG_CONVERT_KEY                 "/IMEngine/Anthy/ConvertKey"
#define SCIM_ANTHY_CONFIG_CANCEL_KEY                  "/IMEngine/Anthy/CancelKey"

#define SCIM_ANTHY_CONFIG_BACKSPACE_KEY               "/IMEngine/Anthy/BackSpaceKey"
#define SCIM_ANTHY_CONFIG_DELETE_KEY                  "/IMEngine/Anthy/DeleteKey"

#define SCIM_ANTHY_CONFIG_MOVE_CARET_FIRST_KEY        "/IMEngine/Anthy/MoveCaretFirstKey"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_LAST_KEY         "/IMEngine/Anthy/MoveCaretLastKey"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_FORWARD_KEY      "/IMEngine/Anthy/MoveCaretForwardKey"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_BACKWARD_KEY     "/IMEngine/Anthy/MoveCaretBackwardKey"

#define SCIM_ANTHY_CONFIG_SELECT_FIRST_SEGMENT_KEY    "/IMEngine/Anthy/SelectFirstSegmentKey"
#define SCIM_ANTHY_CONFIG_SELECT_LAST_SEGMENT_KEY     "/IMEngine/Anthy/SelectLastSegmentKey"
#define SCIM_ANTHY_CONFIG_SELECT_NEXT_SEGMENT_KEY     "/IMEngine/Anthy/SelectNextSegmentKey"
#define SCIM_ANTHY_CONFIG_SELECT_PREV_SEGMENT_KEY     "/IMEngine/Anthy/SelectPrevSegmentKey"
#define SCIM_ANTHY_CONFIG_SHRINK_SEGMENT_KEY          "/IMEngine/Anthy/ShrinkKey"
#define SCIM_ANTHY_CONFIG_EXPAND_SEGMENT_KEY          "/IMEngine/Anthy/ExpandKey"
#define SCIM_ANTHY_CONFIG_COMMIT_FIRST_SEGMENT_KEY    "/IMEngine/Anthy/CommitFirstSegment"
#define SCIM_ANTHY_CONFIG_COMMIT_SELECTED_SEGMENT_KEY "/IMEngine/Anthy/CommitSelectedSegment"

#define SCIM_ANTHY_CONFIG_SELECT_NEXT_CANDIDATE_KEY   "/IMEngine/Anthy/SelectNextCandidateKey"
#define SCIM_ANTHY_CONFIG_SELECT_PREV_CANDIDATE_KEY   "/IMEngine/Anthy/SelectPrevCandidateKey"
#define SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_UP_KEY      "/IMEngine/Anthy/CandidatesPageUpKey"
#define SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_DOWN_KEY    "/IMEngine/Anthy/CandidatesPageDownKey"

#define SCIM_ANTHY_CONFIG_CONV_TO_HIRAGANA_KEY        "/IMEngine/Anthy/ConvertToHiraganaKey"
#define SCIM_ANTHY_CONFIG_CONV_TO_KATAKANA_KEY        "/IMEngine/Anthy/ConvertToKatakanaKey"
#define SCIM_ANTHY_CONFIG_CONV_TO_HALF_KATAKANA_KEY   "/IMEngine/Anthy/ConvertToHalfKatakanaKey"
#define SCIM_ANTHY_CONFIG_CONV_TO_LATIN_KEY           "/IMEngine/Anthy/ConvertToLatinKey"
#define SCIM_ANTHY_CONFIG_CONV_TO_WIDE_LATIN_KEY      "/IMEngine/Anthy/ConvertToWideLatinKey"

#define SCIM_ANTHY_CONFIG_LATIN_MODE_KEY              "/IMEngine/Anthy/LatinModeKey"
#define SCIM_ANTHY_CONFIG_WIDE_LATIN_MODE_KEY         "/IMEngine/Anthy/WideLatinModeKey"
#define SCIM_ANTHY_CONFIG_CIRCLE_KANA_MODE_KEY        "/IMEngine/Anthy/CirclekanaModeKey"


#define SCIM_ANTHY_CONFIG_TYPING_METHOD_DEFAULT               "Roma"
#define SCIM_ANTHY_CONFIG_PERIOD_STYLE_DEFAULT                "Japanese"
#define SCIM_ANTHY_CONFIG_SPACE_TYPE_DEFAULT                  "Wide"
#define SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD_DEFAULT      false
#define SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL_DEFAULT       true
#define SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL_DEFAULT    true
#define SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL_DEFAULT     false

#define SCIM_ANTHY_CONFIG_COMMIT_KEY_DEFAULT                  "Return,KP_Enter,Control+j,Control+m"
#define SCIM_ANTHY_CONFIG_CONVERT_KEY_DEFAULT                 "space"
#define SCIM_ANTHY_CONFIG_CANCEL_KEY_DEFAULT                  "Escape,Control+g"

#define SCIM_ANTHY_CONFIG_BACKSPACE_KEY_DEFAULT               "BackSpace,Control+h"
#define SCIM_ANTHY_CONFIG_DELETE_KEY_DEFAULT                  "Delete,Control+d"

#define SCIM_ANTHY_CONFIG_MOVE_CARET_FIRST_KEY_DEFAULT        "Control+a,Home"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_LAST_KEY_DEFAULT         "Control+e,End"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_FORWARD_KEY_DEFAULT      "Right,Control+f"
#define SCIM_ANTHY_CONFIG_MOVE_CARET_BACKWARD_KEY_DEFAULT     "Left,Control+b"

#define SCIM_ANTHY_CONFIG_SELECT_FIRST_SEGMENT_KEY_DEFAULT    "Control+a,Home"
#define SCIM_ANTHY_CONFIG_SELECT_LAST_SEGMENT_KEY_DEFAULT     "Control+e,End"
#define SCIM_ANTHY_CONFIG_SELECT_NEXT_SEGMENT_KEY_DEFAULT     "Right,Control+f"
#define SCIM_ANTHY_CONFIG_SELECT_PREV_SEGMENT_KEY_DEFAULT     "Left,Control+b"
#define SCIM_ANTHY_CONFIG_SHRINK_SEGMENT_KEY_DEFAULT          "Shift+Left,Control+i"
#define SCIM_ANTHY_CONFIG_EXPAND_SEGMENT_KEY_DEFAULT          "Shift+Right,Control+o"
#define SCIM_ANTHY_CONFIG_COMMIT_FIRST_SEGMENT_KEY_DEFAULT    "Shift+Down"
#define SCIM_ANTHY_CONFIG_COMMIT_SELECTED_SEGMENT_KEY_DEFAULT "Control+Down"

#define SCIM_ANTHY_CONFIG_SELECT_NEXT_CANDIDATE_KEY_DEFAULT   "space,Down,KP_Add,Control+n"
#define SCIM_ANTHY_CONFIG_SELECT_PREV_CANDIDATE_KEY_DEFAULT   "Up,KP_Subtract,Control+p"
#define SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_UP_KEY_DEFAULT      "Page_Up,Prior"
#define SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_DOWN_KEY_DEFAULT    "Page_Down,KP_Tab,Next"

#define SCIM_ANTHY_CONFIG_CONV_TO_HIRAGANA_KEY_DEFAULT        "F6"
#define SCIM_ANTHY_CONFIG_CONV_TO_KATAKANA_KEY_DEFAULT        "F7"
#define SCIM_ANTHY_CONFIG_CONV_TO_HALF_KATAKANA_KEY_DEFAULT   "F8"
#define SCIM_ANTHY_CONFIG_CONV_TO_LATIN_KEY_DEFAULT           "F9"
#define SCIM_ANTHY_CONFIG_CONV_TO_WIDE_LATIN_KEY_DEFAULT      "F10"

#define SCIM_ANTHY_CONFIG_LATIN_MODE_KEY_DEFAULT              "Control+j,Muhenkan,Henkan"
#define SCIM_ANTHY_CONFIG_WIDE_LATIN_MODE_KEY_DEFAULT         ""
#define SCIM_ANTHY_CONFIG_CIRCLE_KANA_MODE_KEY_DEFAULT        "Hiragana_Katakana"


#endif /* __SCIM_ANTHY_PREFS_H__ */
