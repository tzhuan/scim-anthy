/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie
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

#define Uses_SCIM_HELPER
#define Uses_SCIM_CONFIG_BASE

#include <gdk/gdkx.h> // to avoid name confliction (ad-hoc)

#include <map>
#include <scim.h>
#include <gtk/gtk.h>
#include "scim_anthy_intl.h"
#include "scim_anthy_helper.h"
#include "scim_anthy_tray.h"

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

AnthyHelper *helper = NULL;
AnthyTray   *tray   = NULL;
HelperAgent  helper_agent;

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
    if (tray != NULL) {
        delete tray;
        tray = NULL;
    }

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
        guint timeout_id = gtk_timeout_add_full (time_msec,
                                                 timeout_func,
                                                 NULL,
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
            gtk_timeout_remove (tid);
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
    case SCIM_ANTHY_TRANS_CMD_INSTALL_PROPERTIES:
    {
        PropertyList props;
        reader.get_data (props);
        if (tray)
            tray->init_properties (props);
    }
    case SCIM_ANTHY_TRANS_CMD_UPDATE_PROPERTY:
    {
        Property prop;
        reader.get_data (prop);
        if (tray)
            tray->update_property (prop);
    }
    default:
        break;
    }
}

static void
slot_update_screen (const HelperAgent *agent, int ic,
                    const String &ic_uuid, int screen_number)
{
    helper->update_screen (screen_number);
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
    if (tray != NULL)
    {
        tray->attach_input_context (agent, ic, ic_uuid);

        Transaction send;
        send.put_command (SCIM_ANTHY_TRANS_CMD_ATTACHMENT_SUCCESS);
        helper_agent.send_imengine_event (ic, ic_uuid, send);
    }    
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

    argv [0] = "anthy-imengine-helper";
    argv [1] = "--display";
    argv [2] = const_cast<char *> (display.c_str ());
    argv [3] = 0;
 
    setenv ("DISPLAY", display.c_str (), 1);

    gtk_init (&argc, &argv);

    helper = new AnthyHelper;
    tray = new AnthyTray;

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

AnthyHelper::AnthyHelper ()
    : m_config                    (NULL),
      m_display                   (NULL),
      m_current_screen            (NULL),
      spot_location_x             (0),
      spot_location_y             (0),
      m_font_desc                 (NULL),
      m_helper_window             (NULL),
      m_helper_vbox               (NULL),
      aux_string_visible          (false),
      aux_string_label            (NULL),
      lookup_table_visible        (false),
      lookup_table_vbox           (NULL),
      candidates                  (NULL),
      allocated_candidate_num     (0),
      m_note_visible              (false),
      m_note_window               (NULL),
      m_note_event_box            (NULL),
      m_note_label                (NULL)
{
    m_active_bg.red = m_active_bg.green = m_active_bg.blue = 65535;
    m_active_text.red = m_active_text.green = m_active_text.blue = 0;
    m_normal_bg.red = m_normal_bg.green = m_normal_bg.blue = 65535;
    m_normal_text.red = m_normal_text.green = m_normal_text.blue = 0;
}

AnthyHelper::~AnthyHelper ()
{
    if (m_font_desc)
        pango_font_description_free (m_font_desc);

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

    if (aux_string_label)
    {
        gtk_widget_hide (aux_string_label);
        gtk_widget_destroy (aux_string_label);
    }

    if (lookup_table_vbox)
    {
        gtk_widget_hide (lookup_table_vbox);
        gtk_widget_destroy (lookup_table_vbox);
    }

    for (int i = 0; i < allocated_candidate_num; i++)
    {
        gtk_widget_hide (candidates[i].event_box);
        gtk_widget_hide (candidates[i].label);
        gtk_widget_destroy (candidates[i].event_box);
        gtk_widget_destroy (candidates[i].label);
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
    gtk_window_set_default_size (GTK_WINDOW (m_helper_window), 100, 20);
    gtk_window_set_policy (GTK_WINDOW (m_helper_window),
                           TRUE, TRUE, FALSE);
    gtk_window_set_resizable (GTK_WINDOW (m_helper_window), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (m_helper_window), 1);

    m_helper_vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (m_helper_window),
                       m_helper_vbox);

    // aux string
    aux_string_visible = false;
    aux_string_label = gtk_label_new ("");
    gtk_misc_set_alignment (GTK_MISC (aux_string_label),
                            0.0, 0.5); // to left
    gtk_box_pack_end (GTK_BOX(m_helper_vbox),
                      aux_string_label,
                      TRUE, TRUE, 0);

    // lookup table
    lookup_table_visible = false;
    lookup_table_vbox = gtk_vbox_new (TRUE, 0);
    gtk_box_pack_end (GTK_BOX(m_helper_vbox),
                      lookup_table_vbox,
                      TRUE, TRUE, 0);

    // note window
    m_note_visible = false;

    m_note_window = gtk_window_new (GTK_WINDOW_POPUP);
    gtk_window_set_default_size (GTK_WINDOW (m_note_window), 100, 20);
    gtk_window_set_policy (GTK_WINDOW (m_note_window),
                           TRUE, TRUE, FALSE);
    gtk_window_set_resizable (GTK_WINDOW (m_note_window), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (m_note_window), 1);

    m_note_event_box = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (m_note_window),
                       m_note_event_box);

    m_note_label = gtk_label_new ("");
    gtk_container_add (GTK_CONTAINER (m_note_event_box),
                       m_note_label);
}

