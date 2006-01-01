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

ScimAnthyTableEditor::ScimAnthyTableEditor (QWidget *parent,
                                            const QString & chooser_label,
                                            const QString & label1,
                                            const QString & label2,
                                            const QString & label3,
                                            const QString & label4,
                                            const char *name)
    : KDialogBase (KDialogBase::Plain, 0, parent, name, true,
                   i18n ("Edit key table"), KDialogBase::Ok | KDialogBase::Cancel)
{
    setMinimumWidth (300);
    setMinimumHeight (250);

    QVBoxLayout *main_vbox = new QVBoxLayout (plainPage (),6);
    QHBoxLayout *theme_hbox = new QHBoxLayout (main_vbox, 6);
    QHBoxLayout *editor_hbox = new QHBoxLayout (main_vbox, 6);

    m_table_chooser_combo = new KComboBox (plainPage ());
    m_table_chooser_label = new QLabel (chooser_label, plainPage ());
    theme_hbox->addWidget (m_table_chooser_label);
    theme_hbox->addWidget (m_table_chooser_combo);
    theme_hbox->addStretch (20);

    m_table_view = new QListView (plainPage ());
    editor_hbox->addWidget (m_table_view);

    QVBoxLayout *editor_vbox = new QVBoxLayout (editor_hbox, 6);

    // entry1
    m_table_view->addColumn (label1);

    m_label[0] = new QLabel (label1, plainPage ());
    editor_vbox->addWidget (m_label[0]);

    m_line_edit[0] = new KLineEdit (plainPage ());
    m_line_edit[0]->setMaximumWidth (80);
    m_line_edit[0]->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
    editor_vbox->addWidget (m_line_edit[0]);

    // entry2
    m_table_view->addColumn (label2);

    m_label[1] = new QLabel (label2, plainPage ());
    editor_vbox->addWidget (m_label[1]);

    m_line_edit[1] = new KLineEdit (plainPage ());
    m_line_edit[1]->setMaximumWidth (80);
    m_line_edit[1]->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
    editor_vbox->addWidget (m_line_edit[1]);

    // entry3
    if (label3 != QString::null) {
        m_table_view->addColumn (label3);

        m_label[2] = new QLabel (label3, plainPage ());
        editor_vbox->addWidget (m_label[2]);

        m_line_edit[2] = new KLineEdit (plainPage ());
        m_line_edit[2]->setMaximumWidth (80);
        m_line_edit[2]->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
        editor_vbox->addWidget (m_line_edit[2]);
    }

    // entry4
    if (label4 != QString::null) {
        m_table_view->addColumn (label4);

        m_label[3] = new QLabel (label4, plainPage ());
        editor_vbox->addWidget (m_label[3]);

        m_line_edit[3] = new KLineEdit (plainPage ());
        m_line_edit[3]->setMaximumWidth (80);
        m_line_edit[3]->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
        editor_vbox->addWidget (m_line_edit[3]);
    }

    // "Add" button
    m_add_button = new QPushButton (i18n ("Add"), plainPage ());
    m_add_button->setEnabled (false);
    editor_vbox->addWidget (m_add_button);

    // "Remove" button
    m_remove_button = new QPushButton (i18n ("Remove"), plainPage ());
    m_remove_button->setEnabled (false);
    editor_vbox->addWidget (m_remove_button);

    editor_vbox->addStretch (20);
}

ScimAnthyTableEditor::~ScimAnthyTableEditor ()
{
}
