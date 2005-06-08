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

/*
 * Based on scim-hangul.
 * Copyright (c) 2004 James Su <suzhe@turbolinux.com.cn>
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
#include <gtk/scimkeyselection.h>
#include "scim_anthy_intl.h"
#include "scim_anthy_style_file.h"
#include "scim_anthy_prefs.h"
#include "scim_anthy_default_tables.h"

using namespace scim;
using namespace scim_anthy;

#define scim_module_init anthy_imengine_setup_LTX_scim_module_init
#define scim_module_exit anthy_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       anthy_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    anthy_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        anthy_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description anthy_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     anthy_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     anthy_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   anthy_imengine_setup_LTX_scim_setup_module_query_changed

#define DATA_POINTER_KEY "scim-anthy::ConfigPointer"

static GtkWidget * create_setup_window ();
static void        load_style_files    (const char *dirname);
static void        load_config         (const ConfigPointer &config);
static void        save_config         (const ConfigPointer &config);
static bool        query_changed       (void);

static StyleFiles __style_list;
static StyleFile  __user_style_file;
const String __user_config_dir_name =
    scim_get_home_dir () +
    String (SCIM_PATH_DELIM_STRING
            ".scim"
            SCIM_PATH_DELIM_STRING
            "Anthy");
const String __user_style_dir_name =
    __user_config_dir_name +
    String (SCIM_PATH_DELIM_STRING
            "style");
const String __user_style_file_name =
    __user_config_dir_name +
    String (SCIM_PATH_DELIM_STRING
            "config.sty");

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_ANTHY_LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("Anthy"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("An Anthy IMEngine Module."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"


// Internal data structure
struct KeyboardConfigPage
{
    const char       *label;
    StringConfigData *data;
};

struct ComboConfigCandidate
{
    const char *label;
    const char *data;
};

enum {
    COLUMN_LABEL = 0,
    COLUMN_VALUE = 1,
    COLUMN_DESC  = 2,
    COLUMN_DATA  = 3,
    N_COLUMNS    = 4,
};

// Internal data declaration.
static bool __config_changed = true;
static bool __style_changed  = true;
static GtkWidget   * __widget_key_categories_menu   = NULL;
static GtkWidget   * __widget_key_filter            = NULL;
static GtkWidget   * __widget_key_filter_button     = NULL;
static GtkWidget   * __widget_key_theme_menu        = NULL;
static GtkWidget   * __widget_key_list_view         = NULL;
static GtkWidget   * __widget_choose_keys_button    = NULL;
static GtkWidget   * __widget_romaji_theme_menu     = NULL;
static GtkWidget   * __widget_romaji_sequence_entry = NULL;
static GtkWidget   * __widget_romaji_result_entry   = NULL;
static GtkWidget   * __widget_romaji_add_button     = NULL;
static GtkWidget   * __widget_romaji_remove_button  = NULL;
static GtkTooltips * __widget_tooltips              = NULL;

static String __config_key_theme    = SCIM_ANTHY_CONFIG_KEY_THEME_DEFAULT;
static String __config_romaji_theme = SCIM_ANTHY_CONFIG_ROMAJI_THEME_DEFAULT;

static struct KeyboardConfigPage __key_conf_pages[] =
{
    {N_("Mode keys"),          config_keyboards_mode},
    {N_("Edit keys"),          config_keyboards_edit},
    {N_("Caret keys"),         config_keyboards_caret},
    {N_("Segments keys"),      config_keyboards_segments},
    {N_("Candidates keys"),    config_keyboards_candidates},
    {N_("Direct select keys"), config_keyboards_direct_select},
    {N_("Convert keys"),       config_keyboards_converting},
    {N_("Dictionary keys"),    config_keyboards_dict},
};
static unsigned int __key_conf_pages_num = sizeof (__key_conf_pages) / sizeof (KeyboardConfigPage);

const int INDEX_SEARCH_BY_KEY = __key_conf_pages_num;
const int INDEX_ALL           = __key_conf_pages_num + 1;

static ComboConfigCandidate typing_methods[] =
{
    {N_("Romaji typing method"), "Roma"},
    {N_("Kana typing method"),   "Kana"},
    {NULL, NULL},
};

static ComboConfigCandidate period_styles[] =
{
    {"\xE3\x80\x81\xE3\x80\x82", "Japanese"},
    {"\xEF\xBC\x8C\xE3\x80\x82", "WideLatin_Japanese"},
    {"\xEF\xBC\x8C\xEF\xBC\x8E", "WideLatin"},
    {",.",                       "Latin"},
    {NULL, NULL},
};

static ComboConfigCandidate space_types[] =
{
    {N_("Wide"),              "Wide"},
    {N_("Half"),              "Half"},
    {N_("Follow input mode"), "FollowMode"},
    {NULL, NULL},
};

static ComboConfigCandidate ten_key_types[] =
{
    {N_("Wide"),              "Wide"},
    {N_("Half"),              "Half"},
    {N_("Follow input mode"), "FollowMode"},
    {NULL, NULL},
};


static void     on_default_editable_changed       (GtkEditable      *editable,
                                                   gpointer          user_data);
static void     on_default_toggle_button_toggled  (GtkToggleButton  *togglebutton,
                                                   gpointer          user_data);
static void     on_default_key_selection_clicked  (GtkButton        *button,
                                                   gpointer          user_data);
static void     on_default_combo_changed          (GtkEditable      *editable,
                                                   gpointer          user_data);
static void     on_key_filter_selection_clicked   (GtkButton        *button,
                                                   gpointer          user_data);
static void     on_dict_menu_label_toggled        (GtkToggleButton  *togglebutton,
                                                   gpointer          user_data);
static void     on_key_category_menu_changed      (GtkOptionMenu    *omenu,
                                                   gpointer          user_data);
static gboolean on_key_list_view_key_press        (GtkWidget        *widget,
                                                   GdkEventKey      *event,
                                                   gpointer          user_data);
static gboolean on_key_list_view_button_press     (GtkWidget        *widget,
                                                   GdkEventButton   *event,
                                                   gpointer          user_data);
static void     on_key_theme_menu_changed         (GtkOptionMenu    *omenu,
                                                   gpointer          user_data);
static void     on_choose_keys_button_clicked     (GtkWidget        *button,
                                                   gpointer          data);
static void     on_key_list_selection_changed     (GtkTreeSelection *selection,
                                                   gpointer          data);
static void     on_romaji_theme_menu_changed      (GtkOptionMenu    *omenu,
                                                   gpointer          user_data);
static void     on_romaji_customize_button_clicked(GtkWidget        *button,
                                                   gpointer          data);
static void     setup_widget_value ();


static BoolConfigData *
find_bool_config_entry (const char *config_key)
{
    if (!config_key)
        return NULL;

    for (unsigned int i = 0; config_bool_common[i].key; i++) {
        BoolConfigData *entry = &config_bool_common[i];
        if (entry->key && !strcmp (entry->key, config_key))
            return entry;
    }

    return NULL;
}

static StringConfigData *
find_string_config_entry (const char *config_key)
{
    if (!config_key)
        return NULL;

    for (unsigned int i = 0; config_string_common[i].key; i++) {
        StringConfigData *entry = &config_string_common[i];
        if (entry->key && !strcmp (entry->key, config_key))
            return entry;
    }

    return NULL;
}

static StringConfigData *
find_key_config_entry (const char *config_key)
{
    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; i++) {
            StringConfigData *entry = &__key_conf_pages[j].data[i];
            if (entry->key && !strcmp (entry->key, config_key))
                return entry;
        }
    }

    return NULL;
}

static bool
match_key_event (const KeyEventList &list, const KeyEvent &key)
{
    KeyEventList::const_iterator kit;

    for (kit = list.begin (); kit != list.end (); kit++) {
        if (key.code == kit->code && key.mask == kit->mask)
             return true;
    }
    return false;
}

static GtkWidget *
create_check_button (const char *config_key)
{
    BoolConfigData *entry = find_bool_config_entry (config_key);
    if (!entry)
        return NULL;

    entry->widget = gtk_check_button_new_with_mnemonic (_(entry->label));
    gtk_container_set_border_width (GTK_CONTAINER (entry->widget), 4);
    g_signal_connect (G_OBJECT (entry->widget), "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      entry);
    gtk_widget_show (GTK_WIDGET (entry->widget));

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, GTK_WIDGET (entry->widget),
                              _(entry->tooltip), NULL);

    return GTK_WIDGET (entry->widget);
}

#define APPEND_ENTRY(data, i)                                                  \
{                                                                              \
    label = gtk_label_new (NULL);                                              \
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _((data)->label));    \
    gtk_widget_show (label);                                                   \
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);                       \
    gtk_misc_set_padding (GTK_MISC (label), 4, 0);                             \
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,                  \
                      (GtkAttachOptions) (GTK_FILL),                           \
                      (GtkAttachOptions) (GTK_FILL), 4, 4);                    \
    (data)->widget = gtk_entry_new ();                                         \
    gtk_label_set_mnemonic_widget (GTK_LABEL (label),                          \
                                   GTK_WIDGET ((data)->widget));               \
    g_signal_connect ((gpointer) (data)->widget, "changed",                    \
                      G_CALLBACK (on_default_editable_changed),                \
                      (data));                                                 \
    gtk_widget_show (GTK_WIDGET ((data)->widget));                             \
    gtk_table_attach (GTK_TABLE (table), GTK_WIDGET ((data)->widget),          \
                      1, 2, i, i+1,                                            \
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),                \
                      (GtkAttachOptions) (GTK_FILL), 4, 4);                    \
                                                                               \
    if (!__widget_tooltips)                                                    \
        __widget_tooltips = gtk_tooltips_new();                                \
    if ((data)->tooltip)                                                       \
        gtk_tooltips_set_tip (__widget_tooltips, GTK_WIDGET ((data)->widget),  \
                              _((data)->tooltip), NULL);                       \
}

static GtkWidget *
create_combo (const char *config_key, gpointer candidates_p,
              GtkWidget *table, gint idx)
{
    StringConfigData *entry = find_string_config_entry (config_key);
    if (!entry)
        return NULL;

    GtkWidget *label;

    label = gtk_label_new_with_mnemonic (_(entry->label));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label), 4, 0);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, idx, idx + 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);
    gtk_widget_show (label);

    entry->widget = gtk_combo_new ();
    gtk_label_set_mnemonic_widget (GTK_LABEL (label),
                                   GTK_COMBO (entry->widget)->entry);
    gtk_combo_set_value_in_list (GTK_COMBO (entry->widget), TRUE, FALSE);
    gtk_combo_set_case_sensitive (GTK_COMBO (entry->widget), TRUE);
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (entry->widget)->entry),
                            FALSE);
    gtk_widget_show (GTK_WIDGET (entry->widget));
    gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (entry->widget),
                      1, 2, idx, idx + 1,
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);
    g_object_set_data (G_OBJECT (GTK_COMBO (entry->widget)->entry),
                       DATA_POINTER_KEY,
                       (gpointer) candidates_p);

    g_signal_connect ((gpointer) GTK_COMBO (entry->widget)->entry, "changed",
                      G_CALLBACK (on_default_combo_changed),
                      entry);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, GTK_WIDGET (entry->widget),
                              _(entry->tooltip), NULL);

    return GTK_WIDGET (entry->widget);
}

static void
append_key_bindings (GtkTreeView *treeview, gint idx, const gchar *filter)
{
    GtkListStore *store = GTK_LIST_STORE (gtk_tree_view_get_model (treeview));
    KeyEventList keys1, keys2;
    
    if (filter && *filter)
        scim_string_to_key_list (keys1, filter);

    if (idx < 0 || idx >= (gint) __key_conf_pages_num)
        return;

    for (unsigned int i = 0; __key_conf_pages[idx].data[i].key; i++) {
        if (filter && *filter) {
            scim_string_to_key_list (
                keys2,
                __key_conf_pages[idx].data[i].value.c_str());
            KeyEventList::const_iterator kit;
            bool found = true;
            for (kit = keys1.begin (); kit != keys1.end (); kit++) {
                if (!match_key_event (keys2, *kit)) {
                    found = false;
                    break;
                }
            }
            if (!found)
                continue;
        }

        GtkTreeIter iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (
            store, &iter,
            COLUMN_LABEL, _(__key_conf_pages[idx].data[i].label),
            COLUMN_VALUE, __key_conf_pages[idx].data[i].value.c_str (),
            COLUMN_DESC,  _(__key_conf_pages[idx].data[i].tooltip),
            COLUMN_DATA, &__key_conf_pages[idx].data[i],
            -1);
    }
}

static void
key_list_view_popup_key_selection (GtkTreeView *treeview)
{
    GtkTreeModel *model = gtk_tree_view_get_model (treeview);
    GtkTreePath *treepath = NULL;
    GtkTreeIter iter;

    gtk_tree_view_get_cursor (treeview, &treepath, NULL);
    if (!treepath) return;
    gtk_tree_model_get_iter (model, &iter, treepath);
    gtk_tree_path_free (treepath);

    StringConfigData *data;
    gtk_tree_model_get (model, &iter,
                        COLUMN_DATA, &data,
                        -1);
    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys
            (SCIM_KEY_SELECTION_DIALOG (dialog),
             data->value.c_str());

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys
                (SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, data->value.c_str())) {
                data->value = keys;
                gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                                    COLUMN_VALUE, data->value.c_str(),
                                    -1);
                data->changed = true;
                __config_changed = true;
            }
        }

        gtk_widget_destroy (dialog);
    }
}

static GtkWidget *
create_common_page (void)
{
    GtkWidget *vbox, *table, *widget;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    table = gtk_table_new (4, 2, FALSE);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
    gtk_widget_show (table);

    /* typing method */
    widget = create_combo (SCIM_ANTHY_CONFIG_TYPING_METHOD,
                           (gpointer) &typing_methods,
                           table, 0);

    /* period style */
    widget = create_combo (SCIM_ANTHY_CONFIG_PERIOD_STYLE,
                           (gpointer) &period_styles,
                           table, 1);

    /* space_style */
    widget = create_combo (SCIM_ANTHY_CONFIG_SPACE_TYPE,
                           (gpointer) &space_types,
                           table, 2);

    /* ten key_style */
    widget = create_combo (SCIM_ANTHY_CONFIG_TEN_KEY_TYPE,
                           (gpointer) &ten_key_types,
                           table, 3);

    /* auto convert */
    widget = create_check_button (SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* close candidate window on select */
    widget = create_check_button (SCIM_ANTHY_CONFIG_CLOSE_CAND_WIN_ON_SELECT);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    return vbox;
}

