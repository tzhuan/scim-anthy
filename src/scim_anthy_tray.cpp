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

#define SCIM_PROP_IMENGINE_ANTHY_PREFIX  "/IMEngine/Anthy/"
#define TRAY_ICON_SIZE 18

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
      m_tray_image         (NULL),
      m_hiragana_pixbuf    (NULL),
      m_katakana_pixbuf    (NULL),
      m_halfkana_pixbuf    (NULL),
      m_latin_pixbuf       (NULL),
      m_wide_latin_pixbuf  (NULL),
      m_direct_pixbuf      (NULL),
      m_input_mode_menu    (NULL),
      m_general_menu       (NULL),
      m_tooltips           (NULL)
{
}

AnthyTray::~AnthyTray ()
{
    destroy_general_menu ();

    if (m_initialized)
    {
#ifdef USE_GTK_BUTTON_FOR_TRAY
        gtk_widget_destroy (m_tray_button);
#else
        gtk_widget_destroy (m_tray_event_box);
#endif

        gtk_widget_destroy (m_tray_image);
        g_object_unref (G_OBJECT (m_hiragana_pixbuf));
        g_object_unref (G_OBJECT (m_katakana_pixbuf));
        g_object_unref (G_OBJECT (m_halfkana_pixbuf));
        g_object_unref (G_OBJECT (m_latin_pixbuf));
        g_object_unref (G_OBJECT (m_wide_latin_pixbuf));
        g_object_unref (G_OBJECT (m_direct_pixbuf));

        gtk_widget_destroy (m_input_mode_menu);

        gtk_object_destroy (GTK_OBJECT (m_tray));
        gtk_object_destroy (GTK_OBJECT (m_tooltips));
    }
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
AnthyTray::popup_input_mode_menu (GdkEventButton *event)
{
    if (m_input_mode_menu == NULL)
        return;

    guint button = 0;
    if (event != NULL)
        button = event->button;

    gtk_widget_show_all (m_input_mode_menu);
    gtk_menu_popup (GTK_MENU (m_input_mode_menu),
                    NULL, NULL, NULL,
                    NULL, button,
                    gtk_get_current_event_time ());
}

static gboolean
activate_input_mode_menu_item (GtkMenuItem *menuitem, gpointer user_data)
{
    AnthyTray *anthy_tray = (AnthyTray *) user_data;
    anthy_tray->activated_input_mode_menu_item (menuitem);

    return TRUE;
}

void
AnthyTray::activated_input_mode_menu_item (GtkMenuItem *item)
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
AnthyTray::popup_general_menu (GdkEventButton *event)
{
    if (m_general_menu == NULL)
        return;

    gtk_widget_show_all (m_general_menu);
    gtk_menu_popup (GTK_MENU (m_general_menu),
                    NULL, NULL, NULL,
                    NULL, 0,
                    gtk_get_current_event_time ());
}

static gboolean
activate_general_menu_item (GtkMenuItem *menuitem, gpointer user_data)
{
    AnthyTray *anthy_tray = (AnthyTray *) user_data;
    anthy_tray->activated_general_menu_item (menuitem);

    return TRUE;
}

void
AnthyTray::activated_general_menu_item (GtkMenuItem *item)
{
    String key = String (SCIM_PROP_IMENGINE_ANTHY_PREFIX);
    char *suffix =
        (char *) g_object_get_data (G_OBJECT (item),
                                    "scim-anthy-property-key");
    key += suffix;

    Transaction send;
    send.put_command (SCIM_ANTHY_TRANS_CMD_TRIGGER_PROPERTY);
    send.put_data    (String (key));
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
AnthyTray::disable (void)
{
    if (m_initialized == false)
        return;

    gtk_image_set_from_pixbuf (GTK_IMAGE (m_tray_image), m_direct_pixbuf);
}

void
AnthyTray::set_input_mode (InputMode mode)
{
    if (m_initialized == false)
        create_tray ();

    switch (mode)
    {
    case SCIM_ANTHY_MODE_HIRAGANA:
        gtk_image_set_from_pixbuf (GTK_IMAGE (m_tray_image), m_hiragana_pixbuf);
        break;
    case SCIM_ANTHY_MODE_KATAKANA:
        gtk_image_set_from_pixbuf (GTK_IMAGE (m_tray_image), m_katakana_pixbuf);
        break;
    case SCIM_ANTHY_MODE_HALF_KATAKANA:
        gtk_image_set_from_pixbuf (GTK_IMAGE (m_tray_image), m_halfkana_pixbuf);
        break;
    case SCIM_ANTHY_MODE_LATIN:
        gtk_image_set_from_pixbuf (GTK_IMAGE (m_tray_image), m_latin_pixbuf);
        break;
    case SCIM_ANTHY_MODE_WIDE_LATIN:
        gtk_image_set_from_pixbuf (GTK_IMAGE (m_tray_image), m_wide_latin_pixbuf);
        break;
    }
}

void
AnthyTray::create_general_menu (PropertyList &props)
{
    destroy_general_menu ();

    // general menu
    m_general_menu = gtk_menu_new ();
    gtk_menu_shell_set_take_focus (GTK_MENU_SHELL (m_general_menu),
                                   false);

    // general menu items
    PropertyList::iterator it = props.begin ();
    while (it != props.end ())
    {
        String key = it->get_key ();
        String label = it->get_label ();
        String tip = it->get_tip ();

        if (key.find (SCIM_PROP_IMENGINE_ANTHY_PREFIX) != 0)
            continue;
        key.erase (0, strlen(SCIM_PROP_IMENGINE_ANTHY_PREFIX));

        unsigned int pos;
        if ((pos = key.find ("/")) == String::npos)
        {
            GtkWidget *item = gtk_menu_item_new ();
            GtkWidget *item_label = gtk_label_new (label.c_str ());

            gtk_misc_set_alignment (GTK_MISC (item_label),
                                    0.0, 0.5); // to left

            if (m_tooltips == NULL)
                m_tooltips = gtk_tooltips_new ();

            gtk_tooltips_set_tip (m_tooltips, item,
                                  tip.c_str(), tip.c_str());
            gtk_container_add (GTK_CONTAINER (item),
                               item_label);
            g_object_set_data (G_OBJECT (item),
                               "scim-anthy-property-key",
                               strdup (key.c_str ()));
            gtk_menu_shell_append (GTK_MENU_SHELL (m_general_menu),
                                   item);

            GtkWidget *submenu = gtk_menu_new ();

            gtk_menu_shell_set_take_focus (GTK_MENU_SHELL (submenu),
                                           false);
            gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);
        }
        else
        {
            String parent_key = key.substr (0, pos);

            GtkWidget *item = gtk_menu_item_new ();
            GtkWidget *item_label = gtk_label_new (label.c_str ());

            gtk_misc_set_alignment (GTK_MISC (item_label),
                                    0.0, 0.5); // to left

            if (m_tooltips == NULL)
                m_tooltips = gtk_tooltips_new ();

            gtk_tooltips_set_tip (m_tooltips, item,
                                  tip.c_str (), tip.c_str ());
            gtk_container_add (GTK_CONTAINER (item),
                               item_label);
            g_object_set_data (G_OBJECT (item),
                               "scim-anthy-property-key",
                               strdup (key.c_str ()));
            GtkWidget *parent_menu_item = find_menu_item (m_general_menu, parent_key);
            if (parent_menu_item == NULL)
                continue;

            GtkWidget *parent_menu =
                gtk_menu_item_get_submenu (GTK_MENU_ITEM (parent_menu_item));
            if (parent_menu == NULL)
                continue;

            gtk_menu_shell_append (GTK_MENU_SHELL (parent_menu),
                                   item);
            g_signal_connect (G_OBJECT (item), "activate",
                              G_CALLBACK (activate_general_menu_item), this);
        }

        it++;
    }
}

void
AnthyTray::update_general_menu (Property &prop)
{
    String key = prop.get_key ();
    String label = prop.get_label ();
    String tip = prop.get_tip ();

    if (key.find (SCIM_PROP_IMENGINE_ANTHY_PREFIX) != 0)
        return;
    key.erase (0, strlen(SCIM_PROP_IMENGINE_ANTHY_PREFIX));

    GtkWidget *item = find_menu_item (m_general_menu, key);
    GtkWidget *item_label = gtk_bin_get_child (GTK_BIN (item));

    gtk_tooltips_set_tip (m_tooltips, item,
                          tip.c_str (), tip.c_str ());
    gtk_label_set_text (GTK_LABEL (item_label), label.c_str ());
}

GtkWidget *
AnthyTray::find_menu_item (GtkWidget *menu, const String &key)
{
    char *tmp;
    GList *list = gtk_container_get_children (GTK_CONTAINER (menu));

    while (list != NULL)
    {
        GtkWidget *item = (GtkWidget *) (list->data);
        GtkWidget *submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (item));

        tmp = (char *) g_object_get_data (G_OBJECT(item),
                                          "scim-anthy-property-key");
        if (key == tmp)
            return item;

        GList *sublist =
            gtk_container_get_children (GTK_CONTAINER (submenu));
        while (sublist != NULL)
        {
            GtkWidget *subitem = (GtkWidget *) (sublist->data);
            tmp = (char *) g_object_get_data (G_OBJECT (subitem),
                                              "scim-anthy-property-key");

            if (key == tmp)
                return subitem;

            sublist = g_list_next (sublist);
        }

        list = g_list_next (list);
    }

    return NULL;
}

