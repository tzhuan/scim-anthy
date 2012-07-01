/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie
 *  Copyright (C) 2006 - 2007 Takashi Nakamoto <bluedwarf@bpost.plala.or.jp>
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

#define Uses_SCIM_HELPER
#define Uses_SCIM_CONFIG_BASE

#include <gdk/gdkx.h> // to avoid name confliction (ad-hoc)

#include <map>
#include <scim.h>
#include <gtk/gtk.h>
#include "scim_anthy_intl.h"
#include "scim_anthy_helper.h"
#include "scim_anthy_const.h"
#include "scim_anthy_prefs.h"

#ifdef SCIM_ANTHY_BUILD_TRAY
#include "scim_anthy_tray.h"
#endif

using namespace scim;

#define scim_module_init anthy_imengine_helper_LTX_scim_module_init
#define scim_module_exit anthy_imengine_helper_LTX_scim_module_exit
#define scim_helper_module_number_of_helpers anthy_imengine_helper_LTX_scim_helper_module_number_of_helpers
#define scim_helper_module_get_helper_info anthy_imengine_helper_LTX_scim_helper_module_get_helper_info
#define scim_helper_module_run_helper anthy_imengine_helper_LTX_scim_helper_module_run_helper

static gboolean   helper_agent_input_handler  (GIOChannel          *source,
                                               GIOCondition         condition,
                                               gpointer             user_data);
static void       slot_update_spot_location   (const HelperAgent   *agent,
                                               int                  ic,
                                               const String        &uuid,
                                               int                  x,
                                               int                  y);
static void       slot_imengine_event         (const HelperAgent   *agent,
                                               int                  ic,
                                               const String        &uuid,
                                               const Transaction   &trans);
static void       slot_attach_input_context   (const HelperAgent   *agent,
                                               int                  ic,
                                               const String        &ic_uuid);
static gint       timeout_func                (gpointer             data);
static void       timeout_ctx_destroy_func    (gpointer             data);

static void       run                         (const String        &display,
                                               const ConfigPointer &config);

HelperAgent  helper_agent;
AnthyHelper *helper = NULL;
#ifdef SCIM_ANTHY_BUILD_TRAY
AnthyTray   *tray   = NULL;
#endif

HelperInfo helper_info (SCIM_ANTHY_HELPER_UUID,        // uuid
                        "",                            // name
                        "",                            // icon
                        "",
                        SCIM_HELPER_NEED_SCREEN_INFO |
                        SCIM_HELPER_NEED_SPOT_LOCATION_INFO);

class TimeoutContext {
public:
    TimeoutContext (int ic, const String &uuid, uint32 id)
        : m_ic   (ic),
          m_uuid (uuid),
          m_id   (id)
        {}
    virtual ~TimeoutContext () {}
public:
    int    m_ic;
    String m_uuid;
    uint32 m_id;
};
typedef std::map <uint32, guint> TimeoutIDList;
std::map <int, TimeoutIDList> timeout_ids;


//Module Interface
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_ANTHY_LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

        helper_info.name = String (_("Anthy helper"));
        helper_info.description = String (
            _("A helper module for Anthy IMEngine."));
    }

    void scim_module_exit (void)
    {
    }

    unsigned int scim_helper_module_number_of_helpers (void)
    {
        return 1;
    }

    bool scim_helper_module_get_helper_info (unsigned int idx, HelperInfo &info)
    {
        if (idx == 0) {
            info = helper_info; 
            return true;
        }
        return false;
    }

    void scim_helper_module_run_helper (const String &uuid,
                                        const ConfigPointer &config,
                                        const String &display)
    {
        SCIM_DEBUG_MAIN(1) << "anthy_imengine_helper_LTX_scim_helper_module_run_helper ()\n";

        if (uuid == String (SCIM_ANTHY_HELPER_UUID)) {
            run (display, config);
        }

        SCIM_DEBUG_MAIN(1) << "exit anthy_imengine_helper_LTX_scim_helper_module_run_helper ()\n";
    }
}


static gboolean
helper_agent_input_handler (GIOChannel *source,
                            GIOCondition condition,
                            gpointer user_data)
{
    if (condition == G_IO_IN) {
        HelperAgent *agent = static_cast<HelperAgent*> (user_data);
        if (agent && agent->has_pending_event ())
            agent->filter_event ();
    } else if (condition == G_IO_ERR || condition == G_IO_HUP) {
        gtk_main_quit ();
    }
    return TRUE;
}

static void
slot_exit (const HelperAgent *agent, int ic, const String &uuid)
{
#ifdef SCIM_ANTHY_BUILD_TRAY
    if (tray != NULL) {
        delete tray;
        tray = NULL;
    }
#endif

    if (helper != NULL) {
        delete helper;
        helper = NULL;
    }

    gtk_main_quit ();
}

static void
slot_update_spot_location   (const HelperAgent *agent,
                             int ic, const String &uuid,
                             int x, int y)
{
    helper->update_spot_location (x,y);
}

