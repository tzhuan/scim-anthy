/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 3 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3. 
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include "scim_anthy_imengine.h"

#include <anthy/anthy.h>

#define scim_module_init anthy_LTX_scim_module_init
#define scim_module_exit anthy_LTX_scim_module_exit
#define scim_imengine_module_init anthy_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory anthy_LTX_scim_imengine_module_create_factory

#define SCIM_CONFIG_IMENGINE_ANTHY_UUID    "/IMEngine/Anthy/UUID-"
#define SCIM_CONFIG_IMENGINE_ANTHY_ON_KEY  "/IMEngine/Anthy/OnKey"

#define SCIM_PROP_PREFIX                   "/IMEngine/Anthy"
#define SCIM_PROP_INPUT_MODE               "/IMEngine/Anthy/InputMode"

#ifndef SCIM_ANTHY_ICON_FILE
    #define SCIM_ANTHY_ICON_FILE           (SCIM_ICONDIR"/scim-anthy.png")
#endif

// first = name, second = lang
static KeyEvent              __anthy_on_key;

static ConfigPointer _scim_config (0);

extern "C" {
    void scim_module_init (void)
    {
    }

    void scim_module_exit (void)
    {
        anthy_quit ();
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        SCIM_DEBUG_IMENGINE(1) << "Initialize Anthy Engine.\n";

        _scim_config = config;

        if (anthy_init ()) {
            SCIM_DEBUG_IMENGINE(1) << "Failed to initialize Anthy Library!\n";
            return 0;
        }

        String on_key = config->read (SCIM_CONFIG_IMENGINE_ANTHY_ON_KEY, String ("Shift+space"));

        if (!scim_string_to_key (__anthy_on_key, on_key))
            __anthy_on_key = KeyEvent (SCIM_KEY_space, SCIM_KEY_ShiftMask);

        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        AnthyFactory *factory = 0;

        try {
            factory = new AnthyFactory (String ("ja_JP"),
                                        String ("fffb6633-7041-428e-9dfc-139117a71b6e"));
        } catch (...) {
            delete factory;
            factory = 0;
        }

        return factory;
    }
}

AnthyFactory::AnthyFactory (const String &lang,
                            const String &uuid)
    : m_uuid (uuid)
{
    SCIM_DEBUG_IMENGINE(1) << "Create Anthy Factory :\n";
    SCIM_DEBUG_IMENGINE(1) << "  Lang : " << lang << "\n";
    SCIM_DEBUG_IMENGINE(1) << "  UUID : " << uuid << "\n";

    if (lang.length () >= 2)
        set_languages (lang);
    if (!m_iconv.set_encoding ("EUC-JP"))
        return;
}

AnthyFactory::~AnthyFactory ()
{
}

WideString
AnthyFactory::get_name () const
{
    return utf8_mbstowcs (String ("Anthy"));
}

WideString
AnthyFactory::get_authors () const
{
    return WideString ();
}

WideString
AnthyFactory::get_credits () const
{
    return WideString ();
}

WideString
AnthyFactory::get_help () const
{
    return WideString ();
}

String
AnthyFactory::get_uuid () const
{
    return m_uuid;
}

String
AnthyFactory::get_icon_file () const
{
    return String (SCIM_ANTHY_ICON_FILE);
}

IMEngineInstancePointer
AnthyFactory::create_instance (const String &encoding, int id)
{
    return new AnthyInstance (this, encoding, id);
}

