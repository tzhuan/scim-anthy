/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2006 Takashi Nakamoto
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

#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include <gtk/scimtrayicon.h>
#include <gtk/gtk.h>
#include "scim_anthy_intl.h"
#include "scim_anthy_tray.h"
#include "scim_anthy_prefs.h"

#define UTF8_BRACKET_CORNER_BEGIN "\xE3\x80\x8C"
#define UTF8_BRACKET_CORNER_END   "\xE3\x80\x8D"
#define UTF8_BRACKET_WIDE_BEGIN   "\xEF\xBC\xBB"
#define UTF8_BRACKET_WIDE_END     "\xEF\xBC\xBD"
#define UTF8_MIDDLE_DOT           "\xE3\x83\xBB"
#define UTF8_SLASH_WIDE           "\xEF\xBC\x8F"

using namespace scim;

AnthyTray::AnthyTray ()
{
}

AnthyTray::~AnthyTray ()
{
    for (int i = 0; i < SCIM_ANTHY_NUMBER_OF_TRAY; i++)
    {
        if (m_tray_menus[i].menu)
            gtk_widget_destroy (GTK_WIDGET (m_tray_menus[i].menu));
        if (m_tray_menus[i].tray)
            gtk_widget_destroy (GTK_WIDGET (m_tray_menus[i].tray));
        if (m_tray_menus[i].label)
            gtk_widget_destroy (GTK_WIDGET (m_tray_menus[i].label));
        if (m_tray_menus[i].event_box)
            gtk_widget_destroy (GTK_WIDGET (m_tray_menus[i].event_box));

        for (int j = 0; j < m_tray_menus[i].num_of_items; j++)
        {
            if (m_tray_menus[i].items[j].widget)
                gtk_widget_destroy (GTK_WIDGET (m_tray_menus[i].items[j].widget));
            free (m_tray_menus[i].items);
        }
    }
}

