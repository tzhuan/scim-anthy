/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004-2005 Takuro Ashie
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

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_EVENT
#define SCIM_ANTHY_USE_GTK
#include <scim.h>
#include "scim_anthy_intl.h"
#include "scim_anthy_style_file.h"
#include "scim_anthy_prefs.h"
#include "scim_anthy_default_tables.h"

namespace scim_anthy {

StringConfigData *find_string_config_entry (const char *config_key);

GtkWidget *create_check_button (const char *config_key);
void       create_spin_button  (const char *config_key,
                                GtkTable   *table,
                                int         i);
GtkWidget *create_entry        (StringConfigData *data,
                                GtkTable   *table,
                                int         i);
GtkWidget *create_combo        (const char *config_key,
                                gpointer    candidates_p,
                                GtkWidget  *table,
                                gint        idx);
GtkWidget *create_option_menu  (const char *config_key,
                                gpointer    candidates_p);
GtkWidget *create_color_button (const char *config_key);

extern StyleFiles __style_list;
extern StyleFile  __user_style_file;
extern bool __config_changed;
extern bool __style_changed;

};
