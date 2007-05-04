if BUILD_NOT_MINGW32
pkglib_LTLIBRARIES = libconfiguration.la
else
lib_LTLIBRARIES = libconfiguration.la
endif

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
	$(LDFLAGS_AESKULAP_LIBS)

INCLUDES = \
	$(GCONFMM_CFLAGS) \
	$(GTKMM_CFLAGS)

EXTRA_DIST = \
	aconfiguration-gconf.cpp \
	aconfiguration-win32.cpp

