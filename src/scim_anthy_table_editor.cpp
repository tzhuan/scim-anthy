/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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

#include "scim_anthy_table_editor.h"

#include <string.h>
#include <ctype.h>
#include "scim_anthy_intl.h"

static void     scim_anthy_table_editor_class_init (ScimAnthyTableEditorClass *klass);
static void     scim_anthy_table_editor_init       (ScimAnthyTableEditor      *object);

static void add_entry                       (ScimAnthyTableEditor *editor);
static void remove_entry                    (ScimAnthyTableEditor *editor);

static gint compare_sequence_string         (GtkTreeModel     *model,
                                             GtkTreeIter      *a,
                                             GtkTreeIter      *b,
                                             gpointer          user_data);
static gint compare_result_string           (GtkTreeModel     *model,
                                             GtkTreeIter      *a,
                                             GtkTreeIter      *b,
                                             gpointer          user_data);

static void on_table_view_selection_changed (GtkTreeSelection *selection,
                                             gpointer          data);
static void on_add_button_clicked           (GtkButton        *button,
                                             gpointer          data);
static void on_remove_button_clicked        (GtkButton        *button,
                                             gpointer          data);
static void on_entry_activate               (GtkEntry         *entry,
                                             gpointer          data);
static void on_entry_changed                (GtkEditable      *editable,
                                             gpointer          data);
static void on_sequence_entry_insert_text   (GtkEditable      *editable,
                                             const gchar      *text,
                                             gint              length,
                                             gint             *position,
                                             gpointer          data);

static GtkDialogClass *parent_class = NULL;

GType
scim_anthy_table_editor_get_type (void)
{
    static GType type = 0;

    if (!type) {
        static const GTypeInfo info = {
            sizeof (ScimAnthyTableEditorClass),
            NULL,           /* base_init */
            NULL,           /* base_finalize */
            (GClassInitFunc) scim_anthy_table_editor_class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (ScimAnthyTableEditor),
            0,              /* n_preallocs */
            (GInstanceInitFunc) scim_anthy_table_editor_init,
        };

        type = g_type_register_static (GTK_TYPE_DIALOG,
                                       "ScimAnthyTableEditor",
                                       &info, (GTypeFlags) 0);
    }

    return type;
}

static void
scim_anthy_table_editor_class_init (ScimAnthyTableEditorClass *klass)
{
#if 0
    GtkObjectClass *gtk_object_class = GTK_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class     = GTK_WIDGET_CLASS (klass);
#endif  

    parent_class = (GtkDialogClass *) g_type_class_peek_parent (klass);
}

static void
scim_anthy_table_editor_init (ScimAnthyTableEditor *editor)
{
    GtkWidget *label;

#if 0
    GtkWidget *dialog = gtk_dialog_new_with_buttons (
        _("Customize romaji table"),
        parent,
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_STOCK_CLOSE, GTK_RESPONSE_NONE,
        NULL);
#endif

    gtk_window_set_default_size (GTK_WINDOW (editor), 350, 250);
    gtk_window_set_position (GTK_WINDOW (editor),
                             GTK_WIN_POS_CENTER_ON_PARENT);

    // option menu area
    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editor)->vbox), hbox,
                        FALSE, FALSE, 0);
    gtk_widget_show(hbox);

