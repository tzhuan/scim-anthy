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

#include "scimanthysettingplugin.h"

#include "anthy.h"
#include "anthyui.h"

#include <qcheckbox.h>

#include <kgenericfactory.h>
#include <klocale.h>

typedef KGenericFactory<ScimAnthySettingPlugin> ScimAnthySettingLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_anthy, 
    ScimAnthySettingLoaderFactory( "kcm_skimplugin_scim_anthy" ) )

class ScimAnthySettingPlugin::ScimAnthySettingPluginPrivate {
public:
    AnthySettingUI * ui;
};

ScimAnthySettingPlugin::ScimAnthySettingPlugin(QWidget *parent, 
					       const char */*name*/,
					       const QStringList &args)
 : KAutoCModule( ScimAnthySettingLoaderFactory::instance(), 
     parent, args, AnthyConfig::self() ),
   d(new ScimAnthySettingPluginPrivate)
{
    KGlobal::locale()->insertCatalogue("skim-scim-pinyin");
    d->ui = new AnthySettingUI(this);
    setMainWidget(d->ui);
    
    //FIXME: emit toggle signals to update corresponding connected widgets
    d->ui->advancedCBox->setChecked(true);
    d->ui->advancedCBox->toggle();
    d->ui->kcfg__IMEngine_Pinyin_Ambiguities->toggle();
    d->ui->kcfg__IMEngine_Pinyin_Ambiguities->toggle();
}

ScimAnthySettingPlugin::~ScimAnthySettingPlugin() 
{
    KGlobal::locale()->removeCatalogue("skim-scim-pinyin");
}


#include "scimanthysettingplugin.moc"
