/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 Takuro Ashie
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

#define Uses_SCIM_CONFIG_BASE

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include <gtk/scimkeyselection.h>
#include "scim_anthy_prefs.h"

#ifdef HAVE_GETTEXT
  #include <libintl.h>
  #define _(String) dgettext(GETTEXT_PACKAGE,String)
  #define N_(String) (String)
#else
  #define _(String) (String)
  #define N_(String) (String)
  #define bindtextdomain(Package,Directory)
  #define textdomain(domain)
  #define bind_textdomain_codeset(domain,codeset)
#endif

using namespace scim;

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
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

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
struct KeyboardConfigData
{
    const char *key;
    String      data;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *entry;
    GtkWidget  *button;
};

struct KeyboardConfigPage
{
    const char         *label;
    KeyboardConfigData *data;
};

struct ComboConfigData
{
    const char *label;
    const char *data;
};

// Internal data declaration.
static String __config_typing_method            = SCIM_ANTHY_CONFIG_TYPING_METHOD_DEFAULT;
static String __config_period_style             = SCIM_ANTHY_CONFIG_PERIOD_STYLE_DEFAULT;
static String __config_space_type               = SCIM_ANTHY_CONFIG_SPACE_TYPE_DEFAULT;
static bool   __config_auto_convert             = SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD_DEFAULT;
static bool   __config_show_input_mode_label    = SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL_DEFAULT;
static bool   __config_show_typing_method_label = SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL_DEFAULT;
static bool   __config_show_period_style_label  = SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL_DEFAULT;

static bool __have_changed    = true;

static GtkWidget    * __widget_typing_method            = 0;
static GtkWidget    * __widget_period_style             = 0;
static GtkWidget    * __widget_space_type               = 0;
static GtkWidget    * __widget_auto_convert             = 0;
static GtkWidget    * __widget_show_input_mode_label    = 0;
static GtkWidget    * __widget_show_typing_method_label = 0;
static GtkWidget    * __widget_show_period_style_label  = 0;
static GtkTooltips  * __widget_tooltips                 = 0;

