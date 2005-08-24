SUBDIRS = \
	intl \
	po \
	debian \
	dcmtk \
	imagepool \
	widgets \
	src \
	pixmaps

DIST_SUBDIRS = \
	intl \
	po \
	debian \
	imagepool \
	widgets \
	src \
	pixmaps

EXTRA_DIST = \
	config.rpath \
	mkinstalldirs  \
	COPYING \
	COPYING.LIB \
	COPYING.DCMTK \
	gettext.h \
	m4/ChangeLog  \
	autogen.sh \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

ACLOCAL_AMFLAGS = -I m4

dist-hook:
	mkdir -p $(distdir)/dcmtk
	cp -Rf dcmtk/* $(distdir)/dcmtk
	cd $(distdir)/dcmtk && make distclean
	list=`find $(distdir)/dcmtk -name "CVS"` && rm -Rf $$list
	list=`find $(distdir)/dcmtk -name ".cvsignore"` && rm -f $$list
	list=`find $(distdir)/dcmtk -name ".#*"` && rm -f $$list
