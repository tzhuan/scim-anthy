AM_CPPFLAGS = \
	-DSCIM_ICONDIR=\"@SCIM_ICONDIR@\"
INCLUDE	= \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src 

noinst_HEADERS = scim_anthy_imengine.h 

moduledir = @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES = anthy.la

anthy_la_SOURCES = scim_anthy_imengine.cpp

anthy_la_CFLAGS = @SCIM_CFLAGS@ 
anthy_la_CXXFLAGS = @SCIM_CFLAGS@ 

anthy_la_LDFLAGS = \
	-avoid-version \
	-module \
     	-rpath $(moduledir) \
	@SCIM_LIBS@ \
	-lanthy -lanthydic
