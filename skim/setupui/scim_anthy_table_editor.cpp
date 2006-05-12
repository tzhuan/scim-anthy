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
                   i18n ("Edit key table"),
                   KDialogBase::Ok | KDialogBase::Cancel),
      m_changed (false)
{
    setMinimumWidth (300);
    setMinimumHeight (250);

    QVBoxLayout *main_vbox   = new QVBoxLayout (plainPage (),6);
    QHBoxLayout *theme_hbox  = new QHBoxLayout (main_vbox, 6);
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

    m_line_edit[0] = new QLineEdit (plainPage ());
    m_line_edit[0]->setMaximumWidth (80);
    m_line_edit[0]->setSizePolicy (QSizePolicy::Minimum,
                                   QSizePolicy::Minimum);
    m_line_edit[0]->setInputMethodEnabled (false);
    editor_vbox->addWidget (m_line_edit[0]);

    // entry2
    m_table_view->addColumn (label2);

    m_label[1] = new QLabel (label2, plainPage ());
    editor_vbox->addWidget (m_label[1]);

    m_line_edit[1] = new QLineEdit (plainPage ());
    m_line_edit[1]->setMaximumWidth (80);
    m_line_edit[1]->setSizePolicy (QSizePolicy::Minimum,
                                   QSizePolicy::Minimum);
    editor_vbox->addWidget (m_line_edit[1]);

    // entry3
    if (label3 != QString::null) {
        m_table_view->addColumn (label3);

        m_label[2] = new QLabel (label3, plainPage ());
        editor_vbox->addWidget (m_label[2]);

        m_line_edit[2] = new QLineEdit (plainPage ());
        m_line_edit[2]->setMaximumWidth (80);
        m_line_edit[2]->setSizePolicy (QSizePolicy::Minimum,
                                       QSizePolicy::Minimum);
        editor_vbox->addWidget (m_line_edit[2]);
    } else {
        m_label[2]     = NULL;
        m_line_edit[2] = NULL;
    }

    // entry4
    if (label4 != QString::null) {
        m_table_view->addColumn (label4);

        m_label[3] = new QLabel (label4, plainPage ());
        editor_vbox->addWidget (m_label[3]);

        m_line_edit[3] = new QLineEdit (plainPage ());
        m_line_edit[3]->setMaximumWidth (80);
        m_line_edit[3]->setSizePolicy (QSizePolicy::Minimum,
                                       QSizePolicy::Minimum);
        editor_vbox->addWidget (m_line_edit[3]);
    } else {
        m_label[3]     = NULL;
        m_line_edit[3] = NULL;
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

    // connect to signals
    connect (m_table_chooser_combo,
             SIGNAL (activated (int)),
             this, SLOT (table_chooser_combo_changed ()));
    connect (m_table_view, SIGNAL (currentChanged (QListViewItem*)),
             this, SLOT (set_button_enabled ()));
    connect (m_table_view, SIGNAL (currentChanged (QListViewItem*)),
             this, SLOT (set_current_item ()));
    connect (m_line_edit[0], SIGNAL (textChanged (const QString &)),
             this, SLOT (set_button_enabled ()));
    connect (m_add_button, SIGNAL (clicked ()),
             this, SLOT (add_item ()));
    connect (m_remove_button, SIGNAL (clicked ()),
             this, SLOT (remove_item ()));
}

ScimAnthyTableEditor::~ScimAnthyTableEditor ()
{
}

void ScimAnthyTableEditor::setDestructive (bool destructive)
{
    if (destructive)
        setWFlags (getWFlags () | Qt::WDestructiveClose);
    else
        setWFlags (getWFlags () & ~Qt::WDestructiveClose);
}

void ScimAnthyTableEditor::tableChooserComboChanged ()
{
    m_changed = true;
}

void ScimAnthyTableEditor::setButtonEnabled ()
{
    QString str = m_line_edit[0]->text ();
    QListViewItem *item = m_table_view->currentItem ();

    m_add_button->setEnabled (m_line_edit[0]->text().isEmpty() ? false : true);
    m_remove_button->setEnabled (item ? true : false);
}

void ScimAnthyTableEditor::setCurrentItem ()
{
    QListViewItem *item = m_table_view->currentItem ();

    if (m_line_edit[0])
        m_line_edit[0]->setText (item ? item->text (0) : "");
    if (m_line_edit[1])
        m_line_edit[1]->setText (item ? item->text (1) : "");
    if (m_line_edit[2])
        m_line_edit[2]->setText (item ? item->text (2) : "");
    if (m_line_edit[3])
        m_line_edit[3]->setText (item ? item->text (3) : "");
}

void ScimAnthyTableEditor::addItem ()
{
    QListViewItem *item = m_table_view->currentItem ();

    if (item && m_line_edit[0] && item->text (0) == m_line_edit[0]->text ()) {
        // The item to replace is this
    } else {
        // Find the item to replace
        QListViewItem *i;
        item = NULL;
        for (i = m_table_view->firstChild (); i; i = i->nextSibling ()) {
            if (i->text (0) == m_line_edit[0]->text ()) {
                item = i;
                break;
            }
        }

        // Add new item
        if (!item)
            item = new QListViewItem (m_table_view, m_line_edit[0]->text ());
    }

    if (item) {
        if (m_line_edit[1])
            item->setText (1, m_line_edit[1]->text ());
        if (m_line_edit[2])
            item->setText (2, m_line_edit[2]->text ());
        if (m_line_edit[3])
            item->setText (1, m_line_edit[3]->text ());

        m_table_chooser_combo->setCurrentItem (1);
        m_changed = true;
    }
}

void ScimAnthyTableEditor::removeItem ()
{
    QListViewItem *item = m_table_view->currentItem ();
    if (item) {
        m_table_view->takeItem (item);
        delete item;

        m_table_chooser_combo->setCurrentItem (1);
        m_changed = true;
    }
}
