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
#include "scim_anthy_table_editor.h"
#include "scimanthysettingplugin.h"

#include "anthy.h"
#include "anthyui.h"

#include <stdlib.h>

#include <qdir.h>
#include <qfile.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlistview.h>

#include <kgenericfactory.h>
#include <klocale.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kdualcolorbutton.h>

#include <skimkeygrabber.h>

using namespace scim_anthy;

typedef KGenericFactory<ScimAnthySettingPlugin> ScimAnthySettingLoaderFactory;

static const char * const __key_bindings_theme = "KeyBindings";
static const char * const __romaji_fund_table  = "RomajiTable/FundamentalTable";
static const char * const __kana_fund_table    = "KanaTable/FundamentalTable";
static const char * const __nicola_fund_table  = "NICOLATable/FundamentalTable";

const String __user_config_dir_name =
    scim_get_home_dir () +
    String (SCIM_PATH_DELIM_STRING ".scim" SCIM_PATH_DELIM_STRING "Anthy");
const String __user_style_dir_name =
    __user_config_dir_name + String (SCIM_PATH_DELIM_STRING "style");
const String __user_style_file_name =
    __user_config_dir_name + String (SCIM_PATH_DELIM_STRING "config.sty");

typedef enum {
    AllKeys          = 0,
    ModeKeys         = 1,
    EditKeys         = 2,
    CaretKeys        = 3,
    SegmentKeys      = 4,
    CandidateKeys    = 5,
    DirectSelectKeys = 6,
    ConvertKeys      = 7,
    DictionaryKeys   = 8,
    SearchByKey      = 9,
} KeyCategory;

typedef struct _KeyEntry
{
    const char  *label;
    const char  *key;
    const char  *desc;
    KeyCategory  category;
} KeyEntry;

