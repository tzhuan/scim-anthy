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

typedef struct _KeyList
{
    const char  *label;
    const char  *key;
    KeyCategory  category;
} KeyList;

KeyList key_list[] =
{
    {I18N_NOOP("Toggle on/off"),               "_IMEngine_Anthy_OnOffKey",                   ModeKeys},
    {I18N_NOOP("Circle input mode"),           "_IMEngine_Anthy_CircleInputModeKey",         ModeKeys},
    {I18N_NOOP("Circle kana mode"),            "_IMEngine_Anthy_CircleKanaModeKey",          ModeKeys},
    {I18N_NOOP("Latin mode"),                  "_IMEngine_Anthy_LatinModeKey",               ModeKeys},
    {I18N_NOOP("Wide Latin mode"),             "_IMEngine_Anthy_WideLatinModeKey",           ModeKeys},
    {I18N_NOOP("Hiragana mode"),               "_IMEngine_Anthy_HiraganaModeKey",            ModeKeys},
    {I18N_NOOP("Katakana mode"),               "_IMEngine_Anthy_KatakanaModeKey",            ModeKeys},
    {I18N_NOOP("Half katakana mode"),          "_IMEngine_Anthy_HalfKatakanaModeKey",        ModeKeys},
    {I18N_NOOP("Circle typing method"),        "_IMEngine_Anthy_CircleTypingMethodKey",      ModeKeys},

    {I18N_NOOP("Insert space"),                "_IMEngine_Anthy_InsertSpaceKey",             EditKeys},
    {I18N_NOOP("Insert alternative space"),    "_IMEngine_Anthy_InsertAltSpaceKey",          EditKeys},
    {I18N_NOOP("Insert half space"),           "_IMEngine_Anthy_InsertHalfSpaceKey",         EditKeys},
    {I18N_NOOP("Insert wide space"),           "_IMEngine_Anthy_InsertWideSpaceKey",         EditKeys},
    {I18N_NOOP("Backspace"),                   "_IMEngine_Anthy_BackSpaceKey",               EditKeys},
    {I18N_NOOP("Delete"),                      "_IMEngine_Anthy_DeleteKey",                  EditKeys},
    {I18N_NOOP("Commit"),                      "_IMEngine_Anthy_CommitKey",                  EditKeys},
    {I18N_NOOP("Convert"),                     "_IMEngine_Anthy_ConvertKey",                 EditKeys},
    {I18N_NOOP("Predict"),                     "_IMEngine_Anthy_PredictKey",                 EditKeys},
    {I18N_NOOP("Cancel"),                      "_IMEngine_Anthy_CancelKey",                  EditKeys},
    {I18N_NOOP("Cancel all"),                  "_IMEngine_Anthy_CancelAllKey",               EditKeys},
    {I18N_NOOP("Reconvert"),                   "_IMEngine_Anthy_ReconvertKey",               EditKeys},
    {I18N_NOOP("Do nothing"),                  "_IMEngine_Anthy_DoNothingKey",               EditKeys},

    {I18N_NOOP("Move to first"),               "_IMEngine_Anthy_MoveCaretFirstKey",          CaretKeys},
    {I18N_NOOP("Move to last"),                "_IMEngine_Anthy_MoveCaretLastKey",           CaretKeys},
    {I18N_NOOP("Move to forward"),             "_IMEngine_Anthy_MoveCaretForwardKey",        CaretKeys},
    {I18N_NOOP("Move to backward"),            "_IMEngine_Anthy_MoveCaretBackwardKey",       CaretKeys},

    {I18N_NOOP("Select the first segment"),    "_IMEngine_Anthy_SelectFirstSegmentKey",      SegmentKeys},
    {I18N_NOOP("Select the last segment"),     "_IMEngine_Anthy_SelectLastSegmentKey",       SegmentKeys},
    {I18N_NOOP("Select the next segment"),     "_IMEngine_Anthy_SelectNextSegmentKey",       SegmentKeys},
    {I18N_NOOP("Select the previous segment"), "_IMEngine_Anthy_SelectPrevSegmentKey",       SegmentKeys},
    {I18N_NOOP("Shrink the segment"),          "_IMEngine_Anthy_ShrinkSegmentKey",           SegmentKeys},
    {I18N_NOOP("Expand the segment"),          "_IMEngine_Anthy_ExpandSegmentKey",           SegmentKeys},
    {I18N_NOOP("Commit the first segment"),    "_IMEngine_Anthy_CommitFirstSegmentKey",      SegmentKeys},
    {I18N_NOOP("Commit the selected segment"), "_IMEngine_Anthy_CommitSelectedSegmentKey",   SegmentKeys},

    {I18N_NOOP("First candidate"),             "_IMEngine_Anthy_SelectFirstCandidateKey",    CandidateKeys},
    {I18N_NOOP("Last candidate"),              "_IMEngine_Anthy_SelectLastCandidateKey",     CandidateKeys},
    {I18N_NOOP("Next candidate"),              "_IMEngine_Anthy_SelectNextCandidateKey",     CandidateKeys},
    {I18N_NOOP("Previous candidate"),          "_IMEngine_Anthy_SelectPrevCandidateKey",     CandidateKeys},
    {I18N_NOOP("Page up"),                     "_IMEngine_Anthy_CandidatesPageUpKey",        CandidateKeys},
    {I18N_NOOP("Page down"),                   "_IMEngine_Anthy_CandidatesPageDownKey",      CandidateKeys},

    {I18N_NOOP("1st candidate"),               "_IMEngine_Anthy_SelectCandidates1Key",       DirectSelectKeys},
    {I18N_NOOP("2nd candidate"),               "_IMEngine_Anthy_SelectCandidates2Key",       DirectSelectKeys},
    {I18N_NOOP("3rd candidate"),               "_IMEngine_Anthy_SelectCandidates3Key",       DirectSelectKeys},
    {I18N_NOOP("4th candidate"),               "_IMEngine_Anthy_SelectCandidates4Key",       DirectSelectKeys},
    {I18N_NOOP("5th candidate"),               "_IMEngine_Anthy_SelectCandidates5Key",       DirectSelectKeys},
    {I18N_NOOP("6th candidate"),               "_IMEngine_Anthy_SelectCandidates6Key",       DirectSelectKeys},
    {I18N_NOOP("7th candidate"),               "_IMEngine_Anthy_SelectCandidates7Key",       DirectSelectKeys},
    {I18N_NOOP("8th candidate"),               "_IMEngine_Anthy_SelectCandidates8Key",       DirectSelectKeys},
    {I18N_NOOP("9th candidate"),               "_IMEngine_Anthy_SelectCandidates9Key",       DirectSelectKeys},
    {I18N_NOOP("10th candidate"),              "_IMEngine_Anthy_SelectCandidates10Key",      DirectSelectKeys},

    {I18N_NOOP("Convert character type to forward"),  "_IMEngine_Anthy_ConvertCharTypeForwardKey",  ConvertKeys},
    {I18N_NOOP("Convert character type to backward"), "_IMEngine_Anthy_ConvertCharTypeBackwardKey", ConvertKeys},
    {I18N_NOOP("Convert to hiragana"),         "_IMEngine_Anthy_ConvertToHiraganaKey",       ConvertKeys},
    {I18N_NOOP("Convert to katakana"),         "_IMEngine_Anthy_ConvertToKatakanaKey",       ConvertKeys},
    {I18N_NOOP("Convert to half width"),       "_IMEngine_Anthy_ConvertToHalfKey",           ConvertKeys},
    {I18N_NOOP("Convert to half katakana"),    "_IMEngine_Anthy_ConvertToHalfKatakanaKey",   ConvertKeys},
    {I18N_NOOP("Convert to wide latin"),       "_IMEngine_Anthy_ConvertToWideLatinKey",      ConvertKeys},
    {I18N_NOOP("Convert to latin"),            "_IMEngine_Anthy_ConvertToLatinKey",          ConvertKeys},

    {I18N_NOOP("Edit dictionary"),             "_IMEngine_Anthy_DictAdminKey",               DictionaryKeys},
    {I18N_NOOP("Add a word"),                  "_IMEngine_Anthy_AddWordKey",                 DictionaryKeys},

    {NULL, NULL, AllKeys},
};