static void
slot_imengine_event (const HelperAgent *agent, int ic,
                     const String &uuid, const Transaction &recv)
{
    TransactionReader reader (recv);
    int cmd;

    if (!reader.get_command (cmd))
        return;

    switch (cmd) {
    case SCIM_ANTHY_TRANS_CMD_GET_SELECTION:
    {
        GtkClipboard *primary_selection;
        WideString selection;

        primary_selection = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
        if (primary_selection) {
            gchar *text = gtk_clipboard_wait_for_text (primary_selection);
            if (text) {
                selection = utf8_mbstowcs (text);
                g_free (text);
            }
        }

        Transaction send;
        send.put_command (SCIM_ANTHY_TRANS_CMD_GET_SELECTION);
        send.put_data (selection);
        helper_agent.send_imengine_event (ic, uuid, send);

        break;
    }
    case SCIM_ANTHY_TRANS_CMD_TIMEOUT_ADD:
    {
        uint32 id, time_msec;
        if (!reader.get_data (id) || !reader.get_data (time_msec))
            break;
        TimeoutContext *ctx = new TimeoutContext (ic, uuid, id);
        guint timeout_id = g_timeout_add_full (G_PRIORITY_DEFAULT,
												 time_msec,
                                                 timeout_func,
                                                 (gpointer) ctx,
                                                 timeout_ctx_destroy_func);
        timeout_ids[ic][id] = timeout_id;
        break;
    }
    case SCIM_ANTHY_TRANS_CMD_TIMEOUT_REMOVE:
    {
        uint32 id;
        if (reader.get_data (id) &&
            timeout_ids.find (ic) != timeout_ids.end () &&
            timeout_ids[ic].find (id) != timeout_ids[ic].end ())
        {
            guint tid = timeout_ids[ic][id];
            g_source_remove (tid);
        }
        break;
    }
    case SCIM_TRANS_CMD_SHOW_AUX_STRING:
    {
        helper->show_aux_string ();
        break;
    }
    case SCIM_TRANS_CMD_SHOW_LOOKUP_TABLE:
    {
        helper->show_lookup_table ();
        break;
    }
    case SCIM_TRANS_CMD_HIDE_AUX_STRING:
    {
        helper->hide_aux_string ();
        break;
    }
    case SCIM_TRANS_CMD_HIDE_LOOKUP_TABLE:
    {
        helper->hide_lookup_table ();
        break;
    }
    case SCIM_TRANS_CMD_UPDATE_AUX_STRING:
    {
        WideString str;
        AttributeList attr;
        reader.get_data (str);
        reader.get_data (attr);
        helper->update_aux_string (str, attr);
        break;
    }
    case SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE:
    {
        CommonLookupTable table;
        reader.get_data (table);
        helper->update_lookup_table (table);
        break;
    }
    case SCIM_ANTHY_TRANS_CMD_SHOW_NOTE:
    {
        helper->show_note ();
        break;
    }
    case SCIM_ANTHY_TRANS_CMD_HIDE_NOTE:
    {
        helper->hide_note ();
        break;
    }
    case SCIM_ANTHY_TRANS_CMD_UPDATE_NOTE:
    {
        WideString str;
        reader.get_data (str);
        helper->update_note (str);
        break;
    }
#ifdef SCIM_ANTHY_BUILD_TRAY
    case SCIM_ANTHY_TRANS_CMD_SET_INPUT_MODE:
    {
        uint32 mode;
        reader.get_data (mode);
        if (tray)
            tray->set_input_mode ((InputMode)mode);

        break;
    }
    case SCIM_ANTHY_TRANS_CMD_INIT_TRAY_MENU:
    {
        PropertyList props;
        reader.get_data (props);
        if (tray)
            tray->create_general_menu (props);

        break;
    }
    case SCIM_ANTHY_TRANS_CMD_UPDATE_TRAY_MENU:
    {
        Property prop;
        reader.get_data (prop);
        if (tray)
            tray->update_general_menu (prop);

        break;
    }
    case SCIM_TRANS_CMD_FOCUS_IN:
    {
        break;
    }
    case SCIM_TRANS_CMD_FOCUS_OUT:
    {
        if (tray != NULL)
            tray->disable ();

        break;
    }
#endif // SCIM_ANTHY_BUILD_TRAY
    default:
        break;
    }
}

static void
slot_update_screen (const HelperAgent *agent, int ic,
                    const String &ic_uuid, int screen_number)
{
    helper->updated_screen (screen_number);
}

static void
slot_reload_config (const HelperAgent *agent, int ic, const String &ic_uuid)
{
    helper->reload_config ();
}

static void
slot_attach_input_context   (const HelperAgent   *agent,
                             int                  ic,
                             const String        &ic_uuid)
{
#ifdef SCIM_ANTHY_BUILD_TRAY
    if (tray != NULL)
    {
        tray->attach_input_context (agent, ic, ic_uuid);
    }
#endif

    InputContext input_context;
    input_context.agent = agent;
    input_context.ic = ic;
    input_context.ic_uuid = ic_uuid;
    helper->attach_input_context (input_context);

    Transaction send;
    send.put_command (SCIM_ANTHY_TRANS_CMD_ATTACHMENT_SUCCESS);
    helper_agent.send_imengine_event (ic, ic_uuid, send);
}