static GtkWidget *
create_romaji_page (void)
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

#if 1
    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    // test code for romaji table theme
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
#endif

    return vbox;
}

static GtkWidget *
create_learning_page ()
{
    GtkWidget *vbox, *vbox2, *hbox, *alignment, *table;
    GtkWidget *widget, *label, *button;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 4);
    gtk_widget_show (hbox);

    label = gtk_label_new (_("<b>Enable/Disable learning</b>"));
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 4);
    gtk_widget_show (label);

    alignment = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 24, 0);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
    gtk_widget_show (alignment);

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (alignment), vbox2);
    gtk_widget_show (vbox2);

    /* maual commit */
    widget = create_check_button (SCIM_ANTHY_CONFIG_LEARN_ON_MANUAL_COMMIT);
    gtk_box_pack_start (GTK_BOX (vbox2), widget, FALSE, FALSE, 4);

    /* auto commit */
    widget = create_check_button (SCIM_ANTHY_CONFIG_LEARN_ON_AUTO_COMMIT);
    gtk_box_pack_start (GTK_BOX (vbox2), widget, FALSE, FALSE, 4);

    /* key preference */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 4);
    gtk_widget_show (hbox);

    label = gtk_label_new (_("<b>Key preferences to commit "
                             "with reversing learning preference</b>"));
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 4);
    gtk_widget_show (label);

    alignment = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 24, 0);
    gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, FALSE, 0);
    gtk_widget_show (alignment);

    table = gtk_table_new (3, 3, FALSE);
    gtk_container_add (GTK_CONTAINER (alignment), table);
    gtk_widget_show (table);

    for (unsigned int i = 0; config_keyboards_reverse_learning[i].key; i++) {
        StringConfigData *entry = &config_keyboards_reverse_learning[i];
        APPEND_ENTRY (entry, i);
        gtk_entry_set_editable (GTK_ENTRY (entry->widget), FALSE);
        button = gtk_button_new_with_label ("...");
        gtk_widget_show (button);
        gtk_table_attach (GTK_TABLE (table), button, 2, 3, i, i + 1,
                          GTK_FILL, GTK_FILL, 4, 4);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), button);
        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_default_key_selection_clicked),
                          entry);
    }

    return vbox;
}

