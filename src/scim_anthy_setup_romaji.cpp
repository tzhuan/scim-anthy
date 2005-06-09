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
#include "scim_anthy_setup.h"

using namespace scim;

namespace scim_anthy {

#define INDEX_KEY "scim-anthy::Index"

// Internal data declaration.
static GtkWidget   * __widget_romaji_theme_menu     = NULL;
static GtkWidget   * __widget_romaji_sequence_entry = NULL;
static GtkWidget   * __widget_romaji_result_entry   = NULL;
static GtkWidget   * __widget_romaji_add_button     = NULL;
static GtkWidget   * __widget_romaji_remove_button  = NULL;

static String __config_romaji_theme = SCIM_ANTHY_CONFIG_ROMAJI_THEME_DEFAULT;


static GtkWidget *create_romaji_window            (GtkWindow *parent);

static void     setup_romaji_page                 (void);
static void     setup_romaji_theme_menu           (GtkOptionMenu *omenu);
static void     setup_romaji_window_value         (GtkTreeView *treeview);

static bool     load_romaji_theme                 (void);

static void     add_romaji_entry                  (GtkTreeView *treeview);
static void     remove_romaji_entry               (GtkTreeView *treeview);

static void     on_romaji_theme_menu_changed      (GtkOptionMenu    *omenu,
                                                   gpointer          user_data);
static void     on_romaji_customize_button_clicked(GtkWidget        *button,
                                                   gpointer          data);
static void     on_romaji_add_button_clicked      (GtkButton        *button,
                                                   gpointer          data);
static void     on_romaji_remove_button_clicked   (GtkButton        *button,
                                                   gpointer          data);
static void     on_romaji_view_selection_changed  (GtkTreeSelection *selection,
                                                   gpointer          data);
static void     on_romaji_entry_activate          (GtkEntry         *entry,
                                                   gpointer          data);
static void     on_romaji_entry_changed           (GtkEditable      *editable,
                                                   gpointer          data);
static void     on_romaji_sequence_entry_insert_text
                                                  (GtkEditable      *editable,
                                                   const gchar      *text,
                                                   gint              length,
                                                   gint             *position,
                                                   gpointer          data);

GtkWidget *
romaji_page_create_ui (void)
{
    GtkWidget *vbox, *widget;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    /* symbol */
    widget = create_check_button (SCIM_ANTHY_CONFIG_ROMAJI_HALF_SYMBOL);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* number */
    widget = create_check_button (SCIM_ANTHY_CONFIG_ROMAJI_HALF_NUMBER);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* romaji splitting */
    widget = create_check_button (SCIM_ANTHY_CONFIG_ROMAJI_ALLOW_SPLIT);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* romaji table */
    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    GtkWidget *label = gtk_label_new_with_mnemonic (_("Romaji _table:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);

    GtkWidget *omenu = gtk_option_menu_new ();
    __widget_romaji_theme_menu = omenu;
    g_signal_connect (G_OBJECT (omenu), "changed",
                      G_CALLBACK (on_romaji_theme_menu_changed), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), omenu, FALSE, FALSE, 2);
    gtk_widget_show (omenu);

    gtk_label_set_mnemonic_widget (GTK_LABEL(label), omenu);

    GtkWidget *button = gtk_button_new_with_mnemonic (_("_Customize..."));
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_romaji_customize_button_clicked), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
    gtk_widget_show (button);

    setup_romaji_page ();

    return vbox;
}

void
romaji_page_load_config (const ConfigPointer &config)
{
    __config_romaji_theme
        = config->read (String (SCIM_ANTHY_CONFIG_ROMAJI_THEME),
                        String (SCIM_ANTHY_CONFIG_ROMAJI_THEME_DEFAULT));
    setup_romaji_page ();
}

void
romaji_page_save_config (const ConfigPointer &config)
{
    __config_romaji_theme
        = config->write (String (SCIM_ANTHY_CONFIG_ROMAJI_THEME),
                         String (__config_romaji_theme));
}

bool
romaji_page_query_changed (void)
{
    return __config_changed || __style_changed;
}


static GtkWidget *
create_romaji_window (GtkWindow *parent)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons (
        _("Customize romaji table"),
        parent,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_STOCK_CLOSE, GTK_RESPONSE_NONE,
        NULL);
    gtk_window_set_default_size (GTK_WINDOW (dialog), 350, 250);
    gtk_window_set_position (GTK_WINDOW (dialog),
                             GTK_WIN_POS_CENTER_ON_PARENT);