KeyEntry key_list[] =
{
    {I18N_NOOP("Toggle on/off"),                "_IMEngine_Anthy_OnOffKey",
     I18N_NOOP("The key events to toggle on/off Japanese mode. "),
     ModeKeys},
    {I18N_NOOP("Circle input mode"),            "_IMEngine_Anthy_CircleInputModeKey",
     I18N_NOOP("The key events to circle input mode. "),
     ModeKeys},
    {I18N_NOOP("Circle kana mode"),             "_IMEngine_Anthy_CircleKanaModeKey",
     I18N_NOOP("The key events to circle kana mode. "),
     ModeKeys},
    {I18N_NOOP("Latin mode"),                   "_IMEngine_Anthy_LatinModeKey",
     I18N_NOOP("The key events to switch input mode to Latin. "),
     ModeKeys},
    {I18N_NOOP("Wide Latin mode"),              "_IMEngine_Anthy_WideLatinModeKey",
     I18N_NOOP("The key events to switch input mode to wide Latin. "),
     ModeKeys},
    {I18N_NOOP("Hiragana mode"),                "_IMEngine_Anthy_HiraganaModeKey",
     I18N_NOOP("The key events to switch input mode to hiragana. "),
     ModeKeys},
    {I18N_NOOP("Katakana mode"),                "_IMEngine_Anthy_KatakanaModeKey",
     I18N_NOOP("The key events to switch input mode to katakana. "),
     ModeKeys},
    {I18N_NOOP("Half katakana mode"),           "_IMEngine_Anthy_HalfKatakanaModeKey",
     I18N_NOOP("The key events to switch input mode to half katakana. "),
     ModeKeys},
    {I18N_NOOP("Circle typing method"),         "_IMEngine_Anthy_CircleTypingMethodKey",
     I18N_NOOP("The key events to circle typing method. "),
     ModeKeys},

    {I18N_NOOP("Insert space"),                 "_IMEngine_Anthy_InsertSpaceKey",
     I18N_NOOP("The key events to insert a space. "),
     EditKeys},
    {I18N_NOOP("Insert alternative space"),     "_IMEngine_Anthy_InsertAltSpaceKey",
     I18N_NOOP("The key events to insert a alternative space. "),
     EditKeys},
    {I18N_NOOP("Insert half space"),            "_IMEngine_Anthy_InsertHalfSpaceKey",
     I18N_NOOP("The key events to insert a half width space. "),
     EditKeys},
    {I18N_NOOP("Insert wide space"),            "_IMEngine_Anthy_InsertWideSpaceKey",
     I18N_NOOP("The key events to insert a wide space. "),
     EditKeys},
    {I18N_NOOP("Backspace"),                    "_IMEngine_Anthy_BackSpaceKey",
     I18N_NOOP("The key events to delete a character before caret. "),
     EditKeys},
    {I18N_NOOP("Delete"),                       "_IMEngine_Anthy_DeleteKey",
     I18N_NOOP("The key events to delete a character after caret. "),
     EditKeys},
    {I18N_NOOP("Commit"),                       "_IMEngine_Anthy_CommitKey",
     I18N_NOOP("The key events to commit the preedit string. "),
     EditKeys},
    {I18N_NOOP("Convert"),                      "_IMEngine_Anthy_ConvertKey",
     I18N_NOOP("The key events to convert the preedit string to kanji. "),
     EditKeys},
    {I18N_NOOP("Predict"),                      "_IMEngine_Anthy_PredictKey",
     I18N_NOOP("The key events to predict a word or sentence from already inserted text. "),
     EditKeys},
    {I18N_NOOP("Cancel"),                       "_IMEngine_Anthy_CancelKey",
     I18N_NOOP("The key events to cancel preediting or converting. "),
     EditKeys},
    {I18N_NOOP("Cancel all"),                   "_IMEngine_Anthy_CancelAllKey",
     I18N_NOOP("The key events to return to initial state. "),
     EditKeys},
    {I18N_NOOP("Reconvert"),                    "_IMEngine_Anthy_ReconvertKey",
     I18N_NOOP("The key events to reconvert the commited string in selection. "),
     EditKeys},
    {I18N_NOOP("Do nothing"),                   "_IMEngine_Anthy_DoNothingKey",
     I18N_NOOP("The key events to eat and do nothing anymore. "
               "For example, it can be used to disable space key completely."),
     EditKeys},

    {I18N_NOOP("Move to first"),                "_IMEngine_Anthy_MoveCaretFirstKey",
     I18N_NOOP("The key events to move the caret to the first of preedit string. "),
     CaretKeys},
    {I18N_NOOP("Move to last"),                 "_IMEngine_Anthy_MoveCaretLastKey",
     I18N_NOOP("The key events to move the caret to the last of the preedit string. "),
     CaretKeys},
    {I18N_NOOP("Move to forward"),              "_IMEngine_Anthy_MoveCaretForwardKey",
     I18N_NOOP("The key events to move the caret to forward. "),
     CaretKeys},
    {I18N_NOOP("Move to backward"),             "_IMEngine_Anthy_MoveCaretBackwardKey",
     I18N_NOOP("The key events to move the caret to backward. "),
     CaretKeys},

    {I18N_NOOP("Select the first segment"),     "_IMEngine_Anthy_SelectFirstSegmentKey",
     I18N_NOOP("The key events to select the first segment. "),
     SegmentKeys},
    {I18N_NOOP("Select the last segment"),      "_IMEngine_Anthy_SelectLastSegmentKey",
     I18N_NOOP("The key events to select the the last segment. "),
     SegmentKeys},
    {I18N_NOOP("Select the next segment"),      "_IMEngine_Anthy_SelectNextSegmentKey",
     I18N_NOOP("The key events to select the next segment. "),
     SegmentKeys},
    {I18N_NOOP("Select the previous segment"),  "_IMEngine_Anthy_SelectPrevSegmentKey",
     I18N_NOOP("The key events to select the previous segment. "),
     SegmentKeys},
    {I18N_NOOP("Shrink the segment"),           "_IMEngine_Anthy_ShrinkSegmentKey",
     I18N_NOOP("The key events to shrink the selected segment. "),
     SegmentKeys},
    {I18N_NOOP("Expand the segment"),           "_IMEngine_Anthy_ExpandSegmentKey",
     I18N_NOOP("The key events to expand the selected segment. "),
     SegmentKeys},
    {I18N_NOOP("Commit the first segment"),     "_IMEngine_Anthy_CommitFirstSegmentKey",
     I18N_NOOP("The key events to commit the first segment. "),
     SegmentKeys},
    {I18N_NOOP("Commit the selected segment"),  "_IMEngine_Anthy_CommitSelectedSegmentKey",
     I18N_NOOP("The key events to commit the selected segment. "),
     SegmentKeys},

    {I18N_NOOP("First candidate"),              "_IMEngine_Anthy_SelectFirstCandidateKey",
     I18N_NOOP("The key events to select the first candidate. "),
     CandidateKeys},
    {I18N_NOOP("Last candidate"),               "_IMEngine_Anthy_SelectLastCandidateKey",
     I18N_NOOP("The key events to the select last candidate. "),
     CandidateKeys},
    {I18N_NOOP("Next candidate"),               "_IMEngine_Anthy_SelectNextCandidateKey",
     I18N_NOOP("The key events to select the next candidate. "),
     CandidateKeys},
    {I18N_NOOP("Previous candidate"),           "_IMEngine_Anthy_SelectPrevCandidateKey",
     I18N_NOOP("The key events to select the previous candidate. "),
     CandidateKeys},
    {I18N_NOOP("Page up"),                      "_IMEngine_Anthy_CandidatesPageUpKey",
     I18N_NOOP("The key events to switch candidates page up. "),
     CandidateKeys},
    {I18N_NOOP("Page down"),                    "_IMEngine_Anthy_CandidatesPageDownKey",
     I18N_NOOP("The key events to switch candidates page down. "),
     CandidateKeys},

    {I18N_NOOP("1st candidate"),                "_IMEngine_Anthy_SelectCandidates1Key",
     I18N_NOOP("The key events to select the 1st candidate. "),
     DirectSelectKeys},
    {I18N_NOOP("2nd candidate"),                "_IMEngine_Anthy_SelectCandidates2Key",
     I18N_NOOP("The key events to select the 2nd candidate. "),
     DirectSelectKeys},
    {I18N_NOOP("3rd candidate"),                "_IMEngine_Anthy_SelectCandidates3Key",
     I18N_NOOP("The key events to select the 3rd candidate. "),
     DirectSelectKeys},
    {I18N_NOOP("4th candidate"),                "_IMEngine_Anthy_SelectCandidates4Key",
     I18N_NOOP("The key events to select the 4th candidate. "),
     DirectSelectKeys},
    {I18N_NOOP("5th candidate"),                "_IMEngine_Anthy_SelectCandidates5Key",
     I18N_NOOP("The key events to select the 5th candidate. "),
     DirectSelectKeys},
    {I18N_NOOP("6th candidate"),                "_IMEngine_Anthy_SelectCandidates6Key",
     I18N_NOOP("The key events to select the 6th candidate. "),
     DirectSelectKeys},
    {I18N_NOOP("7th candidate"),                "_IMEngine_Anthy_SelectCandidates7Key",
     I18N_NOOP("The key events to select the 7th candidate. "),
     DirectSelectKeys},
    {I18N_NOOP("8th candidate"),                "_IMEngine_Anthy_SelectCandidates8Key",
     I18N_NOOP("The key events to select the 8th candidate. "),
     DirectSelectKeys},
    {I18N_NOOP("9th candidate"),                "_IMEngine_Anthy_SelectCandidates9Key",
     I18N_NOOP("The key events to select the 9th candidate. "),
     DirectSelectKeys},
    {I18N_NOOP("10th candidate"),               "_IMEngine_Anthy_SelectCandidates10Key",
     I18N_NOOP("The key events to select the 10th candidate. "),
     DirectSelectKeys},

    {I18N_NOOP("Convert character type to forward"),  "_IMEngine_Anthy_ConvertCharTypeForwardKey",
     I18N_NOOP("Rotate character type forward."),
     ConvertKeys},

    {I18N_NOOP("Convert character type to backward"), "_IMEngine_Anthy_ConvertCharTypeBackwardKey",
     I18N_NOOP("Rotate character type backward."),
     ConvertKeys},

    {I18N_NOOP("Convert to hiragana"),          "_IMEngine_Anthy_ConvertToHiraganaKey",
     I18N_NOOP("The key events to convert the preedit string to hiragana. "),
     ConvertKeys},

    {I18N_NOOP("Convert to katakana"),          "_IMEngine_Anthy_ConvertToKatakanaKey",
     I18N_NOOP("The key events to convert the preedit string to katakana. "),
     ConvertKeys},

    {I18N_NOOP("Convert to half width"),        "_IMEngine_Anthy_ConvertToHalfKey",
     I18N_NOOP("The key events to convert the preedit string to half width. "),
     ConvertKeys},

    {I18N_NOOP("Convert to half katakana"),     "_IMEngine_Anthy_ConvertToHalfKatakanaKey",
     I18N_NOOP("The key events to convert the preedit string to half width katakana. "),
     ConvertKeys},

    {I18N_NOOP("Convert to wide latin"),        "_IMEngine_Anthy_ConvertToWideLatinKey",
     I18N_NOOP("The key events to convert the preedit string to wide latin. "),
     ConvertKeys},

    {I18N_NOOP("Convert to latin"),             "_IMEngine_Anthy_ConvertToLatinKey",
     I18N_NOOP("The key events to convert the preedit string to latin. "),
     ConvertKeys},

    {I18N_NOOP("Edit dictionary"),              "_IMEngine_Anthy_DictAdminKey",
     I18N_NOOP("The key events to launch dictionary administration tool. "),
     DictionaryKeys},

    {I18N_NOOP("Add a word"),                   "_IMEngine_Anthy_AddWordKey",
     I18N_NOOP("The key events to launch the tool to add a word. "),
     DictionaryKeys},

    {NULL, NULL, NULL, AllKeys},
};