static GtkWidget *
create_toolbar_page ()
{
    GtkWidget *vbox, *hbox, *label, *widget;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    /* show/hide toolbar label */
    widget = create_check_button (SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    widget = create_check_button (SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    widget = create_check_button (SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* dictionary menu */
    widget = create_check_button (SCIM_ANTHY_CONFIG_SHOW_DICT_LABEL);
    g_signal_connect ((gpointer) widget, "toggled",
                      G_CALLBACK (on_dict_menu_label_toggled),
                      NULL);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 4);
    gtk_widget_show (hbox);
    label = gtk_label_new ("    ");
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);
    widget = create_check_button (SCIM_ANTHY_CONFIG_SHOW_DICT_ADMIN_LABEL);
    gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 4);
    gtk_widget_show (hbox);
    label = gtk_label_new ("    ");
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);
    widget = create_check_button (SCIM_ANTHY_CONFIG_SHOW_ADD_WORD_LABEL);
    gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

    // set initial state
    on_dict_menu_label_toggled (GTK_TOGGLE_BUTTON (widget), NULL);

    return vbox;
}

static GtkWidget *
create_dict_page (void)
{
    GtkWidget *table;
    GtkWidget *label;
    StringConfigData *entry;

    table = gtk_table_new (2, 2, FALSE);
    gtk_widget_show (table);

    // dict admin command
    entry = find_string_config_entry (SCIM_ANTHY_CONFIG_DICT_ADMIN_COMMAND);
    APPEND_ENTRY(entry, 0);

    // add word command
    entry = find_string_config_entry (SCIM_ANTHY_CONFIG_ADD_WORD_COMMAND);
    APPEND_ENTRY(entry, 1);

    return table;
}