K_EXPORT_COMPONENT_FACTORY (kcm_skimplugin_scim_anthy, 
                            ScimAnthySettingLoaderFactory ("kcm_skimplugin_scim_anthy"))


inline bool string_color_button_enabled (int n)
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
                              KConfigSkeletonGenericItem<QString> *item)
        : QListViewItem (view, sibling, feature, defval, description),
          m_item (item)
    {
    }
    ~ScimAnthyKeyListViewItem ()
    {
    }

    virtual void setText (int column, const QString &text)
    {
        QListViewItem::setText (column, text);

        // FIXME!
        // Although it should be set when the "OK" or "Apply" button has been
        // pressed, it can't easyly for some reasons especialy "Category" combo
        // box related problems.
        if (m_item)
            m_item->setValue (text);
    }

public:
    ScimAnthySettingPlugin              *m_plugin;
    KConfigSkeletonGenericItem<QString> *m_item;
};

class ScimAnthySettingPlugin::ScimAnthySettingPluginPrivate {
public:
    AnthySettingUI * ui;

    bool       m_our_value_changed;

    StyleFiles m_style_list;
    StyleFile  m_user_style;
    bool       m_style_changed;

public:
    ScimAnthySettingPluginPrivate ()
        : m_our_value_changed (false),
          m_style_changed (false)
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