K_EXPORT_COMPONENT_FACTORY (kcm_skimplugin_scim_anthy, 
                            ScimAnthySettingLoaderFactory ("kcm_skimplugin_scim_anthy"))


inline bool color_enabled (int n)
{
    return n > 3 && n < 7 ? true : false;
}


class ScimAnthyKeyListViewItem : public QListViewItem
{
public:
    ScimAnthyKeyListViewItem (QListView     *view,
                              QListViewItem *sibling,
                              const QString &feature,
                              const QString &defval,
                              const QString &description,
                              KeyEntry *key_entry,
                              KConfigSkeletonGenericItem<QString> *kconf,
                              KeyCategory category)
        : QListViewItem (view, sibling, feature, defval, description),
          m_key_entry (key_entry),
          m_kconf (kconf),
          m_category (category)
    {
    }
    ~ScimAnthyKeyListViewItem ()
    {
    }

    virtual void setVisibleByCategory (int category, const QString &filter = QString::null)
    {
        bool visible = true;

        if (category != (int) AllKeys &&
            category != (int) SearchByKey &&
            category != (int) m_category)
        {
            visible = false;
        }

        if (category == (int) SearchByKey &&
            keyFilter (m_kconf->value (), filter))
        {
            visible = false;
        }

        setVisible (visible);
    }

    void setDefault ()
    {
        m_kconf->swapDefault ();
        setText (1, m_kconf->value ());
        m_kconf->swapDefault ();
    }

    bool isChanged ()
    {
        if (text (1) != m_kconf->value ())
            return true;
        return false;
    }

    void save ()
    {
        if (m_kconf) {
            m_kconf->setValue (text (1));
            m_kconf->writeConfig (AnthyConfig::self()->config());
        }
    }

    QString kconfKey ()
    {
        return m_kconf->key ();
    }

private:
    bool keyFilter (const QString & keys, const QString & filter)
    {
        if (filter.isEmpty ())
            return false;

        QStringList filter_list = QStringList::split (",", filter);
        QStringList key_list = QStringList::split (",", keys);
        QStringList::iterator it;

        for (it = filter_list.begin (); it != filter_list.end (); it++) {
            if (key_list.find (*it) == key_list.end ())
                return true;
        }

        return false;
    }

public:
    ScimAnthySettingPlugin              *m_plugin;
    KeyEntry                            *m_key_entry;
    KConfigSkeletonGenericItem<QString> *m_kconf;
    KeyCategory                          m_category;
};

class ScimAnthySettingPlugin::ScimAnthySettingPluginPrivate {
public:
    AnthySettingUI * ui;

    StyleFiles m_style_list;
    StyleFile  m_user_style;
    bool       m_style_changed;

    ScimAnthyTableEditor *m_table_editor;

public:
    ScimAnthySettingPluginPrivate ()
        : m_style_changed (false),
          m_table_editor (NULL)
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

    KConfigSkeletonGenericItem<QString> *string_config_item (const QString &item_key)
    {
        KConfigSkeletonItem *tmp_item;
        tmp_item = AnthyConfig::self()->findItem(item_key);
        if (!tmp_item)
            return NULL;

        KConfigSkeletonGenericItem<QString> *item;
        item = dynamic_cast<KConfigSkeletonGenericItem<QString>*> (tmp_item);
        return item;
    }

    void save_theme (const QString & item_key,
                     const QString & item_value,
                     const QString & section_name)
    {
        KConfigSkeletonGenericItem<QString> *item = string_config_item (item_key);
        if (!item) return;

        item->setValue (theme2file (item_value, section_name));
        item->writeConfig (AnthyConfig::self()->config());
    }

    void save_color (const QString key, const QColor & c)
    {
        KConfigSkeletonItem *tmp_item;
        tmp_item = AnthyConfig::self()->findItem(key);
        if (!tmp_item) return;

        KConfigSkeletonGenericItem<QString> *item;
        item = dynamic_cast<KConfigSkeletonGenericItem<QString>*> (tmp_item);
        if (!item) return;

        item->setValue (c.name ());
        item->writeConfig (AnthyConfig::self()->config());
    }

