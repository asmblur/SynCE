AM_CXXFLAGS = @LIBRRA_CFLAGS@ @LIBORANGE_CFLAGS@

INCLUDES = -I$(top_srcdir)/raki -I$(top_srcdir)/raki/interfaces -I../ $(all_includes)
noinst_LIBRARIES =  libinterfaces.a
libinterfaces_a_SOURCES = rakisyncfactory.cpp rakisyncplugin.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/synce-kde.pot

include_HEADERS =  rakisyncplugin.h rakisyncfactory.h
noinst_HEADERS =  rakisyncfactory.h rakisyncplugin.h