    void set_keys (const QString & feature, const QString & keys)
    {
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

    void set_theme (const QString & item_key,
                    const QString & item_value,
                    const QString & section_name)
    {
        KConfigSkeletonGenericItem<QString> *item = string_config_item (item_key);
        if (!item) return;

        if (item_value == i18n ("Default")) {
            item->setValue (QString (""));
        } else if (item_value == i18n ("User defined")) {
            item->setValue (QString::fromUtf8 (__user_style_file_name.c_str ()));
        } else {
            // FIXME! shoudn't match by title name.
            StyleFiles::iterator it;
            for (it = m_style_list.begin (); it != m_style_list.end (); it++) {
                StyleLines section;
                if (!it->get_entry_list (section, section_name))
                    continue;
                if (item_value == QString::fromUtf8 (it->get_title().c_str())) {
                    item->setValue (QString::fromUtf8 (it->get_file_name().c_str()));
                    break;
                }
            }
        }
        item->writeConfig (AnthyConfig::self()->config());
    }

    void set_color (const QString key, const QColor & c)
    {
        KConfigSkeletonItem *tmp_item;
        tmp_item = AnthyConfig::self()->findItem(key);
        if (!tmp_item) return;

        KConfigSkeletonGenericItem<QString> *item;
        item = dynamic_cast<KConfigSkeletonGenericItem<QString>*> (tmp_item);
        if (!item) return;

        item->setValue (c.name ());
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
        ui->PreeditStringDualColorButton->setEnabled (string_color_button_enabled (n));
        n = ui->kcfg__IMEngine_Anthy_ConversionStyle->currentItem ();
        ui->ConversionStringDualColorButton->setEnabled (string_color_button_enabled (n));
        n = ui->kcfg__IMEngine_Anthy_SelectedSegmentStyle->currentItem ();
        ui->SelectedSegmentDualColorButton->setEnabled (string_color_button_enabled (n));
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
        int category = ui->KeyBindingsGroupComboBox->currentItem ();

        ui->KeyBindingsView->clear ();
        ui->KeyBindingsView->setSorting (-1);
        ui->KeyBindingsSelectButton->setEnabled (false);

        QListViewItem *prev_item = NULL;

        for (unsigned int i = 0; key_list[i].key; i++) {
            if (category != (int) AllKeys &&
                category != (int) SearchByKey &&
                category != (int) key_list[i].category)
            {
                continue;
            }

            KConfigSkeletonGenericItem<QString> *item;
            item = string_config_item (key_list[i].key);
            if (!item) return;

            if (category == (int) SearchByKey &&
                key_filter (item->value (),
                            ui->KeyBindingsFilterLineEdit->text ()))
            {
                    continue;
            }

            ScimAnthyKeyListViewItem *list_item;
            list_item = new ScimAnthyKeyListViewItem (ui->KeyBindingsView,
                                                      prev_item,
                                                      i18n (key_list[i].label),
                                                      item->value (),
                                                      item->whatsThis (),
                                                      item);
            prev_item = list_item;
        }
    }

    bool key_filter (const QString & keys, const QString & filter)
    {
        if (filter == QString::null)
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
    // Launch buttons
    connect (d->ui->LaunchDictAdminCommandButton,
             SIGNAL (clicked ()),
             this, SLOT (launch_dict_admin_command ()));
    connect (d->ui->LaunchAddWordCommandButton,
             SIGNAL (clicked ()),
             this, SLOT (launch_add_word_command ()));

    // line edit
    connect (d->ui->KeyBindingsFilterLineEdit,
             SIGNAL (textChanged (const QString &)),
             this, SLOT (set_key_bindings_group ()));

    // combo boxes
    connect (d->ui->KeyBindingsGroupComboBox,
             SIGNAL (activated (int)),
             this, SLOT (set_key_bindings_group ()));
    connect (d->ui->KeyBindingsThemeComboBox,
             SIGNAL (activated (int)),
             this, SLOT (set_key_bindings_theme (int)));
    connect (d->ui->RomajiComboBox,
             SIGNAL (activated (const QString &)),
             this, SLOT (set_romaji_theme (const QString &)));
    connect (d->ui->KanaComboBox,
             SIGNAL (activated (const QString &)),
             this, SLOT (set_kana_theme (const QString &)));
    connect (d->ui->ThumbShiftComboBox,
             SIGNAL (activated (const QString &)),
             this, SLOT (set_nicola_theme (const QString &)));
    connect (d->ui->kcfg__IMEngine_Anthy_PreeditStyle,
             SIGNAL (activated (int)),
             this, SLOT (preedit_string_style_changed (int)));
    connect (d->ui->kcfg__IMEngine_Anthy_ConversionStyle,
             SIGNAL (activated (int)),
             this, SLOT (conversion_string_style_changed (int)));
    connect (d->ui->kcfg__IMEngine_Anthy_SelectedSegmentStyle,
             SIGNAL (activated (int)),
             this, SLOT (selected_segment_style_changed (int)));

    // push buttons
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

    // key bindings view
    connect (d->ui->KeyBindingsView,
             SIGNAL (selectionChanged (QListViewItem*)),
             this, SLOT (key_bindings_view_selection_changed (QListViewItem*)));
    connect (d->ui->KeyBindingsView,
             SIGNAL (doubleClicked (QListViewItem*)),
             this, SLOT (choose_keys ()));

    // preedit string color
    connect (d->ui->PreeditStringDualColorButton,
             SIGNAL (fgChanged(const QColor &)),
             this, SLOT (set_preedit_string_fg_color(const QColor &)));
    connect (d->ui->PreeditStringDualColorButton,
             SIGNAL (bgChanged(const QColor &)),
             this, SLOT (set_preedit_string_bg_color(const QColor &)));

    // conversion string color
    connect (d->ui->ConversionStringDualColorButton,
             SIGNAL (fgChanged(const QColor &)),
             this, SLOT (set_conversion_string_fg_color(const QColor &)));
    connect (d->ui->ConversionStringDualColorButton,
             SIGNAL (bgChanged(const QColor &)),
             this, SLOT (set_conversion_string_bg_color(const QColor &)));

    // selected segment color
    connect (d->ui->SelectedSegmentDualColorButton,
             SIGNAL (fgChanged(const QColor &)),
             this, SLOT (set_selected_segment_fg_color(const QColor &)));
    connect (d->ui->SelectedSegmentDualColorButton,
             SIGNAL (bgChanged(const QColor &)),
             this, SLOT (set_selected_segment_bg_color(const QColor &)));
}

ScimAnthySettingPlugin::~ScimAnthySettingPlugin () 
{
    KGlobal::locale()->removeCatalogue ("skim-scim-anthy");
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

    d->save_style_files ();
}

void ScimAnthySettingPlugin::defaults ()
{
    KAutoCModule::defaults ();

    set_key_bindings_theme (0);
    d->set_theme ("_IMEngine_Anthy_RomajiThemeFile",  i18n ("Default"),
                  __romaji_fund_table);
    d->set_theme ("_IMEngine_Anthy_KanaLayoutFile",   i18n ("Default"),
                  __kana_fund_table);
    d->set_theme ("_IMEngine_Anthy_NICOLALayoutFile", i18n ("Default"),
                  __nicola_fund_table);

    d->reset_custom_widgets ();
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
    d->setup_key_bindings ();
}

void ScimAnthySettingPlugin::set_key_bindings_theme (int n)
{
    d->ui->KeyBindingsThemeComboBox->setCurrentItem (n);

    QString theme_name = d->ui->KeyBindingsThemeComboBox->currentText ();
    int current = d->ui->KeyBindingsThemeComboBox->currentItem ();
    StyleFiles::iterator it;
    std::vector<String> keys;
    std::vector<String>::iterator kit;

    KConfigSkeletonGenericItem<QString> *theme_item, *theme_file_item;
    theme_item = d->string_config_item ("_IMEngine_Anthy_KeyTheme");
    if (theme_item)
        theme_item->setValue (theme_name);

    d->set_theme ("_IMEngine_Anthy_KeyThemeFile", theme_name, __key_bindings_theme);

    // Set all key bindings as empty
    for (unsigned int i = 0; current != 1 && key_list[i].key; i++) {
        KConfigSkeletonGenericItem<QString> *item;
        item = d->string_config_item (key_list[i].key);
        if (!item) continue;
        item->setValue ("");
    }

    // Handle "Default" or "User defined" and return
    if (current == 0) {
        for (unsigned int i = 0; key_list[i].key; i++) {
            KConfigSkeletonGenericItem<QString> *item;
            item = d->string_config_item (key_list[i].key);
            if (!item) continue;
            item->setDefault ();
        }

        if (theme_item)
            theme_item->setValue ("Default");

        goto SET_WIDGET;

    } else if (current == 1) {
        if (theme_item)
            theme_item->setValue ("User defined");

        theme_file_item = d->string_config_item ("_IMEngine_Anthy_KeyThemeFile");
        if (theme_file_item)
            theme_file_item->setValue ("");

        goto SET_WIDGET;
    }

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

    // Set found key bindings to KConfig
    it->get_key_list (keys, __key_bindings_theme);

    for (kit = keys.begin (); kit != keys.end (); kit++) {
        if (kit->empty ()) continue;

        QString entry = QString ("/IMEngine/Anthy/") + QString (kit->c_str ());

        // find from known key bindings list
        for (unsigned int i = 0; key_list[i].key; i++) {
            KConfigSkeletonGenericItem<QString> *item;
            item = d->string_config_item (key_list[i].key);
            if (!item) continue;

            if (item->key () == entry) {
                String v;
                it->get_string (v, __key_bindings_theme, *kit);
                item->setValue (v);
            }
        }
    }

SET_WIDGET:
    d->setup_key_bindings ();
    changed (true);
}

void ScimAnthySettingPlugin::set_romaji_theme (const QString & value)
{
    d->set_theme ("_IMEngine_Anthy_RomajiThemeFile", value, __romaji_fund_table);
    d->m_our_value_changed = true;
    changed (true);
}

void ScimAnthySettingPlugin::set_kana_theme (const QString & value)
{
    d->set_theme ("_IMEngine_Anthy_KanaLayoutFile", value, __kana_fund_table);
    d->m_our_value_changed = true;
    changed (true);
}

void ScimAnthySettingPlugin::set_nicola_theme (const QString & value)
{
    d->set_theme ("_IMEngine_Anthy_NICOLALayoutFile", value, __nicola_fund_table);
    d->m_our_value_changed = true;
    changed (true);
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
        set_key_bindings_theme (1);
        d->m_our_value_changed = true;
        changed (true);
    }
}

void ScimAnthySettingPlugin::customize_romaji_table ()
{
    ScimAnthyTableEditor editor (d->ui, i18n ("Romaji Table:"), i18n ("Sequence"), i18n ("Result"));
    editor.setCaption (i18n ("Edit romajit table"));
    if (editor.exec () == QDialog::Accepted) {
    }
}

void ScimAnthySettingPlugin::customize_kana_table ()
{
    ScimAnthyTableEditor editor (d->ui, i18n ("Layout:"), i18n ("Key"), i18n ("Result"));
    editor.setCaption (i18n ("Edit kana layout table"));
    if (editor.exec () == QDialog::Accepted) {
    }
}

void ScimAnthySettingPlugin::customize_nicola_table ()
{
    ScimAnthyTableEditor editor (d->ui, i18n ("Layout:"),
                                 i18n ("Key"), i18n ("Single press"),
                                 i18n ("Left thumb shift"), i18n ("Right thumb shift"));
    editor.setCaption (i18n ("Edit thumb shift layout table"));
    if (editor.exec () == QDialog::Accepted) {
    }
}

void ScimAnthySettingPlugin::key_bindings_view_selection_changed (QListViewItem *item)
{
    d->ui->KeyBindingsSelectButton->setEnabled (item ? true : false);
}

void ScimAnthySettingPlugin::preedit_string_style_changed (int n)
{
    d->ui->PreeditStringDualColorButton->setEnabled (string_color_button_enabled (n));
}

void ScimAnthySettingPlugin::conversion_string_style_changed (int n)
{
    d->ui->ConversionStringDualColorButton->setEnabled (string_color_button_enabled (n));
}

void ScimAnthySettingPlugin::selected_segment_style_changed (int n)
{
    d->ui->SelectedSegmentDualColorButton->setEnabled (string_color_button_enabled (n));
}

void ScimAnthySettingPlugin::set_preedit_string_fg_color (const QColor & c)
{
    d->set_color ("_IMEngine_Anthy_PreeditFGColor", c);
    d->m_our_value_changed = true;
    changed (true);
}

void ScimAnthySettingPlugin::set_preedit_string_bg_color (const QColor & c)
{
    d->set_color ("_IMEngine_Anthy_PreeditBGColor", c);
    d->m_our_value_changed = true;
    changed (true);
}

void ScimAnthySettingPlugin::set_conversion_string_fg_color (const QColor & c)
{
    d->set_color ("_IMEngine_Anthy_ConversionFGColor", c);
    d->m_our_value_changed = true;
    changed (true);
}

void ScimAnthySettingPlugin::set_conversion_string_bg_color (const QColor & c)
{
    d->set_color ("_IMEngine_Anthy_ConversionBGColor", c);
    d->m_our_value_changed = true;
    changed (true);
}

void ScimAnthySettingPlugin::set_selected_segment_fg_color (const QColor & c)
{
    d->set_color ("_IMEngine_Anthy_SelectedSegmentFGColor", c);
    d->m_our_value_changed = true;
    changed (true);
}

void ScimAnthySettingPlugin::set_selected_segment_bg_color (const QColor & c)
{
    d->set_color ("_IMEngine_Anthy_SelectedSegmentBGColor", c);
    d->m_our_value_changed = true;
    changed (true);
}

#include "scimanthysettingplugin.moc"
