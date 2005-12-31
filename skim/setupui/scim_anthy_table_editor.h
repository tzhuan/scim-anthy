// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
/***************************************************************************
 *   Copyright (C) 2005 Takuro Ashie                                       *
 *   ashie@homa.ne.jp                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef SCIM_ANTHY_TABLE_EDITOR_H
#define SCIM_ANTHY_TABLE_EDITOR_H

#include <kdialogbase.h>

class ScimAnthyTableEditor : public KDialogBase
{
Q_OBJECT
public:
    ScimAnthyTableEditor  (QWidget *parent = 0, const char *name = 0);
    ~ScimAnthyTableEditor ();
};

#endif // SCIM_ANTHY_TABLE_EDITOR
