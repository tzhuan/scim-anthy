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
	intl.h \
	scim_anthy_action.h \
	scim_anthy_imengine_factory.h \
	scim_anthy_imengine.h \
	scim_anthy_preedit.h \
	scim_anthy_automaton.h \
	scim_anthy_conv_table.h \
	scim_anthy_style_file.h \
	scim_anthy_prefs.h

moduledir = @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES = anthy.la

anthy_la_SOURCES = \
	scim_anthy_action.cpp \
	scim_anthy_imengine_factory.cpp \
	scim_anthy_imengine.cpp \
	scim_anthy_preedit.cpp \
	scim_anthy_automaton.cpp \
	scim_anthy_conv_table.cpp \
	scim_anthy_style_file.cpp

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

anthy_imengine_setup_la_SOURCES = scim_anthy_imengine_setup.cpp scim_anthy_style_file.cpp

anthy_imengine_setup_la_CFLAGS  = @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

anthy_imengine_setup_la_CXXFLAGS= @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@ @SCIM_DEBUG_FLAGS@

anthy_imengine_setup_la_LDFLAGS = -avoid-version \
                                   -rpath $(setupmoduledir) \
                                   -module \
                                   @LIBTOOL_EXPORT_OPTIONS@ \
                                   @SCIM_GTKUTILS_LIBS@ \
                                   @INTLLIBS@ \
                                   @SCIM_LIBS@