static gint
timeout_func (gpointer data)
{
    TimeoutContext *ctx = static_cast<TimeoutContext*> (data);

    Transaction send;
    send.put_command (SCIM_ANTHY_TRANS_CMD_TIMEOUT_NOTIFY);
    send.put_data (ctx->m_id);
    helper_agent.send_imengine_event (ctx->m_ic, ctx->m_uuid, send);

    return FALSE;
}

static void
timeout_ctx_destroy_func (gpointer data)
{
    TimeoutContext *ctx = static_cast<TimeoutContext*> (data);
    int ic = ctx->m_ic;
    uint32 id = ctx->m_id;

    if (timeout_ids.find (ic) != timeout_ids.end () &&
        timeout_ids[ic].find (id) != timeout_ids[ic].end ())
    {
        timeout_ids[ic].erase (id);
    }
    delete ctx;
}

static void
run (const String &display, const ConfigPointer &config)
{
    char **argv = new char * [4];
    int    argc = 3;

    argv [0] = const_cast<char*> ("anthy-imengine-helper");
    argv [1] = const_cast<char*> ("--display");
    argv [2] = const_cast<char *> (display.c_str ());
    argv [3] = 0;
 
    setenv ("DISPLAY", display.c_str (), 1);

    gtk_init (&argc, &argv);

    helper = new AnthyHelper;
#ifdef SCIM_ANTHY_BUILD_TRAY
    tray = new AnthyTray;
#endif

    helper->init (config, argv[2]);

    helper_agent.signal_connect_exit (slot (slot_exit));
    helper_agent.signal_connect_update_spot_location (slot (slot_update_spot_location));
    helper_agent.signal_connect_process_imengine_event (slot (slot_imengine_event));
    helper_agent.signal_connect_update_screen(slot (slot_update_screen));
    helper_agent.signal_connect_reload_config(slot (slot_reload_config));
    helper_agent.signal_connect_attach_input_context(slot (slot_attach_input_context));

    // open connection
    int fd = helper_agent.open_connection (helper_info, display);
    GIOChannel *ch = g_io_channel_unix_new (fd);

    if (fd >= 0 && ch) {
        g_io_add_watch (ch, G_IO_IN,
                        helper_agent_input_handler,
                        (gpointer) &helper_agent);
        g_io_add_watch (ch, G_IO_ERR,
                        helper_agent_input_handler,
                        (gpointer) &helper_agent);
        g_io_add_watch (ch, G_IO_HUP,
                        helper_agent_input_handler,
                        (gpointer) &helper_agent);
    }

    gtk_main ();

    // close connection
    helper_agent.close_connection ();
    fd = -1;
}

#define ACTIVE_BG_COLOR     "/Panel/Gtk/Color/ActiveBackground"
#define ACTIVE_TEXT_COLOR   "/Panel/Gtk/Color/ActiveText"
#define NORMAL_BG_COLOR     "/Panel/Gtk/Color/NormalBackground"
#define NORMAL_TEXT_COLOR   "/Panel/Gtk/Color/NormalText"

#define LOOKUP_FONT         "/Panel/Gtk/Font"

AnthyHelper::AnthyHelper ()
    : m_initialized               (false),
      m_config                    (NULL),
      m_display                   (NULL),
      m_current_screen            (NULL),
      spot_location_x             (0),
      spot_location_y             (0),
      m_helper_window             (NULL),
      m_helper_vbox               (NULL),
      m_aux_string_visible        (false),
      m_aux_event_box             (NULL),
      m_aux_string_label          (NULL),
      m_current_lookup_table_page_size (0),
      m_lookup_table_visible      (false),
      m_lookup_table_vbox         (NULL),
      m_candidates                (NULL),
      m_allocated_candidate_num   (0),
      m_note_visible              (false),
      m_note_window               (NULL),
      m_note_event_box            (NULL),
      m_note_label                (NULL)
{
// input context
    m_input_context.agent   = NULL;
    m_input_context.ic      = 0;
    m_input_context.ic_uuid = String ();

// default colors
#define REGISTER_DEFAULT_COLOR( key, color ) \
    m_default_colors.insert (make_pair (String (key), String(color)));

    REGISTER_DEFAULT_COLOR(ACTIVE_BG_COLOR, "light sky blue");
    REGISTER_DEFAULT_COLOR(ACTIVE_TEXT_COLOR, "black");
    REGISTER_DEFAULT_COLOR(NORMAL_BG_COLOR, "white");
    REGISTER_DEFAULT_COLOR(NORMAL_TEXT_COLOR, "black");
    REGISTER_DEFAULT_COLOR(SCIM_ANTHY_CONFIG_LOOKUP_BORDER_COLOR,
                           SCIM_ANTHY_CONFIG_LOOKUP_BORDER_COLOR_DEFAULT);
    REGISTER_DEFAULT_COLOR(SCIM_ANTHY_CONFIG_NOTE_BORDER_COLOR,
                           SCIM_ANTHY_CONFIG_NOTE_BORDER_COLOR_DEFAULT);
    REGISTER_DEFAULT_COLOR(SCIM_ANTHY_CONFIG_NOTE_BG_COLOR,
                           SCIM_ANTHY_CONFIG_NOTE_BG_COLOR_DEFAULT);
    REGISTER_DEFAULT_COLOR(SCIM_ANTHY_CONFIG_NOTE_TEXT_COLOR,
                           SCIM_ANTHY_CONFIG_NOTE_TEXT_COLOR_DEFAULT);
    REGISTER_DEFAULT_COLOR(SCIM_ANTHY_CONFIG_AUX_TEXT_COLOR,
                           SCIM_ANTHY_CONFIG_AUX_TEXT_COLOR_DEFAULT);
    REGISTER_DEFAULT_COLOR(SCIM_ANTHY_CONFIG_AUX_BG_COLOR,
                           SCIM_ANTHY_CONFIG_AUX_BG_COLOR_DEFAULT);

#define REGISTER_DEFAULT_FONT( key, font )\
    m_default_fonts.insert (make_pair (String (key), String(font)));

    REGISTER_DEFAULT_FONT(LOOKUP_FONT, "Sans 12");
    REGISTER_DEFAULT_FONT(SCIM_ANTHY_CONFIG_NOTE_FONT,
                          SCIM_ANTHY_CONFIG_NOTE_FONT_DEFAULT);
}

