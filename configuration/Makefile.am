pkglib_LTLIBRARIES = libconfiguration.la

libconfiguration_la_SOURCES = \
	awindowlevel.h \
	aconfiguration.h \
	aconfiguration.cpp \
	aconfigclient.h \
	aconfigclient.cpp

libconfiguration_la_LIBADD = \
	$(GCONFMM_LIBS) \
	$(GTKMM_LIBS)

libconfiguration_la_LDFLAGS = \
	-Wl,-rpath,${prefix}/lib/aeskulap -avoid-version

INCLUDES = \
	$(GCONFMM_CFLAGS) \
	$(GTKMM_CFLAGS)

EXTRA_DIST = \
	aconfiguration-gconf.cpp \
	aconfiguration-win32.cpp