#if 0
    label = gtk_label_new_with_mnemonic (_("Romaji _table:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);

    GtkWidget *omenu = gtk_option_menu_new ();
    __widget_romaji_theme_menu2 = omenu;
    g_object_add_weak_pointer (G_OBJECT (omenu),
                               (gpointer*) &__widget_romaji_theme_menu2);
    gtk_box_pack_start (GTK_BOX (hbox), omenu, FALSE, FALSE, 2);
    setup_romaji_theme_menu (GTK_OPTION_MENU (omenu));
    gtk_option_menu_set_history
        (Gtk_OPTION_MENU (omenu),
         gtk_option_menu_get_history (
             GTK_OPTION_MENU (__widget_romaji_theme_menu)));
    gtk_widget_show (omenu);

    gtk_label_set_mnemonic_widget (GTK_LABEL(label), omenu);
#endif

#if 0
    GtkWidget *button = gtk_button_new_with_mnemonic ("Save _as...");
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
    gtk_widget_show (button);
#endif


    // edit area
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editor)->vbox), hbox,
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
    editor->treeview = treeview;
    gtk_container_add (GTK_CONTAINER (scrwin), treeview);
    gtk_widget_show (treeview);

    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (on_table_view_selection_changed), editor);

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
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (store), 0,
                                     compare_sequence_string,
                                     NULL, NULL);
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
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (store), 1,
                                     compare_result_string,
                                     NULL, NULL);
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
    editor->sequence_entry = entry;
    gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 2);
    gtk_widget_set_size_request (entry, 80, -1);
    g_signal_connect (G_OBJECT (entry), "activate",
                      G_CALLBACK (on_entry_activate), editor);
    g_signal_connect (G_OBJECT (entry), "changed",
                      G_CALLBACK (on_entry_changed), editor);
    g_signal_connect (G_OBJECT (entry), "insert-text",
                      G_CALLBACK (on_sequence_entry_insert_text),
                      editor);
    gtk_widget_show (entry);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    label = gtk_label_new_with_mnemonic (_("_Result:"));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);

    entry = gtk_entry_new ();
    editor->result_entry = entry;
    gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 2);
    gtk_widget_set_size_request (entry, 80, -1);
    g_signal_connect (G_OBJECT (entry), "activate",
                      G_CALLBACK (on_entry_activate), editor);
    g_signal_connect (G_OBJECT (entry), "changed",
                      G_CALLBACK (on_entry_changed), editor);
    gtk_widget_show (entry);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

    GtkWidget *button = gtk_button_new_from_stock (GTK_STOCK_ADD);
    editor->add_button = button;
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 5);
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_add_button_clicked), editor);
    gtk_widget_set_sensitive (button, FALSE);
    gtk_widget_show (button);

    button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    editor->remove_button = button;
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 5);
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_remove_button_clicked), editor);
    gtk_widget_set_sensitive (button, FALSE);
    gtk_widget_show (button);

    // set data and connect signals
#if 0
    setup_romaji_window_value (GTK_TREE_VIEW (treeview));
    g_signal_connect (G_OBJECT (omenu), "changed",
                      G_CALLBACK (on_romaji_theme_menu_changed),
                      treeview);
#endif

    // clearn
    g_object_unref (store);
}

GtkWidget *
scim_anthy_table_editor_new (void)
{
    return GTK_WIDGET(g_object_new (SCIM_ANTHY_TYPE_TABLE_EDITOR,
                                    NULL));
}


static void
add_entry (ScimAnthyTableEditor *editor)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (editor->treeview);
    GtkTreeModel *model = gtk_tree_view_get_model (treeview);
    GtkTreeIter iter;
 
    gboolean go_next;
    bool found = false;
    const gchar *sequence, *result;
    sequence = gtk_entry_get_text (GTK_ENTRY (editor->sequence_entry));
    result   = gtk_entry_get_text (GTK_ENTRY (editor->result_entry));

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
                        2, "",
                        -1);

    GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_view_set_cursor (treeview, path, NULL, FALSE);
    gtk_tree_path_free (path);

#if 0
    // real add
    __user_style_file.set_string (__romaji_fund_table,
                                  sequence, result);

    // change menu item to "User defined"
    gtk_option_menu_set_history (
        GTK_OPTION_MENU (__widget_romaji_theme_menu2), 0);

    __style_changed = true;
#endif
}

static void
remove_entry (ScimAnthyTableEditor *editor)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (editor->treeview);
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

    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

    g_free (sequence);

#if 0
    // real remove
    __user_style_file.delete_key (__romaji_fund_table, sequence);

    // change menu item to "User deined"
    gtk_option_menu_set_history (
        GTK_OPTION_MENU (__widget_romaji_theme_menu2), 0);

    __style_changed = true;
#endif
}