AnthyHelper::~AnthyHelper ()
{
    free_all_font_descs ();

    if (m_helper_window)
    {
        gtk_widget_hide (m_helper_window);
        gtk_widget_destroy (m_helper_window);
    }

    if (m_helper_vbox)
    {
        gtk_widget_hide (m_helper_vbox);
        gtk_widget_destroy (m_helper_vbox);
    }

    if (m_aux_event_box)
    {
        gtk_widget_hide (m_aux_event_box);
        gtk_widget_destroy (m_aux_event_box);
    }

    if (m_aux_string_label)
    {
        gtk_widget_hide (m_aux_string_label);
        gtk_widget_destroy (m_aux_string_label);
    }

    if (m_lookup_table_vbox)
    {
        gtk_widget_hide (m_lookup_table_vbox);
        gtk_widget_destroy (m_lookup_table_vbox);
    }

    for (int i = 0; i < m_allocated_candidate_num; i++)
    {
        gtk_widget_hide (m_candidates[i].event_box);
        gtk_widget_hide (m_candidates[i].label);
        gtk_widget_destroy (m_candidates[i].event_box);
        gtk_widget_destroy (m_candidates[i].label);
        free (m_candidates[i].index);
    }

    if (m_note_window)
    {
        gtk_widget_hide (m_note_window);
        gtk_widget_destroy (m_note_window);
    }

    if (m_note_event_box)
    {
        gtk_widget_hide (m_note_event_box);
        gtk_widget_destroy (m_note_event_box);
    }

    if (m_note_label)
    {
        gtk_widget_hide (m_note_label);
        gtk_widget_destroy (m_note_label);
    }
}

void
AnthyHelper::attach_input_context (InputContext input_context)
{
    m_input_context.agent   = input_context.agent;
    m_input_context.ic      = input_context.ic;
    m_input_context.ic_uuid = input_context.ic_uuid;
}

InputContext
AnthyHelper::get_input_context ()
{
    return m_input_context;
}

void
AnthyHelper::free_all_font_descs(void)
{
    map< String, PangoFontDescription* >::iterator p = m_fonts.begin ();
    while (p != m_fonts.end ())
    {
        if (p->second)
            pango_font_description_free (p->second);
    }

    m_fonts.clear ();
}

void
AnthyHelper::init (const ConfigPointer &config, const char *dsp)
{
    m_config = config;
    reload_config ();

    // get display and screen
    m_display = gdk_display_open (dsp);
    if (m_display == NULL)
        return;

    m_current_screen = gdk_display_get_default_screen (m_display);

    // helper window
    m_helper_window = gtk_window_new (GTK_WINDOW_POPUP);
    if (m_helper_window == NULL)
        return;
    gtk_window_set_default_size (GTK_WINDOW (m_helper_window), 100, 20);
	/*
    gtk_window_set_policy (GTK_WINDOW (m_helper_window),
                           TRUE, TRUE, FALSE);
	*/
    gtk_window_set_resizable (GTK_WINDOW (m_helper_window), FALSE);

#if GTK_CHECK_VERSION(3, 0, 0)
    m_helper_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
    m_helper_vbox = gtk_vbox_new (FALSE, 0);
#endif
    if (m_helper_vbox == NULL)
        return;
    gtk_container_add (GTK_CONTAINER (m_helper_window),
                       m_helper_vbox);

    // aux string
    m_aux_string_visible = false;

    m_aux_event_box = gtk_event_box_new ();
    if (m_aux_event_box == NULL)
        return;
    gtk_box_pack_end (GTK_BOX(m_helper_vbox),
                      m_aux_event_box,
                      TRUE, TRUE, 0);
    
    m_aux_string_label = gtk_label_new ("");
    if (m_aux_string_label == NULL)
        return;
    gtk_misc_set_alignment (GTK_MISC (m_aux_string_label),
                            0.0, 0.5); // to left
    gtk_container_add (GTK_CONTAINER (m_aux_event_box),
                       m_aux_string_label);

    // lookup table
    m_lookup_table_visible = false;
#if GTK_CHECK_VERSION(3, 0, 0)
    m_lookup_table_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
    m_lookup_table_vbox = gtk_vbox_new (TRUE, 0);
#endif
    if (m_lookup_table_vbox == NULL)
        return;
    gtk_box_pack_end (GTK_BOX(m_helper_vbox),
                      m_lookup_table_vbox,
                      TRUE, TRUE, 0);

    // note window
    m_note_visible = false;

    m_note_window = gtk_window_new (GTK_WINDOW_POPUP);
    if (m_note_window == NULL)
        return;
    gtk_window_set_default_size (GTK_WINDOW (m_note_window), 100, 20);
	/*
    gtk_window_set_policy (GTK_WINDOW (m_note_window),
                           TRUE, TRUE, FALSE);
	*/
    gtk_window_set_resizable (GTK_WINDOW (m_note_window), FALSE);

    m_note_event_box = gtk_event_box_new ();
    if (m_note_event_box == NULL)
        return;
    gtk_container_add (GTK_CONTAINER (m_note_window),
                       m_note_event_box);

    m_note_label = gtk_label_new ("");
    if (m_note_label == NULL)
        return;
    gtk_container_add (GTK_CONTAINER (m_note_event_box),
                       m_note_label);

    // change styles
    update_lookup_table_style ();
    update_aux_string_style ();
    update_note_style ();

    m_initialized = true;
}