void
AnthyHelper::reload_config ()
{
    String tmp;

    // change colors
    tmp = m_config->read (String ("/Panel/Gtk/Color/ActiveBackground"),
                          String ("light sky blue"));
    if (gdk_color_parse (tmp.c_str(), &m_active_bg) == FALSE)
    {
        m_active_bg.red = 135 * 255;
        m_active_bg.green = 206 * 255;
        m_active_bg.blue = 250 * 255;
    }

    tmp = m_config->read (String ("/Panel/Gtk/Color/ActiveText"),
                          String ("black"));
    if (gdk_color_parse (tmp.c_str(), &m_active_text) == FALSE)
        m_active_text.red = m_active_text.green = m_active_text.blue = 0;

    tmp = m_config->read (String ("/Panel/Gtk/Color/NormalBackground"),
                          String ("white"));
    if (gdk_color_parse (tmp.c_str(), &m_normal_bg) == FALSE)
        m_normal_bg.red = m_normal_bg.green = m_normal_bg.blue =65535;

    tmp = m_config->read (String ("/Panel/Gtk/Color/NormalText"),
                          String ("black"));
    if (gdk_color_parse (tmp.c_str(), &m_normal_text) == FALSE)
        m_normal_text.red = m_normal_text.green = m_normal_text.blue = 0;

    // change font
    tmp = m_config->read (String ("/Panel/Gtk/Font"),
                          String ("Sans 12"));
    if (m_font_desc)
        pango_font_description_free (m_font_desc);
    m_font_desc = pango_font_description_from_string (tmp.c_str ());

    // change colors
    update_lookup_table_style ();
    update_aux_string_style ();
    update_note_style ();
}

void
AnthyHelper::show_aux_string (void)
{
    if (m_helper_window == NULL ||
        m_helper_vbox == NULL ||
        aux_string_label == NULL)
        return;

    gtk_widget_show (aux_string_label);
    gtk_widget_show (m_helper_vbox);
    gtk_widget_show (m_helper_window);
    aux_string_visible = true;

    relocate_windows ();
    update_aux_string_style ();
}

void
AnthyHelper::show_lookup_table (void)
{
    if (m_helper_window == NULL ||
        m_helper_vbox == NULL ||
        lookup_table_vbox == NULL)
        return;

    gtk_widget_show (lookup_table_vbox);
    gtk_widget_show (m_helper_vbox);
    gtk_widget_show (m_helper_window);
    lookup_table_visible = true;

    relocate_windows ();
    update_lookup_table_style ();
}

void
AnthyHelper::hide_aux_string (void)
{
    if (m_helper_window == NULL ||
        m_helper_vbox == NULL ||
        aux_string_label == NULL)
        return;

    gtk_widget_hide (aux_string_label);
    if (lookup_table_visible == FALSE)
    {
        gtk_widget_hide (m_helper_vbox);
        gtk_widget_hide (m_helper_window);
    }
    aux_string_visible = false;

    relocate_windows ();
}

