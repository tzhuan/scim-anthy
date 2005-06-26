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

class AnthySettingPlugin : public KAutoCModule
{
Q_OBJECT
public:
    AnthyPinyinSettingPlugin(QWidget *parent, 
			     const char */*name*/,
			     const QStringList &args);

    ~AnthySettingPlugin();
private:
    class AnthySettingPluginPrivate;
    AnthySettingPluginPrivate * d;
};

#endif
