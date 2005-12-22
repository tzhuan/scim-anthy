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
#include <kcombobox.h>

using namespace scim_anthy;

typedef KGenericFactory<ScimAnthySettingPlugin> ScimAnthySettingLoaderFactory;

static const char * const __key_bindings_theme = "KeyBindings";
static const char * const __romaji_fund_table  = "RomajiTable/FundamentalTable";
static const char * const __kana_fund_table    = "KanaTable/FundamentalTable";
static const char * const __nicola_fund_table  = "NICOLATable/FundamentalTable";

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


K_EXPORT_COMPONENT_FACTORY (kcm_skimplugin_scim_anthy, 
                            ScimAnthySettingLoaderFactory ("kcm_skimplugin_scim_anthy"))


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
    ~ScimAnthySettingPluginPrivate ()
    {
    }

public:
    void load_style_files ()
    {
        m_style_list.clear ();
        load_style_dir (SCIM_ANTHY_STYLEDIR);
        load_style_dir (__user_style_dir_name.c_str ());
        m_user_style.load (__user_style_file_name.c_str ());
    }

    void save_style_files ()
    {
        if (m_style_changed) {
            m_user_style.save (__user_style_file_name.c_str ());
            m_style_changed = false;
        }
    }

    void setup_combo_box (KComboBox *combo, const char *section_name, QString default_file)
    {
        QStringList theme_list;
        theme_list.append ("Default");
        theme_list.append ("User defined");

        std::cout << default_file << std::endl;

        StyleFiles::iterator it;
        QString cur_item = "Default";
        for (it = m_style_list.begin(); it != m_style_list.end (); it++) {
            StyleLines section;
            if (!it->get_entry_list (section, section_name))
                continue;
            theme_list.append (QString::fromUtf8 (it->get_title().c_str()));
            if (default_file == QString::fromUtf8 (it->get_file_name().c_str()))
                cur_item = theme_list.back ();
        }

        combo->insertStringList (theme_list);
        combo->setCurrentText (cur_item);
    }

private:
    void load_style_dir (const char *dirname)
    {
        QDir dir (dirname, "*.sty");
        dir.setFilter (QDir::Files | QDir::Readable);
        for (unsigned int i = 0; i < dir.count (); i++)
        {
            QString path = dirname;
            path += QDir::separator ();
            path += dir[i];
            m_style_list.push_back (StyleFile ());
            StyleFile &style = m_style_list.back ();
            bool success = style.load (path.ascii ());
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

    load ();
}

ScimAnthySettingPlugin::~ScimAnthySettingPlugin () 
{
    KGlobal::locale()->removeCatalogue ("skim-scim-anthy");
}

void ScimAnthySettingPlugin::load ()
{
    KCModule::load ();
    d->load_style_files ();

    StyleFiles::iterator it;

    // set key bindings theme list
    d->setup_combo_box (d->ui->KeyBindingsThemeComboBox,
                        __key_bindings_theme,
                        QString());
    // set romaji table list
    d->setup_combo_box (d->ui->RomajiComboBox,
                        __romaji_fund_table,
                        AnthyConfig::_IMEngine_Anthy_RomajiThemeFile());
    // set kana layout list
    d->setup_combo_box (d->ui->KanaComboBox,
                        __kana_fund_table,
                        AnthyConfig::_IMEngine_Anthy_KanaLayoutFile());
    // set NICOLA layout list
    d->setup_combo_box (d->ui->ThumbShiftComboBox,
                        __nicola_fund_table,
                        AnthyConfig::_IMEngine_Anthy_NICOLALayoutFile());
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
