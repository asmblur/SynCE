## Makefile.am -- Process this file with automake to produce Makefile.in
@SET_MAKE@

## don't complain about missing AUTHORS, NEWS, ChangeLog, etc
AUTOMAKE_OPTIONS = foreign 1.4

pcfiles = liborange.pc

all_local: $(pcfiles)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)

EXTRA_DIST = liborange.pc.in LICENSE

CLEANFILES = $(pcfiles)

SUBDIRS = lib src man