void
AnthyTray::destroy_general_menu (void)
{
    if (m_general_menu == NULL)
        return;

    GList *list = gtk_container_get_children (GTK_CONTAINER (m_general_menu));
    char *tmp;

    while (list != NULL)
    {
        GtkWidget *item = (GtkWidget *) (list->data);
        GtkWidget *submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (item));

        tmp = (char *) g_object_get_data (G_OBJECT(item),
                                          "scim-anthy-property-key");
        if (tmp != NULL)
            free (tmp);

        GList *sublist =
            gtk_container_get_children (GTK_CONTAINER (submenu));
        while (sublist != NULL)
        {
            GtkWidget *subitem = (GtkWidget *) (sublist->data);
            tmp = (char *) g_object_get_data (G_OBJECT (subitem),
                                              "scim-anthy-property-key");
            if (tmp != NULL)
                free (tmp);
            sublist = g_list_next (sublist);
        }

        list = g_list_next (list);
    }

    gtk_widget_destroy (m_general_menu);
    m_general_menu = NULL;
}

static gboolean
transparent_expose_event (GtkWidget *widget, GdkEventExpose *event)
{
  gdk_window_clear_area (widget->window, event->area.x, event->area.y,
                         event->area.width, event->area.height);
  return FALSE;
}

static void
make_transparent_again (GtkWidget *widget, GtkStyle *previous_style)
{
  gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
}