void
AnthyHelper::reload_config ()
{
    String tmp_str;
    GdkColor tmp_color;
    PangoFontDescription *tmp_desc;

    // change colors
    m_colors.clear ();
    map< String, String >::iterator p = m_default_colors.begin ();
    while (p != m_default_colors.end ())
    {
        tmp_str = m_config->read (p->first, p->second);
        if (gdk_color_parse (tmp_str.c_str(), &tmp_color) == FALSE)
        {   // If it failed to parse user defiend color, go to the following
            // fallback codes to the default color
            if (gdk_color_parse (p->second.c_str(), &tmp_color) == FALSE)
            {
                // Even if failed to parse the default color, use black
                tmp_color.red = tmp_color.green = tmp_color.blue = 0;
            }
        }
        m_colors.insert (make_pair (p->first, tmp_color));
                
        p++;
    }

    // change font
    free_all_font_descs ();
    p = m_default_fonts.begin ();
    while (p != m_default_fonts.end ())
    {
        tmp_str = m_config->read (p->first, p->second);
        tmp_desc = pango_font_description_from_string (tmp_str.c_str ());
        // ToDo: check the validity of the tmp_desc
        
        m_fonts.insert (make_pair (p->first, tmp_desc));

        p++;
    }
}

PangoFontDescription *
AnthyHelper::get_font_desc_from_key (const String &key)
{
    map< String, PangoFontDescription* >::iterator p = m_fonts.find (key);
    if (p == m_fonts.end ())
    {
        // use a system default font
        PangoFontDescription *tmp_desc = pango_font_description_new ();
        m_fonts.insert (make_pair (key, tmp_desc));
        return tmp_desc;
    }

    return p->second;
}

GdkColor
AnthyHelper::get_color_from_key (const String &key)
{
    map< String, GdkColor >::iterator p = m_colors.find (key);
    if (p == m_colors.end ())
    {
        GdkColor ret;
        ret.pixel = ret.red = ret.green = ret.blue = 0;
        return ret;
    }

    return p->second;
}

void
AnthyHelper::show_aux_string (void)
{
    if (!m_initialized ||
        m_aux_string_visible)
        return;

    m_aux_string_visible = true;

    // temporarily move the helper window to the current spot location
    if (!m_lookup_table_visible)
        move_helper_window_to_spot_location ();

    // show the helper window and the aux string
    gtk_widget_show (m_aux_string_label);
    gtk_widget_show (m_aux_event_box);
    gtk_widget_show (m_helper_vbox);
    gtk_widget_show (m_helper_window);

    rearrange_helper_window ();
}

void
AnthyHelper::show_lookup_table (void)
{
    if (!m_initialized ||
        m_lookup_table_visible)
        return;

    m_lookup_table_visible = true;

    // temporarily move the helper window to the current spot location
    if (!m_aux_string_visible)
        move_helper_window_to_spot_location ();

    // show the helper window and the aux string
    for (int i = 0; i < m_current_lookup_table_page_size; i++)
    {
        gtk_widget_show (m_candidates[i].label);
        gtk_widget_show (m_candidates[i].event_box);
    }
    gtk_widget_show (m_lookup_table_vbox);         
    gtk_widget_show (m_helper_vbox);
    gtk_widget_show (m_helper_window);

    rearrange_helper_window ();
}

void
AnthyHelper::hide_aux_string (void)
{
    if (!m_initialized ||
        !m_aux_string_visible)
        return;

    m_aux_string_visible = false;

    // hide the aux string
    gtk_widget_hide (m_aux_string_label);
    gtk_widget_hide (m_aux_event_box);

    if (m_lookup_table_visible)
    {
        // temporarily move the helper window to the current spot location
        // if the lookup table is visible
        move_helper_window_to_spot_location ();

        rearrange_helper_window ();
    }
    else
    {
        // hide the helper window
        gtk_widget_hide (m_helper_vbox);
        gtk_widget_hide (m_helper_window);
    }
}

