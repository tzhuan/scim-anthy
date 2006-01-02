#prefix = @skim_prefix@
AM_CPPFLAGS = \
	-DSCIM_ANTHY_DATADIR=\"@SCIM_ANTHY_DATADIR@\" \
	-DSCIM_ANTHY_STYLEDIR=\"@SCIM_ANTHY_DATADIR@/style\"
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src $(all_includes) @SCIM_KDEUTILS_CFLAGS@ 
METASOURCES = AUTO
kde_module_LTLIBRARIES =  kcm_skimplugin_scim_anthy.la

noinst_HEADERS = \
	scimanthysettingplugin.h \
	scim_anthy_default_tables.h \
	scim_anthy_key2kana_table.h \
	scim_anthy_table_editor.h \
	scim_anthy_style_file.h

kcm_skimplugin_scim_anthy_la_LDFLAGS = \
	-module -no-undefined $(KDE_PLUGIN) $(all_libraries)

kcm_skimplugin_scim_anthy_la_SOURCES = \
	anthy.kcfgc \
	anthyui.ui \
	scimanthysettingplugin.cpp \
	scim_anthy_default_tables.cpp \
	scim_anthy_key2kana_table.cpp \
	scim_anthy_style_file.cpp \
	scim_anthy_table_editor.cpp

kcm_skimplugin_scim_anthy_la_LIBADD = \
	$(LIB_QT) $(LIB_KDEUI) $(LIB_KDECORE) @SCIM_KDEUTILS_LIBS@

kde_kcfg_DATA = anthy.kcfg

kcm_DATA = skimplugin_scim_anthy_config.desktop

kcmdir = $(kde_servicesdir)/skimconfiguredialog