static void
make_transparent (GtkWidget *widget)
{
  if (GTK_WIDGET_NO_WINDOW (widget) || GTK_WIDGET_APP_PAINTABLE (widget))
    return;

  gtk_widget_set_app_paintable (widget, TRUE);
  gtk_widget_set_double_buffered (widget, FALSE);
  gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
  g_signal_connect (widget, "expose_event",
                    G_CALLBACK (transparent_expose_event), NULL);
  g_signal_connect (widget, "style_set",
                    G_CALLBACK (make_transparent_again), NULL);
}

void
AnthyTray::create_tray (void)
{
    if (m_tooltips == NULL)
        m_tooltips = gtk_tooltips_new ();

    // input mode menu
    m_input_mode_menu = gtk_menu_new ();
    gtk_menu_shell_set_take_focus (GTK_MENU_SHELL (m_input_mode_menu),
                                   false);

    // input mode menu items
    InputModeMenuItemProperties props[5];

    props[0].label = props[0].tips = _("Hiragana");
    props[0].command = SCIM_ANTHY_TRANS_CMD_CHANGE_INPUT_MODE;
    props[0].command_data = SCIM_ANTHY_MODE_HIRAGANA;

    props[1].label = props[1].tips = _("Katakana");
    props[1].command = SCIM_ANTHY_TRANS_CMD_CHANGE_INPUT_MODE;
    props[1].command_data = SCIM_ANTHY_MODE_KATAKANA;

    props[2].label = props[2].tips = _("Half width katakana");
    props[2].command = SCIM_ANTHY_TRANS_CMD_CHANGE_INPUT_MODE;
    props[2].command_data = SCIM_ANTHY_MODE_HALF_KATAKANA;

    props[3].label = props[3].tips =  _("Latin");
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
                          G_CALLBACK (activate_input_mode_menu_item), this);
    }

    // tray
    m_tray = scim_tray_icon_new ("scim-anthy-input-mode-tray");
    g_signal_connect (G_OBJECT (m_tray), "realize",
                      G_CALLBACK (make_transparent), NULL);
    gtk_widget_show (GTK_WIDGET (m_tray));

