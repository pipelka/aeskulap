SUBDIRS = \
	po \
	$(DCMTK_SUBDIR) \
	configuration \
	imagepool \
	widgets \
	src \
	pixmaps

DIST_SUBDIRS = \
	po \
	configuration \
	imagepool \
	widgets \
	src \
	pixmaps

EXTRA_DIST = \
	COPYING \
	COPYING.LIB \
	COPYING.DCMTK \
	INSTALL \
	README \
	gettext.h \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	configure.mingw32

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltoo-update

@INTLTOOL_DESKTOP_RULE@

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
	if test x"" != "x" ; then cd ./dcmtk && make distclean ; fi
	-rm -f $(am__CONFIG_DISTCLEAN_FILES)
	-rm -f Makefile

rpm: dist
	mkdir -p rpmbuild
	cp $(PACKAGE)-$(VERSION).tar.gz rpmbuild
	rpmbuild --buildroot rpmbuild -ba $(PACKAGE).spec

dist-win32:
	-rm -Rf $(PACKAGE)-$(VERSION)
	make prefix=/tmp/$(PACKAGE)-$(VERSION) install-strip
	for i in ofstd ijg8 ijg16 dcmdata dcmnet dcmimgle dcmimage dcmtlc do ; \
	done
	cd /tmp && zip -r -9 $(PACKAGE)-$(VERSION).zip $(PACKAGE)-$(VERSION)
	-rm -Rf /tmp/$(PACKAGE)-$(VERSION)
	mv /tmp/$(PACKAGE)-$(VERSION).zip .

push-dist-win32: dist-win32
	scp $(PACKAGE)-$(VERSION).zip pipelka@www.bms-austria.com:/home/pipelka/public_html/aeskulap/win32
