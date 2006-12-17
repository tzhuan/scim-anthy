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

#define Uses_SCIM_HELPER
#define Uses_SCIM_CONFIG_BASE
#define Uses_STL_MAP
#include <gtk/scimtrayicon.h>
#include <gtk/gtk.h>

using namespace scim;

class AnthyTray; // pre definition

typedef struct _TrayMenuItem
{
    GtkWidget *item;
    GtkWidget *label;
} TrayMenuItem;

typedef struct _ActivateEvent
{
    char      *key;
    AnthyTray *owner;
} ActivateEvent;

typedef struct _TrayMenu
{
    GtkWidget *button;
    GtkWidget *menu;
} TrayMenu;

class AnthyTray
{
public:
    AnthyTray ();
    ~AnthyTray ();
    void init_properties        (const PropertyList &properties);
    void update_property        (const Property     &property);
    void activated_item         (GtkMenuItem *menuitem);
    void attach_input_context   (const HelperAgent  *agent,
                                 int                 ic,
                                 const String       &ic_uuid);

private:
    void destroy_current_tray   (void);

private:
    const HelperAgent           *m_agent;
    int                          m_ic;
    String                       m_ic_uuid;

    ScimTrayIcon                *m_tray;
    GtkWidget                   *m_hbox;
    GtkTooltips                 *m_tooltips;
    std::map< String, TrayMenu >      m_menus;
    std::map< String, TrayMenuItem >  m_items;
};

#endif /* __SCIM_ANTHY_TRAY_H__ */