    void reset_custom_widgets ()
    {
        // set key bindings theme list
        setup_combo_box (ui->KeyBindingsThemeComboBox,
                         __key_bindings_theme,
                         AnthyConfig::_IMEngine_Anthy_KeyThemeFile());
        KConfigSkeletonGenericItem<QString> *item;
        item = string_config_item ("_IMEngine_Anthy_KeyTheme");
        if (item->value () == "User defined")
            ui->KeyBindingsThemeComboBox->setCurrentItem (1);

        // set key bindings
        setup_key_bindings ();

        // set romaji table list
        setup_combo_box (ui->RomajiComboBox,
                         __romaji_fund_table,
                         AnthyConfig::_IMEngine_Anthy_RomajiThemeFile());

        // set kana layout list
        setup_combo_box (ui->KanaComboBox,
                         __kana_fund_table,
                         AnthyConfig::_IMEngine_Anthy_KanaLayoutFile());

        // set NICOLA layout list
        setup_combo_box (ui->ThumbShiftComboBox,
                         __nicola_fund_table,
                         AnthyConfig::_IMEngine_Anthy_NICOLALayoutFile());

        // set preedit string color
        ui->PreeditStringDualColorButton->setForeground (
            QColor (AnthyConfig::_IMEngine_Anthy_PreeditFGColor()));
        ui->PreeditStringDualColorButton->setBackground (
            QColor (AnthyConfig::_IMEngine_Anthy_PreeditBGColor()));

        // set conversion string color
        ui->ConversionStringDualColorButton->setForeground (
            QColor (AnthyConfig::_IMEngine_Anthy_ConversionFGColor()));
        ui->ConversionStringDualColorButton->setBackground (
            QColor (AnthyConfig::_IMEngine_Anthy_ConversionBGColor()));

        // set selected segment color
        ui->SelectedSegmentDualColorButton->setForeground (
            QColor (AnthyConfig::_IMEngine_Anthy_SelectedSegmentFGColor()));
        ui->SelectedSegmentDualColorButton->setBackground (
            QColor (AnthyConfig::_IMEngine_Anthy_SelectedSegmentBGColor()));

        // set sensitivity of dual color buttons
        int n = ui->kcfg__IMEngine_Anthy_PreeditStyle->currentItem ();
        ui->PreeditStringDualColorButton->setEnabled (color_enabled (n));
        n = ui->kcfg__IMEngine_Anthy_ConversionStyle->currentItem ();
        ui->ConversionStringDualColorButton->setEnabled (color_enabled (n));
        n = ui->kcfg__IMEngine_Anthy_SelectedSegmentStyle->currentItem ();
        ui->SelectedSegmentDualColorButton->setEnabled (color_enabled (n));
    }

    QString theme2file (const QString theme,
                        const char *section_name)
    {
        if (theme == i18n ("Default"))
            return QString ("");
        if (theme == i18n ("User defined"))
            return QString::fromUtf8 (__user_style_file_name.c_str ());

        StyleFiles::iterator it;

        for (it = m_style_list.begin(); it != m_style_list.end (); it++) {
            StyleLines section;
            if (!it->get_entry_list (section, section_name))
                continue;
            if (QString::fromUtf8 (it->get_title().c_str()) == theme)
                return QString::fromUtf8 (it->get_file_name().c_str ());
        }

        return QString ("");
    }

    void setup_combo_box (KComboBox  *combo,
                          const char *section_name,
                          QString     default_file)
    {
        QStringList theme_list;
        theme_list.append (i18n ("Default"));
        theme_list.append (i18n ("User defined"));

        StyleFiles::iterator it;
        QString cur_item = i18n ("Default");

        if (default_file == __user_style_file_name)
            cur_item = i18n ("User defined");

        for (it = m_style_list.begin(); it != m_style_list.end (); it++) {
            StyleLines section;
            if (!it->get_entry_list (section, section_name))
                continue;
            theme_list.append (QString::fromUtf8 (it->get_title().c_str()));
            if (default_file == QString::fromUtf8 (it->get_file_name().c_str()))
                cur_item = theme_list.back ();
        }

        combo->clear ();
        combo->insertStringList (theme_list);
        combo->setCurrentText (cur_item);
    }

    void setup_key_bindings ()
    {
        ui->KeyBindingsView->clear ();
        ui->KeyBindingsView->setSorting (-1);
        ui->KeyBindingsSelectButton->setEnabled (false);

        QListViewItem *prev_item = NULL;

        for (unsigned int i = 0; key_list[i].key; i++) {
            KConfigSkeletonGenericItem<QString> *item;
            item = string_config_item (key_list[i].key);
            if (!item) return;

            ScimAnthyKeyListViewItem *list_item;
            list_item = new ScimAnthyKeyListViewItem (ui->KeyBindingsView,
                                                      prev_item,
                                                      i18n (key_list[i].label),
                                                      item->value (),
                                                      //item->whatsThis (),
                                                      i18n (key_list[i].desc),
                                                      &key_list[i],
                                                      item,
                                                      key_list[i].category);
            prev_item = list_item;
        }
    }

    void setup_table_view (QListView *view,
                           ConvRule *table, NicolaRule *nicola_table,
                           const QString & default_theme,
                           const QString & section_name)
    {
        view->clear ();
        view->setSorting (-1);

        // Find current style file
        StyleFile *style = NULL;
        if (default_theme == i18n ("User defined")) {
            style = &m_user_style;
        } else {
            StyleFiles::iterator it;
            for (it = m_style_list.begin (); it != m_style_list.end (); it++) {
                StyleLines section;
                if (!it->get_entry_list (section, section_name))
                    continue;
                if (default_theme == QString::fromUtf8 (it->get_title().c_str())) {
                    style = &(*it);
                    break;
                }
            }
        }

        QListViewItem *item = NULL;

        if (style) {
            // set the table which is defined in the current style file
            std::vector<String> keys;
            style->get_key_list (keys, section_name);
            std::vector<String>::iterator kit;
            for (kit = keys.begin (); kit != keys.end (); kit++) {
                std::vector<String> value;
                style->get_string_array (value, section_name, *kit);

                QString v[3];
                if (value.size() > 0)
                    v[0] = QString::fromUtf8 (value[0].c_str ());
                if (value.size() > 1)
                    v[1] = QString::fromUtf8 (value[1].c_str ());
                if (value.size() > 2)
                    v[2] = QString::fromUtf8 (value[2].c_str ());

                if (nicola_table) {
                    item = new QListViewItem (view, item,
                                              QString::fromUtf8 (kit->c_str ()),
                                              v[0], v[1], v[2]);
                } else if (table) {
                    QString result = !v[0].isEmpty () ? v[0] : v[1];
                    item = new QListViewItem (view, item,
                                              QString::fromUtf8 (kit->c_str ()),
                                              result);
                }
            }

        } else if (nicola_table) {
            // default NICOLA table
            for (unsigned int i = 0; nicola_table[i].key; i++) {
                item = new QListViewItem (
                    view, item,
                    QString::fromUtf8 (nicola_table[i].key),
                    QString::fromUtf8 (nicola_table[i].single),
                    QString::fromUtf8 (nicola_table[i].left_shift),
                    QString::fromUtf8 (nicola_table[i].right_shift));
            }

        } else if (table) {
            // default Kana table
            for (unsigned int i = 0; table[i].string; i++) {
                QString result = table[i].result && *table[i].result ?
                    table[i].result : table[i].cont;
                item = new QListViewItem (
                    view, item,
                    QString::fromUtf8 (table[i].string),
                    QString::fromUtf8 (result));
            }
        }
    }