    // option menu area
    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox,
                        FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    GtkWidget *label = gtk_label_new_with_mnemonic (_("Romaji _table:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);

    GtkWidget *omenu = gtk_option_menu_new ();
    gtk_box_pack_start (GTK_BOX (hbox), omenu, FALSE, FALSE, 2);
    setup_romaji_theme_menu (GTK_OPTION_MENU (omenu));
    gtk_option_menu_set_history
        (GTK_OPTION_MENU (omenu),
         gtk_option_menu_get_history (
             GTK_OPTION_MENU (__widget_romaji_theme_menu)));
    gtk_widget_show (omenu);

    gtk_label_set_mnemonic_widget (GTK_LABEL(label), omenu);


    // edit area
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox,
                        TRUE, TRUE, 0);
    gtk_widget_show (hbox);

    // tree view area
    GtkWidget *scrwin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrwin),
                                         GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrwin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (hbox), scrwin, TRUE, TRUE, 0);
    gtk_widget_show (scrwin);

    GtkListStore *store = gtk_list_store_new (3,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING);

    GtkWidget *treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    gtk_container_add (GTK_CONTAINER (scrwin), treeview);
    gtk_widget_show (treeview);

    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (on_romaji_view_selection_changed), treeview);

    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
    gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (treeview), TRUE);
    // sequence column
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
        _("Sequence"), cell,
        "text", 0,
        NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 80);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_sort_column_id (column, 0);

    // result column
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
        _("Result"), cell,
        "text", 1,
        NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 80);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_sort_column_id (column, 1);

#if 0
    // pending column
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
        _("Pending"), cell,
        "text", 2,
        NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 80);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_sort_column_id (column, 2);
#endif

    // button area
    GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 5);
    gtk_widget_show (vbox);

    label = gtk_label_new_with_mnemonic (_("_Sequence:"));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);

    GtkWidget *entry = gtk_entry_new ();
    __widget_romaji_sequence_entry = entry;
    gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 2);
    gtk_widget_set_size_request (entry, 80, -1);
    g_signal_connect (G_OBJECT (entry), "activate",
                      G_CALLBACK (on_romaji_entry_activate), treeview);
    g_signal_connect (G_OBJECT (entry), "changed",
                      G_CALLBACK (on_romaji_entry_changed), treeview);
    g_signal_connect (G_OBJECT (entry), "insert-text",
                      G_CALLBACK (on_romaji_sequence_entry_insert_text),
                      treeview);
    gtk_widget_show (entry);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    label = gtk_label_new_with_mnemonic (_("_Result:"));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);

    entry = gtk_entry_new ();
    __widget_romaji_result_entry = entry;
    gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 2);
    gtk_widget_set_size_request (entry, 80, -1);
    g_signal_connect (G_OBJECT (entry), "activate",
                      G_CALLBACK (on_romaji_entry_activate), treeview);
    g_signal_connect (G_OBJECT (entry), "changed",
                      G_CALLBACK (on_romaji_entry_changed), treeview);
    gtk_widget_show (entry);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    GtkWidget *button = gtk_button_new_from_stock (GTK_STOCK_ADD);
    __widget_romaji_add_button = button;
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 5);
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_romaji_add_button_clicked), treeview);
    gtk_widget_set_sensitive (button, FALSE);
    gtk_widget_show (button);

    button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    __widget_romaji_remove_button = button;
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 5);
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_romaji_remove_button_clicked), treeview);
    gtk_widget_set_sensitive (button, FALSE);
    gtk_widget_show (button);

    // set data and connect signals
    setup_romaji_window_value (GTK_TREE_VIEW (treeview));
    g_signal_connect (G_OBJECT (omenu), "changed",
                      G_CALLBACK (on_romaji_theme_menu_changed),
                      treeview);

    // clearn
    g_object_unref (store);

    return dialog;
}

static void
setup_romaji_page (void)
{
    setup_romaji_theme_menu (GTK_OPTION_MENU (__widget_romaji_theme_menu));
}

static void
setup_romaji_theme_menu (GtkOptionMenu *omenu)
{
    GtkWidget *menu = gtk_menu_new ();
    gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu),
                              menu);
    gtk_widget_show (menu);

    // create menu items
    GtkWidget *menuitem = gtk_menu_item_new_with_label (_("Default"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    StyleFiles::iterator it;
    unsigned int i;
    for (i = 0, it = __style_list.begin ();
         it != __style_list.end ();
         i++, it++)
    {
        const char *section_name = "RomajiTable/FundamentalTable";
        StyleLines section;
        if (!it->get_entry_list (section, section_name))
            continue;

        menuitem = gtk_menu_item_new_with_label (_(it->get_title().c_str()));
        g_object_set_data (G_OBJECT (menuitem),
                           INDEX_KEY, GINT_TO_POINTER (i));
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        gtk_widget_show (menuitem);
    }

    // set default value
    g_signal_handlers_block_by_func (
        G_OBJECT (omenu),
        (gpointer) (on_romaji_theme_menu_changed),
        NULL);

    gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), 0);

    GList *node, *list = gtk_container_get_children (GTK_CONTAINER (menu));
    for (i = 1, node = g_list_next (list);
         node;
         i++, node = g_list_next (node))
    {
        gint idx = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (node->data),
                                                       INDEX_KEY));
        if (__style_list[idx].get_title () == __config_romaji_theme) {
            gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), i);
            break;
        }
    }

    g_signal_handlers_unblock_by_func (
        G_OBJECT (omenu),
        (gpointer) (on_romaji_theme_menu_changed),
        NULL);
}

