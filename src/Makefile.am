bin_PROGRAMS = aeskulap

aeskulap_SOURCES = \
	binreloc.h \
	binreloc.c \
	abusycursor.h \
	abusycursor.cpp \
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
	prescandialog.cpp \
	prescandialog.h \
	aboutdialog.h \
	aboutdialog.cpp \
	windowleveldialog.h \
	windowleveldialog.cpp \
	main.cpp


aeskulap_LDADD = \
	$(GTKMM_LIBS) \
	$(GLADEMM_LIBS) \
	$(GTHREAD_LIBS) \
	../widgets/libgtkdicom.la \
	../imagepool/libimagepool.la \
	../configuration/libconfiguration.la \
	$(GCONFMM_LIBS) \
	$(DCMTK_LIBS)

aeskulap_LDFLAGS = \
	-Wl,-rpath,${prefix}/lib/aeskulap

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = aeskulap.schemas

@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
endif

INCLUDES = \
	-DAESKULAP_DATADIR=\""$(datadir)"\" \
	-DLOCALEDIR=\""$(datadir)/locale"\" \
	$(GTKMM_CFLAGS) \
	$(GLADEMM_CFLAGS) \
	$(GCONFMM_CFLAGS) \
	$(DCMTK_CFLAGS) \
	-I$(top_srcdir)/configuration \
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
