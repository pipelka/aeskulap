pkglib_LTLIBRARIES = libimagepool.la

libimagepool_la_SOURCES = \
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
	../configuration/libconfiguration.la
	-lpng \
	-ltiff

libimagepool_la_LDFLAGS = \
	-Wl,-rpath,${prefix}/lib/aeskulap -avoid-version

INCLUDES = \
	-I$(top_srcdir)/configuration \
	$(DCMTK_CFLAGS) \
	$(GTKMM_CFLAGS)