void
AnthyHelper::hide_lookup_table (void)
{
    if (!m_initialized ||
        !m_lookup_table_visible)
        return;

    m_lookup_table_visible = false;

    // hide the aux string
    // show the helper window and the aux string
    gtk_widget_hide (m_lookup_table_vbox);         
    for (int i = 0; i < m_current_lookup_table_page_size; i++)
    {
        gtk_widget_hide (m_candidates[i].label);
        gtk_widget_hide (m_candidates[i].event_box);
    }

    if (m_aux_string_visible)
    {
        // temporarily move the helper window to the current spot location
        // if the lookup table is visible
        move_helper_window_to_spot_location ();

        rearrange_helper_window ();
    }
    else
    {
        // hide the helper window
        gtk_widget_hide (m_helper_vbox);
        gtk_widget_hide (m_helper_window);
    }
}

void
AnthyHelper::update_aux_string (const WideString &str,
                                const AttributeList &attrs)
{
    if (!m_initialized)
        return;

    // set the aux string label
    gtk_label_set_text (GTK_LABEL (m_aux_string_label),
                        utf8_wcstombs(str).c_str());

    if (m_aux_string_visible)
        rearrange_helper_window ();
    // ToDo: handle attrs
}

void
AnthyHelper::update_lookup_table (const LookupTable &table)
{
    if (!m_initialized)
        return;

    if (m_current_lookup_table_page_size != table.get_current_page_size ())
    {
        int prev_size = m_current_lookup_table_page_size;
        m_current_lookup_table_page_size = table.get_current_page_size ();

        if (m_current_lookup_table_page_size > prev_size)
        {
            allocate_candidates_label (m_current_lookup_table_page_size);

            for (int i = prev_size; i < m_current_lookup_table_page_size; i++)
            {
                gtk_widget_show (m_candidates[i].label);
                gtk_widget_show (m_candidates[i].event_box);
            }
        }
        else if (m_current_lookup_table_page_size < prev_size)
        {
            for (int i = m_current_lookup_table_page_size; i < prev_size; i++) {
                gtk_widget_hide (m_candidates[i].label);
                gtk_widget_hide (m_candidates[i].event_box);
            }
        }
    }

    for (int i = 0; i < m_current_lookup_table_page_size; i++)
    {
        GdkColor tmp_color;
        String tmp_str;

        // set the candidate
        tmp_str = utf8_wcstombs (table.get_candidate_label (i));
        tmp_str += ". ";
        tmp_str += utf8_wcstombs (table.get_candidate (
                                      i + table.get_current_page_start ()));

        gtk_label_set_label (GTK_LABEL (m_candidates[i].label),
                             tmp_str.c_str ());

        // set the background color
        if (table.is_cursor_visible () &&
            i == table.get_cursor_pos_in_current_page ())
        {
            // selected candidate
            tmp_color
                = get_color_from_key (String (ACTIVE_BG_COLOR));
            gtk_widget_modify_bg (m_candidates[i].event_box,
                                  GTK_STATE_NORMAL,
                                  &tmp_color);
            tmp_color
                = get_color_from_key (String (ACTIVE_TEXT_COLOR));
            gtk_widget_modify_fg (m_candidates[i].label,
                                  GTK_STATE_NORMAL,
                                  &tmp_color);
        }
        else
        {
            // not selected candidate
            tmp_color
                = get_color_from_key (String (NORMAL_BG_COLOR));
            gtk_widget_modify_bg (m_candidates[i].event_box,
                                  GTK_STATE_NORMAL,
                                  &tmp_color);
            tmp_color
                = get_color_from_key (String (NORMAL_TEXT_COLOR));
            gtk_widget_modify_fg (m_candidates[i].label,
                                  GTK_STATE_NORMAL,
                                  &tmp_color);
        }
    }

    if (m_lookup_table_visible)
        rearrange_helper_window ();
}

void
AnthyHelper::update_spot_location (int x, int y)
{
    spot_location_x = x;
    spot_location_y = y;

    if (m_lookup_table_visible || 
        m_aux_string_visible)
        move_helper_window_to_spot_location ();
    else if (m_note_visible)
    {
        gtk_window_move (GTK_WINDOW (m_note_window),
                         spot_location_x,
                         spot_location_y);
    }

    rearrange_helper_window ();
}

void
AnthyHelper::show_note ()
{
    if (!m_initialized ||
        m_note_visible)
        return;

    m_note_visible = true;

    if (m_lookup_table_visible || m_aux_string_visible)
    {
        // get the real size of the helper window
        GtkRequisition req;
        gtk_widget_size_request (m_helper_vbox, &req);
        gint helper_window_width = req.width;

        gtk_window_move (GTK_WINDOW (m_note_window),
                         m_helper_window_x + helper_window_width,
                         spot_location_y);
    }
    else
    {
        gtk_window_move (GTK_WINDOW (m_note_window),
                         spot_location_x,
                         spot_location_y);
    }

    gtk_widget_show_all (m_note_window);
    rearrange_note_window ();
}

