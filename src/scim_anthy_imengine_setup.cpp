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

#define Uses_SCIM_CONFIG_BASE

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include <gtk/scimkeyselection.h>
#include "scim_anthy_prefs.h"
#include "intl.h"

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
struct BoolConfigData
{
    const char *key;
    bool        value;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *widget;
    bool        changed;
};

struct StringConfigData
{
    const char *key;
    String      value;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *widget;
    bool        changed;
};

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

// Internal data declaration.
static bool __have_changed = true;
static GtkTooltips * __widget_tooltips = 0;

static BoolConfigData __config_bool_common [] =
{
    {
        SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD,
        SCIM_ANTHY_CONFIG_AUTO_CONVERT_ON_PERIOD_DEFAULT,
        N_("Start _conversion on inputting a comma or a period."),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_CLOSE_CAND_WIN_ON_SELECT,
        SCIM_ANTHY_CONFIG_CLOSE_CAND_WIN_ON_SELECT_DEFAULT,
        N_("Close candidate window when select a candidate _directly."),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_ROMAJI_HALF_SYMBOL,
        SCIM_ANTHY_CONFIG_ROMAJI_HALF_SYMBOL_DEFAULT,
        N_("Use half-width characters for _symbols"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_ROMAJI_HALF_NUMBER,
        SCIM_ANTHY_CONFIG_ROMAJI_HALF_NUMBER_DEFAULT,
        N_("Use half-width characters for _numbers"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL,
        SCIM_ANTHY_CONFIG_SHOW_INPUT_MODE_LABEL_DEFAULT,
        N_("Show _input mode label"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL,
        SCIM_ANTHY_CONFIG_SHOW_TYPING_METHOD_LABEL_DEFAULT,
        N_("Show _typing method label"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL,
        SCIM_ANTHY_CONFIG_SHOW_PERIOD_STYLE_LABEL_DEFAULT,
        N_("Show _period style label"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SHOW_DICT_LABEL,
        SCIM_ANTHY_CONFIG_SHOW_DICT_LABEL_DEFAULT,
        N_("Show _dictionary menu label"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SHOW_DICT_ADMIN_LABEL,
        SCIM_ANTHY_CONFIG_SHOW_DICT_ADMIN_LABEL_DEFAULT,
        N_("Show _edit dictionary label"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SHOW_ADD_WORD_LABEL,
        SCIM_ANTHY_CONFIG_SHOW_ADD_WORD_LABEL_DEFAULT,
        N_("Show _add word label"),
        NULL,
        NULL,
        NULL,
        false,
    },
};
static unsigned int __config_bool_common_num = sizeof (__config_bool_common) / sizeof (BoolConfigData);

static StringConfigData __config_string_common [] =
{
    {
        SCIM_ANTHY_CONFIG_TYPING_METHOD,
        SCIM_ANTHY_CONFIG_TYPING_METHOD_DEFAULT,
        N_("Typing _method: "),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_PERIOD_STYLE,
        SCIM_ANTHY_CONFIG_PERIOD_STYLE_DEFAULT,
        N_("St_yle of comma and period: "),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SPACE_TYPE,
        SCIM_ANTHY_CONFIG_SPACE_TYPE_DEFAULT,
        N_("_Space type: "),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_TEN_KEY_TYPE,
        SCIM_ANTHY_CONFIG_TEN_KEY_TYPE_DEFAULT,
        N_("Input from _ten key: "),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_DICT_ADMIN_COMMAND,
        SCIM_ANTHY_CONFIG_DICT_ADMIN_COMMAND_DEFAULT,
        N_("_Edit dictionary command:"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_ADD_WORD_COMMAND,
        SCIM_ANTHY_CONFIG_ADD_WORD_COMMAND_DEFAULT,
        N_("_Add word command:"),
        NULL,
        NULL,
        NULL,
        false,
    },
};
static unsigned int __config_string_common_num = sizeof (__config_string_common) / sizeof (StringConfigData);

static StringConfigData __config_keyboards_common [] =
{
    {
        SCIM_ANTHY_CONFIG_COMMIT_KEY,
        SCIM_ANTHY_CONFIG_COMMIT_KEY_DEFAULT,
        N_("Commit keys:"),
        N_("Select commit keys"),
        N_("The key events to commit the preedit string. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_CONVERT_KEY,
        SCIM_ANTHY_CONFIG_CONVERT_KEY_DEFAULT,
        N_("Convert keys:"),
        N_("Select convert keys"),
        N_("The key events to convert the preedit string to kanji. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_CANCEL_KEY,
        SCIM_ANTHY_CONFIG_CANCEL_KEY_DEFAULT,
        N_("Cancel keys:"),
        N_("Select cancel keys"),
        N_("The key events to cancel preediting or converting. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_BACKSPACE_KEY,
        SCIM_ANTHY_CONFIG_BACKSPACE_KEY_DEFAULT,
        N_("Backspace keys:"),
        N_("Select backspace keys"),
        N_("The key events to delete a character before caret. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_DELETE_KEY,
        SCIM_ANTHY_CONFIG_DELETE_KEY_DEFAULT,
        N_("Delete keys:"),
        N_("Select delete keys"),
        N_("The key events to delete a character after caret. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_caret [] =
{
    {
        SCIM_ANTHY_CONFIG_MOVE_CARET_FIRST_KEY,
        SCIM_ANTHY_CONFIG_MOVE_CARET_FIRST_KEY_DEFAULT,
        N_("Move to first keys:"),
        N_("Select move caret to first keys"),
        N_("The key events to move the caret to the first of preedit string. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_MOVE_CARET_LAST_KEY,
        SCIM_ANTHY_CONFIG_MOVE_CARET_LAST_KEY_DEFAULT,
        N_("Move to last keys:"),
        N_("Select move caret to last keys"),
        N_("The key events to move the caret to the last of the preedit string. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_MOVE_CARET_FORWARD_KEY,
        SCIM_ANTHY_CONFIG_MOVE_CARET_FORWARD_KEY_DEFAULT,
        N_("Move forward keys:"),
        N_("Select move caret forward keys"),
        N_("The key events to move the caret to forward. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_MOVE_CARET_BACKWARD_KEY,
        SCIM_ANTHY_CONFIG_MOVE_CARET_BACKWARD_KEY_DEFAULT,
        N_("Move backward keys:"),
        N_("Select move caret backward keys"),
        N_("The key events to move the caret to backward. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_segments [] =
{
    {
        SCIM_ANTHY_CONFIG_SELECT_FIRST_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_SELECT_FIRST_SEGMENT_KEY_DEFAULT,
        N_("First segment keys:"),
        N_("Select first segment keys"),
        N_("The key events to select first segment. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_LAST_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_SELECT_LAST_SEGMENT_KEY_DEFAULT,
        N_("Last segment keys:"),
        N_("Select last segment keys"),
        N_("The key events to select last segment. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_NEXT_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_SELECT_NEXT_SEGMENT_KEY_DEFAULT,
        N_("Next segment keys:"),
        N_("Select next segment keys"),
        N_("The key events to select next segment. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_PREV_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_SELECT_PREV_SEGMENT_KEY_DEFAULT,
        N_("Previous segment keys:"),
        N_("Select previous segment keys"),
        N_("The key events to select previous segment. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SHRINK_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_SHRINK_SEGMENT_KEY_DEFAULT,
        N_("Shrink segment keys:"),
        N_("Select shrink segment keys"),
        N_("The key events to shrink the selected segment. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_EXPAND_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_EXPAND_SEGMENT_KEY_DEFAULT,
        N_("Expand segment keys:"),
        N_("Select expand segment keys"),
        N_("The key events to expand the selected segment. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_COMMIT_FIRST_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_COMMIT_FIRST_SEGMENT_KEY_DEFAULT,
        N_("Commit the first segment keys:"),
        N_("Select commiting the first segment keys"),
        N_("The key events to commit the first segment. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_COMMIT_SELECTED_SEGMENT_KEY,
        SCIM_ANTHY_CONFIG_COMMIT_SELECTED_SEGMENT_KEY_DEFAULT,
        N_("Commit the selected segment keys:"),
        N_("Select commiting the selected segment keys"),
        N_("The key events to commit the selected segment. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_candidates [] =
{
    {
        SCIM_ANTHY_CONFIG_SELECT_NEXT_CANDIDATE_KEY,
        SCIM_ANTHY_CONFIG_SELECT_NEXT_CANDIDATE_KEY_DEFAULT,
        N_("Next keys:"),
        N_("Select next candidate keys"),
        N_("The key events to select next candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_PREV_CANDIDATE_KEY,
        SCIM_ANTHY_CONFIG_SELECT_PREV_CANDIDATE_KEY_DEFAULT,
        N_("Previous keys:"),
        N_("Select previous candidate keys"),
        N_("The key events to select previous candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_UP_KEY,
        SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_UP_KEY_DEFAULT,
        N_("Page up keys:"),
        N_("Select page up candidates keys"),
        N_("The key events to select page up candidates. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_DOWN_KEY,
        SCIM_ANTHY_CONFIG_CANDIDATES_PAGE_DOWN_KEY_DEFAULT,
        N_("Page down keys:"),
        N_("Select page down candidates keys"),
        N_("The key events to select page down candidates. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_direct_select [] =
{
    {
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_1_KEY,
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_1_KEY_DEFAULT,
        N_("1st candidate keys:"),
        N_("Select keys to select 1st candidate"),
        N_("The key events to select 1st candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_2_KEY,
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_2_KEY_DEFAULT,
        N_("2nd candidate keys:"),
        N_("Select keys to select 2nd candidate"),
        N_("The key events to select 2nd candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_3_KEY,
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_3_KEY_DEFAULT,
        N_("3rd candidate keys:"),
        N_("Select keys to select 3rd candidate"),
        N_("The key events to select 3rd candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_4_KEY,
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_4_KEY_DEFAULT,
        N_("4th candidate keys:"),
        N_("Select keys to select 4th candidate"),
        N_("The key events to select 4th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_5_KEY,
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_5_KEY_DEFAULT,
        N_("5th candidate keys:"),
        N_("Select keys to select 5th candidate"),
        N_("The key events to select 5th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_6_KEY,
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_6_KEY_DEFAULT,
        N_("6th candidate keys:"),
        N_("Select keys to select 6th candidate"),
        N_("The key events to select 6th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_7_KEY,
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_7_KEY_DEFAULT,
        N_("7th candidate keys:"),
        N_("Select keys to select 7th candidate"),
        N_("The key events to select 7th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_8_KEY,
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_8_KEY_DEFAULT,
        N_("8th candidate keys:"),
        N_("Select keys to select 8th candidate"),
        N_("The key events to select 8th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_9_KEY,
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_9_KEY_DEFAULT,
        N_("9th candidate keys:"),
        N_("Select keys to select 9th candidate"),
        N_("The key events to select 9th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_10_KEY,
        SCIM_ANTHY_CONFIG_SELECT_CANDIDATE_10_KEY_DEFAULT,
        N_("10th candidate keys:"),
        N_("Select keys to select 10th candidate"),
        N_("The key events to select 10th candidate. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_converting [] =
{
    {
        SCIM_ANTHY_CONFIG_CONV_TO_HIRAGANA_KEY,
        SCIM_ANTHY_CONFIG_CONV_TO_HIRAGANA_KEY_DEFAULT,
        N_("Hiragana keys:"),
        N_("Select convert to hiragana keys"),
        N_("The key events to convert the preedit string to hiragana. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_CONV_TO_KATAKANA_KEY,
        SCIM_ANTHY_CONFIG_CONV_TO_KATAKANA_KEY_DEFAULT,
        N_("Katakana keys:"),
        N_("Select convert to katakana keys"),
        N_("The key events to convert the preedit string to katakana. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_CONV_TO_HALF_KATAKANA_KEY,
        SCIM_ANTHY_CONFIG_CONV_TO_HALF_KATAKANA_KEY_DEFAULT,
        N_("Half width keys:"),
        N_("Select convert to half width katakana keys"),
        N_("The key events to convert the preedit string to half width katakana. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_CONV_TO_LATIN_KEY,
        SCIM_ANTHY_CONFIG_CONV_TO_LATIN_KEY_DEFAULT,
        N_("Latin keys:"),
        N_("Select convert to latin keys"),
        N_("The key events to convert the preedit string to latin. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_CONV_TO_WIDE_LATIN_KEY,
        SCIM_ANTHY_CONFIG_CONV_TO_WIDE_LATIN_KEY_DEFAULT,
        N_("Wide latin keys:"),
        N_("Select convert to wide latin keys"),
        N_("The key events to convert the preedit string to wide latin. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_mode [] =
{
    {
        SCIM_ANTHY_CONFIG_CIRCLE_KANA_MODE_KEY,
        SCIM_ANTHY_CONFIG_CIRCLE_KANA_MODE_KEY_DEFAULT,
        N_("Circle kana mode keys:"),
        N_("Select circle kana mode keys"),
        N_("The key events to circle kana mode. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_LATIN_MODE_KEY,
        SCIM_ANTHY_CONFIG_LATIN_MODE_KEY_DEFAULT,
        N_("Latin mode keys:"),
        N_("Select latin mode keys"),
        N_("The key events to toggle latin mode. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_WIDE_LATIN_MODE_KEY,
        SCIM_ANTHY_CONFIG_WIDE_LATIN_MODE_KEY_DEFAULT,
        N_("Wide latin mode keys:"),
        N_("Select wide latin mode keys"),
        N_("The key events to toggle wide latin mode. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static StringConfigData __config_keyboards_dict [] =
{
    {
        SCIM_ANTHY_CONFIG_DICT_ADMIN_KEY,
        SCIM_ANTHY_CONFIG_DICT_ADMIN_KEY_DEFAULT,
        N_("Edit dictionary keys:"),
        N_("Select edit dictionary keys"),
        N_("The key events to launch dictionary administration tool. "),
        NULL,
        false,
    },
    {
        SCIM_ANTHY_CONFIG_ADD_WORD_KEY,
        SCIM_ANTHY_CONFIG_ADD_WORD_KEY_DEFAULT,
        N_("Add a word keys:"),
        N_("Select add a word keys"),
        N_("The key events to launch the tool to add a word. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

static struct KeyboardConfigPage __key_conf_pages[] =
{
    {N_("Common keys"),        __config_keyboards_common},
    {N_("Mode keys"),          __config_keyboards_mode},
    {N_("Caret keys"),         __config_keyboards_caret},
    {N_("Segments keys"),      __config_keyboards_segments},
    {N_("Candidates keys"),    __config_keyboards_candidates},
    {N_("Direct select keys"), __config_keyboards_direct_select},
    {N_("Convert keys"),       __config_keyboards_converting},
    {N_("Dictionary keys"),    __config_keyboards_dict},
};
static unsigned int __key_conf_pages_num = sizeof (__key_conf_pages) / sizeof (KeyboardConfigPage);

static ComboConfigCandidate typing_methods[] =
{
    {N_("Roma typing method"), "Roma"},
    {N_("Kana typing method"), "Kana"},
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
    {N_("Wide"), "Wide"},
    {N_("Half"), "Half"},
    {NULL, NULL},
};

static ComboConfigCandidate ten_key_types[] =
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
static void on_dict_menu_label_toggled        (GtkToggleButton *togglebutton,
                                               gpointer         user_data);
static void setup_widget_value ();


static BoolConfigData *
find_bool_config_entry (const char *config_key)
{
    if (!config_key)
        return NULL;

    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData *entry = &__config_bool_common[i];
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

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData *entry = &__config_string_common[i];
        if (entry->key && !strcmp (entry->key, config_key))
            return entry;
    }

    return NULL;
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
    gtk_widget_show (entry->widget);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, entry->widget,
                              _(entry->tooltip), NULL);

    return entry->widget;
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
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), (data)->widget);         \
    g_signal_connect ((gpointer) (data)->widget, "changed",                    \
                      G_CALLBACK (on_default_editable_changed),                \
                      (data));                                                 \
    gtk_widget_show ((data)->widget);                                          \
    gtk_table_attach (GTK_TABLE (table), (data)->widget, 1, 2, i, i+1,         \
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),                \
                      (GtkAttachOptions) (GTK_FILL), 4, 4);                    \
                                                                               \
    if (!__widget_tooltips)                                                    \
        __widget_tooltips = gtk_tooltips_new();                                \
    if ((data)->tooltip)                                                       \
        gtk_tooltips_set_tip (__widget_tooltips, (data)->widget,               \
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
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (entry->widget)->entry), FALSE);
    gtk_widget_show (entry->widget);
    gtk_table_attach (GTK_TABLE (table), entry->widget, 1, 2, idx, idx + 1,
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);
    g_object_set_data (G_OBJECT (GTK_COMBO (entry->widget)->entry), DATA_POINTER_KEY,
                       (gpointer) candidates_p);

    g_signal_connect ((gpointer) GTK_COMBO (entry->widget)->entry, "changed",
                      G_CALLBACK (on_default_combo_changed),
                      entry);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, entry->widget,
                              _(entry->tooltip), NULL);

    return entry->widget;
}

static GtkWidget *
create_options_page ()
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
create_romaji_page ()
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
create_keyboard_page (unsigned int page)
{
    GtkWidget *table;
    GtkWidget *label;

    if (page >= __key_conf_pages_num)
        return NULL;

    StringConfigData *data = __key_conf_pages[page].data;

    table = gtk_table_new (3, 3, FALSE);
    gtk_widget_show (table);

    // Create keyboard setting.
    for (unsigned int i = 0; data[i].key; ++ i) {
        APPEND_ENTRY(&data[i], i);
        gtk_entry_set_editable (GTK_ENTRY (data[i].widget), FALSE);

        GtkWidget *button = gtk_button_new_with_label ("...");
        g_signal_connect ((gpointer) button, "clicked",
                          G_CALLBACK (on_default_key_selection_clicked),
                          &(data[i]));
        gtk_widget_show (button);
        gtk_table_attach (GTK_TABLE (table), button, 2, 3, i, i+1,
                          (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (GTK_FILL), 4, 4);
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), button);
    }

    return table;
}

static GtkWidget *
create_keyboard_page2 (void)
{
    GtkWidget *vbox, *hbox, *omenu, *menu, *menuitem, *entry, *button;
    GtkWidget *scrwin, *treeview;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    omenu = gtk_option_menu_new ();
    gtk_box_pack_start (GTK_BOX (hbox), omenu, FALSE, FALSE, 2);
    gtk_widget_show (omenu);

    menu = gtk_menu_new ();
    gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), menu);
    gtk_widget_show (menu);

    menuitem = gtk_menu_item_new_with_label (_("all"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    for (unsigned int i = 0; i < __key_conf_pages_num; i++) {
        menuitem = gtk_menu_item_new_with_label (_(__key_conf_pages[i].label));
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        gtk_widget_show (menuitem);
    }

    menuitem = gtk_menu_item_new_with_label (_("Search by key"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), 0);

    entry = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 2);
    gtk_widget_show(entry);

    button = gtk_button_new_with_label ("...");
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
    gtk_widget_show (button);

    scrwin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrwin),
                                         GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (scrwin), 4);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrwin),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), scrwin, TRUE, TRUE, 2);
    gtk_widget_show (scrwin);

    treeview = gtk_tree_view_new ();
    gtk_container_add (GTK_CONTAINER (scrwin), treeview);
    gtk_widget_show (treeview);

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

        // Create the options page.
        GtkWidget *page = create_options_page ();
        GtkWidget *label = gtk_label_new (_("Options"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the romaji page.
        page = create_romaji_page ();
        label = gtk_label_new (_("Romaji Typing"));
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

        // Create the key bind pages.
#if 1
        for (unsigned int i = 0; i < __key_conf_pages_num; i++) {
            page = create_keyboard_page (i);
            label = gtk_label_new (_(__key_conf_pages[i].label));
            gtk_widget_show (label);
            gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
        }
#else
        page = create_keyboard_page2 ();
        label = gtk_label_new (_("Key binding"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
#endif

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
setup_widget_value ()
{
    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData &entry = __config_bool_common[i];
        if (entry.widget)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (entry.widget),
                                          entry.value);
    }

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData &entry = __config_string_common[i];
        if (entry.widget && GTK_IS_COMBO (entry.widget))
            setup_combo_value (GTK_COMBO (entry.widget), entry.value);
        else if (entry.widget && GTK_IS_ENTRY (entry.widget))
            gtk_entry_set_text (GTK_ENTRY (entry.widget),
                                entry.value.c_str ());
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; ++j) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            if (__key_conf_pages[j].data[i].widget) {
                gtk_entry_set_text (
                    GTK_ENTRY (__key_conf_pages[j].data[i].widget),
                    __key_conf_pages[j].data[i].value.c_str ());
            }
        }
    }
}

static void
load_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        for (unsigned int i = 0; i < __config_bool_common_num; i++) {
            BoolConfigData &entry = __config_bool_common[i];
            entry.value = config->read (String (entry.key), entry.value);
        }

        for (unsigned int i = 0; i < __config_string_common_num; i++) {
            StringConfigData &entry = __config_string_common[i];
            entry.value = config->read (String (entry.key), entry.value);
        }

        for (unsigned int j = 0; j < __key_conf_pages_num; ++ j) {
            for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
                __key_conf_pages[j].data[i].value =
                    config->read (String (__key_conf_pages[j].data[i].key),
                                  __key_conf_pages[j].data[i].value);
            }
        }

        setup_widget_value ();

        for (unsigned int i = 0; i < __config_bool_common_num; i++)
            __config_bool_common[i].changed = false;

        for (unsigned int i = 0; i < __config_string_common_num; i++)
            __config_string_common[i].changed = false;

        for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
            for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i)
                __key_conf_pages[j].data[i].changed = false;
        }

        __have_changed = false;
    }
}

static void
save_config (const ConfigPointer &config)
{
    if (!config.null ()) {
        for (unsigned int i = 0; i < __config_bool_common_num; i++) {
            BoolConfigData &entry = __config_bool_common[i];
            if (entry.changed)
                entry.value = config->write (String (entry.key), entry.value);
            entry.changed = false;
        }

        for (unsigned int i = 0; i < __config_string_common_num; i++) {
            StringConfigData &entry = __config_string_common[i];
            if (entry.changed)
                entry.value = config->write (String (entry.key), entry.value);
            entry.changed = false;
        }

        for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
            for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
                if (__key_conf_pages[j].data[i].changed)
                    config->write (String (__key_conf_pages[j].data[i].key),
                                   __key_conf_pages[j].data[i].value);
                __key_conf_pages[j].data[i].changed = false;
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
    BoolConfigData *entry = static_cast<BoolConfigData*> (user_data);

    if (entry) {
        entry->value = gtk_toggle_button_get_active (togglebutton);
        entry->changed = true;
        __have_changed = true;
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
        __have_changed = true;
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
            __have_changed = true;
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
        gtk_widget_set_sensitive (entry->widget, active);
    entry = find_bool_config_entry (SCIM_ANTHY_CONFIG_SHOW_ADD_WORD_LABEL);
    if (entry->widget)
        gtk_widget_set_sensitive (entry->widget, active);
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
