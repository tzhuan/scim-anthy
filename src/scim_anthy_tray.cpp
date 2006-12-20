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
#define Uses_STL_MAP
#define Uses_STL_VECTOR

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

#define SCIM_PROP_IMENGINE_ANTHY_PREFIX  "/IMEngine/Anthy/"

using namespace scim;
using namespace std;

AnthyTray::AnthyTray ()
    : m_agent    (NULL),
      m_ic       (0),
      m_ic_uuid  (String ()),
      m_tray     (NULL),
      m_hbox     (NULL),
      m_tooltips (NULL)
{
}

AnthyTray::~AnthyTray ()
{
    if (m_tooltips)
        gtk_object_destroy (GTK_OBJECT (m_tooltips));

    destroy_current_tray ();

    if (m_tray)
        gtk_object_destroy (GTK_OBJECT (m_tray));
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
    GtkWidget *menu = (GtkWidget *) user_data;
    
    gtk_widget_show_all (menu);
    gtk_menu_popup (GTK_MENU (menu),
                    NULL, NULL, NULL,
                    NULL, event->button,
                    gtk_get_current_event_time ());                    
}

void
AnthyTray::init_properties (const PropertyList &properties)
{
    destroy_current_tray ();

    if (m_tooltips == NULL)
        m_tooltips = gtk_tooltips_new ();

    if (m_tray == NULL)
        m_tray = scim_tray_icon_new ("scim-anthy-tray");

    if (m_hbox == NULL)
    {
        m_hbox = gtk_hbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (m_tray), m_hbox);
    }

    if (properties.size () == 0)
    {
        // hide all tray icons but a dummy widget
        GtkWidget *dummy = gtk_label_new ("");
        gtk_box_pack_start (GTK_BOX (m_hbox), dummy,
                            TRUE, TRUE, 0);
    }

    for(int i = 0; i < properties.size (); i++)
    {
        String key = properties[i].get_key ();
        String label = properties[i].get_label ();
        String tip = properties[i].get_tip ();
        String icon = properties[i].get_icon ();

        if (key.find (SCIM_PROP_IMENGINE_ANTHY_PREFIX) != 0)
            continue;

        key.erase (0, strlen(SCIM_PROP_IMENGINE_ANTHY_PREFIX));
        int pos;
        if ((pos = key.find ("/")) != String::npos)
        {
            String parent_key = key.substr (0, pos);
            std::map< String, TrayMenu >::iterator menu_it = m_menus.find (parent_key);

            // create a menu item with a label widget
            TrayMenuItem a_item;
            a_item.item = gtk_menu_item_new ();
            a_item.label = gtk_label_new (label.c_str ());

            gtk_misc_set_alignment (GTK_MISC (a_item.label),
                                    0.0, 0.5); // to left
            gtk_tooltips_set_tip (m_tooltips, a_item.item,
                                  tip.c_str(), tip.c_str());
            gtk_container_add (GTK_CONTAINER (a_item.item),
                               a_item.label);
            gtk_menu_append (GTK_MENU (menu_it->second.menu),
                             a_item.item);
            g_signal_connect (G_OBJECT (a_item.item), "activate",
                              G_CALLBACK (activate), this);
            
            // menu item
            m_items.insert (make_pair (key, a_item));
        }
        else
        {
            TrayMenu a_menu;

            // create a menu widget for popup
            a_menu.menu = gtk_menu_new ();

            // create a button in tray
            a_menu.button = gtk_button_new_with_label (label.c_str());
            gtk_tooltips_set_tip (m_tooltips, a_menu.button,
                                  tip.c_str(), tip.c_str());
            gtk_button_set_relief (GTK_BUTTON (a_menu.button),
                                   GTK_RELIEF_NONE);
            if (icon.size() != 0)
            {
                GdkPixbuf *buf = gdk_pixbuf_new_from_file_at_size (icon.c_str (),
                                                                   16, 16,
                                                                   NULL);
                if (buf)
                    gtk_button_set_image (GTK_BUTTON (a_menu.button),
                                          gtk_image_new_from_pixbuf (buf));
            }
            if (m_hbox)
                gtk_box_pack_start (GTK_BOX (m_hbox), a_menu.button,
                                    TRUE, TRUE, 0);
            g_signal_connect (G_OBJECT (a_menu.button), "button-release-event",
                              G_CALLBACK (popup), a_menu.menu);


            // menu
            m_menus.insert (make_pair (key, a_menu));
        }
    }

    gtk_widget_show_all (GTK_WIDGET (m_tray));
}

void
AnthyTray::update_property (const Property &property)
{
    String key = property.get_key ();
    String label = property.get_label ();
    String tip = property.get_tip ();
    String icon = property.get_icon ();

    if (key.find (SCIM_PROP_IMENGINE_ANTHY_PREFIX) != 0)
        return;

    key.erase (0, strlen(SCIM_PROP_IMENGINE_ANTHY_PREFIX));
    if (key.find ("/") != String::npos)
    {
        // menu item
        std::map< String, TrayMenuItem >::iterator it;
        it = m_items.find (key);
        if (it != m_items.end ())
        {
            if (it->second.label)
                gtk_label_set_text (GTK_LABEL (it->second.label),
                                    label.c_str ());
        }
    }
    else
    {
        // menu tray
        std::map< String, TrayMenu >::iterator it;
        it = m_menus.find (key);
        if (it != m_menus.end ())
        {
            if (it->second.button)
            {
                gtk_button_set_label (GTK_BUTTON (it->second.button),
                                      label.c_str ());
                if (icon.size () != 0)
                    gtk_button_set_image (GTK_BUTTON (it->second.button),
                                          gtk_image_new_from_file (icon.c_str ()));
            }
        }
    }
}

void
AnthyTray::activated_item         (GtkMenuItem *menuitem)
{
    if (m_agent)
    {
        // find key
        String key = String (SCIM_PROP_IMENGINE_ANTHY_PREFIX);
        std::map< String, TrayMenuItem >::iterator it = m_items.begin ();
        while (it != m_items.end ())
        {
            if (it->second.item == (GtkWidget *) menuitem)
            {
                key += it->first;
                break;
            }

            ++it;
        }

        if (key.size () == 0)
            return;

        Transaction send;
        send.put_command (SCIM_ANTHY_TRANS_CMD_TRIGGER_PROPERTY);
        send.put_data (key);
        m_agent->send_imengine_event (m_ic, m_ic_uuid, send);
    }
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
AnthyTray::destroy_current_tray ()
{
    std::map< String, TrayMenuItem >::iterator item_it = m_items.begin ();
    while (item_it != m_items.end ())
    {
        if (item_it->second.label)
            gtk_widget_destroy (item_it->second.label);

        if (item_it->second.item)
            gtk_widget_destroy (item_it->second.item);

        ++item_it;
    }

    std::map< String, TrayMenu >::iterator menu_it = m_menus.begin ();
    while (menu_it != m_menus.end ())
    {
        if (menu_it->second.button)
            gtk_widget_destroy (menu_it->second.button);

        if (menu_it->second.menu)
            gtk_widget_destroy (menu_it->second.menu);

        ++menu_it;
    }

    if (m_hbox)
    {
        gtk_container_remove (GTK_CONTAINER (m_tray), m_hbox);
        gtk_widget_destroy (m_hbox);
        m_hbox = NULL;
    }

    if (m_tooltips)
    {
        gtk_object_destroy (GTK_OBJECT (m_tooltips));
        m_tooltips = NULL;
    }

    m_menus.clear ();
    m_items.clear ();
}