void
AnthyHelper::hide_note ()
{
    if (!m_initialized ||
        !m_note_visible)
        return;

    m_note_visible = false;

#if GTK_CHECK_VERSION(2, 24, 0)
    gtk_widget_hide (m_note_window);
#else
    gtk_widget_hide_all (m_note_window);
#endif
}

void
AnthyHelper::update_note (const WideString &str)
{
    if (!m_initialized)
        return;

    // set the note label
    gtk_label_set_text (GTK_LABEL (m_note_label),
                        utf8_wcstombs(str).c_str());

    rearrange_note_window ();
}

void
AnthyHelper::updated_screen (int screen_num)
{
    gint n_screens;

    n_screens = gdk_display_get_n_screens (m_display);
    if (screen_num < 0 || screen_num >= n_screens)
        m_current_screen = gdk_display_get_default_screen (m_display);
    else
        m_current_screen = gdk_display_get_screen (m_display, screen_num);

    move_helper_window_to_spot_location ();
    rearrange_helper_window ();
}

static gboolean
select_candidate (GtkWidget *widget, 
                  GdkEventButton *event,
                  gpointer data)
{
    InputContext input_context = helper->get_input_context ();
    int *index = (int *) data;

    Transaction send;
    send.put_command (SCIM_ANTHY_TRANS_CMD_SELECT_CANDIDATE);
    send.put_data ((uint32) *(index));
    helper_agent.send_imengine_event (input_context.ic,
                                      input_context.ic_uuid,
                                      send);

    return FALSE;
}

void
AnthyHelper::allocate_candidates_label(int size)
{
    PangoFontDescription *tmp_desc;

    // initialize not allocated candidate label
    if (size > m_allocated_candidate_num)
    {
        m_candidates = (CandidateLabel *)realloc (m_candidates,
                                                  sizeof(CandidateLabel) * size);
        for (int i = m_allocated_candidate_num; i < size; i++)
        {
            m_candidates[i].label = gtk_label_new ("");
            gtk_misc_set_alignment (GTK_MISC (m_candidates[i].label),
                                    0.0, 0.5); // to left
            tmp_desc = get_font_desc_from_key (String (LOOKUP_FONT));
            gtk_widget_modify_font (m_candidates[i].label, tmp_desc);

            m_candidates[i].event_box = gtk_event_box_new ();
            gtk_container_add (GTK_CONTAINER (m_candidates[i].event_box),
                               m_candidates[i].label);
            gtk_box_pack_start (GTK_BOX (m_lookup_table_vbox),
                                m_candidates[i].event_box,
                                TRUE, TRUE, 0);

            m_candidates[i].index = (int *)malloc (sizeof(int));
            *(m_candidates[i].index) = i;

            g_signal_connect (G_OBJECT (m_candidates[i].event_box),
                              "button-press-event",
                              G_CALLBACK (select_candidate),
                              (gpointer) (m_candidates[i].index));
        }

        m_allocated_candidate_num = size;
    }
}

void
AnthyHelper::move_helper_window_to_spot_location ()
{
    gtk_window_move (GTK_WINDOW (m_helper_window),
                     spot_location_x, spot_location_y);
    m_helper_window_x = spot_location_x;
    m_helper_window_y = spot_location_y;

    if (m_note_visible)
    {
        // get the real size of the helper window
        GtkRequisition req;
        gtk_widget_size_request (m_helper_vbox, &req);
        gint helper_window_width = req.width;

        gtk_window_move (GTK_WINDOW (m_note_window),
                         m_helper_window_x + helper_window_width,
                         spot_location_y);
    }
}

void
AnthyHelper::rearrange_helper_window ()
{
    // get screen size
    gint screen_width, screen_height;
    if (m_current_screen != NULL)
    {
        screen_width = gdk_screen_get_width (m_current_screen);
        screen_height = gdk_screen_get_height (m_current_screen);
    }
    else
        screen_width = screen_height = G_MAXINT;

    // get the real size of the helper window
    GtkRequisition req;
    gtk_widget_size_request (m_helper_vbox, &req);
    gint helper_window_width = req.width;
    gint helper_window_height = req.height;
    bool need_to_move = false;

    // confines the helper window to screen size
    if ((m_helper_window_x + helper_window_width) >= screen_width)
    {
        need_to_move = true;
        m_helper_window_x = screen_width - helper_window_width;
    }

    if ((m_helper_window_y + helper_window_height) >= screen_height)
    {
        need_to_move = true;
        m_helper_window_y = screen_height - helper_window_height;
    }

    if (need_to_move)
    {
        gtk_window_move (GTK_WINDOW (m_helper_window),
                         m_helper_window_x, m_helper_window_y);

        if (m_note_visible)
            rearrange_note_window ();
    }
}