void
AnthyHelper::hide_lookup_table (void)
{
    if (m_helper_window == NULL ||
        m_helper_vbox == NULL ||
        lookup_table_vbox == NULL)
        return;

    gtk_widget_hide (lookup_table_vbox);
    if (aux_string_visible == FALSE)
    {
        gtk_widget_hide (m_helper_vbox);
        gtk_widget_hide (m_helper_window);
    }
    lookup_table_visible = false;

    relocate_windows ();
}

void
AnthyHelper::update_aux_string (const WideString &str,
                                const AttributeList &attrs)
{
    if (aux_string_label == NULL)
        return;

    String aux_string = utf8_wcstombs (str);
    gtk_label_set_text (GTK_LABEL (aux_string_label),
                        aux_string.c_str());

    relocate_windows ();
    // ToDo: handle attrs
}

void
AnthyHelper::update_lookup_table (const LookupTable &table)
{
    if (lookup_table_vbox == NULL)
        return;

    int size = table.get_current_page_size ();

    // initialize not allocated candidate label
    if (size > allocated_candidate_num)
    {
        candidates = (CandidateLabel *)realloc (candidates,
                                                sizeof(CandidateLabel) * size);
        for (int i = allocated_candidate_num; i < size; i++)
        {
            candidates[i].label = gtk_label_new ("");
            gtk_misc_set_alignment (GTK_MISC (candidates[i].label),
                                    0.0, 0.5); // to left
            gtk_widget_modify_font (candidates[i].label, m_font_desc);

            candidates[i].event_box = gtk_event_box_new ();
            gtk_container_add (GTK_CONTAINER (candidates[i].event_box),
                               candidates[i].label);
            gtk_box_pack_start (GTK_BOX (lookup_table_vbox),
                                candidates[i].event_box,
                                TRUE, TRUE, 0);
        }

        allocated_candidate_num = size;
    }

    for (int i = 0; i < size; i++)
    {
        String tmp;

        tmp += utf8_wcstombs (table.get_candidate_label (i));
        tmp += ". ";
        tmp += utf8_wcstombs (table.get_candidate (
                                  i + table.get_current_page_start ()));

        gtk_label_set_label (GTK_LABEL (candidates[i].label),
                             tmp.c_str ());
        gtk_widget_show (candidates[i].label);

        gtk_widget_show (candidates[i].event_box);
        if (table.is_cursor_visible () &&
            i == table.get_cursor_pos_in_current_page ())
        {
            // selected candidate
            gtk_widget_modify_bg (candidates[i].event_box,
                                  GTK_STATE_NORMAL,
                                  &m_active_bg);
            gtk_widget_modify_text (candidates[i].event_box,
                                    GTK_STATE_NORMAL,
                                    &m_active_text);
        }
        else
        {
            // not selected candidate
            gtk_widget_modify_bg (candidates[i].event_box,
                                  GTK_STATE_NORMAL,
                                  &m_normal_bg);
            gtk_widget_modify_text (candidates[i].event_box,
                                    GTK_STATE_NORMAL,
                                    &m_normal_text);
        }
    }

    for (int i = size; i < allocated_candidate_num; i++)
    {
        gtk_widget_hide (candidates[i].event_box);
        gtk_widget_hide (candidates[i].label);
    }

    relocate_windows ();
}

void
AnthyHelper::update_spot_location (int x, int y)
{
    spot_location_x = x;
    spot_location_y = y;

    relocate_windows ();
}

void
AnthyHelper::show_note ()
{
    if (m_note_window == NULL ||
        m_note_event_box == NULL ||
        m_note_label == NULL)
        return;

    gtk_widget_show (m_note_label);
    gtk_widget_show (m_note_event_box);
    gtk_widget_show (m_note_window);
    m_note_visible = true;

    relocate_windows();
    update_note_style ();
}

void
AnthyHelper::hide_note ()
{
    if (m_note_window == NULL ||
        m_note_event_box == NULL ||
        m_note_label == NULL)
        return;

    gtk_widget_hide (m_note_label);
    gtk_widget_hide (m_note_event_box);
    gtk_widget_hide (m_note_window);
    m_note_visible = false;

    relocate_windows();
}

