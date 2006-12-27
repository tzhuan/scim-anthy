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
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_HELPER

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include <gtk/scimtrayicon.h>
#include <gtk/gtk.h>
#include "scim_anthy_intl.h"
#include "scim_anthy_tray.h"
#include "scim_anthy_prefs.h"
#include "scim_anthy_helper.h"
#include "scim_anthy_const.h"

using namespace scim;
using namespace std;

AnthyTray::AnthyTray ()
    : m_agent              (NULL),
      m_ic                 (0),
      m_ic_uuid            (String ()),
      m_initialized        (false),
      m_tray               (NULL),
#ifdef USE_GTK_BUTTON_FOR_TRAY
      m_tray_button        (NULL),
#else
      m_tray_event_box     (NULL),
#endif
      m_input_mode_menu    (NULL),
      m_tooltips           (NULL),
      m_dummy              (NULL)
{
}

AnthyTray::~AnthyTray ()
{
    if (m_initialized)
    {
#ifdef USE_GTK_BUTTON_FOR_TRAY
        if (m_tray_button)
            gtk_widget_destroy (m_tray_button);
#else
        if (m_tray_event_box)
            gtk_widget_destroy (m_tray_event_box);

        if (m_tray_label)
            gtk_widget_destroy (m_tray_label);
#endif

        if (m_input_mode_menu)
            gtk_widget_destroy (m_input_mode_menu);

        if (m_dummy)
            gtk_widget_destroy (m_dummy);

        if (m_box)
            gtk_widget_destroy (m_box);
        
        if (m_tray)
            gtk_object_destroy (GTK_OBJECT (m_tray));

        if (m_tooltips)
            gtk_object_destroy (GTK_OBJECT (m_tooltips));
    }
}

void
AnthyTray::popup_input_mode_menu (GdkEventButton *event)
{
    guint button = 0;
    if (event != NULL)
        button = event->button;

    gtk_widget_show_all (m_input_mode_menu);
    gtk_menu_popup (GTK_MENU (m_input_mode_menu),
                    NULL, NULL, NULL,
                    NULL, button,
                    gtk_get_current_event_time ());
}

void
AnthyTray::popup_general_menu (GdkEventButton *event)
{
    guint button = 0;
    if (event != NULL)
        button = event->button;

}

void
AnthyTray::activated_item (GtkMenuItem *item)
{
    const uint32 command =
        GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (item),
                                            "scim-anthy-item-command"));
    const uint32 command_data =
        GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (item),
                                            "scim-anthy-item-command-data"));

    Transaction send;
    send.put_command (command);
    send.put_data    (command_data);
    m_agent->send_imengine_event (m_ic, m_ic_uuid, send);
}

void
AnthyTray::attach_input_context   (const HelperAgent  *agent,
                                   int                 ic,
                                   const String       &ic_uuid)
{
    m_agent   = agent;
    m_ic      = ic;
    m_ic_uuid = ic_uuid;
}

void
AnthyTray::set_input_mode (InputMode mode)
{
    if (m_initialized == false)
        create_tray ();

    char *label = "a";
    switch (mode)
    {
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
    }

#ifdef  USE_GTK_BUTTON_FOR_TRAY
    gtk_button_set_label (GTK_BUTTON (m_tray_button),
                          label);
#else
    gtk_label_set_text (GTK_LABEL (m_tray_label),
                        label);
#endif
    gtk_widget_show_all (GTK_WIDGET (m_tray));
}

void
AnthyTray::hide (void)
{
    if (m_initialized == false)
        return;

#ifdef  USE_GTK_BUTTON_FOR_TRAY
    gtk_widget_hide (m_tray_button);
#else
    gtk_widget_hide (m_tray_label);
#endif
}

static gboolean
activate (GtkMenuItem *menuitem, gpointer user_data)
{
    AnthyTray *anthy_tray = (AnthyTray *) user_data;
    anthy_tray->activated_item (menuitem);

    return TRUE;
}

static gboolean
popup (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    AnthyTray *anthy_tray = (AnthyTray *) user_data;

    if (event->button  == 1) // left button
        anthy_tray->popup_input_mode_menu (event);
    else if (event->button == 3) // right button
        anthy_tray->popup_general_menu (event);

    return TRUE;
}