    void set_key_category (int category)
    {
        QListViewItemIterator it (ui->KeyBindingsView);
        while (it.current ()) {
            ScimAnthyKeyListViewItem *item;
            item = dynamic_cast <ScimAnthyKeyListViewItem *> (it.current ());
            if (!item) continue;
            item->setVisibleByCategory (category,
                                        ui->KeyBindingsFilterLineEdit->text ());
            it++;
        }
    }

    bool key_filter (const QString & keys, const QString & filter)
    {
        if (filter.isEmpty ())
            return false;

        QStringList filter_list = QStringList::split (",", filter);
        QStringList key_list = QStringList::split (",", keys);
        QStringList::iterator it;

        for (it = filter_list.begin (); it != filter_list.end (); it++) {
            if (key_list.find (*it) == key_list.end ())
                return true;
        }

        return false;
    }

    bool theme_is_changed (KComboBox *combo,
                           const QString &item_key,
                           const QString &section_name)
    {
        QString cur_item;
        KConfigSkeletonGenericItem<QString> *item;

        cur_item = combo->currentText ();
        item = string_config_item (item_key);

        if ((cur_item == i18n ("Default"))) {
            if (item->value () != "")
                return true;
            else
                return false;
        }
        else if ((cur_item == i18n ("User defined")))
        {
            if (item->value () != __user_style_file_name)
                return true;
            else
                return false;
        } else {
            // FIXME! shoudn't match by title name.
            StyleFiles::iterator it;
            for (it = m_style_list.begin (); it != m_style_list.end (); it++) {
                StyleLines section;
                if (!it->get_entry_list (section, section_name))
                    continue;
                if (combo->currentText () == QString::fromUtf8 (it->get_title().c_str()) &&
                    item->value () == QString::fromUtf8 (it->get_file_name().c_str()))
                    return false;
            }
            return true;
        }
    }

    bool key_theme_is_changed ()
    {
        KConfigSkeletonGenericItem<QString> *key_theme_item, *key_theme_file_item;
        key_theme_item      = string_config_item ("_IMEngine_Anthy_KeyTheme");
        key_theme_file_item = string_config_item ("_IMEngine_Anthy_KeyThemeFile");
        int current = ui->KeyBindingsThemeComboBox->currentItem ();
        QString theme_name = ui->KeyBindingsThemeComboBox->currentText ();

        if (current == 0) {
            if (key_theme_item->value () == "Default")
                return false;
            else
                return true;
        } else if (current == 1) {
            if (key_theme_item->value () == "User defined")
                return false;
            else
                return true;
        } else {
            if (key_theme_item->value () == theme_name &&
                key_theme_file_item->value () == theme2file (theme_name, __key_bindings_theme))
                return false;
            else
                return true;
        }

        return false;
    }

