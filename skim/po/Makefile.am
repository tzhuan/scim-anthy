## Copyright (C) 2004-2005 Takuro Ashie
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

POFILES = AUTO
messages:
	@echo "Generating/Updating Messages" ;\
	echo "Generating/Updating Source Messages" ;\
	find $(top_srcdir) -type d | grep -v CVS | sed -e 's,$$,/,' > dirs ;\
	mfs=`find $(top_srcdir) -name Makefile.am | xargs egrep -l '^messages:'` ;\
	for dir in $$mfs; do \
	  if test ! "$$dir" = "./Makefile.am"; then \
	    dir=`dirname $$dir` ;\
	    egrep -v "^$$dir" dirs > dirs.new && mv dirs.new dirs ;\
	  fi ;\
	done;\
	egrep -v "/tests" dirs > dirs.new && mv dirs.new dirs ;\
	dirs=`cat dirs` ;\
	find $$dirs -maxdepth 1 -name "*.cpp" -print > files ;\
	find $$dirs -maxdepth 1 -name "*.cc" -print >> files ;\
	find $$dirs -maxdepth 1 -name "*.h" -print >> files ; \
	if test -z "$$EXTRACTRC"; then \
	  EXTRACTRC=extractrc ;\
	fi;\
	$$EXTRACTRC $(top_srcdir)/*/*.rc > rc.cpp ;\
	echo "Files considered:";\
	cat files;\
	$(XGETTEXT)  rc.cpp `cat files` -ktr2i18n -ki18n -kI18N_NOOP -o $(top_srcdir)/po/$(PACKAGE).pot ;\
	rm -f rc.cpp dirs files ;\
#	echo "Generating/Updating Desktop File Messages" ;\
	cd .. && ./po/createdesktop ./po/desktop_file_list ./po/desktop.pot && cd po
update-all: messages merge
#	$(EXTRACTRC) `find $$dirs -maxdepth 1 -name "*.rc" -o -name "*.ui"` > rc.cpp