void
AnthyHelper::rearrange_note_window (void)
{
    // get screen size
    gint screen_width, screen_height;
    if (m_current_screen != NULL)
    {
        screen_width = gdk_screen_get_width (m_current_screen);
        screen_height = gdk_screen_get_height (m_current_screen);
    }
    else
        screen_width = screen_height = G_MAXINT;

    // get the real size of the note window and the helper window
    GtkRequisition req;
    gtk_widget_size_request (m_note_event_box, &req);
    gint note_window_width = req.width;
    gint note_window_height = req.height;
    gint note_window_x, note_window_y;
    bool need_to_move = false;

    if (m_aux_string_visible || m_lookup_table_visible)
    {
        gtk_widget_size_request (m_helper_vbox, &req);
        gint helper_window_width = req.width;

        note_window_x = m_helper_window_x + helper_window_width;
        note_window_y = spot_location_y;

        if ((note_window_x + note_window_width) >= screen_width)
        {
            need_to_move = true;
            note_window_x = m_helper_window_x - note_window_width;
        }
    }
    else
    {
        note_window_x = spot_location_x;
        note_window_y = spot_location_y;

        if ((note_window_x + note_window_width) >= screen_width)
        {
            need_to_move = true;
            note_window_x = screen_width - note_window_width;
        }
    }

    if ((note_window_y + note_window_height) >= screen_height)
    {
        need_to_move = true;
        note_window_y = screen_height - note_window_height;
    }

    if (need_to_move)
        gtk_window_move (GTK_WINDOW (m_note_window),
                         note_window_x, note_window_y);
}

void
AnthyHelper::update_lookup_table_style ()
{
    GdkColor tmp_color;
    PangoFontDescription *tmp_desc;

    if (m_helper_window != NULL)
    {
        tmp_color
            = get_color_from_key (String (SCIM_ANTHY_CONFIG_LOOKUP_BORDER_COLOR));
        gtk_widget_modify_bg (m_helper_window,
                              GTK_STATE_NORMAL,
                              &tmp_color);
    }

    if (m_helper_vbox != NULL)
    {
        gtk_container_set_border_width (GTK_CONTAINER (m_helper_vbox), 1);
    }

    for (int i = 0; i < m_allocated_candidate_num; i++)
    {
        if (m_candidates[i].label != NULL)
        {
            tmp_desc = get_font_desc_from_key (String (LOOKUP_FONT));
            gtk_widget_modify_font (m_candidates[i].label, tmp_desc);
        }

        if (m_candidates[i].event_box != NULL)
        {
            tmp_color
                = get_color_from_key (String (NORMAL_BG_COLOR));
            gtk_widget_modify_bg (m_candidates[i].event_box,
                                  GTK_STATE_NORMAL,
                                  &tmp_color);
            tmp_color
                = get_color_from_key (String (NORMAL_TEXT_COLOR));
            gtk_widget_modify_fg (m_candidates[i].label,
                                  GTK_STATE_NORMAL,
                                  &tmp_color);
        }

        if (m_candidates[i].label != NULL)
        {
            gtk_misc_set_padding (GTK_MISC (m_candidates[i].label), 4, 1);
        }
    }
}

void
AnthyHelper::update_aux_string_style ()
{
    GdkColor tmp_color;
    if (m_aux_string_label != NULL)
    {
        tmp_color
            = get_color_from_key (String (SCIM_ANTHY_CONFIG_AUX_BG_COLOR));
        gtk_widget_modify_bg (m_aux_event_box,
                              GTK_STATE_NORMAL,
                              &tmp_color);
        tmp_color
            = get_color_from_key (String (SCIM_ANTHY_CONFIG_AUX_TEXT_COLOR));
        gtk_widget_modify_fg (m_aux_string_label,
                              GTK_STATE_NORMAL,
                              &tmp_color);
    }

    PangoFontDescription *tmp_desc;
    tmp_desc = get_font_desc_from_key (String (LOOKUP_FONT));
    if (m_aux_string_label != NULL)
        gtk_widget_modify_font (m_aux_string_label, tmp_desc);
}

void
AnthyHelper::update_note_style ()
{
    GdkColor tmp_color;
    if (m_note_window != NULL)
    {
        tmp_color
            = get_color_from_key (String (SCIM_ANTHY_CONFIG_NOTE_BORDER_COLOR));
        gtk_widget_modify_bg (m_note_window,
                              GTK_STATE_NORMAL,
                              &tmp_color);
    }

    if (m_note_event_box != NULL)
    {
        tmp_color
            = get_color_from_key (String (SCIM_ANTHY_CONFIG_NOTE_BG_COLOR));
        gtk_widget_modify_bg (m_note_event_box,
                              GTK_STATE_NORMAL,
                              &tmp_color);
        tmp_color
            = get_color_from_key (String (SCIM_ANTHY_CONFIG_NOTE_TEXT_COLOR));
        gtk_widget_modify_fg (m_note_label,
                              GTK_STATE_NORMAL,
                              &tmp_color);

        gtk_container_set_border_width (GTK_CONTAINER (m_note_event_box), 1);
    }

    PangoFontDescription *tmp_desc;
    tmp_desc = get_font_desc_from_key (String (SCIM_ANTHY_CONFIG_NOTE_FONT));
    if (m_note_label != NULL)
        gtk_widget_modify_font (m_note_label, tmp_desc);
}