static GtkWidget *
create_keyboard_page (void)
{
    GtkWidget *vbox, *hbox;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    // category menu
    GtkWidget *label = gtk_label_new_with_mnemonic (_("_Group:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);

    GtkWidget *omenu = gtk_option_menu_new ();
    __widget_key_categories_menu = omenu;
    gtk_box_pack_start (GTK_BOX (hbox), omenu, FALSE, FALSE, 2);
    gtk_widget_show (omenu);

    gtk_label_set_mnemonic_widget (GTK_LABEL (label), omenu);

    GtkWidget *menu = gtk_menu_new ();

    GtkWidget *menuitem;

    for (unsigned int i = 0; i < __key_conf_pages_num; i++) {
        menuitem = gtk_menu_item_new_with_label (_(__key_conf_pages[i].label));
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        gtk_widget_show (menuitem);
    }

    menuitem = gtk_menu_item_new_with_label (_("Search by key"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    menuitem = gtk_menu_item_new_with_label (_("all"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), menu);
    gtk_widget_show (menu);

    GtkWidget *entry = gtk_entry_new ();
    __widget_key_filter = entry;
    gtk_entry_set_editable (GTK_ENTRY (entry), FALSE);
    gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 2);
    gtk_widget_show(entry);

    GtkWidget *button = gtk_button_new_with_label ("...");
    __widget_key_filter_button = button;
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_key_filter_selection_clicked), entry);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
    gtk_widget_show (button);

    // key bindings view
    GtkWidget *scrwin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrwin),
                                         GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (scrwin), 4);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrwin),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), scrwin, TRUE, TRUE, 2);
    gtk_widget_show (scrwin);

    GtkListStore *store = gtk_list_store_new (N_COLUMNS,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_POINTER);
    GtkWidget *treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    __widget_key_list_view = treeview;
    gtk_container_add (GTK_CONTAINER (scrwin), treeview);
    gtk_widget_show (treeview);

    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Feature"), cell,
                                                       "text", COLUMN_LABEL,
                                                       NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 120);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Key bindings"), cell,
                                                       "text", COLUMN_VALUE,
                                                       NULL);
	gtk_tree_view_column_set_fixed_width (column, 200);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Description"), cell,
                                                       "text", COLUMN_DESC,
                                                       NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

    // connect signals
    g_signal_connect (G_OBJECT (omenu), "changed",
                      G_CALLBACK (on_key_category_menu_changed), treeview);
    g_signal_connect (G_OBJECT (treeview), "key-press-event",
                      G_CALLBACK (on_key_list_view_key_press), treeview);
    g_signal_connect (G_OBJECT (treeview), "button-press-event",
                      G_CALLBACK (on_key_list_view_button_press), treeview);
    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (on_key_list_selection_changed), treeview);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

