/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
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

#ifndef __SCIM_ANTHY_HELPER_H__
#define __SCIM_ANTHY_HELPER_H__

#include <scim_trans_commands.h>
#include <scim_helper.h>
#include <gtk/gtk.h>

#define SCIM_ANTHY_HELPER_UUID "24a65e2b-10a8-4d4c-adc9-266678cb1a38"

#define SCIM_ANTHY_TRANS_CMD_NEW_IC         SCIM_TRANS_CMD_USER_DEFINED + 1
#define SCIM_ANTHY_TRANS_CMD_DELETE_IC      SCIM_TRANS_CMD_USER_DEFINED + 2
#define SCIM_ANTHY_TRANS_CMD_GET_SELECTION  SCIM_TRANS_CMD_USER_DEFINED + 3
#define SCIM_ANTHY_TRANS_CMD_TIMEOUT_ADD    SCIM_TRANS_CMD_USER_DEFINED + 4
#define SCIM_ANTHY_TRANS_CMD_TIMEOUT_REMOVE SCIM_TRANS_CMD_USER_DEFINED + 5
#define SCIM_ANTHY_TRANS_CMD_TIMEOUT_NOTIFY SCIM_TRANS_CMD_USER_DEFINED + 6
#define SCIM_ANTHY_TRANS_CMD_SHOW_NOTE      SCIM_TRANS_CMD_USER_DEFINED + 7
#define SCIM_ANTHY_TRANS_CMD_HIDE_NOTE      SCIM_TRANS_CMD_USER_DEFINED + 8
#define SCIM_ANTHY_TRANS_CMD_UPDATE_NOTE    SCIM_TRANS_CMD_USER_DEFINED + 9

using namespace scim;

struct _scim_anthy_candidate_label
{
    GtkWidget *label;
    GtkWidget *event_box;
};
typedef struct _scim_anthy_candidate_label CandidateLabel;

class AnthyHelper
{
public:
    AnthyHelper ();
    virtual ~AnthyHelper ();

    void init                 (int argc, char **argv,
                               const ConfigPointer &config);
    void reload_config        (void);

    void show_aux_string      (void);
    void show_lookup_table    (void);
    void hide_aux_string      (void);
    void hide_lookup_table    (void);
    void update_aux_string    (const WideString &str,
                               const AttributeList &attrs
                               = AttributeList());
    void update_lookup_table  (const LookupTable &table);

    void show_note            (void);
    void hide_note            (void);
    void update_note          (const WideString &str);

    void update_spot_location (int x, int y);
    void update_screen (int screen_num);

private:
    /* config */
    ConfigPointer m_config;

    /* fundamental information of screen */
    GdkDisplay *m_display;
    GdkScreen *m_current_screen;

    int spot_location_x;
    int spot_location_y;

    PangoFontDescription *m_font_desc;
    GdkColor m_active_bg;
    GdkColor m_active_text;
    GdkColor m_normal_bg;
    GdkColor m_normal_text;

    /* aux string */
    bool aux_string_window_visible;
    GtkWidget *aux_string_window;
    GtkWidget *aux_string_label;

    /* lookup table and candidates */
    GtkWidget *lookup_table_window;
    GtkWidget *lookup_table_vbox;
    CandidateLabel *candidates;
    int allocated_candidate_num;
    bool lookup_table_window_visible;

private:
    void relocate_windows     (void);
    
};

#endif /* __SCIM_ANTHY_HELPER_H__ */
