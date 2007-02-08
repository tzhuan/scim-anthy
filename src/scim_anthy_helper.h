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

#define Uses_SCIM_CONFIG_BASE
#include <scim_trans_commands.h>
#include <scim_helper.h>
#include <gtk/gtk.h>
#include <map>
#include "scim_anthy_const.h"

#define SCIM_ANTHY_HELPER_UUID "24a65e2b-10a8-4d4c-adc9-266678cb1a38"

using namespace scim;
using std::map;

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

    void init                   (const ConfigPointer &config, const char *dsp);
    void reload_config          (void);

    void show_aux_string        (void);
    void show_lookup_table      (void);
    void hide_aux_string        (void);
    void hide_lookup_table      (void);
    void update_aux_string      (const WideString &str,
                                 const AttributeList &attrs
                                 = AttributeList());
    void update_lookup_table    (const LookupTable &table);

    void show_note              (void);
    void hide_note              (void);
    void update_note            (const WideString &str);

    void update_spot_location   (int x, int y);
    void update_screen          (int screen_num);

private:
    /* config */
    ConfigPointer m_config;

    /* fundamental information of screen */
    GdkDisplay *m_display;
    GdkScreen *m_current_screen;

    int spot_location_x;
    int spot_location_y;

    map< String, GdkColor > m_colors;
    map< String, String > m_default_colors;

    map< String, PangoFontDescription* > m_fonts;
    map< String, String > m_default_fonts;

    /* herlper main window */
    GtkWidget *m_helper_window;
    GtkWidget *m_helper_vbox;

    /* aux string */
    bool m_aux_string_visible;
    GtkWidget *m_aux_event_box;
    GtkWidget *m_aux_string_label;

    /* lookup table and candidates */
    bool lookup_table_visible;
    GtkWidget *lookup_table_vbox;
    CandidateLabel *candidates;
    int allocated_candidate_num;

    /* note window */
    bool m_note_visible;
    GtkWidget *m_note_window;
    GtkWidget *m_note_event_box;
    GtkWidget *m_note_label;

private:
    void free_all_font_descs       (void);
    PangoFontDescription *get_font_desc_from_key (const String &key);
    GdkColor get_color_from_key    (const String &key);

    void relocate_windows          (void);
    void update_lookup_table_style (void);
    void update_aux_string_style   (void);
    void update_note_style         (void);
    
};

#endif /* __SCIM_ANTHY_HELPER_H__ */