#if 1
    // test code for key bind theme
    label = gtk_label_new_with_mnemonic (_("Key bindings _theme:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
    gtk_widget_show (label);

    omenu = gtk_option_menu_new ();
    __widget_key_theme_menu = omenu;
    g_signal_connect (G_OBJECT (omenu), "changed",
                      G_CALLBACK (on_key_theme_menu_changed), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), omenu, FALSE, FALSE, 2);
    gtk_widget_show (omenu);

    gtk_label_set_mnemonic_widget (GTK_LABEL(label), omenu);
#endif

    button = gtk_button_new_with_mnemonic (_("_Choose keys..."));
    __widget_choose_keys_button = button;
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_choose_keys_button_clicked), treeview);
    gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 2);
    gtk_widget_set_sensitive (button, false);
    gtk_widget_show (button);

    // clean
    g_object_unref (G_OBJECT (store));

    return vbox;
}

static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = NULL;

    if (!window) {
        GtkWidget *notebook = gtk_notebook_new();
        gtk_notebook_popup_enable(GTK_NOTEBOOK(notebook));
        gtk_widget_show (notebook);
        window = notebook;
        gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

        // Create the common page.
        GtkWidget *page = create_common_page ();
        GtkWidget *label = gtk_label_new (_("Common"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the key bind pages.
        page = create_keyboard_page ();
        label = gtk_label_new (_("Key bindings"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the romaji page.
        page = create_romaji_page ();
        label = gtk_label_new (_("Romaji Typing"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the learning page.
        page = create_learning_page ();
        label = gtk_label_new (_("Learning"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the toolbar page.
        page = create_toolbar_page ();
        label = gtk_label_new (_("Toolbar"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the dictionary page.
        page = create_dict_page ();
        label = gtk_label_new (_("Dictionary"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // for preventing enabling left arrow.
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 1);
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);

        setup_widget_value ();
    }

    return window;
}

static void
setup_combo_value (GtkCombo *combo, const String & str)
{
    GList *list = NULL;
    const char *defval = NULL;

    ComboConfigCandidate *data
        = static_cast<ComboConfigCandidate*>
        (g_object_get_data (G_OBJECT (GTK_COMBO(combo)->entry),
                            DATA_POINTER_KEY));

    for (unsigned int i = 0; data[i].label; i++) {
        list = g_list_append (list, (gpointer) _(data[i].label));
        if (!strcmp (data[i].data, str.c_str ()))
            defval = _(data[i].label);
    }

    gtk_combo_set_popdown_strings (combo, list);
    g_list_free (list);

    if (defval)
        gtk_entry_set_text (GTK_ENTRY (combo->entry), defval);
}

static void
setup_key_theme_menu (GtkOptionMenu *omenu)
{
    GtkWidget *keymenu = gtk_menu_new ();
    gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu),
                              keymenu);
    gtk_widget_show (keymenu);

    GtkWidget *menuitem = gtk_menu_item_new_with_label (_("Default"));
    gtk_menu_shell_append (GTK_MENU_SHELL (keymenu), menuitem);
    gtk_widget_show (menuitem);

    StyleFiles::iterator it;
    for (it = __style_list.begin (); it != __style_list.end (); it++) {
        menuitem = gtk_menu_item_new_with_label (_(it->get_title().c_str()));
        gtk_menu_shell_append (GTK_MENU_SHELL (keymenu), menuitem);
        gtk_widget_show (menuitem);
    }

    // set default value
    g_signal_handlers_block_by_func (G_OBJECT (__widget_key_theme_menu),
                                     (gpointer) (on_key_theme_menu_changed),
                                     NULL);
    gtk_option_menu_set_history (
        GTK_OPTION_MENU (__widget_key_theme_menu), 0);
    for (unsigned int i = 0; i < __style_list.size (); i++) {
        if (__style_list[i].get_title () == __config_key_theme) {
            gtk_option_menu_set_history (
                GTK_OPTION_MENU (__widget_key_theme_menu), i + 1);
            break;
        }
    }
    g_signal_handlers_unblock_by_func (G_OBJECT (__widget_key_theme_menu),
                                       (gpointer) (on_key_theme_menu_changed),
                                       NULL);
}

static void
setup_romaji_theme_menu (GtkOptionMenu *omenu)
{
    GtkWidget *romajimenu = gtk_menu_new ();
    gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu),
                              romajimenu);
    gtk_widget_show (romajimenu);

    GtkWidget *menuitem = gtk_menu_item_new_with_label (_("Default"));
    gtk_menu_shell_append (GTK_MENU_SHELL (romajimenu), menuitem);
    gtk_widget_show (menuitem);

    StyleFiles::iterator it;
    for (it = __style_list.begin (); it != __style_list.end (); it++) {
        menuitem = gtk_menu_item_new_with_label (_(it->get_title().c_str()));
        gtk_menu_shell_append (GTK_MENU_SHELL (romajimenu), menuitem);
        gtk_widget_show (menuitem);
    }

    g_signal_handlers_block_by_func (
        G_OBJECT (__widget_romaji_theme_menu),
        (gpointer) (on_romaji_theme_menu_changed),
        NULL);

    // set default value
    gtk_option_menu_set_history (
        GTK_OPTION_MENU (__widget_romaji_theme_menu), 0);
    for (unsigned int i = 0; i < __style_list.size (); i++) {
        if (__style_list[i].get_title () == __config_romaji_theme) {
            gtk_option_menu_set_history (
                GTK_OPTION_MENU (__widget_romaji_theme_menu), i + 1);
            break;
        }
    }

    g_signal_handlers_unblock_by_func (
        G_OBJECT (__widget_romaji_theme_menu),
        (gpointer) (on_romaji_theme_menu_changed),
        NULL);
}

static void
setup_widget_value ()
{
    for (unsigned int i = 0; config_bool_common[i].key; i++) {
        BoolConfigData &entry = config_bool_common[i];
        if (entry.widget)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (entry.widget),
                                          entry.value);
    }

    for (unsigned int i = 0; config_string_common[i].key; i++) {
        StringConfigData &entry = config_string_common[i];
        if (entry.widget && GTK_IS_COMBO (entry.widget))
            setup_combo_value (GTK_COMBO (entry.widget), entry.value);
        else if (entry.widget && GTK_IS_ENTRY (entry.widget))
            gtk_entry_set_text (GTK_ENTRY (entry.widget),
                                entry.value.c_str ());
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; i++) {
            if (__key_conf_pages[j].data[i].widget) {
                gtk_entry_set_text (
                    GTK_ENTRY (__key_conf_pages[j].data[i].widget),
                    __key_conf_pages[j].data[i].value.c_str ());
            }
        }
    }

    for (unsigned int i = 0; config_keyboards_reverse_learning[i].key; i++) {
        if (config_keyboards_reverse_learning[i].widget) {
            gtk_entry_set_text (
                GTK_ENTRY (config_keyboards_reverse_learning[i].widget),
                config_keyboards_reverse_learning[i].value.c_str ());
        }
    }

    gtk_option_menu_set_history
        (GTK_OPTION_MENU (__widget_key_categories_menu), 0);
    gtk_widget_set_sensitive (__widget_key_filter, FALSE);
    gtk_widget_set_sensitive (__widget_key_filter_button, FALSE);
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (__widget_key_list_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));
    append_key_bindings (GTK_TREE_VIEW (__widget_key_list_view), 0, NULL);

    setup_key_theme_menu    (GTK_OPTION_MENU (__widget_key_theme_menu));
    setup_romaji_theme_menu (GTK_OPTION_MENU (__widget_romaji_theme_menu));
}

static void
load_style_files (const char *dirname)
{
	GDir *dir;
	GError *error = NULL;
	const gchar *entry;

    // load system wide style files
	dir = g_dir_open (dirname, 0, &error);
	if (error)
	{
		//g_warning ("%s", error->message);
		g_error_free (error);
	}

	if (dir) {
        while ((entry = g_dir_read_name (dir)))
        {
            String file = dirname;
            file += SCIM_PATH_DELIM_STRING;
            file += entry;

            // FIXME! check duplicates
            __style_list.push_back (StyleFile ());
            StyleFile &style = __style_list.back ();
            bool success = style.load (file.c_str ());
            if (!success)
                __style_list.pop_back ();
        }
        g_dir_close (dir);
    }
}

static void
load_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    __style_list.clear ();

    load_style_files (SCIM_ANTHY_STYLEDIR);
    load_style_files (__user_style_dir_name.c_str ());

    __user_style_file.load (__user_style_file_name.c_str ());

    __config_key_theme
        = config->read (String (SCIM_ANTHY_CONFIG_KEY_THEME),
                        String (SCIM_ANTHY_CONFIG_KEY_THEME_DEFAULT));
    __config_romaji_theme
        = config->read (String (SCIM_ANTHY_CONFIG_ROMAJI_THEME),
                        String (SCIM_ANTHY_CONFIG_ROMAJI_THEME_DEFAULT));

    for (unsigned int i = 0; config_bool_common[i].key; i++) {
        BoolConfigData &entry = config_bool_common[i];
        entry.value = config->read (String (entry.key), entry.value);
    }

    for (unsigned int i = 0; config_string_common[i].key; i++) {
        StringConfigData &entry = config_string_common[i];
        entry.value = config->read (String (entry.key), entry.value);
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; i++) {
            __key_conf_pages[j].data[i].value =
                config->read (String (__key_conf_pages[j].data[i].key),
                              __key_conf_pages[j].data[i].value);
        }
    }

    for (unsigned int i = 0; config_keyboards_reverse_learning[i].key; i++) {
        StringConfigData &entry = config_keyboards_reverse_learning[i];
        entry.value = config->read (String (entry.key), entry.value);
    }

    setup_widget_value ();

    for (unsigned int i = 0; config_bool_common[i].key; i++)
        config_bool_common[i].changed = false;

    for (unsigned int i = 0; config_string_common[i].key; i++)
        config_string_common[i].changed = false;

    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; i++)
            __key_conf_pages[j].data[i].changed = false;
    }

    for (unsigned int i = 0; config_keyboards_reverse_learning[i].key; i++)
        config_keyboards_reverse_learning[i].changed = false;

    __config_changed = false;
}

