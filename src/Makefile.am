AM_CPPFLAGS = \
	-DSCIM_ANTHY_LOCALEDIR=\"$(datadir)/locale\" \
	-DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
	-DSCIM_ANTHY_DATADIR=\"@SCIM_ANTHY_DATADIR@\" \
	-DSCIM_ANTHY_STYLEDIR=\"@SCIM_ANTHY_DATADIR@/style\"
INCLUDE	= \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src 

noinst_HEADERS = \
	scim_anthy_intl.h \
	scim_anthy_prefs.h \
	scim_anthy_factory.h \
	scim_anthy_imengine.h \
	scim_anthy_action.h \
	scim_anthy_preedit.h \
	scim_anthy_reading.h \
	scim_anthy_conversion.h \
	scim_anthy_key2kana_table.h \
	scim_anthy_key2kana_base.h \
	scim_anthy_key2kana.h \
	scim_anthy_nicola.h \
	scim_anthy_default_tables.h \
	scim_anthy_style_file.h \
	scim_anthy_setup.h \
	scim_anthy_setup_romaji.h \
	scim_anthy_table_editor.h \
	scim_anthy_utils.h \
	scim_color_button.h

moduledir = @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES = anthy.la

anthy_la_SOURCES = \
	scim_anthy_factory.cpp \
	scim_anthy_imengine.cpp \
	scim_anthy_action.cpp \
	scim_anthy_preedit.cpp \
	scim_anthy_reading.cpp \
	scim_anthy_conversion.cpp \
	scim_anthy_key2kana.cpp \
	scim_anthy_key2kana_table.cpp \
	scim_anthy_nicola.cpp \
	scim_anthy_default_tables.cpp \
	scim_anthy_style_file.cpp \
	scim_anthy_utils.cpp

anthy_la_CFLAGS = @SCIM_CFLAGS@  @SCIM_DEBUG_FLAGS@ @ANTHY_CFLAGS@
anthy_la_CXXFLAGS = @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@ @ANTHY_CFLAGS@

anthy_la_LDFLAGS = \
	-avoid-version \
	-rpath $(moduledir) \
	-module \
	@LIBTOOL_EXPORT_OPTIONS@ \
	@SCIM_LIBS@ \
	@ANTHY_LIBS@

if SCIM_BUILD_SETUP
SETUP_MODULE = anthy-imengine-setup.la
endif

setupmoduledir           = @SCIM_MODULEDIR@/SetupUI
setupmodule_LTLIBRARIES  = $(SETUP_MODULE)

anthy_imengine_setup_la_SOURCES = \
	scim_anthy_setup.cpp \
	scim_anthy_setup_romaji.cpp \
	scim_anthy_style_file.cpp \
	scim_anthy_default_tables.cpp \
	scim_anthy_prefs.cpp \
	scim_anthy_utils.cpp \
	scim_anthy_table_editor.cpp \
	scim_color_button.cpp

anthy_imengine_setup_la_CFLAGS  = @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

anthy_imengine_setup_la_CXXFLAGS= @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

anthy_imengine_setup_la_LDFLAGS = -avoid-version \
                                   -rpath $(setupmoduledir) \
                                   -module \
                                   @LIBTOOL_EXPORT_OPTIONS@ \
                                   @SCIM_GTKUTILS_LIBS@ \
                                   @INTLLIBS@ \
                                   @SCIM_LIBS@
