// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/*
 *  2005-06-26 Takuro Ashie <ashie@homa.ne.jp>
 *
 *    * Adapt to Anthy IMEngine.
 */

#include "config.h"
#include "scim_anthy_style_file.h"
#include "scimanthysettingplugin.h"

#include "anthy.h"
#include "anthyui.h"

#include <qdir.h>
#include <qfile.h>
#include <qcheckbox.h>

#include <kgenericfactory.h>
#include <klocale.h>

using namespace scim_anthy;

typedef KGenericFactory<ScimAnthySettingPlugin> ScimAnthySettingLoaderFactory;

K_EXPORT_COMPONENT_FACTORY (kcm_skimplugin_scim_anthy, 
                            ScimAnthySettingLoaderFactory ("kcm_skimplugin_scim_anthy"))

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

class ScimAnthySettingPlugin::ScimAnthySettingPluginPrivate {
public:
    AnthySettingUI * ui;

    StyleFiles m_style_list;
    StyleFile  m_user_style;
    bool       m_style_changed;

public:
    ScimAnthySettingPluginPrivate ()
        : m_style_changed (false)
    {
    }
    ~ScimAnthySettingPluginPrivate ();

public:
    void load_style_files ()
    {
        load_style_dir (SCIM_ANTHY_STYLEDIR);
        load_style_dir (__user_style_dir_name.c_str ());
        m_user_style.load (__user_style_file_name.c_str ());
    }

    void save_style_files ()
    {
        m_user_style.save (__user_style_file_name.c_str ());
    }

private:
    void load_style_dir (const char *dirname)
    {
        QDir dir (dirname);
        for (unsigned int i = 0; i < dir.count (); i++)
        {
            m_style_list.push_back (StyleFile ());
            StyleFile &style = m_style_list.back ();
            bool success = style.load (dir[i]);
            if (!success)
                m_style_list.pop_back ();
        }
    }
};

ScimAnthySettingPlugin::ScimAnthySettingPlugin (QWidget *parent, 
                                                const char *name,
                                                const QStringList &args)
    : KAutoCModule (ScimAnthySettingLoaderFactory::instance (), 
                    parent, args, AnthyConfig::self ()),
      d (new ScimAnthySettingPluginPrivate)
{
    KGlobal::locale()->insertCatalogue ("skim-scim-anthy");
    d->ui = new AnthySettingUI (this);
    setMainWidget (d->ui);
}

ScimAnthySettingPlugin::~ScimAnthySettingPlugin () 
{
    KGlobal::locale()->removeCatalogue ("skim-scim-anthy");
}

void ScimAnthySettingPlugin::load ()
{
    KCModule::load ();
    d->load_style_files ();
}

void ScimAnthySettingPlugin::save ()
{
    KCModule::save ();
    d->save_style_files ();
}

void ScimAnthySettingPlugin::defaults ()
{
    KCModule::defaults ();
}

#include "scimanthysettingplugin.moc"