static void
save_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    __config_key_theme
        = config->write (String (SCIM_ANTHY_CONFIG_KEY_THEME),
                         String (__config_key_theme));
    __config_romaji_theme
        = config->write (String (SCIM_ANTHY_CONFIG_ROMAJI_THEME),
                         String (__config_romaji_theme));

    for (unsigned int i = 0; config_bool_common[i].key; i++) {
        BoolConfigData &entry = config_bool_common[i];
        if (entry.changed)
            entry.value = config->write (String (entry.key), entry.value);
        entry.changed = false;
    }

    for (unsigned int i = 0; config_string_common[i].key; i++) {
        StringConfigData &entry = config_string_common[i];
        if (entry.changed)
            entry.value = config->write (String (entry.key), entry.value);
        entry.changed = false;
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; i++) {
            if (__key_conf_pages[j].data[i].changed)
                config->write (String (__key_conf_pages[j].data[i].key),
                               String (__key_conf_pages[j].data[i].value));
            __key_conf_pages[j].data[i].changed = false;
        }
    }

    for (unsigned int i = 0; config_keyboards_reverse_learning[i].key; i++) {
        StringConfigData &entry = config_keyboards_reverse_learning[i];
        if (entry.changed)
            entry.value = config->write (String (entry.key), entry.value);
        entry.changed = false;
    }
    __config_changed = false;

    if (__style_changed) {
        scim_make_dir (__user_config_dir_name);
        __user_style_file.save (__user_style_file_name.c_str ());
        __style_changed = false;
    }
}