#ifdef USE_GTK_BUTTON_FOR_TRAY
    // tray button
    m_tray_button = gtk_button_new ();
    gtk_button_set_relief (GTK_BUTTON (m_tray_button),
                           GTK_RELIEF_NONE);
    gtk_tooltips_set_tip (m_tooltips, m_tray_button,
                          _("Input mode"), _("Input mode"));
    g_signal_connect (G_OBJECT (m_tray_button), "button-release-event",
                      G_CALLBACK (popup), this);
    gtk_container_add (GTK_CONTAINER (m_tray), m_tray_button);
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
    g_signal_connect (G_OBJECT (m_tray_event_box), "realize",
                      G_CALLBACK (make_transparent), NULL);
    gtk_tooltips_set_tip (m_tooltips, m_tray_event_box,
                          _("Input mode"), _("Input mode"));
    g_signal_connect (G_OBJECT (m_tray_event_box), "button-release-event",
                      G_CALLBACK (popup), this);
    gtk_container_add (GTK_CONTAINER (m_tray), m_tray_event_box);
    gtk_widget_show (m_tray_event_box);
#endif

    // images for tray icon
    m_hiragana_pixbuf = gdk_pixbuf_scale_simple (
        gdk_pixbuf_new_from_file (SCIM_ICONDIR"/scim-anthy-hiragana.png", NULL),
        TRAY_ICON_SIZE, TRAY_ICON_SIZE, GDK_INTERP_BILINEAR);
    m_katakana_pixbuf = gdk_pixbuf_scale_simple (
        gdk_pixbuf_new_from_file (SCIM_ICONDIR"/scim-anthy-katakana.png", NULL),
        TRAY_ICON_SIZE, TRAY_ICON_SIZE, GDK_INTERP_BILINEAR);
    m_halfkana_pixbuf = gdk_pixbuf_scale_simple (
        gdk_pixbuf_new_from_file (SCIM_ICONDIR"/scim-anthy-halfkana.png", NULL),
        TRAY_ICON_SIZE, TRAY_ICON_SIZE, GDK_INTERP_BILINEAR);
    m_latin_pixbuf = gdk_pixbuf_scale_simple (
        gdk_pixbuf_new_from_file (SCIM_ICONDIR"/scim-anthy-halfwidth-alnum.png", NULL),
        TRAY_ICON_SIZE, TRAY_ICON_SIZE, GDK_INTERP_BILINEAR);
    m_wide_latin_pixbuf = gdk_pixbuf_scale_simple (
        gdk_pixbuf_new_from_file (SCIM_ICONDIR"/scim-anthy-fullwidth-alnum.png", NULL),
        TRAY_ICON_SIZE, TRAY_ICON_SIZE, GDK_INTERP_BILINEAR);
    m_direct_pixbuf = gdk_pixbuf_scale_simple (
        gdk_pixbuf_new_from_file (SCIM_ICONDIR"/scim-anthy-direct.png", NULL),
        TRAY_ICON_SIZE, TRAY_ICON_SIZE, GDK_INTERP_BILINEAR);
    m_tray_image = gtk_image_new_from_pixbuf (m_direct_pixbuf);

#ifdef USE_GTK_BUTTON_FOR_TRAY
    gtk_container_add (GTK_CONTAINER (m_tray_button), m_tray_image);
#else
    gtk_container_add (GTK_CONTAINER (m_tray_event_box), m_tray_image);
    gtk_misc_set_alignment (GTK_MISC (m_tray_image), 0.5, 0.5);
    gtk_widget_set_size_request (m_tray_image, 24, 24);
#endif
    gtk_widget_show (m_tray_image);

    m_initialized = true;
}
