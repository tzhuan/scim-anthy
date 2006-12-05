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
#include <map>
#include <scim.h>
#include <gtk/gtk.h>
#include "scim_anthy_intl.h"
#include "scim_anthy_helper.h"

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
static gint       timeout_func                (gpointer             data);
static void       timeout_ctx_destroy_func    (gpointer             data);

static void       run                         (const String        &display,
                                               const ConfigPointer &config);

AnthyHelper helper;
HelperAgent helper_agent;

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
    gtk_main_quit ();
}

static void
slot_update_spot_location   (const HelperAgent *agent,
                             int ic, const String &uuid,
                             int x, int y)
{
    helper.update_spot_location (x,y);
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
        helper.show_aux_string ();
        break;
    }
    case SCIM_TRANS_CMD_SHOW_LOOKUP_TABLE:
    {
        helper.show_lookup_table ();
        break;
    }
    case SCIM_TRANS_CMD_HIDE_AUX_STRING:
    {
        helper.hide_aux_string ();
        break;
    }
    case SCIM_TRANS_CMD_HIDE_LOOKUP_TABLE:
    {
        helper.hide_lookup_table ();
        break;
    }
    case SCIM_TRANS_CMD_UPDATE_AUX_STRING:
    {
        WideString str;
        AttributeList attr;
        reader.get_data (str);
        reader.get_data (attr);
        helper.update_aux_string (str, attr);
        break;
    }
    case SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE:
    {
        CommonLookupTable table;
        reader.get_data (table);
        helper.update_lookup_table (table);
        break;
    }
    default:
        break;
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

    helper.init (argc, argv);

    helper_agent.signal_connect_exit (slot (slot_exit));
    helper_agent.signal_connect_update_spot_location (slot (slot_update_spot_location));
    helper_agent.signal_connect_process_imengine_event (slot (slot_imengine_event));

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
    : spot_location_x             (0),
      spot_location_y             (0),
      aux_string_window           (NULL),
      aux_string_label            (NULL),
      lookup_table_window         (NULL),
      lookup_table_label          (NULL),
      aux_string_window_visible   (false),
      lookup_table_window_visible (false)
{
}

AnthyHelper::~AnthyHelper ()
{
    if (aux_string_window)
    {
        gtk_widget_hide (aux_string_window);
        gtk_widget_destroy (aux_string_window);
    }

    if (aux_string_label)
    {
        gtk_widget_hide (aux_string_label);
        gtk_widget_destroy (aux_string_label);
    }

    if (lookup_table_window)
    {
        gtk_widget_hide (lookup_table_window);
        gtk_widget_destroy (lookup_table_window);
    }

    if (lookup_table_label)
    {
        gtk_widget_hide (lookup_table_label);
        gtk_widget_destroy (lookup_table_label);
    }
}

void
AnthyHelper::init (int argc, char **argv)
{
    gtk_init (&argc, &argv);

    // aux string window
    aux_string_window = gtk_window_new (GTK_WINDOW_POPUP);
    gtk_window_set_default_size (GTK_WINDOW (aux_string_window), 100, 20);
    gtk_window_set_policy (GTK_WINDOW (aux_string_window),
                           TRUE, TRUE, FALSE);
    gtk_window_set_resizable (GTK_WINDOW (aux_string_window), FALSE);
    aux_string_window_visible = false;

    aux_string_label = gtk_label_new ("");
    gtk_container_add (GTK_CONTAINER (aux_string_window),
                       aux_string_label);

    // lookup table window
    lookup_table_window = gtk_window_new (GTK_WINDOW_POPUP);
    gtk_window_set_default_size (GTK_WINDOW (lookup_table_window), 50, 100);
    gtk_window_set_policy (GTK_WINDOW (lookup_table_window),
                           TRUE, TRUE, FALSE);
    gtk_window_set_resizable (GTK_WINDOW (lookup_table_window), FALSE);
    lookup_table_window_visible = false;

    lookup_table_label = gtk_label_new ("");
    gtk_container_add (GTK_CONTAINER (lookup_table_window),
                       lookup_table_label);
}

void
AnthyHelper::show_aux_string (void)
{
    if (aux_string_window == NULL ||
        aux_string_label == NULL)
        return;

    gtk_widget_show (aux_string_label);
    gtk_widget_show (aux_string_window);
    aux_string_window_visible = true;

    relocate_windows ();
}

void
AnthyHelper::show_lookup_table (void)
{
    if (lookup_table_window == NULL ||
        lookup_table_label == NULL)
        return;

    gtk_widget_show (lookup_table_label);
    gtk_widget_show (lookup_table_window);
    lookup_table_window_visible = true;

    relocate_windows ();
}

void
AnthyHelper::hide_aux_string (void)
{
    if (aux_string_window == NULL ||
        aux_string_label == NULL)
        return;

    gtk_widget_hide (aux_string_label);
    gtk_widget_hide (aux_string_window);
    aux_string_window_visible = false;
}

void
AnthyHelper::hide_lookup_table (void)
{
    if (lookup_table_window == NULL ||
        lookup_table_label == NULL)
        return;

    gtk_widget_hide (lookup_table_label);
    gtk_widget_hide (lookup_table_window);
    lookup_table_window_visible = false;
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
    if (lookup_table_label == NULL)
        return;

    int size = table.get_current_page_size ();
    String table_str;

    for (int i = 0; i < size; i++)
    {
        if (table.is_cursor_visible () &&
            i == table.get_cursor_pos_in_current_page ())
            table_str += "* ";
        else
            table_str += "  ";

        table_str += utf8_wcstombs (table.get_candidate_label (i));
        table_str += ":";
        table_str += utf8_wcstombs (table.get_candidate (
                                        i + table.get_current_page_start ()));

        if (i != size - 1)
            table_str += "\n";
    }
    gtk_label_set_text (GTK_LABEL (lookup_table_label),
                        table_str.c_str());

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
AnthyHelper::relocate_windows (void)
{
    if (aux_string_window == NULL ||
        lookup_table_window == NULL)
        return; // not initialized yet

    // move aux string window and lookup table window
    if (lookup_table_window_visible)
    {
        // move the lookup table window to the spot location and move the
        // aux string window beneath it.
        int lookup_table_window_width, lookup_table_window_height;
        gtk_window_move (GTK_WINDOW (lookup_table_window),
                         spot_location_x,
                         spot_location_y);
        gtk_window_get_size (GTK_WINDOW (lookup_table_window),
                             &lookup_table_window_width,
                             &lookup_table_window_height);

        if (aux_string_window_visible)
        {
            gtk_window_move (GTK_WINDOW (aux_string_window),
                             spot_location_x,
                             spot_location_y + lookup_table_window_height);
        }
    }
    else if (aux_string_window_visible)
    {
        // only the aux string window to move
        gtk_window_move (GTK_WINDOW (aux_string_window),
                         spot_location_x,
                         spot_location_y);
    }
}