static bool
query_changed (void)
{
    return __config_changed || __style_changed;
}

static bool
load_romaji_theme (void)
{
    gint idx = gtk_option_menu_get_history (
        GTK_OPTION_MENU (__widget_romaji_theme_menu));
    gint theme_idx = idx - 1;

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
on_default_toggle_button_toggled (GtkToggleButton *togglebutton,
                                  gpointer         user_data)
{
    BoolConfigData *entry = static_cast<BoolConfigData*> (user_data);

    if (entry) {
        entry->value = gtk_toggle_button_get_active (togglebutton);
        entry->changed = true;
        __config_changed = true;
    }
}

static void
on_default_editable_changed (GtkEditable *editable,
                             gpointer     user_data)
{
    StringConfigData *entry = static_cast <StringConfigData*> (user_data);

    if (entry) {
        entry->value = String (gtk_entry_get_text (GTK_ENTRY (editable)));
        entry->changed = true;
        __config_changed = true;
    }
}

static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    StringConfigData *data = static_cast <StringConfigData*> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (GTK_ENTRY (data->widget)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (GTK_ENTRY (data->widget))))
                gtk_entry_set_text (GTK_ENTRY (data->widget), keys);
        }

        gtk_widget_destroy (dialog);
    }
}

static void
on_default_combo_changed (GtkEditable *editable,
                          gpointer user_data)
{
    StringConfigData *entry = static_cast<StringConfigData*> (user_data);
    ComboConfigCandidate *data = static_cast<ComboConfigCandidate*>
        (g_object_get_data (G_OBJECT (editable),
                            DATA_POINTER_KEY));

    if (!entry) return;
    if (!data) return;

    const char *label =  gtk_entry_get_text (GTK_ENTRY (editable));

    for (unsigned int i = 0; data[i].label; i++) {
        if (label && !strcmp (_(data[i].label), label)) {
            entry->value = data[i].data;
            entry->changed = true;
            __config_changed = true;
            break;
        }
    }
}

static void
on_dict_menu_label_toggled (GtkToggleButton *togglebutton,
                            gpointer         user_data)
{
    bool active = gtk_toggle_button_get_active (togglebutton);

    BoolConfigData *entry;
    entry = find_bool_config_entry (SCIM_ANTHY_CONFIG_SHOW_DICT_ADMIN_LABEL);
    if (entry->widget)
        gtk_widget_set_sensitive (GTK_WIDGET (entry->widget), active);
    entry = find_bool_config_entry (SCIM_ANTHY_CONFIG_SHOW_ADD_WORD_LABEL);
    if (entry->widget)
        gtk_widget_set_sensitive (GTK_WIDGET (entry->widget), active);
}

static void
on_key_category_menu_changed (GtkOptionMenu *omenu, gpointer user_data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (user_data);
    GtkListStore *store = GTK_LIST_STORE (gtk_tree_view_get_model (treeview));

    gtk_list_store_clear (store);

    gint idx = gtk_option_menu_get_history (omenu);

    bool use_filter = false;

    if (idx >= 0 && idx < (gint) __key_conf_pages_num) {
        append_key_bindings (treeview, idx, NULL);

    } else if (idx == INDEX_SEARCH_BY_KEY) {
        // search by key
        use_filter = true;
        const char *str = gtk_entry_get_text (GTK_ENTRY (__widget_key_filter));
        for (unsigned int i = 0; i < __key_conf_pages_num; i++)
            append_key_bindings (treeview, i, str);

    } else if (idx == INDEX_ALL) {
        // all
        for (unsigned int i = 0; i < __key_conf_pages_num; i++)
            append_key_bindings (treeview, i, NULL);
    }

    gtk_widget_set_sensitive (__widget_key_filter,        use_filter);
    gtk_widget_set_sensitive (__widget_key_filter_button, use_filter);
}