void
AnthyTray::init (const ConfigPointer &config)
{
    /* tray menus */
    m_tray_menus[0].id                 = SCIM_ANTHY_TRAY_ID_INPUT_MODE;
    m_tray_menus[0].tooltip            = _("Input mode");
    m_tray_menus[0].default_label_text = "\xE3\x81\x82";
    m_tray_menus[0].num_of_items       = 5;
    m_tray_menus[0].items              = NULL;
    m_tray_menus[0].menu               = NULL;
    m_tray_menus[0].tray               = NULL;
    m_tray_menus[0].label              = NULL;
    m_tray_menus[0].event_box          = NULL;
    m_tray_menus[0].visible            = false;

    m_tray_menus[1].id                 = SCIM_ANTHY_TRAY_ID_CONV_MODE;
    m_tray_menus[1].tooltip            = _("Conversion method");
    m_tray_menus[1].default_label_text = "\xE9\x80\xA3";
    m_tray_menus[1].num_of_items       = 4;
    m_tray_menus[1].items              = NULL;
    m_tray_menus[1].menu               = NULL;
    m_tray_menus[1].tray               = NULL;
    m_tray_menus[1].label              = NULL;
    m_tray_menus[1].event_box          = NULL;
    m_tray_menus[1].visible            = false;

    m_tray_menus[2].id                 = SCIM_ANTHY_TRAY_ID_TYPING_MODE;
    m_tray_menus[2].tooltip            = _("Typing method");
    m_tray_menus[2].default_label_text = "\xEF\xBC\xB2";
    m_tray_menus[2].num_of_items       = 3;
    m_tray_menus[2].items              = NULL;
    m_tray_menus[2].menu               = NULL;
    m_tray_menus[2].tray               = NULL;
    m_tray_menus[2].label              = NULL;
    m_tray_menus[2].event_box          = NULL;
    m_tray_menus[2].visible            = false;

    m_tray_menus[3].id                 = SCIM_ANTHY_TRAY_ID_PERIOD_STYLE;
    m_tray_menus[3].tooltip            = _("Period style");
    m_tray_menus[3].default_label_text = "\xE3\x80\x81\xE3\x80\x82";
    m_tray_menus[3].num_of_items       = 4;
    m_tray_menus[3].items              = NULL;
    m_tray_menus[3].menu               = NULL;
    m_tray_menus[3].tray               = NULL;
    m_tray_menus[3].label              = NULL;
    m_tray_menus[3].event_box          = NULL;
    m_tray_menus[3].visible            = false;

    m_tray_menus[4].id                 = SCIM_ANTHY_TRAY_ID_SYMBOL_STYLE;
    m_tray_menus[4].tooltip            = _("Symbol style");
    m_tray_menus[4].default_label_text = UTF8_BRACKET_CORNER_BEGIN UTF8_BRACKET_CORNER_END UTF8_MIDDLE_DOT;
    m_tray_menus[4].num_of_items       = 4;
    m_tray_menus[4].items              = NULL;
    m_tray_menus[4].menu               = NULL;
    m_tray_menus[4].tray               = NULL;
    m_tray_menus[4].label              = NULL;
    m_tray_menus[4].event_box          = NULL;
    m_tray_menus[4].visible            = false;

    /* menu items */
    m_tray_menus[0].items =
        (TrayMenuItem *) malloc (sizeof(TrayMenuItem)*m_tray_menus[0].num_of_items);

    m_tray_menus[0].items[0].id          = SCIM_ANTHY_ITEM_ID_INPUT_MODE_HIRAGANA;
    m_tray_menus[0].items[0].short_label = "\xE3\x81\x82";
    m_tray_menus[0].items[0].label       = _("Hiragana");
    m_tray_menus[0].items[0].tooltip     = _("Hiragana");
    m_tray_menus[0].items[0].widget      = NULL;
        
    m_tray_menus[0].items[1].id          = SCIM_ANTHY_ITEM_ID_INPUT_MODE_KATAKANA;
    m_tray_menus[0].items[1].short_label = "\xE3\x82\xA2";
    m_tray_menus[0].items[1].label       = _("Katakana");
    m_tray_menus[0].items[1].tooltip     = _("Katakana");
    m_tray_menus[0].items[1].widget      = NULL;

    m_tray_menus[0].items[2].id          = SCIM_ANTHY_ITEM_ID_INPUT_MODE_HALF_KATAKANA;
    m_tray_menus[0].items[2].short_label = "_\xEF\xBD\xB1";
    m_tray_menus[0].items[2].label       = _("Half width katakana");
    m_tray_menus[0].items[2].tooltip     = _("Half width katakana");
    m_tray_menus[0].items[2].widget      = NULL;

    m_tray_menus[0].items[3].id          = SCIM_ANTHY_ITEM_ID_INPUT_MODE_LATIN;
    m_tray_menus[0].items[3].short_label = "_A";
    m_tray_menus[0].items[3].label       = _("Latin");
    m_tray_menus[0].items[3].tooltip     = _("Direct input");
    m_tray_menus[0].items[3].widget      = NULL;

    m_tray_menus[0].items[4].id          = SCIM_ANTHY_ITEM_ID_INPUT_MODE_WIDE_LATIN;
    m_tray_menus[0].items[4].short_label = "\xEF\xBC\xA1";
    m_tray_menus[0].items[4].label       = _("Wide latin");
    m_tray_menus[0].items[4].tooltip     = _("Wide latin");
    m_tray_menus[0].items[4].widget      = NULL;

    m_tray_menus[1].items =
        (TrayMenuItem *) malloc (sizeof(TrayMenuItem)*m_tray_menus[1].num_of_items);

    m_tray_menus[1].items[0].id          = SCIM_ANTHY_ITEM_ID_CONV_MODE_MULTI_SEG;
    m_tray_menus[1].items[0].short_label = "\xE9\x80\xA3";
    m_tray_menus[1].items[0].label       = _("Multi segment");
    m_tray_menus[1].items[0].tooltip     = _("Multi segment");
    m_tray_menus[1].items[0].widget      = NULL;
        
    m_tray_menus[1].items[1].id          = SCIM_ANTHY_ITEM_ID_CONV_MODE_SINGLE_SEG;
    m_tray_menus[1].items[1].short_label = "\xE5\x8D\x98";
    m_tray_menus[1].items[1].label       = _("Single segment");
    m_tray_menus[1].items[1].tooltip     = _("Single segment");
    m_tray_menus[1].items[1].widget      = NULL;

    m_tray_menus[1].items[2].id          = SCIM_ANTHY_ITEM_ID_CONV_MODE_MULTI_REAL_TIME;
    m_tray_menus[1].items[2].short_label = "\xE9\x80\x90 \xE9\x80\xA3";
    m_tray_menus[1].items[2].label       = _("Convert as you type (Multi segment)");
    m_tray_menus[1].items[2].tooltip     = _("Convert as you type (Multi segment)");
    m_tray_menus[1].items[2].widget      = NULL;

    m_tray_menus[1].items[3].id          = SCIM_ANTHY_ITEM_ID_CONV_MODE_SINGLE_REAL_TIME;
    m_tray_menus[1].items[3].short_label = "\xE9\x80\x90 \xE5\x8D\x98";
    m_tray_menus[1].items[3].label       = _("Convert as you type (Single segment)");
    m_tray_menus[1].items[3].tooltip     = _("Convert as you type (Single segment)");
    m_tray_menus[1].items[3].widget      = NULL;

    m_tray_menus[2].items =
        (TrayMenuItem *) malloc (sizeof(TrayMenuItem)*m_tray_menus[2].num_of_items);

    m_tray_menus[2].items[0].id          = SCIM_ANTHY_ITEM_ID_TYPING_METHOD_ROMAJI;
    m_tray_menus[2].items[0].short_label = "\xEF\xBC\xB2";
    m_tray_menus[2].items[0].label       = _("Romaji");
    m_tray_menus[2].items[0].tooltip     = _("Romaji");
    m_tray_menus[2].items[0].widget      = NULL;

    m_tray_menus[2].items[1].id          = SCIM_ANTHY_ITEM_ID_TYPING_METHOD_KANA;
    m_tray_menus[2].items[1].short_label = "\xE3\x81\x8B";
    m_tray_menus[2].items[1].label       = _("Kana");
    m_tray_menus[2].items[1].tooltip     = _("Kana");
    m_tray_menus[2].items[1].widget      = NULL;

    m_tray_menus[2].items[2].id          = SCIM_ANTHY_ITEM_ID_TYPING_METHOD_NICOLA;
    m_tray_menus[2].items[2].short_label = "\xE8\xA6\xAA";
    m_tray_menus[2].items[2].label       = _("Thumb shift");
    m_tray_menus[2].items[2].tooltip     = _("Thumb shift");
    m_tray_menus[2].items[2].widget      = NULL;

    m_tray_menus[3].items =
        (TrayMenuItem *) malloc (sizeof(TrayMenuItem)*m_tray_menus[3].num_of_items);

    m_tray_menus[3].items[0].id          = SCIM_ANTHY_ITEM_ID_PERIOD_STYLE_JAPANESE;
    m_tray_menus[3].items[0].short_label = "\xE3\x80\x81\xE3\x80\x82";
    m_tray_menus[3].items[0].label       = "\xE3\x80\x81\xE3\x80\x82";
    m_tray_menus[3].items[0].tooltip     = "\xE3\x80\x81\xE3\x80\x82";
    m_tray_menus[3].items[0].widget      = NULL;

    m_tray_menus[3].items[1].id          = SCIM_ANTHY_ITEM_ID_PERIOD_STYLE_WIDE_LATIN;
    m_tray_menus[3].items[1].short_label = "\xEF\xBC\x8C\xE3\x80\x82";
    m_tray_menus[3].items[1].label       = "\xEF\xBC\x8C\xE3\x80\x82";
    m_tray_menus[3].items[1].tooltip     = "\xEF\xBC\x8C\xE3\x80\x82";
    m_tray_menus[3].items[1].widget      = NULL;

    m_tray_menus[3].items[2].id          = SCIM_ANTHY_ITEM_ID_PERIOD_STYLE_LATIN;
    m_tray_menus[3].items[2].short_label = "\xEF\xBC\x8C\xEF\xBC\x8E";
    m_tray_menus[3].items[2].label       = "\xEF\xBC\x8C\xEF\xBC\x8E";
    m_tray_menus[3].items[2].tooltip     = "\xEF\xBC\x8C\xEF\xBC\x8E";
    m_tray_menus[3].items[2].widget      = NULL;

    m_tray_menus[3].items[3].id          = SCIM_ANTHY_ITEM_ID_PERIOD_STYLE_WIDE_LATIN_JAPANESE;
    m_tray_menus[3].items[3].short_label = ",.";
    m_tray_menus[3].items[3].label       = ",.";
    m_tray_menus[3].items[3].tooltip     = ",.";
    m_tray_menus[3].items[3].widget      = NULL;

    m_tray_menus[4].items =
        (TrayMenuItem *) malloc (sizeof(TrayMenuItem)*m_tray_menus[4].num_of_items);

    m_tray_menus[4].items[0].id          = SCIM_ANTHY_ITEM_ID_SYMBOL_STYLE_JAPANESE;
    m_tray_menus[4].items[0].short_label = UTF8_BRACKET_CORNER_BEGIN
                                           UTF8_BRACKET_CORNER_END
                                           UTF8_MIDDLE_DOT;
    m_tray_menus[4].items[0].label       = m_tray_menus[4].items[0].short_label;
    m_tray_menus[4].items[0].tooltip     = m_tray_menus[4].items[0].short_label;
    m_tray_menus[4].items[0].widget      = NULL;

    m_tray_menus[4].items[1].id          = SCIM_ANTHY_ITEM_ID_SYMBOL_STYLE_BRACKET_SLASH;
    m_tray_menus[4].items[1].short_label = UTF8_BRACKET_CORNER_BEGIN
                                           UTF8_BRACKET_CORNER_END
                                           UTF8_SLASH_WIDE;
    m_tray_menus[4].items[1].label       = m_tray_menus[4].items[1].short_label;
    m_tray_menus[4].items[1].tooltip     = m_tray_menus[4].items[1].short_label;
    m_tray_menus[4].items[1].widget      = NULL;

    m_tray_menus[4].items[2].id          = SCIM_ANTHY_ITEM_ID_SYMBOL_STYLE_CORNER_BRACKET_SLASH;
    m_tray_menus[4].items[2].short_label = UTF8_BRACKET_WIDE_BEGIN
                                           UTF8_BRACKET_WIDE_END
                                           UTF8_MIDDLE_DOT;
    m_tray_menus[4].items[2].label       = m_tray_menus[4].items[2].short_label;
    m_tray_menus[4].items[2].tooltip     = m_tray_menus[4].items[2].short_label;
    m_tray_menus[4].items[2].widget      = NULL;

    m_tray_menus[4].items[3].id          = SCIM_ANTHY_ITEM_ID_SYMBOL_STYLE_BRACKET_MIDDLE_DOT;
    m_tray_menus[4].items[3].short_label = UTF8_BRACKET_WIDE_BEGIN
                                           UTF8_BRACKET_WIDE_END
                                           UTF8_SLASH_WIDE;
    m_tray_menus[4].items[3].label       = m_tray_menus[4].items[3].short_label;
    m_tray_menus[4].items[3].tooltip     = m_tray_menus[4].items[3].short_label;
    m_tray_menus[4].items[3].widget      = NULL;
        
    m_config = config;
    reload_config ();
}

