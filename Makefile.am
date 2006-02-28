SUBDIRS = \
	intl \
	po \
	dcmtk \
	imagepool \
	widgets \
	src \
	pixmaps

DIST_SUBDIRS = \
	intl \
	po \
	imagepool \
	widgets \
	src \
	pixmaps

EXTRA_DIST = \
	config.rpath \
	mkinstalldirs  \
	README \
	COPYING \
	COPYING.LIB \
	COPYING.DCMTK \
	gettext.h \
	autogen.sh \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

ACLOCAL_AMFLAGS = -I m4

dist-hook:
	mkdir -p $(distdir)/dcmtk
	cp -Rf $(top_srcdir)/dcmtk/* $(distdir)/dcmtk
	cd $(distdir)/dcmtk && make distclean
	list=`find $(distdir)/dcmtk -name "CVS"` && rm -Rf $$list
	list=`find $(distdir)/dcmtk -name ".cvsignore"` && rm -f $$list
	list=`find $(distdir)/dcmtk -name ".#*"` && rm -f $$list
	list=`find $(distdir)/dcmtk -name "*.a"` && rm -f $$list
	list=`find $(distdir)/dcmtk -name "*.so"` && rm -f $$list

distclean: distclean-recursive
	cd $(top_builddir)/dcmtk && make distclean
	-rm -f $(am__CONFIG_DISTCLEAN_FILES)
	-rm -f Makefile
