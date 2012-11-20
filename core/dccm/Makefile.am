AM_CFLAGS = -Wall \
	-DLOCALSTATEDIR=\""$(localstatedir)"\"

libexec_PROGRAMS = dccm synce-serial-chat

BUILT_SOURCES = synce-device-signals-marshal.h \
		synce-device-signals-marshal.c

BUILT_SOURCES += synce-device-glue.h \
		synce-device-manager-control-signals-marshal.h \
		synce-device-manager-control-signals-marshal.c \
		synce-device-manager-control-glue.h \
		synce-device-manager-glue.h

CLEANFILES = $(BUILT_SOURCES)

dccm_SOURCES = $(BUILT_SOURCES) \
		log.c log.h \
		synce-device.c synce-device.h synce-device-internal.h \
		synce-device-rndis.c synce-device-rndis.h \
		synce-device-legacy.c synce-device-legacy.h \
		synce-connection-broker.c synce-connection-broker.h \
		synce-errors.c synce-errors.h \
		utils.c utils.h

dccm_SOURCES += main.c synce-device-manager.c synce-device-manager.h \
		synce-device-manager-control.c synce-device-manager-control.h

dccm_CPPFLAGS = \
	-I$(top_srcdir)/lib/utils \
	$(GLIB_CFLAGS) \
	$(DBUS_CFLAGS)

dccm_LDADD = \
	$(top_builddir)/lib/libsynce.la \
	$(GLIB_LIBS) \
	$(DBUS_LIBS)

dccm_CPPFLAGS += $(GUDEV_CFLAGS) -DG_UDEV_API_IS_SUBJECT_TO_CHANGE
dccm_LDADD += $(GUDEV_LIBS)

synce_serial_chat_SOURCES = synce-serial-chat.c

EXTRA_DIST = synce-device-signals-marshal.list \
		synce-device.xml \
		synce-device-manager-control-signals-marshal.list \
		synce-device-manager-control.xml \
		synce-device-manager.xml \
		synce-unlock

bin_SCRIPTS = synce-unlock

%-glue.h: %.xml
	dbus-binding-tool --mode=glib-server --output=$@ --prefix=$(subst -,_,$*) $<

%-signals-marshal.h: %-signals-marshal.list
	glib-genmarshal --header --prefix=$(subst -,_,$*)_marshal $< > $*-signals-marshal.h

%-signals-marshal.c: %-signals-marshal.list
	glib-genmarshal --body --prefix=$(subst -,_,$*)_marshal $< > $*-signals-marshal.c