/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Takashi Nakamoto <bluedwarf@bpost.plala.or.jp>
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

#ifndef __SCIM_ANTHY_TRAY_H__
#define __SCIM_ANTHY_TRAY_H__

#define Uses_SCIM_CONFIG_BASE
#include <gtk/scimtrayicon.h>
#include <scim_config_base.h>
#include <gtk/gtk.h>

using namespace scim;

#define SCIM_ANTHY_TRAY_ID_INPUT_MODE                             1
#define SCIM_ANTHY_TRAY_ID_TYPING_MODE                            2
#define SCIM_ANTHY_TRAY_ID_CONV_MODE                              3
#define SCIM_ANTHY_TRAY_ID_PERIOD_STYLE                           4
#define SCIM_ANTHY_TRAY_ID_PERIOD_SYMBOL_STYLE                    5

#define SCIM_ANTHY_ITEM_ID_INPUT_MODE_HIRAGANA                    1
#define SCIM_ANTHY_ITEM_ID_INPUT_MODE_KATAKANA                    2
#define SCIM_ANTHY_ITEM_ID_INPUT_MODE_HALF_KATAKANA               3
#define SCIM_ANTHY_ITEM_ID_INPUT_MODE_LATIN                       4
#define SCIM_ANTHY_ITEM_ID_INPUT_MODE_WIDE_LATIN                  5

#define SCIM_ANTHY_ITEM_ID_CONV_MODE_MULTI_SEG                    6
#define SCIM_ANTHY_ITEM_ID_CONV_MODE_SINGLE_SEG                   7
#define SCIM_ANTHY_ITEM_ID_CONV_MODE_MULTI_REAL_TIME              8
#define SCIM_ANTHY_ITEM_ID_CONV_MODE_SINGLE_REAL_TIME             9

#define SCIM_ANTHY_ITEM_ID_TYPING_METHOD_ROMAJI                  10
#define SCIM_ANTHY_ITEM_ID_TYPING_METHOD_KANA                    11
#define SCIM_ANTHY_ITEM_ID_TYPING_METHOD_NICOLA                  12

#define SCIM_ANTHY_ITEM_ID_PERIOD_STYLE_JAPANESE                 13
#define SCIM_ANTHY_ITEM_ID_PERIOD_STYLE_WIDE_LATIN               14
#define SCIM_ANTHY_ITEM_ID_PERIOD_STYLE_LATIN                    15
#define SCIM_ANTHY_ITEM_ID_PERIOD_STYLE_WIDE_LATIN_JAPANESE      16

#define SCIM_ANTHY_ITEM_ID_SYMBOL_STYLE_JAPANESE                 17
#define SCIM_ANTHY_ITEM_ID_SYMBOL_STYLE_BRACKET_SLASH            18
#define SCIM_ANTHY_ITEM_ID_SYMBOL_STYLE_CORNER_BRACKET_SLASH     19
#define SCIM_ANTHY_ITEM_ID_SYMBOL_STYLE_BRACKET_MIDDLE_DOT       20

typedef struct _TrayMenuItem TrayMenuItem;
struct _TrayMenuItem
{
    guint id;
    gchar *short_label;
    gchar *label;
    gchar *tooltip;
};

typedef struct _TrayMenu TrayMenu;
struct _TrayMenu
{
    guint id;
    gchar *tooltip;
    TrayMenuItem *items;
};

class AnthyTray
{
public:
    AnthyTray ();
    ~AnthyTray ();
    void init (const ConfigPointer &config);
    void reload_config ();

private:
    ConfigPointer m_config;

    bool m_show_input_mode;
    bool m_show_conversion_mode;
    bool m_show_typing_method;
    bool m_show_period_style;
    bool m_show_symbol_style;

    ScimTrayIcon *m_input_mode_tray;
    ScimTrayIcon *m_conversion_mode_tray;
    ScimTrayIcon *m_typing_method_tray;
    ScimTrayIcon *m_period_styll_tray;
    ScimTrayIcon *m_symbol_style_tray;
};

#endif /* __SCIM_ANTHY_TRAY_H__ */