static gboolean
popup (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    TrayMenu *tray_menu = (TrayMenu *) user_data;
    GtkWidget *menu = tray_menu->menu;

    gtk_widget_show_all (menu);
    gtk_menu_popup (GTK_MENU (menu),
                    NULL, NULL, NULL, NULL, event->button,
                    gtk_get_current_event_time ());

    return TRUE;
}

static gboolean
activate (GtkMenuItem *menuitem, gpointer user_data)
{
    
}

static void
create_tray_menu (TrayMenu *tray_menu)
{
    tray_menu->menu = gtk_menu_new ();

    for (int i = 0; i < tray_menu->num_of_items; i++)
    {
        GtkWidget *item = gtk_menu_item_new_with_label (tray_menu->items[i].label);
        tray_menu->items[i].widget = item;
        gtk_menu_append (tray_menu->menu, item);
    }

    tray_menu->tray = scim_tray_icon_new (tray_menu->tooltip);
    tray_menu->label = gtk_label_new (tray_menu->default_label_text);
    tray_menu->event_box = gtk_event_box_new ();

    gtk_container_add (GTK_CONTAINER (tray_menu->event_box),
                       tray_menu->label);
    gtk_container_add (GTK_CONTAINER (tray_menu->tray),
                       tray_menu->event_box);
    g_signal_connect (G_OBJECT (tray_menu->event_box), "button-release-event",
                      G_CALLBACK (popup), tray_menu);
}

