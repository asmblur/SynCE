
bin_PROGRAMS = gcemirror

INCLUDES  = \
	-DDATADIR=\"$(datadir)\"               \
	-DPREFIX=\""$(prefix)"\"               \
	-DSYSCONFDIR=\""$(sysconfdir)"\"       \
	-DLIBDIR=\""$(libdir)"\"               \
	-DSYNCE_DATA=\""$(datadir)/synce/"\"

BUILT_SOURCES = gcemirror-signals-marshal.h gcemirror-signals-marshal.c

CLEANFILES = $(BUILT_SOURCES)

gcemirror_SOURCES = $(BUILT_SOURCES) \
			decoder.c decoder.h \
			xordecoder.c xordecoder.h \
			rledecoder.c rledecoder.h \
			huffmandecoder.c huffmandecoder.h \
			imageviewer.c imageviewer.h \
			cescreen.c cescreen.h \
			main.c

EXTRA_DIST = gcemirror-signals-marshal.list

gcemirror_CPPFLAGS = \
	$(GLIB2_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(LIBSYNCE_CFLAGS)

gcemirror_LDADD = \
	$(GLIB2_LIBS) \
	$(GTK3_LIBS) \
	$(LIBSYNCE_LIBS)


%-signals-marshal.h: %-signals-marshal.list
	glib-genmarshal --header --prefix=$(subst -,_,$*)_marshal $< > $*-signals-marshal.h

%-signals-marshal.c: %-signals-marshal.list
	glib-genmarshal --body --prefix=$(subst -,_,$*)_marshal $< > $*-signals-marshal.c
