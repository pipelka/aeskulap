pkglib_LTLIBRARIES = libimagepool.la

libimagepool_la_SOURCES = \
	dicombase.h \
	dicombase.cpp \
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
	$(GTKMM_LIBS) \
	$(GTHREAD_LIBS) \
	$(GCONFMM_LIBS) \
	$(DCMTK_LIBS) \
	-lpng \
	-ltiff

libimagepool_la_LDFLAGS = \
        -Wl,-rpath,${prefix}/lib/aeskulap

INCLUDES = \
	$(DCMTK_CFLAGS) \
	$(GTKMM_CFLAGS) \
	$(GCONFMM_CFLAGS)