void
AnthyTray::reload_config ()
{
    m_tray_menus[0].visible
        = m_config->read (String (SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_TRAY),
                          SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_TRAY_DEFAULT);

    m_tray_menus[1].visible
        = m_config->read (String (SCIM_ANTHY_CONFIG_SHOW_CONVERSION_MODE_TRAY),
                          SCIM_ANTHY_CONFIG_SHOW_CONVERSION_MODE_TRAY_DEFAULT);

    m_tray_menus[2].visible
        = m_config->read (String (SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_TRAY),
                          SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_TRAY_DEFAULT);

    m_tray_menus[3].visible
        = m_config->read (String (SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_TRAY),
                          SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_TRAY_DEFAULT);

    m_tray_menus[4].visible
        = m_config->read (String (SCIM_ANTHY_CONFIG_SHOW_SYMBOL_STYLE_TRAY),
                          SCIM_ANTHY_CONFIG_SHOW_SYMBOL_STYLE_TRAY_DEFAULT);

    for (int i = 0; i < SCIM_ANTHY_NUMBER_OF_TRAY; i++)
    {
        m_tray_menus[i].visible = true;
        if (m_tray_menus[i].visible)
        {
            if (m_tray_menus[i].menu == NULL &&
                m_tray_menus[i].tray == NULL)
                create_tray_menu (&(m_tray_menus[i]));

            gtk_widget_show_all (GTK_WIDGET (m_tray_menus[i].tray));
        }
        else
        {
            if (m_tray_menus[i].tray)
                gtk_widget_hide_all (GTK_WIDGET (m_tray_menus[i].tray));
        }
    }
}
