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

using namespace scim;
/*
TrayMenuItem input_mode_items[] =
{
    {
        SCIM_ANTHY_ITEM_ID_INPUT_MODE_HIRAGANA,
        "\xE3\x81\x82",
        _("Hiragana"),
        _("Hiragana")
    },
    {
        SCIM_ANTHY_ITEM_ID_INPUT_MODE_KATAKANA,
        "\xE3\x82\xA2",
        _("Katakana"),
        _("Katakana")
    },
    {
        SCIM_ANTHY_ITEM_ID_INPUT_MODE_HALF_KATAKANA,
        "_\xEF\xBD\xB1",
        _("Half width katakana"),
        _("Half width katakana")
    },
    {
        SCIM_ANTHY_ITEM_ID_INPUT_MODE_LATIN,
        "_A",
        _("Latin"),
        _("Direct input")
    },
    {
        SCIM_ANTHY_ITEM_ID_INPUT_MODE_WIDE_LATIN,
        "\xEF\xBC\xA1",
        _("Wide latin"),
        _("Wide latin")
    }
};

TrayMenuItem conv_mode_items[] =
{
    {
        SCIM_ANTHY_ITEM_ID_CONV_MODE_MULTI_SEG,
        "\xE9\x80\xA3",
        _("Multi segment"),
        _("Multi segment")
    },
    {
        SCIM_ANTHY_ITEM_ID_CONV_MODE_SINGLE_SEG,
        "\xE5\x8D\x98",
        _("Single segment"),
        _("Single segment")
    },
    {
        SCIM_ANTHY_ITEM_ID_CONV_MODE_MULTI_REAL_TIME,
        "\xE9\x80\x90 \xE9\x80\xA3",
        _("Convert as you type (Multi segment)"),
        _("Convert as you type (Multi segment)")
    },
    {
        SCIM_ANTHY_ITEM_ID_CONV_MODE_SINGLE_REAL_TIME,
        "\xE9\x80\x90 \xE5\x8D\x98",
        _("Convert as you type (Single segment)"),
        _("Convert as you type (Single segment)")
    }
};
*/
/*
TrayMenu input_mode_tray =
{
    SCIM_ANTHY_TRAY_ID_INPUT_MODE,
    _("Input mode"),
    {
        item_input_mode
    };
};

TrayMenu conv_mode_tray =
{
    SCIM_ANTHY_TRAY_ID_CONV_MODE,
    _("Conversion method"),
    {
    };
};
*/

AnthyTray::AnthyTray ()
    : m_show_input_mode      (false),
      m_show_conversion_mode (false),
      m_show_typing_method   (false),
      m_show_period_style    (false),
      m_show_symbol_style    (false)
//      m_input_mode_tray      (NULL),
//      m_conversion_mode_tray (NULL),
//      m_typing_method_tray   (NULL),
//      m_period_styll_tray    (NULL),
//      m_symbol_style_tray    (NULL)
      
{
}

AnthyTray::~AnthyTray ()
{
/*

    if (m_show_input_mode)
        gtk_widget_hide_all (GTK_WIDGET (m_input_mode_tray));
    if (m_input_mode_tray)
        gtk_widget_destroy (GTK_WIDGET (m_input_mode_tray));
*/
}

void
AnthyTray::init (const ConfigPointer &config)
{
    m_config = config;
    reload_config ();
}

void
AnthyTray::reload_config ()
{
    m_show_input_mode = true;
/*
    m_show_input_mode
	= config->read (String (SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_TRAY),
			SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_TRAY_DEFAULT);

    if (m_show_input_mode)
    {
        if (m_input_mode_tray == NULL)
        {
            // init
            m_input_mode_tray = scim_tray_icon_new ("scim-anthy input mode");
            
            GtkWidget *button = gtk_button_new_with_label ("\xE3\x81\x82");
            gtk_container_add (GTK_CONTAINER (m_input_mode_tray), button);
//            g_signal_connect (G_OBJECT (button), "button-release-event",
//                              G_CALLBACK (popup), NULL);
        }
        else
        {
            gtk_widget_show_all (GTK_WIDGET (m_input_mode_tray));
        }
    }
    else
    {
        if (m_input_mode_tray)
            gtk_widget_hide_all (GTK_WIDGET (m_input_mode_tray));
    }

    m_show_conversion_mode
	= config->read (String (SCIM_ANTHY_CONFIG_SHOW_CONVERSION_MODE_TRAY),
			SCIM_ANTHY_CONFIG_SHOW_CONVERSION_MODE_TRAY_DEFAULT);

    m_show_typing_method
	= config->read (String (SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_TRAY),
			SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_TRAY_DEFAULT);
    m_show_period_style
	= config->read (String (SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_TRAY),
			SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_TRAY_DEFAULT);

    m_show_symbol_style
	= config->read (String (SCIM_ANTHY_CONFIG_SHOW_SYMBOL_STYLE_TRAY),
			SCIM_ANTHY_CONFIG_SHOW_SYMBOL_STYLE_TRAY_DEFAULT);
*/
}
