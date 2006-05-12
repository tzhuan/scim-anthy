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
#include <klocale.h>

class QLabel;
class QPushButton;
class QLineEdit;
class QListView;
class KComboBox;

class ScimAnthyTableEditor : public KDialogBase
{
Q_OBJECT
public:
    ScimAnthyTableEditor  (QWidget *parent = 0,
                           const QString & chooser_label = i18n ("Table:"),
                           const QString & label1 = i18n ("Sequence"),
                           const QString & label2 = i18n ("Result"),
                           const QString & label3 = QString::null,
                           const QString & label4 = QString::null,
                           const char *name = 0);
    ~ScimAnthyTableEditor ();

    bool isChanged        () { return m_changed; }
    void setDestructive   (bool destructive);

public slots:
    void tableChooserComboChanged ();
    void setButtonEnabled         ();
    void setCurrentItem           ();
    void addItem                  ();
    void removeItem               ();

public:
    QLabel      *m_table_chooser_label;
    KComboBox   *m_table_chooser_combo;

    QListView   *m_table_view;

    QLabel      *m_label[4];
    QLineEdit   *m_line_edit[4];

    QPushButton *m_add_button;
    QPushButton *m_remove_button;

    bool         m_changed;
};

#endif // SCIM_ANTHY_TABLE_EDITOR