void
AnthyHelper::update_note (const WideString &str)
{
    if (m_note_label == NULL)
        return;

    String note = utf8_wcstombs (str);
    gtk_label_set_text (GTK_LABEL (m_note_label),
                        note.c_str());

    relocate_windows ();
}

void
AnthyHelper::update_screen (int screen_num)
{
    gint n_screens;

    n_screens = gdk_display_get_n_screens (m_display);
    if (screen_num < 0 || screen_num >= n_screens)
        m_current_screen = gdk_display_get_default_screen (m_display);
    else
        m_current_screen = gdk_display_get_screen (m_display, screen_num);

    relocate_windows ();
}

void
AnthyHelper::relocate_windows (void)
{
    if (m_helper_window == NULL)
        return; // not initialized yet

    // reset the size
    gtk_widget_set_size_request (m_helper_window, -1, -1);
    gtk_widget_set_size_request (m_note_window, -1, -1);

    // get the requested size of lookup table window and aux string window
    // Note:
    // if gtk_window_resize() is called then immediately gtk_window_get_size()
    // is called, the size won't have taken effect yet because the window
    // manager processes the resize request out of time. That is why
    // gtk_window_resize() is not used and gtk_widget_size_request() 
    // is used in the following code chunk. 

    GtkRequisition req;
    gtk_widget_size_request (m_helper_window, &req);
    gint helper_window_width = req.width;
    gint helper_window_height = req.height;

    gtk_widget_size_request (m_note_window, &req);
    gint note_window_width = req.width;
    gint note_window_height = req.height;

    // get screen size
    gint screen_width, screen_height;
    if (m_current_screen != NULL)
    {
        screen_width = gdk_screen_get_width (m_current_screen);
        screen_height = gdk_screen_get_height (m_current_screen);
    }
    else
    {
        screen_width = screen_height = G_MAXINT;
    }

    // current spot location
    gint fixed_x = spot_location_x;
    gint fixed_y = spot_location_y;

    // move helper window
    if (lookup_table_visible || aux_string_visible)
    {
        // confines lookup table window to screen size
        if ((spot_location_x + helper_window_width) >= screen_width)
            fixed_x = screen_width - helper_window_width;
        if ((spot_location_y + helper_window_height) >= screen_height)
            fixed_y = screen_height - helper_window_height;

        gtk_window_move (GTK_WINDOW (m_helper_window),
                         fixed_x, fixed_y);
    }

    fixed_x = spot_location_x + helper_window_width;
    fixed_y = spot_location_y;

    // move note window
    if (m_note_visible)
    {
        if ( (fixed_x + note_window_width) >= screen_width)
            fixed_x = spot_location_x - note_window_width;
            // left of the main helper window
        if ( (fixed_y + note_window_height) >= screen_height)
            fixed_y = screen_height - note_window_height;

        gtk_window_move (GTK_WINDOW (m_note_window),
                         fixed_x, fixed_y);
    }
}

void
AnthyHelper::update_lookup_table_style ()
{
    for (int i = 0; i < allocated_candidate_num; i++)
    {
        if (candidates[i].label != NULL)
        {
            gtk_widget_modify_font (candidates[i].label, m_font_desc);
        }

        if (candidates[i].event_box != NULL)
        {
            gtk_widget_modify_bg (candidates[i].event_box,
                                  GTK_STATE_NORMAL,
                                  &m_normal_bg);
            gtk_widget_modify_text (candidates[i].event_box,
                                    GTK_STATE_NORMAL,
                                    &m_normal_text);
        }
    }
}

void
AnthyHelper::update_aux_string_style ()
{
    if (aux_string_label != NULL)
        gtk_widget_modify_font (aux_string_label, m_font_desc);
}

void
AnthyHelper::update_note_style ()
{
    if (m_note_event_box != NULL)
    {
        gtk_widget_modify_bg (m_note_event_box,
                              GTK_STATE_NORMAL,
                              &m_normal_bg);
        gtk_widget_modify_text (m_note_event_box,
                                GTK_STATE_NORMAL,
                                &m_normal_text);
    }

    if (m_note_label != NULL)
        gtk_widget_modify_font (m_note_label, m_font_desc);
}
