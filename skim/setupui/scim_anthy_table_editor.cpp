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

#include <qlayout.h>
#include <klineedit.h>
#include "scim_anthy_table_editor.h"

ScimAnthyTableEditor::ScimAnthyTableEditor (QWidget *parent, const char *name)
    : KDialogBase (KDialogBase::Plain, 0, parent, name, true,
                   "Edit key table", KDialogBase::Ok | KDialogBase::Cancel)
{
    QVBoxLayout *vbox = new QVBoxLayout (plainPage());
    KLineEdit *editor = new KLineEdit (plainPage ());
    vbox->addWidget (editor);
}

ScimAnthyTableEditor::~ScimAnthyTableEditor ()
{
}