void
AnthyTray::create_tray (void)
{
    m_tooltips = gtk_tooltips_new ();

    // input mode menu
    m_input_mode_menu = gtk_menu_new ();
    gtk_menu_shell_set_take_focus (GTK_MENU_SHELL (m_input_mode_menu),
                                   false);

    // input mode menu items
    MenuItemProperties props[5];

    props[0].label = props[0].tips = _("Hiragana");
    props[0].command = SCIM_ANTHY_TRANS_CMD_CHANGE_INPUT_MODE;
    props[0].command_data = SCIM_ANTHY_MODE_HIRAGANA;

    props[1].label = props[1].tips = _("Katakana");
    props[1].command = SCIM_ANTHY_TRANS_CMD_CHANGE_INPUT_MODE;
    props[1].command_data = SCIM_ANTHY_MODE_KATAKANA;

    props[2].label = props[2].tips = _("Half width katakana");
    props[2].command = SCIM_ANTHY_TRANS_CMD_CHANGE_INPUT_MODE;
    props[2].command_data = SCIM_ANTHY_MODE_HALF_KATAKANA;

    props[3].label = _("Latin");
    props[3].tips = _("Direct input");
    props[3].command = SCIM_ANTHY_TRANS_CMD_CHANGE_INPUT_MODE;
    props[3].command_data = SCIM_ANTHY_MODE_LATIN;

    props[4].label = props[4].tips = _("Wide latin");
    props[4].command = SCIM_ANTHY_TRANS_CMD_CHANGE_INPUT_MODE;
    props[4].command_data = SCIM_ANTHY_MODE_WIDE_LATIN;

    for (int i = 0; i < 5; i++)
    {
        GtkWidget *item = gtk_menu_item_new ();
        GtkWidget *item_label = gtk_label_new (props[i].label);
        
        gtk_misc_set_alignment (GTK_MISC (item_label),
                                0.0, 0.5); // to left
        gtk_tooltips_set_tip (m_tooltips, item,
                              props[i].tips, props[i].tips);
        gtk_container_add (GTK_CONTAINER (item),
                           item_label);
        gtk_menu_shell_append (GTK_MENU_SHELL (m_input_mode_menu),
                               item);
        g_object_set_data (G_OBJECT (item),
                           "scim-anthy-item-command",
                           GUINT_TO_POINTER(props[i].command));
        g_object_set_data (G_OBJECT (item),
                           "scim-anthy-item-command-data",
                           GUINT_TO_POINTER(props[i].command_data));
        g_signal_connect (G_OBJECT (item), "activate",
                          G_CALLBACK (activate), this);
    }

    // tray
    m_tray = scim_tray_icon_new ("scim-anthy-input-mode-tray");
    gtk_widget_show (GTK_WIDGET (m_tray));

    // box of tray
    m_box = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (m_tray), m_box);
    gtk_widget_show (m_box);

#ifdef USE_GTK_BUTTON_FOR_TRAY
    // tray button
    m_tray_button = gtk_button_new_with_label ("\xE3\x81\x82");
    gtk_button_set_relief (GTK_BUTTON (m_tray_button),
                           GTK_RELIEF_NONE);
    gtk_tooltips_set_tip (m_tooltips, m_tray_button,
                          _("Input mode"), _("Input mode"));
    g_signal_connect (G_OBJECT (m_tray_button), "button-release-event",
                      G_CALLBACK (popup), this);
    gtk_box_pack_start (GTK_BOX (m_box), m_tray_button,
                        TRUE, TRUE, 0);
    gtk_widget_show (m_tray_button);

    // configure the button padding to be 0
    gtk_rc_parse_string (
        "\n"
        "   style \"scim-anthy-button-style\"\n"
        "   {\n"
        "      GtkWidget::focus-line-width=0\n"
        "      GtkWidget::focus-padding=0\n"
        "   }\n"
        "\n"
        "    widget \"*.scim-anthy-button\" style \"scim-anthy-button-style\"\n"
        "\n");
    gtk_widget_set_name (m_tray_button, "scim-anthy-button");
#else
    // event box for tray icon
    m_tray_event_box = gtk_event_box_new ();
    g_signal_connect (G_OBJECT (m_tray_event_box), "button-release-event",
                      G_CALLBACK (popup), this);
    gtk_box_pack_start (GTK_BOX (m_box), m_tray_event_box,
                        TRUE, TRUE, 0);
    gtk_widget_show (m_tray_event_box);

    // label for tray icon
    m_tray_label = gtk_label_new ("\xE3\x81\x82");
    gtk_container_add (GTK_CONTAINER (m_tray_event_box), m_tray_label);
    gtk_widget_show (m_tray_label);
#endif

    // dummy
    m_dummy = gtk_label_new ("");
    gtk_box_pack_start (GTK_BOX (m_box), m_dummy,
                        TRUE, TRUE, 0);
    gtk_widget_show (m_dummy);

    m_initialized = true;
}