AnthyInstance::AnthyInstance (AnthyFactory   *factory,
                              const String   &encoding,
                              int             id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_show_lookup_table (false)
{

    SCIM_DEBUG_IMENGINE(1) << "Create Anthy Instance : ";
}

AnthyInstance::~AnthyInstance ()
{
}

bool
AnthyInstance::candidate_key_event (const KeyEvent &key)
{
    switch (key.code) {
        case SCIM_KEY_Return:
        case SCIM_KEY_KP_Enter:
            select_candidate (m_lookup_table.get_cursor_pos_in_current_page ());
            break;
        case SCIM_KEY_Left:
        case SCIM_KEY_h:
        case SCIM_KEY_KP_Subtract:
        case SCIM_KEY_BackSpace:
            m_lookup_table.cursor_up ();
            update_lookup_table (m_lookup_table);
            break;
        case SCIM_KEY_Right:
        case SCIM_KEY_l:
        case SCIM_KEY_space:
        case SCIM_KEY_KP_Add:
            m_lookup_table.cursor_down ();
            update_lookup_table (m_lookup_table);
            break;
        case SCIM_KEY_Up:
        case SCIM_KEY_Page_Up:
        case SCIM_KEY_k:
            m_lookup_table.page_up ();
            update_lookup_table (m_lookup_table);
            break;
        case SCIM_KEY_Down:
        case SCIM_KEY_Page_Down:
        case SCIM_KEY_j:
        case SCIM_KEY_KP_Tab:
            m_lookup_table.page_down ();
            update_lookup_table (m_lookup_table);
            break;
        case SCIM_KEY_Escape:
            break;
        case SCIM_KEY_1: 
        case SCIM_KEY_2: 
        case SCIM_KEY_3: 
        case SCIM_KEY_4: 
        case SCIM_KEY_5: 
        case SCIM_KEY_6: 
        case SCIM_KEY_7: 
        case SCIM_KEY_8: 
        case SCIM_KEY_9: 
            select_candidate (key.code - SCIM_KEY_1);
            break;
        default:
            break;
    }

    return true;
}

bool
AnthyInstance::process_key_event (const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE(2) << "process_key_event.\n";
    KeyEvent newkey;

    newkey.code = key.code;
    newkey.mask = key.mask & (SCIM_KEY_ShiftMask | SCIM_KEY_ControlMask | SCIM_KEY_AltMask | SCIM_KEY_ReleaseMask);

    /* change input mode */
    if (__anthy_on_key.is_key_press ()) {
        trigger_property (SCIM_PROP_INPUT_MODE);
        return true;
    }

    /* ignore key release. */
    if (key.is_key_release ())
        return false;

    /* ignore modifier keys */
    if (key.code == SCIM_KEY_Shift_L || key.code == SCIM_KEY_Shift_R ||
        key.code == SCIM_KEY_Control_L || key.code == SCIM_KEY_Control_R ||
        key.code == SCIM_KEY_Alt_L || key.code == SCIM_KEY_Alt_R)
        return false;

    /* candidate keys */
    if (m_lookup_table.number_of_candidates ()) {
        return candidate_key_event (key);
    }

    return false;
}

void
AnthyInstance::move_preedit_caret (unsigned int pos)
{
}

void
AnthyInstance::select_candidate (unsigned int item)
{
    if (!m_lookup_table.number_of_candidates ()) return;

    SCIM_DEBUG_IMENGINE(2) << "select_candidate.\n";

    int current = m_lookup_table.get_cursor_pos_in_current_page ();
 
    if (current != item) {
        m_lookup_table.set_cursor_pos_in_current_page (item);
        update_lookup_table (m_lookup_table);
    }
}

void
AnthyInstance::update_lookup_table_page_size (unsigned int page_size)
{
    SCIM_DEBUG_IMENGINE(2) << "update_lookup_table_page_size.\n";

    m_lookup_table.set_page_size (page_size);
}

void
AnthyInstance::lookup_table_page_up ()
{
    if (!m_lookup_table.number_of_candidates () || !m_lookup_table.get_current_page_start ()) return;

    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_up.\n";

    m_lookup_table.page_up ();

    update_lookup_table (m_lookup_table);
}

void
AnthyInstance::lookup_table_page_down ()
{
    if (!m_lookup_table.number_of_candidates () ||
        m_lookup_table.get_current_page_start () + m_lookup_table.get_current_page_size () >=
          m_lookup_table.number_of_candidates ())
        return;

    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_down.\n";

    m_lookup_table.page_down ();

    update_lookup_table (m_lookup_table);
}

void
AnthyInstance::reset ()
{
    SCIM_DEBUG_IMENGINE(2) << "reset.\n";
}

void
AnthyInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";

    hide_aux_string ();

    if (m_show_lookup_table && m_lookup_table.number_of_candidates ()) {
        update_lookup_table (m_lookup_table);
        show_lookup_table ();
    } else {
        hide_lookup_table ();
    }
}

void
AnthyInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_out.\n";
}

void
AnthyInstance::trigger_property (const String &property)
{
    String anthy_prop = property.substr (property.find_last_of ('/') + 1);

    SCIM_DEBUG_IMENGINE(2) << "trigger_property : " << property << " - " << anthy_prop << "\n";
}