static void
setup_romaji_window_value (GtkTreeView *treeview)
{
    GtkTreeModel *model = gtk_tree_view_get_model (treeview);
    GtkListStore *store = GTK_LIST_STORE (model);

    gtk_list_store_clear (store);

    const char *section_name = "RomajiTable/FundamentalTable";
    StyleLines section;
    __user_style_file.get_entry_list (section, section_name);
    if (section.empty ()) {
        load_romaji_theme ();
        __user_style_file.get_entry_list (section, section_name);
    }

    StyleLines::iterator it;
    for (it = section.begin (); it != section.end (); it++) {
        if (it->get_type() != SCIM_ANTHY_STYLE_LINE_KEY)
            continue;

        String key;
        WideString value;
        it->get_key (key);
        __user_style_file.get_string (value, section_name, key);
        GtkTreeIter iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            0, key.c_str (),
                            1, utf8_wcstombs(value).c_str (),
                            2, "",
                            -1);
    }
}

static bool
load_romaji_theme (void)
{
    GtkOptionMenu *omenu = GTK_OPTION_MENU (__widget_romaji_theme_menu);
    gint idx = gtk_option_menu_get_history (omenu);
    GtkWidget *menu = gtk_option_menu_get_menu (omenu);
    GList *list = gtk_container_get_children (GTK_CONTAINER (menu));
    GtkWidget *menuitem = GTK_WIDGET (g_list_nth_data (list, idx));

    if (!menuitem)
        return false;

    gint theme_idx = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (menuitem),
                                                         INDEX_KEY));

    const char *section = "RomajiTable/FundamentalTable";

    // set new key bindings
    if (idx == 0) {
        __config_romaji_theme = "Default";
        __user_style_file.delete_section (section);

        ConvRule *table = scim_anthy_romaji_typing_rule;
        for (unsigned int i = 0; table[i].string; i++) {
            __user_style_file.set_string (section,
                                          table[i].string,
                                          table[i].result);
        }
        return true;

    } else if (theme_idx >= 0) {
        __config_romaji_theme = __style_list[theme_idx].get_title ();
        __user_style_file.delete_section (section);

        StyleLines lines;
        bool success = __style_list[theme_idx].get_entry_list (
            lines, section);
        if (success) {
            StyleLines::iterator it;
            for (it = lines.begin (); it != lines.end (); it++) {
                if (it->get_type () != SCIM_ANTHY_STYLE_LINE_KEY)
                    continue;
                String key;
                WideString value;
                it->get_key (key);
                //it->get_value (value);
                __style_list[theme_idx].get_string (value, section, key);
                __user_style_file.set_string (section,
                                              key,
                                              utf8_wcstombs (value));
            }
        }
        return true;

    } else {
        // error
        return false;
    }
}

static void
add_romaji_entry (GtkTreeView *treeview)
{
    GtkTreeModel *model = gtk_tree_view_get_model (treeview);
    GtkTreeIter iter;
 
    gboolean go_next;
    bool found = false;
    const gchar *sequence, *result;
    sequence = gtk_entry_get_text (GTK_ENTRY (__widget_romaji_sequence_entry));
    result   = gtk_entry_get_text (GTK_ENTRY (__widget_romaji_result_entry));

    if (!sequence || !result)
        return;

    for (go_next = gtk_tree_model_get_iter_first (model, &iter);
         go_next;
         go_next = gtk_tree_model_iter_next (model, &iter))
    {
        gchar *seq = NULL;
        gtk_tree_model_get (model, &iter,
                            0, &seq,
                            -1);
        if (seq && !strcmp (sequence, seq)) {
            found = true;
            g_free (seq);
            break;
        }
        g_free (seq);
    }

    if (!found)
        gtk_list_store_append (GTK_LIST_STORE (model), &iter);

    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        0, sequence,
                        1, result,
                        -1);
    __user_style_file.set_string ("RomajiTable/FundamentalTable",
                                  sequence, result);

    GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_view_set_cursor (treeview, path, NULL, FALSE);
    gtk_tree_path_free (path);

    __style_changed = true;
}