    bool is_changed ()
    {
        // key theme
        if (key_theme_is_changed ())
            return true;

        // key bindings
        QListViewItemIterator it (ui->KeyBindingsView);
        while (it.current ()) {
            ScimAnthyKeyListViewItem *item;
            item = dynamic_cast <ScimAnthyKeyListViewItem *> (it.current ());
            if (!item) continue;
            if (item->isChanged ())
                return true;
            it++;
        }

        // layout table
        if (theme_is_changed (ui->RomajiComboBox,
                              "_IMEngine_Anthy_RomajiThemeFile",
                              __romaji_fund_table))
            return true;
        if (theme_is_changed (ui->KanaComboBox,
                              "_IMEngine_Anthy_KanaLayoutFile",
                              __kana_fund_table))
            return true;
        if (theme_is_changed (ui->ThumbShiftComboBox,
                              "_IMEngine_Anthy_NICOLALayoutFile",
                              __nicola_fund_table))
            return true;

        // double color
        if (ui->PreeditStringDualColorButton->foreground () !=
            QColor (AnthyConfig::_IMEngine_Anthy_PreeditFGColor ()))
            return true;
        if (ui->PreeditStringDualColorButton->background () !=
            QColor (AnthyConfig::_IMEngine_Anthy_PreeditBGColor ()))
            return true;
        if (ui->ConversionStringDualColorButton->foreground () !=
            QColor (AnthyConfig::_IMEngine_Anthy_ConversionFGColor ()))
            return true;
        if (ui->ConversionStringDualColorButton->background () !=
            QColor (AnthyConfig::_IMEngine_Anthy_ConversionBGColor ()))
            return true;
        if (ui->SelectedSegmentDualColorButton->foreground () !=
            QColor (AnthyConfig::_IMEngine_Anthy_SelectedSegmentFGColor ()))
            return true;
        if (ui->SelectedSegmentDualColorButton->background () !=
            QColor (AnthyConfig::_IMEngine_Anthy_SelectedSegmentBGColor ()))
            return true;

        return m_style_changed;
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
      d (new ScimAnthySettingPluginPrivate),
      m_name (name ? name : "")

{
    KGlobal::locale()->insertCatalogue ("skim-scim-anthy");

    // Load scim-anthy style files.
    d->load_style_files ();

    // Setup user interface.
    d->ui = new AnthySettingUI (this);
    setMainWidget (d->ui);

    // Set default values to our custom widgets
    d->reset_custom_widgets ();

    // Connect to signals
    // Launch Buttons
    connect (d->ui->LaunchDictAdminCommandButton,
             SIGNAL (clicked ()),
             this, SLOT (launch_dict_admin_command ()));
    connect (d->ui->LaunchAddWordCommandButton,
             SIGNAL (clicked ()),
             this, SLOT (launch_add_word_command ()));

    // Line Edit
    connect (d->ui->KeyBindingsFilterLineEdit,
             SIGNAL (textChanged (const QString &)),
             this, SLOT (set_key_bindings_group ()));

    // Combo Boxes
    connect (d->ui->KeyBindingsGroupComboBox,
             SIGNAL (activated (int)),
             this, SLOT (set_key_bindings_group ()));
    connect (d->ui->KeyBindingsThemeComboBox,
             SIGNAL (activated (int)),
             this, SLOT (set_key_bindings_theme (int)));
    connect (d->ui->RomajiComboBox,
             SIGNAL (activated (const QString &)),
             this, SLOT (slotWidgetModified ()));
    connect (d->ui->KanaComboBox,
             SIGNAL (activated (const QString &)),
             this, SLOT (slotWidgetModified ()));
    connect (d->ui->ThumbShiftComboBox,
             SIGNAL (activated (const QString &)),
             this, SLOT (slotWidgetModified ()));
    connect (d->ui->kcfg__IMEngine_Anthy_PreeditStyle,
             SIGNAL (activated (int)),
             this, SLOT (preedit_string_style_changed (int)));
    connect (d->ui->kcfg__IMEngine_Anthy_ConversionStyle,
             SIGNAL (activated (int)),
             this, SLOT (conversion_string_style_changed (int)));
    connect (d->ui->kcfg__IMEngine_Anthy_SelectedSegmentStyle,
             SIGNAL (activated (int)),
             this, SLOT (selected_segment_style_changed (int)));

    // Push Buttons
    connect (d->ui->KeyBindingsSelectButton,
             SIGNAL (clicked ()),
             this, SLOT (choose_keys ()));
    connect (d->ui->RomajiCustomizeButton,
             SIGNAL (clicked ()),
             this, SLOT (customize_romaji_table ()));
    connect (d->ui->KanaCustomizeButton,
             SIGNAL (clicked ()),
             this, SLOT (customize_kana_table ()));
    connect (d->ui->ThumbShiftCustomizeButton,
             SIGNAL (clicked ()),
             this, SLOT (customize_nicola_table ()));

    // Key Bindings View
    connect (d->ui->KeyBindingsView,
             SIGNAL (currentChanged (QListViewItem*)),
             this, SLOT (key_bindings_view_selection_changed (QListViewItem*)));
    connect (d->ui->KeyBindingsView,
             SIGNAL (selectionChanged (QListViewItem*)),
             this, SLOT (key_bindings_view_selection_changed (QListViewItem*)));
    connect (d->ui->KeyBindingsView,
             SIGNAL (doubleClicked (QListViewItem*)),
             this, SLOT (choose_keys ()));

    // Preedit String Color
    connect (d->ui->PreeditStringDualColorButton,
             SIGNAL (fgChanged(const QColor &)),
             this, SLOT (slotWidgetModified()));
    connect (d->ui->PreeditStringDualColorButton,
             SIGNAL (bgChanged(const QColor &)),
             this, SLOT (slotWidgetModified()));

    // Conversion String Color
    connect (d->ui->ConversionStringDualColorButton,
             SIGNAL (fgChanged(const QColor &)),
             this, SLOT (slotWidgetModified()));
    connect (d->ui->ConversionStringDualColorButton,
             SIGNAL (bgChanged(const QColor &)),
             this, SLOT (slotWidgetModified()));

    // Selected Segment Color
    connect (d->ui->SelectedSegmentDualColorButton,
             SIGNAL (fgChanged(const QColor &)),
             this, SLOT (slotWidgetModified()));
    connect (d->ui->SelectedSegmentDualColorButton,
             SIGNAL (bgChanged(const QColor &)),
             this, SLOT (slotWidgetModified()));
}

ScimAnthySettingPlugin::~ScimAnthySettingPlugin () 
{
    KGlobal::locale()->removeCatalogue ("skim-scim-anthy");
    delete d;
}

void ScimAnthySettingPlugin::load ()
{
    KAutoCModule::load ();

    d->load_style_files ();
    d->reset_custom_widgets ();
}

void ScimAnthySettingPlugin::save ()
{
    KAutoCModule::save ();

    // key theme name and key theme file
    KConfigSkeletonGenericItem<QString> *key_theme_item, *key_theme_file_item;
    key_theme_item = d->string_config_item ("_IMEngine_Anthy_KeyTheme");
    key_theme_file_item = d->string_config_item ("_IMEngine_Anthy_KeyThemeFile");
    int current_key_theme = d->ui->KeyBindingsThemeComboBox->currentItem ();
    if (current_key_theme == 0) {
        if (key_theme_item)
            key_theme_item->setValue ("Default");
        if (key_theme_file_item)
            key_theme_file_item->setValue ("");
    } else if (current_key_theme == 1) {
        if (key_theme_item)
            key_theme_item->setValue ("User defined");
        if (key_theme_file_item)
            key_theme_file_item->setValue ("");
    } else {
        QString theme_name = d->ui->KeyBindingsThemeComboBox->currentText ();
        if (key_theme_item)
            key_theme_item->setValue (theme_name);
        if (key_theme_file_item)
            key_theme_file_item->setValue
                (d->theme2file (theme_name, __key_bindings_theme));
    }
    key_theme_item->writeConfig (AnthyConfig::self()->config());
    key_theme_file_item->writeConfig (AnthyConfig::self()->config());

    // key bindings
    QListViewItemIterator it (d->ui->KeyBindingsView);
    while (it.current ()) {
        ScimAnthyKeyListViewItem *item;
        item = dynamic_cast <ScimAnthyKeyListViewItem *> (it.current ());
        if (!item) continue;
        item->save ();
        it++;
    }

    // layout table file
    d->save_theme ("_IMEngine_Anthy_RomajiThemeFile",
                   d->ui->RomajiComboBox->currentText (),
                   __romaji_fund_table);
    d->save_theme ("_IMEngine_Anthy_KanaLayoutFile",
                   d->ui->KanaComboBox->currentText (),
                   __kana_fund_table);
    d->save_theme ("_IMEngine_Anthy_NICOLALayoutFile",
                   d->ui->ThumbShiftComboBox->currentText (),
                   __nicola_fund_table);

    // color
    d->save_color ("_IMEngine_Anthy_PreeditFGColor",
                   d->ui->PreeditStringDualColorButton->foreground ());
    d->save_color ("_IMEngine_Anthy_PreeditBGColor",
                   d->ui->PreeditStringDualColorButton->background ());
    d->save_color ("_IMEngine_Anthy_ConversionFGColor",
                   d->ui->ConversionStringDualColorButton->foreground ());
    d->save_color ("_IMEngine_Anthy_ConversionBGColor",
                   d->ui->ConversionStringDualColorButton->background ());
    d->save_color ("_IMEngine_Anthy_SelectedSegmentFGColor",
                   d->ui->SelectedSegmentDualColorButton->foreground ());
    d->save_color ("_IMEngine_Anthy_SelectedSegmentBGColor",
                   d->ui->SelectedSegmentDualColorButton->background ());

    // style file
    d->save_style_files ();
}

void ScimAnthySettingPlugin::defaults ()
{
    KAutoCModule::defaults ();

    set_key_bindings_theme (0);

    d->ui->RomajiComboBox->setCurrentItem (0);
    d->ui->KanaComboBox->setCurrentItem (0);
    d->ui->ThumbShiftComboBox->setCurrentItem (0);

    d->reset_custom_widgets ();
}

void ScimAnthySettingPlugin::slotWidgetModified ()
{
    if (d->is_changed ()) {
        emit changed (true);
    } else {
        KAutoCModule::slotWidgetModified ();
    }
}

void ScimAnthySettingPlugin::launch_dict_admin_command ()
{
    QString command;
    command = d->ui->kcfg__IMEngine_Anthy_DictAdminCommand->text().ascii ()
        + QString (" &");
    system (command.ascii ());
}

void ScimAnthySettingPlugin::launch_add_word_command ()
{
    QString command;
    command = d->ui->kcfg__IMEngine_Anthy_AddWordCommand->text().ascii ()
        + QString (" &");
    system (command.ascii ());
}

void ScimAnthySettingPlugin::set_key_bindings_group ()
{
    int n = d->ui->KeyBindingsGroupComboBox->currentItem ();
    bool enabled = n == (int) SearchByKey ? true : false;
    d->ui->KeyBindingsFilterLineEdit->setEnabled (enabled);
    d->ui->KeyBindingsFilterSelectButton->setEnabled (enabled);
    d->set_key_category (n);
}

void ScimAnthySettingPlugin::set_key_bindings_theme (int n)
{
    d->ui->KeyBindingsThemeComboBox->setCurrentItem (n);

    QString theme_name = d->ui->KeyBindingsThemeComboBox->currentText ();
    int current = d->ui->KeyBindingsThemeComboBox->currentItem ();
    StyleFiles::iterator it;
    std::vector<String> keys;
    std::vector<String>::iterator kit;

    if (current == 0) {
        // Handle "Default" theme.
        QListViewItemIterator it (d->ui->KeyBindingsView);
        while (it.current ()) {
            ScimAnthyKeyListViewItem *item;
            item = dynamic_cast <ScimAnthyKeyListViewItem *> (it.current ());
            if (!item) continue;
            item->setDefault ();
            it++;
        }

    } else if (current == 1) {
        // Handle "User defined" theme.

        // Nothing to do.

    } else {
        // Handle other themes

        // Find theme file
        for (it = d->m_style_list.begin (); it != d->m_style_list.end (); it++) {
            StyleLines section;
            if (!it->get_entry_list (section, __key_bindings_theme))
                continue;
            if (theme_name == QString::fromUtf8 (it->get_title().c_str ()))
                break;
        }
        if (it == d->m_style_list.end ())
            goto SET_WIDGET;

        // Set new bindings to QListViewItem
        it->get_key_list (keys, __key_bindings_theme);

        QListViewItemIterator lit (d->ui->KeyBindingsView);
        while (lit.current ()) {
            ScimAnthyKeyListViewItem *view_item;
            view_item = dynamic_cast <ScimAnthyKeyListViewItem *> (lit.current ());
            if (!view_item) continue;

            view_item->setText (1, "");

            for (kit = keys.begin (); kit != keys.end (); kit++) {
                QString key = QString ("/IMEngine/Anthy/")
                    + QString (kit->c_str ());
                if (view_item->kconfKey () == key) {
                    String v;
                    it->get_string (v, __key_bindings_theme, *kit);
                    view_item->setText (1, v);
                    break;
                }
            }

            lit++;
        }
    }

SET_WIDGET:
    slotWidgetModified ();
}

void ScimAnthySettingPlugin::choose_keys ()
{
    QListViewItem *item = d->ui->KeyBindingsView->currentItem ();
    if (!item) return;

    QStringList keys = QStringList::split (",", item->text (1));
    SkimShortcutListEditor editor (d->ui);
    editor.setStringList (keys);
    if (editor.exec () == QDialog::Accepted) {
        item->setText (1, editor.getCombinedString ());
        d->ui->KeyBindingsThemeComboBox->setCurrentItem (1);
        slotWidgetModified ();
    }
}

void ScimAnthySettingPlugin::customize_romaji_table ()
{
    ScimAnthyTableEditor editor (d->ui, i18n ("Romaji Table:"),
                                 i18n ("Sequence"), i18n ("Result"));
    editor.setCaption (i18n ("Edit romajit table"));
    editor.setModal (true);
    d->m_table_editor = &editor;

    d->setup_combo_box (editor.m_table_chooser_combo,
                        __romaji_fund_table,
                        d->theme2file (d->ui->RomajiComboBox->currentText(),
                                       __romaji_fund_table));
    set_romaji_table_view ();
    connect (editor.m_table_chooser_combo,
             SIGNAL (activated (int)),
             this, SLOT (set_romaji_table_view ()));


    if (editor.exec () != QDialog::Accepted || !editor.changed ())
        return;

    int n = editor.m_table_chooser_combo->currentItem ();
    d->ui->RomajiComboBox->setCurrentItem (n);
    if (n == 1) {
        d->m_user_style.delete_section (__romaji_fund_table);

        QListViewItem *i;
        for (i = editor.m_table_view->firstChild (); i; i = i->nextSibling ()) {
            String seq = i->text(0).isNull () ?
                String ("") : String (i->text(0).utf8 ());
            String res = i->text(1).isNull () ?
                String ("") : String (i->text(1).utf8 ());
            d->m_user_style.set_string (__romaji_fund_table, seq, res);
        }

        d->m_style_changed = true;
    }

    slotWidgetModified ();

    d->m_table_editor = NULL;
}

static bool
has_voiced_consonant (String str)
{
    ConvRule *table = scim_anthy_kana_voiced_consonant_rule;

    WideString str1_wide = utf8_mbstowcs (str);
    if (str1_wide.length () <= 0)
        return false;

    for (unsigned int i = 0; table[i].string; i++) {
        WideString str2_wide = utf8_mbstowcs (table[i].string);
        if (str2_wide.length () <= 0)
            continue;
        if (str1_wide[0] == str2_wide[0])
            return true;
    }

    return false;
}

void ScimAnthySettingPlugin::customize_kana_table ()
{
    ScimAnthyTableEditor editor (d->ui, i18n ("Layout:"), i18n ("Key"), i18n ("Result"));
    editor.setCaption (i18n ("Edit kana layout table")); 
    editor.setModal (true);
    d->m_table_editor = &editor;

    d->setup_combo_box (editor.m_table_chooser_combo,
                        __kana_fund_table,
                        d->theme2file (d->ui->KanaComboBox->currentText(),
                                       __kana_fund_table));
    set_kana_table_view ();
    connect (editor.m_table_chooser_combo,
             SIGNAL (activated (int)),
             this, SLOT (set_kana_table_view ()));

    if (editor.exec () != QDialog::Accepted || !editor.changed ())
        return;

    int n = editor.m_table_chooser_combo->currentItem ();
    d->ui->KanaComboBox->setCurrentItem (n);
    if (n == 1) {
        d->m_user_style.delete_section (__kana_fund_table);

        QListViewItem *i;
        for (i = editor.m_table_view->firstChild (); i; i = i->nextSibling ()) {
            String seq = i->text(0).isNull () ?
                String ("") : String (i->text(0).utf8 ());
            std::vector<String> value;
            if (has_voiced_consonant (String (i->text(1).utf8 ())))
                value.push_back (String (""));
            value.push_back (i->text(1).isNull () ?
                             String ("") : String (i->text(1).utf8 ()));
            d->m_user_style.set_string_array (__kana_fund_table, seq, value);
        }

        d->m_style_changed = true;
    }

    slotWidgetModified ();

    d->m_table_editor = NULL;
}

void ScimAnthySettingPlugin::customize_nicola_table ()
{
    ScimAnthyTableEditor editor (d->ui,
                                 i18n ("Layout:"),
                                 i18n ("Key"),
                                 i18n ("Single press"),
                                 i18n ("Left thumb shift"),
                                 i18n ("Right thumb shift"));
    editor.setCaption (i18n ("Edit thumb shift layout table"));
    editor.setModal (true);
    d->m_table_editor = &editor;

    d->setup_combo_box (editor.m_table_chooser_combo,
                        __nicola_fund_table,
                        d->theme2file (d->ui->ThumbShiftComboBox->currentText(),
                                       __nicola_fund_table));
    set_thumb_shift_table_view ();
    connect (editor.m_table_chooser_combo,
             SIGNAL (activated (int)),
             this, SLOT (set_thumb_shift_table_view ()));
    editor.show ();

    if (editor.exec () != QDialog::Accepted || !editor.changed ())
        return;

    int n = editor.m_table_chooser_combo->currentItem ();
    d->ui->ThumbShiftComboBox->setCurrentItem (n);
    if (n == 1) {
        d->m_user_style.delete_section (__nicola_fund_table);

        QListViewItem *i;
        for (i = editor.m_table_view->firstChild (); i; i = i->nextSibling ()) {
            String seq = i->text(0).isNull () ?
                String ("") : String (i->text(0).utf8 ());
            std::vector<String> value;
            value.push_back (i->text(1).isNull () ?
                             String ("") : String (i->text(1).utf8 ()));
            value.push_back (i->text(2).isNull () ?
                             String ("") : String (i->text(2).utf8 ()));
            value.push_back (i->text(3).isNull () ?
                             String ("") : String (i->text(3).utf8 ()));
            d->m_user_style.set_string_array (__nicola_fund_table, seq, value);
        }

        d->m_style_changed = true;
    }

    slotWidgetModified ();

    d->m_table_editor = NULL;
}

void ScimAnthySettingPlugin::key_bindings_view_selection_changed (QListViewItem *item)
{
    d->ui->KeyBindingsSelectButton->setEnabled (item ? true : false);
}

void ScimAnthySettingPlugin::preedit_string_style_changed (int n)
{
    d->ui->PreeditStringDualColorButton->setEnabled (color_enabled (n));
}

void ScimAnthySettingPlugin::conversion_string_style_changed (int n)
{
    d->ui->ConversionStringDualColorButton->setEnabled (color_enabled (n));
}

void ScimAnthySettingPlugin::selected_segment_style_changed (int n)
{
    d->ui->SelectedSegmentDualColorButton->setEnabled (color_enabled (n));
}
void ScimAnthySettingPlugin::set_romaji_table_view ()
{
    d->setup_table_view (d->m_table_editor->m_table_view,
                         scim_anthy_romaji_typing_rule, NULL,
                         d->m_table_editor->m_table_chooser_combo->currentText (),
                         __romaji_fund_table);
}

void ScimAnthySettingPlugin::set_kana_table_view ()
{
    d->setup_table_view (d->m_table_editor->m_table_view,
                         scim_anthy_kana_typing_rule, NULL,
                         d->m_table_editor->m_table_chooser_combo->currentText (),
                         __kana_fund_table);
}

void ScimAnthySettingPlugin::set_thumb_shift_table_view ()
{
    d->setup_table_view (d->m_table_editor->m_table_view,
                         NULL, scim_anthy_nicola_table,
                         d->m_table_editor->m_table_chooser_combo->currentText (),
                         __nicola_fund_table);
}

#include "scimanthysettingplugin.moc"