static KeyboardConfigData __config_keyboards_common [] =
{
    {
        SCIM_ANTHY_CONFIG_COMMIT_KEY,
        SCIM_ANTHY_CONFIG_COMMIT_KEY_DEFAULT,
        N_("Commit keys:"),
        N_("Select commit keys"),
        N_("The key events to commit the preedit string. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_CONVERT_KEY,
        SCIM_ANTHY_CONFIG_CONVERT_KEY_DEFAULT,
        N_("Convert keys:"),
        N_("Select convert keys"),
        N_("The key events to convert the preedit string to kanji. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_CANCEL_KEY,
        SCIM_ANTHY_CONFIG_CANCEL_KEY_DEFAULT,
        N_("Cancel keys:"),
        N_("Select cancel keys"),
        N_("The key events to cancel preediting or converting. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_BACKSPACE_KEY,
        SCIM_ANTHY_CONFIG_BACKSPACE_KEY_DEFAULT,
        N_("Backspace keys:"),
        N_("Select backspace keys"),
        N_("The key events to delete a character before caret. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_DELETE_KEY,
        SCIM_ANTHY_CONFIG_DELETE_KEY_DEFAULT,
        N_("Delete keys:"),
        N_("Select delete keys"),
        N_("The key events to delete a character after caret. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_caret [] =
{
    {
        SCIM_ANTHY_CONFIG_MOVE_CARET_FIRST_KEY,
        SCIM_ANTHY_CONFIG_MOVE_CARET_FIRST_KEY_DEFAULT,
        N_("Move to first keys:"),
        N_("Select move caret to first keys"),
        N_("The key events to move the caret to the first of preedit string. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_MOVE_CARET_LAST_KEY,
        SCIM_ANTHY_CONFIG_MOVE_CARET_LAST_KEY_DEFAULT,
        N_("Move to last keys:"),
        N_("Select move caret to last keys"),
        N_("The key events to move the caret to the last of the preedit string. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_MOVE_CARET_FORWARD_KEY,
        SCIM_ANTHY_CONFIG_MOVE_CARET_FORWARD_KEY_DEFAULT,
        N_("Move forward keys:"),
        N_("Select move caret forward keys"),
        N_("The key events to move the caret to forward. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_MOVE_CARET_BACKWARD_KEY,
        SCIM_ANTHY_CONFIG_MOVE_CARET_BACKWARD_KEY_DEFAULT,
        N_("Move backward keys:"),
        N_("Select move caret backward keys"),
        N_("The key events to move the caret to backward. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_segments [] =
{
    {
        SCIM_ANTHY_CONFIG_SELECT_FIRST_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_SELECT_FIRST_SEGMENT_KEY_DEFAULT,
        N_("First segment keys:"),
        N_("Select first segment keys"),
        N_("The key events to select first segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_LAST_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_SELECT_LAST_SEGMENT_KEY_DEFAULT,
        N_("Last segment keys:"),
        N_("Select last segment keys"),
        N_("The key events to select last segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_NEXT_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_SELECT_NEXT_SEGMENT_KEY_DEFAULT,
        N_("Next segment keys:"),
        N_("Select next segment keys"),
        N_("The key events to select next segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_PREV_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_SELECT_PREV_SEGMENT_KEY_DEFAULT,
        N_("Previous segment keys:"),
        N_("Select previous segment keys"),
        N_("The key events to select previous segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_SHRINK_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_SHRINK_SEGMENT_KEY_DEFAULT,
        N_("Shrink segment keys:"),
        N_("Select shrink segment keys"),
        N_("The key events to shrink the selected segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_EXPAND_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_EXPAND_SEGMENT_KEY_DEFAULT,
        N_("Expand segment keys:"),
        N_("Select expand segment keys"),
        N_("The key events to expand the selected segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_COMMIT_FIRST_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_COMMIT_FIRST_SEGMENT_KEY_DEFAULT,
        N_("Commit the first segment keys:"),
        N_("Select commiting the first segment keys"),
        N_("The key events to commit the first segment. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_COMMIT_SELECTED_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_COMMIT_SELECTED_SEGMENT_KEY_DEFAULT,
        N_("Commit the selected segment keys:"),
        N_("Select commiting the selected segment keys"),
        N_("The key events to commit the selected segment. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_candidates [] =
{
    {
        SCIM_ANTHY_CONFIG_SELECT_NEXT_CANDIDATE_KEY,
        SCIM_ANTHY_CONFIG_SELECT_NEXT_CANDIDATE_KEY_DEFAULT,
        N_("Next keys:"),
        N_("Select next candidate keys"),
        N_("The key events to select next candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_PREV_CANDIDATE_KEY,
        SCIM_ANTHY_CONFIG_SELECT_PREV_CANDIDATE_KEY_DEFAULT,
        N_("Previous keys:"),
        N_("Select previous candidate keys"),
        N_("The key events to select previous candidate. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_UP_KEY,
        SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_UP_KEY_DEFAULT,
        N_("Page up keys:"),
        N_("Select page up candidates keys"),
        N_("The key events to select page up candidates. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_DOWN_KEY,
        SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_DOWN_KEY_DEFAULT,
        N_("Page down keys:"),
        N_("Select page down candidates keys"),
        N_("The key events to select page down candidates. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_converting [] =
{
    {
        SCIM_ANTHY_CONFIG_CONV_TO_HIRAGANA_KEY,
        SCIM_ANTHY_CONFIG_CONV_TO_HIRAGANA_KEY_DEFAULT,
        N_("Hiragana keys:"),
        N_("Select convert to hiragana keys"),
        N_("The key events to convert the preedit string to hiragana. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_CONV_TO_KATAKANA_KEY,
        SCIM_ANTHY_CONFIG_CONV_TO_KATAKANA_KEY_DEFAULT,
        N_("Katakana keys:"),
        N_("Select convert to katakana keys"),
        N_("The key events to convert the preedit string to katakana. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_CONV_TO_HALF_KATAKANA_KEY,
        SCIM_ANTHY_CONFIG_CONV_TO_HALF_KATAKANA_KEY_DEFAULT,
        N_("Half width keys:"),
        N_("Select convert to half width katakana keys"),
        N_("The key events to convert the preedit string to half width katakana. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_CONV_TO_LATIN_KEY,
        SCIM_ANTHY_CONFIG_CONV_TO_LATIN_KEY_DEFAULT,
        N_("Latin keys:"),
        N_("Select convert to latin keys"),
        N_("The key events to convert the preedit string to latin. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_CONV_TO_WIDE_LATIN_KEY,
        SCIM_ANTHY_CONFIG_CONV_TO_WIDE_LATIN_KEY_DEFAULT,
        N_("Wide latin keys:"),
        N_("Select convert to wide latin keys"),
        N_("The key events to convert the preedit string to wide latin. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static KeyboardConfigData __config_keyboards_mode [] =
{
    {
        SCIM_ANTHY_CONFIG_CIRCLE_KANA_MODE_KEY,
        SCIM_ANTHY_CONFIG_CIRCLE_KANA_MODE_KEY_DEFAULT,
        N_("Circle kana mode keys:"),
        N_("Select circle kana mode keys"),
        N_("The key events to circle kana mode. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_LATIN_MODE_KEY,
        SCIM_ANTHY_CONFIG_LATIN_MODE_KEY_DEFAULT,
        N_("Latin mode keys:"),
        N_("Select latin mode keys"),
        N_("The key events to toggle latin mode. "),
        NULL,
        NULL,
    },
    {
        SCIM_ANTHY_CONFIG_WIDE_LATIN_MODE_KEY,
        SCIM_ANTHY_CONFIG_WIDE_LATIN_MODE_KEY_DEFAULT,
        N_("Wide latin mode keys:"),
        N_("Select wide latin mode keys"),
        N_("The key events to toggle wide latin mode. "),
        NULL,
        NULL,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static struct KeyboardConfigPage __key_conf_pages[] =
{
    {"Common keys",     __config_keyboards_common},
    {"Caret keys",      __config_keyboards_caret},
    {"Segments keys",   __config_keyboards_segments},
    {"Candidates keys", __config_keyboards_candidates},
    {"Converting keys", __config_keyboards_converting},
    {"Mode keys",       __config_keyboards_mode},
};
static unsigned int __key_conf_pages_num = sizeof (__key_conf_pages) / sizeof (KeyboardConfigPage);

static ComboConfigData typing_methods[] =
{
    {N_("Roma typing method"), "Roma"},
    {N_("Kana typing method"), "Kana"},
    {NULL, NULL},
};

static ComboConfigData period_styles[] =
{
    {"\xE3\x80\x81\xE3\x80\x82", "Japanese"},
    {"\xEF\xBC\x8C\xE3\x80\x82", "WideLatin_Japanese"},
    {"\xEF\xBC\x8C\xEF\xBC\x8E", "WideLatin"},
    {",.",                       "Latin"},
    {NULL, NULL},
};

static ComboConfigData space_types[] =
{
    {N_("Wide"), "Wide"},
    {N_("Half"), "Half"},
    {NULL, NULL},
};


static void on_default_editable_changed       (GtkEditable     *editable,
                                               gpointer         user_data);
static void on_default_toggle_button_toggled  (GtkToggleButton *togglebutton,
                                               gpointer         user_data);
static void on_default_key_selection_clicked  (GtkButton       *button,
                                               gpointer         user_data);
static void on_default_combo_changed          (GtkEditable     *editable,
                                               gpointer         user_data);
static void setup_widget_value ();


static GtkWidget *
create_combo_widget (const char *label_text, GtkWidget **widget,
                     gpointer data_p, gpointer candidates_p)
{
    GtkWidget *hbox, *label;

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);

    label = gtk_label_new (label_text);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 4);

    *widget = gtk_combo_new ();
    gtk_combo_set_value_in_list (GTK_COMBO (*widget), TRUE, FALSE);
    gtk_combo_set_case_sensitive (GTK_COMBO (*widget), TRUE);
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (*widget)->entry), FALSE);
    gtk_widget_show (*widget);
    gtk_box_pack_start (GTK_BOX (hbox), *widget, FALSE, FALSE, 4);
    g_object_set_data (G_OBJECT (GTK_COMBO (*widget)->entry), DATA_POINTER_KEY,
                       (gpointer) candidates_p);

    g_signal_connect ((gpointer) GTK_COMBO (*widget)->entry, "changed",
                      G_CALLBACK (on_default_combo_changed),
                      data_p);

    return hbox;
}

static GtkWidget *
create_options_page ()
{
    GtkWidget *vbox, *widget;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    /* typing method */
    widget = create_combo_widget (_("Typing method: "),
                                  &__widget_typing_method,
                                  (gpointer) &__config_typing_method,
                                  (gpointer) &typing_methods);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* period style */
    widget = create_combo_widget (_("Style of comma and period: "),
                                  &__widget_period_style,
                                  (gpointer) &__config_period_style,
                                  (gpointer) &period_styles);
    gtk_widget_set_size_request (__widget_period_style, 100, -1);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* space_style */
    widget = create_combo_widget (_("Space type: "),
                                  &__widget_space_type,
                                  (gpointer) &__config_space_type,
                                  (gpointer) &space_types);
    gtk_widget_set_size_request (__widget_space_type, 100, -1);
    gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 4);

    /* auto convert */
    __widget_auto_convert = gtk_check_button_new_with_mnemonic (_("Start conversion on inputting a comma or a period."));
    gtk_widget_show (__widget_auto_convert);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_auto_convert, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_auto_convert), 4);

    /* show/hide toolbar label */
    __widget_show_input_mode_label = gtk_check_button_new_with_mnemonic (_("Show _input mode label"));
    gtk_widget_show (__widget_show_input_mode_label);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_show_input_mode_label, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_show_input_mode_label), 4);

    __widget_show_typing_method_label = gtk_check_button_new_with_mnemonic (_("Show _typing method label"));
    gtk_widget_show (__widget_show_typing_method_label);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_show_typing_method_label, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_show_typing_method_label), 4);

    __widget_show_period_style_label = gtk_check_button_new_with_mnemonic (_("Show _period style label"));
    gtk_widget_show (__widget_show_period_style_label);
    gtk_box_pack_start (GTK_BOX (vbox), __widget_show_period_style_label, FALSE, FALSE, 4);
    gtk_container_set_border_width (GTK_CONTAINER (__widget_show_period_style_label), 4);

    // Connect all signals.
    g_signal_connect ((gpointer) __widget_auto_convert, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_auto_convert);
    g_signal_connect ((gpointer) __widget_show_input_mode_label, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_show_input_mode_label);
    g_signal_connect ((gpointer) __widget_show_typing_method_label, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_show_typing_method_label);
    g_signal_connect ((gpointer) __widget_show_period_style_label, "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      &__config_show_period_style_label);

    return vbox;
}

static GtkWidget *
create_keyboard_page (unsigned int page)
{
    GtkWidget *table;
    GtkWidget *label;

    if (page >= __key_conf_pages_num)
        return NULL;

    KeyboardConfigData *data = __key_conf_pages[page].data;

    table = gtk_table_new (3, 3, FALSE);
    gtk_widget_show (table);

    // Create keyboard setting.
    for (unsigned int i = 0; data[i].key; ++ i) {
        label = gtk_label_new (NULL);
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _(data[i].label));
        gtk_widget_show (label);
        gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);

        data[i].entry = gtk_entry_new ();
        gtk_widget_show (data[i].entry);
        gtk_table_attach (GTK_TABLE (table), data[i].entry, 1, 2, i, i+1,
                          (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);
        gtk_entry_set_editable (GTK_ENTRY (data[i].entry), FALSE);

        data[i].button = gtk_button_new_with_label ("...");
        gtk_widget_show (data[i].button);
        gtk_table_attach (GTK_TABLE (table), data[i].button, 2, 3, i, i+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), data[i].button);
    }

    for (unsigned int i = 0; data[i].key; ++ i) {
        g_signal_connect ((gpointer) data[i].button, "clicked",
                          G_CALLBACK (on_default_key_selection_clicked),
                          &(data[i]));
        g_signal_connect ((gpointer) data[i].entry, "changed",
                          G_CALLBACK (on_default_editable_changed),
                          &(data[i].data));
    }

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    for (unsigned int i = 0; data[i].key; ++ i) {
        gtk_tooltips_set_tip (__widget_tooltips, data[i].entry,
                              _(data[i].tooltip), NULL);
    }

    return table;
}

static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = NULL;

    if (!window) {
        GtkWidget *notebook = gtk_notebook_new();
        gtk_widget_show (notebook);
        window = notebook;
        gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

        // Create the first page.
        GtkWidget *page = create_options_page ();
        GtkWidget *label = gtk_label_new (_("Options"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        setup_widget_value ();

        // Create the second page.
        for (unsigned int i = 0; i < __key_conf_pages_num; i++) {
            page = create_keyboard_page (i);
            label = gtk_label_new (_(__key_conf_pages[i].label));
            gtk_widget_show (label);
            gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
        }

        setup_widget_value ();
    }

    return window;
}

static void
setup_combo_value (GtkCombo *combo,
                   ComboConfigData *data, const String & str)
{
    GList *list = NULL;
    const char *defval = NULL;

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
setup_widget_value ()
{
    if (__widget_typing_method) {
        setup_combo_value (GTK_COMBO (__widget_typing_method),
                           typing_methods, __config_typing_method);
    }

    if (__widget_period_style) {
        setup_combo_value (GTK_COMBO (__widget_period_style),
                           period_styles, __config_period_style);
    }

    if (__widget_space_type) {
        setup_combo_value (GTK_COMBO (__widget_space_type),
                           space_types, __config_space_type);
    }

    if (__widget_auto_convert) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_auto_convert),
            __config_auto_convert);
    }

    if (__widget_show_input_mode_label) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_show_input_mode_label),
            __config_show_input_mode_label);
    }

    if (__widget_show_typing_method_label) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_show_typing_method_label),
            __config_show_typing_method_label);
    }

    if (__widget_show_period_style_label) {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (__widget_show_period_style_label),
            __config_show_period_style_label);
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; ++j) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            if (__key_conf_pages[j].data[i].entry) {
                gtk_entry_set_text (
                    GTK_ENTRY (__key_conf_pages[j].data[i].entry),
                    __key_conf_pages[j].data[i].data.c_str ());
            }
        }
    }
}

static void
load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        __config_typing_method =
            config->read (String (SCIM_ANTHY_CONFIG_TYPING_METHOD),
                          __config_typing_method);
        __config_period_style =
            config->read (String (SCIM_ANTHY_CONFIG_PERIOD_STYLE),
                          __config_period_style);
        __config_space_type =
            config->read (String (SCIM_ANTHY_CONFIG_SPACE_TYPE),
                          __config_space_type);
        __config_auto_convert =
            config->read (String (SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD),
                          __config_auto_convert);
        __config_show_input_mode_label =
            config->read (String (SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL),
                          __config_show_input_mode_label);
        __config_show_typing_method_label =
            config->read (String (SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL),
                          __config_show_typing_method_label);
        __config_show_period_style_label =
            config->read (String (SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL),
                          __config_show_period_style_label);

        for (unsigned int j = 0; j < __key_conf_pages_num; ++ j) {
            for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
                __key_conf_pages[j].data[i].data =
                    config->read (String (__key_conf_pages[j].data[i].key),
                                  __key_conf_pages[j].data[i].data);
            }
        }

        setup_widget_value ();

        __have_changed = false;
    }
}

static void
save_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        config->write (String (SCIM_ANTHY_CONFIG_TYPING_METHOD),
                        __config_typing_method);
        config->write (String (SCIM_ANTHY_CONFIG_PERIOD_STYLE),
                        __config_period_style);
        config->write (String (SCIM_ANTHY_CONFIG_SPACE_TYPE),
                        __config_space_type);
        config->write (String (SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD),
                        __config_auto_convert);
        config->write (String (SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL),
                        __config_show_input_mode_label);
        config->write (String (SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL),
                        __config_show_typing_method_label);
        config->write (String (SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL),
                        __config_show_period_style_label);

        for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
            for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
                config->write (String (__key_conf_pages[j].data[i].key),
                               __key_conf_pages[j].data[i].data);
            }
        }

        __have_changed = false;
    }
}

static bool
query_changed ()
{
    return __have_changed;
}


static void
on_default_toggle_button_toggled (GtkToggleButton *togglebutton,
                                  gpointer         user_data)
{
    bool *toggle = static_cast<bool*> (user_data);

    if (toggle) {
        *toggle = gtk_toggle_button_get_active (togglebutton);
        __have_changed = true;
    }
}

static void
on_default_editable_changed (GtkEditable *editable,
                             gpointer     user_data)
{
    String *str = static_cast <String *> (user_data);

    if (str) {
        *str = String (gtk_entry_get_text (GTK_ENTRY (editable)));
        __have_changed = true;
    }
}

static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    KeyboardConfigData *data = static_cast <KeyboardConfigData *> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (GTK_ENTRY (data->entry)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (GTK_ENTRY (data->entry))) != 0)
                gtk_entry_set_text (GTK_ENTRY (data->entry), keys);
        }

        gtk_widget_destroy (dialog);
    }
}

static void
on_default_combo_changed (GtkEditable *editable,
                          gpointer user_data)
{
    String *str = static_cast<String *> (user_data);
    ComboConfigData *data
        = static_cast<ComboConfigData *> (g_object_get_data (G_OBJECT (editable),
                                                             DATA_POINTER_KEY));

    if (!str) return;
    if (!data) return;

    const char *label =  gtk_entry_get_text (GTK_ENTRY (editable));

    for (unsigned int i = 0; data[i].label; i++) {
        if (label && !strcmp (_(data[i].label), label)) {
            *str = data[i].data;
            __have_changed = true;
            break;
        }
    }
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
