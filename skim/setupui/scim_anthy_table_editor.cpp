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
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <klocale.h>
#include <klineedit.h>
#include <kcombobox.h>
#include "scim_anthy_table_editor.h"

ScimAnthyTableEditor::ScimAnthyTableEditor (QWidget *parent, const char *name)
    : KDialogBase (KDialogBase::Plain, 0, parent, name, true,
                   i18n ("Edit key table", KDialogBase::Ok | KDialogBase::Cancel)
{
    setMinimumWidth (300);
    setMinimumHeight (250);

    QVBoxLayout *main_vbox = new QVBoxLayout (plainPage (),6);
    QHBoxLayout *theme_hbox = new QHBoxLayout (main_vbox, 6);
    QHBoxLayout *editor_hbox = new QHBoxLayout (main_vbox, 6);

    KComboBox *combo = new KComboBox (plainPage ());
    QLabel *label = new QLabel (i18n ("Romaji table:"), plainPage ());
    theme_hbox->addWidget (label);
    theme_hbox->addWidget (combo);
    theme_hbox->addStretch (20);

    QListView *view = new QListView (plainPage ());
    editor_hbox->addWidget (view);

    QVBoxLayout *editor_vbox = new QVBoxLayout (editor_hbox, 6);

    //
    label = new QLabel (i18n ("Sequence:"), plainPage ());
    editor_vbox->addWidget (label);

    KLineEdit *entry = new KLineEdit (plainPage ());
    entry->setMaximumWidth (80);
    entry->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
    editor_vbox->addWidget (entry);

    //
    label = new QLabel (i18n ("Result:"), plainPage ());
    editor_vbox->addWidget (label);

    entry = new KLineEdit (plainPage ());
    entry->setMaximumWidth (80);
    entry->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
    editor_vbox->addWidget (entry);

    //
    QPushButton *add_button = new QPushButton (i18n ("Add"), plainPage ());
    editor_vbox->addWidget (add_button);

    //
    QPushButton *remove_button = new QPushButton (i18n ("Remove"), plainPage ());
    editor_vbox->addWidget (remove_button);

    editor_vbox->addStretch (20);
}

ScimAnthyTableEditor::~ScimAnthyTableEditor ()
{
}