static void
remove_romaji_entry (GtkTreeView *treeview)
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview);
    GtkTreeModel *model = NULL;
    GtkTreeIter iter, next;
    gboolean selected, success;

    selected = gtk_tree_selection_get_selected (selection, &model, &iter);
    if (!selected)
        return;

    gchar *sequence = NULL;
    gtk_tree_model_get (model, &iter,
                        0, &sequence,
                        -1);

    next = iter;
    success = gtk_tree_model_iter_next (model, &next);
    GtkTreePath *path = NULL;
    if (success) {
        path = gtk_tree_model_get_path (model, &next);
    } else {
        path = gtk_tree_model_get_path (model, &iter);
        if (path)
            success = gtk_tree_path_prev (path);
    }

    if (success && path)
        gtk_tree_view_set_cursor (GTK_TREE_VIEW (treeview), path, NULL, FALSE);
    if (path)
        gtk_tree_path_free (path);

    __user_style_file.delete_key ("RomajiTable/FundamentalTable", sequence);
    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

    __style_changed = true;
    g_free (sequence);
}

static void
on_romaji_theme_menu_changed (GtkOptionMenu *omenu, gpointer user_data)
{
    bool success;

    if (__widget_romaji_theme_menu != GTK_WIDGET (omenu)) {
        g_signal_handlers_block_by_func (
            G_OBJECT (__widget_romaji_theme_menu),
            (gpointer) (on_romaji_theme_menu_changed),
            NULL);
        gtk_option_menu_set_history (
            GTK_OPTION_MENU (__widget_romaji_theme_menu),
            gtk_option_menu_get_history (omenu));
        g_signal_handlers_unblock_by_func (
            G_OBJECT (__widget_romaji_theme_menu),
            (gpointer) (on_romaji_theme_menu_changed),
            NULL);

        success = load_romaji_theme ();

        setup_romaji_window_value (GTK_TREE_VIEW (user_data));
    } else {
        success = load_romaji_theme ();
    }

    if (success) {
        // sync widgets
        __style_changed  = true;
        __config_changed = true;
    }
}

static void
on_romaji_customize_button_clicked (GtkWidget *button, gpointer data)
{
    GtkWidget *widget = create_romaji_window (
        GTK_WINDOW (gtk_widget_get_toplevel (button)));
    gtk_dialog_run (GTK_DIALOG (widget));
    gtk_widget_destroy (widget);
}

static void
on_romaji_add_button_clicked (GtkButton *button, gpointer data)
{
    add_romaji_entry (GTK_TREE_VIEW (data));
}

static void
on_romaji_remove_button_clicked (GtkButton *button, gpointer data)
{
    remove_romaji_entry (GTK_TREE_VIEW (data));
}

static void
on_romaji_view_selection_changed (GtkTreeSelection *selection, gpointer data)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;

    gboolean selected;

    selected = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (__widget_romaji_remove_button) {
        if (selected) {
            gtk_widget_set_sensitive (__widget_romaji_remove_button, true);
        } else {
            gtk_widget_set_sensitive (__widget_romaji_remove_button, false);
        }
    }

    if (selected) {
        gchar *sequence = NULL, *result = NULL;
        gtk_tree_model_get (model, &iter,
                            0, &sequence,
                            1, &result,
                            -1);
        if (__widget_romaji_sequence_entry)
            gtk_entry_set_text (GTK_ENTRY (__widget_romaji_sequence_entry),
                                sequence);
        if (__widget_romaji_result_entry)
            gtk_entry_set_text (GTK_ENTRY (__widget_romaji_result_entry),
                                result);
        g_free (sequence);
        g_free (result);
    } else {
        if (__widget_romaji_sequence_entry)
            gtk_entry_set_text (GTK_ENTRY (__widget_romaji_sequence_entry), "");
        if (__widget_romaji_result_entry)
            gtk_entry_set_text (GTK_ENTRY (__widget_romaji_result_entry), "");
    }
}

static void
on_romaji_entry_activate (GtkEntry *entry, gpointer data)
{
    add_romaji_entry (GTK_TREE_VIEW (data));
}

static void
on_romaji_entry_changed (GtkEditable *editable, gpointer data)
{
    const char *seq, *res;
    seq = gtk_entry_get_text (GTK_ENTRY (__widget_romaji_sequence_entry));
    res = gtk_entry_get_text (GTK_ENTRY (__widget_romaji_result_entry));
    gtk_widget_set_sensitive (__widget_romaji_add_button,
                              seq && *seq && res && *res);
}

static void
on_romaji_sequence_entry_insert_text (GtkEditable *editable,
                                      const gchar *text,
                                      gint length,
                                      gint *position,
                                      gpointer data)
{
    for (int i = 0; i < length; i++) {
       if (!isascii (text[i]) || isspace (text[i])) {
            g_signal_stop_emission_by_name (editable, "insert_text");
            return;
        }
    }
}

}