static void
on_key_theme_menu_changed (GtkOptionMenu *omenu, gpointer user_data)
{
    gint idx = gtk_option_menu_get_history (omenu);
    gint theme_idx = idx - 1;

    // clear all key bindings
    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; i++) {
            __key_conf_pages[j].data[i].value = "";
            __key_conf_pages[j].data[i].changed = true;
        }
    }

    // set new key bindings
    if (idx == 0) {
        for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
            for (unsigned int i = 0; __key_conf_pages[j].data[i].key; i++) {
                __key_conf_pages[j].data[i].value
                    = __key_conf_pages[j].data[i].default_value;
            }
        }
        __config_key_theme = "Default";
    } else if (theme_idx >= 0) {
        // reset key bindings
        StyleLines lines;
        StyleLines::iterator it;
        __style_list[theme_idx].get_entry_list (lines, "KeyBindings");
        for (it = lines.begin (); it != lines.end (); it++) {
            if (it->get_type () != SCIM_ANTHY_STYLE_LINE_KEY)
                continue;
            String key, fullkey;
            it->get_key (key);
            fullkey = String ("/IMEngine/Anthy/") + key;
            StringConfigData *entry = find_key_config_entry (fullkey.c_str ());
            if (entry) {
                it->get_value (entry->value);
                entry->changed = true;
            } else {
                std::cerr << "No entry for : " << key << std::endl;
            }
        }
        __config_key_theme = __style_list[theme_idx].get_title ();
    }

    // sync widgets
    gtk_option_menu_set_history
        (GTK_OPTION_MENU (__widget_key_categories_menu), 0);
    gtk_widget_set_sensitive (__widget_key_filter, FALSE);
    gtk_widget_set_sensitive (__widget_key_filter_button, FALSE);
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (__widget_key_list_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));
    append_key_bindings (GTK_TREE_VIEW (__widget_key_list_view), 0, NULL);

    __config_changed = true;
}

static void
on_key_filter_selection_clicked (GtkButton *button,
                                 gpointer   user_data)
{
    GtkEntry *entry = static_cast <GtkEntry*> (user_data);

    if (entry) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_("Set key filter"));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (entry));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (entry)))
                gtk_entry_set_text (entry, keys);

            GtkTreeModel *model;
            model = gtk_tree_view_get_model (
                GTK_TREE_VIEW (__widget_key_list_view));
            gtk_list_store_clear (GTK_LIST_STORE (model));
            for (unsigned int i = 0; i < __key_conf_pages_num; i++)
                append_key_bindings (GTK_TREE_VIEW (__widget_key_list_view),
                                     i, keys);
        }

        gtk_widget_destroy (dialog);
    }
}

static gboolean
on_key_list_view_key_press (GtkWidget *widget, GdkEventKey *event,
                            gpointer user_data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (widget);

    switch (event->keyval) {
    case GDK_Return:
    case GDK_KP_Enter:
        key_list_view_popup_key_selection (treeview);
        break;
    }

    return FALSE;
}

static gboolean
on_key_list_view_button_press (GtkWidget *widget, GdkEventButton *event,
                               gpointer user_data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (widget);

    if (event->type == GDK_2BUTTON_PRESS) {
        key_list_view_popup_key_selection (treeview);
        return TRUE;
    }

    return FALSE;
}

static void
on_key_list_selection_changed (GtkTreeSelection *selection, gpointer data)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;

    gboolean selected;

    selected = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (__widget_choose_keys_button) {
        if (selected) {
            gtk_widget_set_sensitive (__widget_choose_keys_button, true);
        } else {
            gtk_widget_set_sensitive (__widget_choose_keys_button, false);
        }
    }
}

static void
on_choose_keys_button_clicked (GtkWidget *button, gpointer data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (data);
    key_list_view_popup_key_selection (treeview);
}

static void
on_romaji_theme_menu_changed (GtkOptionMenu *omenu, gpointer user_data)
{
    bool success = load_romaji_theme ();

    if (success) {
        // sync widgets
        __style_changed  = true;
        __config_changed = true;
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

    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
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

    GtkWidget *label = gtk_label_new_with_mnemonic (_("_Sequence:"));
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

    // set data
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
        gtk_list_store_append (GTK_LIST_STORE (store), &iter);
        gtk_list_store_set (GTK_LIST_STORE (store), &iter,
                            0, key.c_str (),
                            1, utf8_wcstombs(value).c_str (),
                            2, "",
                            -1);
    }

    // clearn
    g_object_unref (store);

    return dialog;
}

static void
on_romaji_customize_button_clicked (GtkWidget *button, gpointer data)
{
    GtkWidget *widget = create_romaji_window (
        GTK_WINDOW (gtk_widget_get_toplevel (button)));
    gtk_dialog_run (GTK_DIALOG (widget));
    gtk_widget_destroy (widget);
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
