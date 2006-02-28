SUBDIRS = \
	include \
	libsrc

noinst_PROGRAMS = \
	extractimg

extractimg_SOURCES = \
	extractimg.cpp

extractimg_LDADD = \
	$(DCMTK_LIBS)

extractimg_LDFLAGS = \
        -Wl,-rpath,${prefix}/lib/aeskulap

INCLUDES = \
	$(DCMTK_CFLAGS)