static gint
compare_sequence_string (GtkTreeModel *model,
                         GtkTreeIter *a,
                         GtkTreeIter *b,
                         gpointer user_data)
{
    gint ret;

    gchar *seq1 = NULL, *seq2 = NULL;
    gtk_tree_model_get (model, a,
                        0, &seq1,
                        -1);
    gtk_tree_model_get (model, b,
                        0, &seq2,
                        -1);
    ret = strcmp (seq1, seq2);
    g_free (seq1);
    g_free (seq2);

    if (ret == 0) {
        gchar *res1 = NULL, *res2 = NULL;
        gtk_tree_model_get (model, a,
                            1, &res1,
                            -1);
        gtk_tree_model_get (model, b,
                            1, &res2,
                            -1);
        ret = strcmp (res1, res2);
        g_free (res1);
        g_free (res2);
    }

    return ret;
}

static gint
compare_result_string (GtkTreeModel *model,
                       GtkTreeIter *a,
                       GtkTreeIter *b,
                       gpointer user_data)
{
    gint ret;

    gchar *res1 = NULL, *res2 = NULL;
    gtk_tree_model_get (model, a,
                        1, &res1,
                        -1);
    gtk_tree_model_get (model, b,
                        1, &res2,
                        -1);
    ret = strcmp (res1, res2);
    g_free (res1);
    g_free (res2);

    if (ret == 0) {
        gchar *seq1 = NULL, *seq2 = NULL;
        gtk_tree_model_get (model, a,
                            0, &seq1,
                            -1);
        gtk_tree_model_get (model, b,
                            0, &seq2,
                            -1);
        ret = strcmp (seq1, seq2);
        g_free (seq1);
        g_free (seq2);
    }

    return ret;
}

static void
on_table_view_selection_changed (GtkTreeSelection *selection, gpointer data)
{
    ScimAnthyTableEditor *editor = SCIM_ANTHY_TABLE_EDITOR (data);
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;

    gboolean selected;

    selected = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (editor->remove_button) {
        if (selected) {
            gtk_widget_set_sensitive (editor->remove_button, true);
        } else {
            gtk_widget_set_sensitive (editor->remove_button, false);
        }
    }

    if (selected) {
        gchar *sequence = NULL, *result = NULL;
        gtk_tree_model_get (model, &iter,
                            0, &sequence,
                            1, &result,
                            -1);
        if (editor->sequence_entry)
            gtk_entry_set_text (GTK_ENTRY (editor->sequence_entry),
                                sequence);
        if (editor->result_entry)
            gtk_entry_set_text (GTK_ENTRY (editor->result_entry),
                                result);
        g_free (sequence);
        g_free (result);
    } else {
        if (editor->sequence_entry)
            gtk_entry_set_text (GTK_ENTRY (editor->sequence_entry), "");
        if (editor->result_entry)
            gtk_entry_set_text (GTK_ENTRY (editor->result_entry), "");
    }
}

static void
on_add_button_clicked (GtkButton *button, gpointer data)
{
    ScimAnthyTableEditor *editor = SCIM_ANTHY_TABLE_EDITOR (data);
    add_entry (editor);
}

static void
on_remove_button_clicked (GtkButton *button, gpointer data)
{
    ScimAnthyTableEditor *editor = SCIM_ANTHY_TABLE_EDITOR (data);
    remove_entry (editor);
}

static void
on_entry_activate (GtkEntry *entry, gpointer data)
{
    ScimAnthyTableEditor *editor = SCIM_ANTHY_TABLE_EDITOR (data);
    add_entry (editor);
}

static void
on_entry_changed (GtkEditable *editable, gpointer data)
{
    ScimAnthyTableEditor *editor = SCIM_ANTHY_TABLE_EDITOR (data);
    const char *seq, *res;
    seq = gtk_entry_get_text (GTK_ENTRY (editor->sequence_entry));
    res = gtk_entry_get_text (GTK_ENTRY (editor->result_entry));
    gtk_widget_set_sensitive (editor->add_button,
                              seq && *seq && res && *res);
}

static void
on_sequence_entry_insert_text (GtkEditable *editable,
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
