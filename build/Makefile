#
# Makefile for the `defer' bash loadable builtin.
#
# Derived from bash's examples/loadables/Makefile.sample.in.
# Copyright (C) 2022 Free Software Foundation, Inc.
# Copyright (C) 2026 Léa Gris <lea.gris@noiraude.net>
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

# these should match the ones in Makefile.in (for the make install target)

prefix = /usr/local
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib

# ${loadablesdir} is where the example loadable builtins and data files
# are installed (make install target in Makefile.in)

loadablesdir = /usr/lib/bash
DESTDIR =

# include Makefile.inc for all boilerplate definitions

include $(DESTDIR)$(loadablesdir)/Makefile.inc

# loadables.h installs to loadablesdir, which Makefile.inc's INC does not cover.
INC += -I$(loadablesdir)

# `defer' is the name of the shared object, built from defer.c

all:	defer

defer:	defer.o
	$(SHOBJ_LD) $(SHOBJ_LDFLAGS) $(SHOBJ_XLDFLAGS) -o $@ defer.o $(SHOBJ_LIBS)

defer.o: defer.c

test:	defer
	$${THIS_SH:-bash} test/defer_test.sh

lint:
	clang-tidy defer.c -- $(CCFLAGS) $(INC)

clean:
	$(RM) defer.o defer

distclean: clean
	$(RM) Makefile config.log config.status
	$(RM) -r autom4te.cache

.PHONY:	all test lint clean distclean
