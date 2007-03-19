/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 - 2007 Takashi Nakamoto
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

#ifndef __SCIM_ANTHY_CONST_H__
#define __SCIM_ANTHY_CONST_H__

#include <scim.h>

using namespace scim;

#define SCIM_ANTHY_HELPER_UUID "24a65e2b-10a8-4d4c-adc9-266678cb1a38"

// transaction commands from IMEngine to Helper
#define SCIM_ANTHY_TRANS_CMD_NEW_IC                (SCIM_TRANS_CMD_USER_DEFINED + 1)
#define SCIM_ANTHY_TRANS_CMD_DELETE_IC             (SCIM_TRANS_CMD_USER_DEFINED + 2)
#define SCIM_ANTHY_TRANS_CMD_GET_SELECTION         (SCIM_TRANS_CMD_USER_DEFINED + 3)
#define SCIM_ANTHY_TRANS_CMD_TIMEOUT_ADD           (SCIM_TRANS_CMD_USER_DEFINED + 4)
#define SCIM_ANTHY_TRANS_CMD_TIMEOUT_REMOVE        (SCIM_TRANS_CMD_USER_DEFINED + 5)
#define SCIM_ANTHY_TRANS_CMD_TIMEOUT_NOTIFY        (SCIM_TRANS_CMD_USER_DEFINED + 6)
#define SCIM_ANTHY_TRANS_CMD_SHOW_NOTE             (SCIM_TRANS_CMD_USER_DEFINED + 7)
#define SCIM_ANTHY_TRANS_CMD_HIDE_NOTE             (SCIM_TRANS_CMD_USER_DEFINED + 8)
#define SCIM_ANTHY_TRANS_CMD_UPDATE_NOTE           (SCIM_TRANS_CMD_USER_DEFINED + 9)
#define SCIM_ANTHY_TRANS_CMD_SET_INPUT_MODE        (SCIM_TRANS_CMD_USER_DEFINED + 10)
#define SCIM_ANTHY_TRANS_CMD_INIT_TRAY_MENU        (SCIM_TRANS_CMD_USER_DEFINED + 11)
#define SCIM_ANTHY_TRANS_CMD_UPDATE_TRAY_MENU      (SCIM_TRANS_CMD_USER_DEFINED + 12)

// transaction command from Helper to IMEngine
#define SCIM_ANTHY_TRANS_CMD_CHANGE_INPUT_MODE     (SCIM_TRANS_CMD_USER_DEFINED + 20)
#define SCIM_ANTHY_TRANS_CMD_TRIGGER_PROPERTY      (SCIM_TRANS_CMD_USER_DEFINED + 21)
#define SCIM_ANTHY_TRANS_CMD_ATTACHMENT_SUCCESS    (SCIM_TRANS_CMD_USER_DEFINED + 22)
#define SCIM_ANTHY_TRANS_CMD_SELECT_CANDIDATE      (SCIM_TRANS_CMD_USER_DEFINED + 23)

typedef uint32 InputMode;
#define SCIM_ANTHY_MODE_HIRAGANA      0
#define SCIM_ANTHY_MODE_KATAKANA      1
#define SCIM_ANTHY_MODE_HALF_KATAKANA 2
#define SCIM_ANTHY_MODE_LATIN         3
#define SCIM_ANTHY_MODE_WIDE_LATIN    4

namespace scim_anthy {

/*
typedef enum {
    SCIM_ANTHY_MODE_HIRAGANA,
    SCIM_ANTHY_MODE_KATAKANA,
    SCIM_ANTHY_MODE_HALF_KATAKANA,
    SCIM_ANTHY_MODE_LATIN,
    SCIM_ANTHY_MODE_WIDE_LATIN,
} InputMode;
*/

typedef enum {
    SCIM_ANTHY_CONVERSION_MULTI_SEGMENT,
    SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT,
    SCIM_ANTHY_CONVERSION_MULTI_SEGMENT_IMMEDIATE,
    SCIM_ANTHY_CONVERSION_SINGLE_SEGMENT_IMMEDIATE,
} ConversionMode;

typedef enum {
    SCIM_ANTHY_PERIOD_JAPANESE,
    SCIM_ANTHY_PERIOD_WIDE,
    SCIM_ANTHY_PERIOD_HALF,
} PeriodStyle;

typedef enum {
    SCIM_ANTHY_COMMA_JAPANESE,
    SCIM_ANTHY_COMMA_WIDE,
    SCIM_ANTHY_COMMA_HALF,
} CommaStyle;

typedef enum {
    SCIM_ANTHY_BRACKET_JAPANESE,
    SCIM_ANTHY_BRACKET_WIDE,
} BracketStyle;

typedef enum {
    SCIM_ANTHY_SLASH_JAPANESE,
    SCIM_ANTHY_SLASH_WIDE,
} SlashStyle;

typedef enum {
    SCIM_ANTHY_TYPING_METHOD_ROMAJI,
    SCIM_ANTHY_TYPING_METHOD_KANA,
    SCIM_ANTHY_TYPING_METHOD_NICOLA,
    SCIM_ANTHY_TYPING_METHOD_CUSTOM,
} TypingMethod;

}

#endif /* __SCIM_ANTHY_CONST_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
