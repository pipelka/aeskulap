bin_PROGRAMS = aeskulap

aeskulap_SOURCES = \
	astockids.cpp \
	astockids.h \
	aiconfactory.cpp \
	aiconfactory.h \
	mainwindow.cpp \
	mainwindow.h \
	studymanager.cpp \
	studymanager.h \
	settings.cpp \
	settings.h \
	main.cpp

aeskulap_LDADD = \
	$(GTKMM_LIBS) \
	$(GLADEMM_LIBS) \
	$(GTHREAD_LIBS) \
	$(GCONFMM_LIBS) \
	../widgets/libgtkdicom.la \
	../imagepool/libimagepool.la

aeskulap_LDFLAGS = \
	-Wl,-rpath,${prefix}/lib/aeskulap

schemadir = \
	$(sysconfdir)/gconf/schemas

schema_DATA = \
	aeskulap.schema

install-data-hook:
	GCONF_CONFIG_SOURCE="" gconftool-2 --makefile-install-rule $(srcdir)/$(schema_DATA) > /dev/null 2>&1 || true
#	GCONF_CONFIG_SOURCE=$(INSTALL_GCONF_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(srcdir)/$(schema_DATA)

INCLUDES = \
	-DAESKULAP_GLADEDIR=\""$(gladedir)"\" \
	-DAESKULAP_IMAGESDIR=\""$(aeskulap_images_dir)"\" \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	$(GTKMM_CFLAGS) \
	$(GLADEMM_CFLAGS) \
	$(GCONFMM_CFLAGS) \
	$(DCMTK_CFLAGS) \
	-I$(top_srcdir)/widgets \
	-I$(top_srcdir)/imagepool

gladedir = $(datadir)/aeskulap/glade
glade_DATA = aeskulap.glade

desktopdir = $(datadir)/applications
desktop_DATA = aeskulap.desktop

EXTRA_DIST = \
	aeskulap.glade \
	aeskulap.desktop \
	$(schema_DATA)
