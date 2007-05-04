if BUILD_NOT_MINGW32
pkglib_LTLIBRARIES = libimagepool.la
else
lib_LTLIBRARIES = libimagepool.la
endif

libimagepool_la_SOURCES = \
	dicomdir.cpp \
	dicomdirloader.h \
	dicomdirloader.cpp \
	poolservers.h \
	poolservers.cpp \
	netclient.h \
	loader.cpp \
	loader.h \
	fileloader.h \
	fileloader.cpp \
	netloader.h \
	netloader.cpp \
	netquery.cpp \
	imagepool.h \
	imagepool.cpp \
	poolinstance.h \
	poolinstance.cpp \
	poolseries.h \
	poolseries.cpp \
	poolstudy.h \
	poolstudy.cpp \
	poolassociation.cpp \
	poolassociation.h \
	poolfindassociation.cpp \
	poolfindassociation.h \
	poolmoveassociation.cpp \
	poolmoveassociation.h \
	poolnetwork.cpp \
	poolnetwork.h

libimagepool_la_LIBADD = \
	$(DCMTK_LIBS) \
	$(GTKMM_LIBS) \
	../configuration/libconfiguration.la \
	-lpng \
	-ltiff

libimagepool_la_LDFLAGS = \
	$(LDFLAGS_AESKULAP_LIBS)

INCLUDES = \
	-I$(top_srcdir)/configuration \
	$(DCMTK_CFLAGS) \
	$(GTKMM_CFLAGS)
