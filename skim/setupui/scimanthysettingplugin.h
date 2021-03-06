// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/

/*
 *  2005-06-26 Takuro Ashie <ashie@homa.ne.jp>
 *
 *    * Adapt to Anthy IMEngine.
 */

#ifndef SCIMANTHYSETTINGPLUGIN_H
#define SCIMANTHYSETTINGPLUGIN_H

#include "utils/kautocmodule.h"

class QListViewItem *item;

class ScimAnthySettingPlugin : public KAutoCModule
{
Q_OBJECT
public:
    ScimAnthySettingPlugin  (QWidget           *parent, 
                             const char        *name,
                             const QStringList &args);
    ~ScimAnthySettingPlugin ();

    // override KCModule's functions to load & save scim-anthy's style file.
    virtual void load       ();
    virtual void save       ();
    virtual void defaults   ();

protected slots:
    void slotWidgetModified                  ();

    void launch_dict_admin_command           ();
    void launch_add_word_command             ();
    void set_key_bindings_group              ();
    void set_key_bindings_theme              (int n);
    void choose_keys                         ();
    void customize_romaji_table              ();
    void customize_kana_table                ();
    void customize_nicola_table              ();
    void key_bindings_view_selection_changed (QListViewItem *item);
    void preedit_string_style_changed        (int n);
    void conversion_string_style_changed     (int n);
    void selected_segment_style_changed      (int n);
    void set_romaji_table_view               ();
    void set_kana_table_view                 ();
    void set_thumb_shift_table_view          ();
    void romaji_customize_ok                 ();
    void kana_customize_ok                   ();
    void nicola_customize_ok                 ();
    void table_dialog_cancel                 ();

private:
    class ScimAnthySettingPluginPrivate;
    ScimAnthySettingPluginPrivate * d;
    QString m_name;
};

#endif
