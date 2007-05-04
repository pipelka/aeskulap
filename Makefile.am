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
	AUTHORS \
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
	-rm -Rf /tmp/$(PACKAGE)-$(VERSION)
	make prefix=/tmp/$(PACKAGE)-$(VERSION)$(prefix) install-strip
	-cp -R -f /tmp/$(PACKAGE)-$(VERSION)$(prefix)/* /tmp/$(PACKAGE)-$(VERSION)
	-rm -Rf /tmp/$(PACKAGE)-$(VERSION)$(prefix)
	-rm -Rf /tmp/$(PACKAGE)-$(VERSION)/usr
	if [ -f /tmp/$(PACKAGE)-$(VERSION)/bin/$(target_alias)-aeskulap.exe ] ; then \
	    mv /tmp/$(PACKAGE)-$(VERSION)/bin/$(target_alias)-aeskulap.exe /tmp/$(PACKAGE)-$(VERSION)/bin/aeskulap.exe ; \
	fi
	for i in ofstd ijg8 ijg12 ijg16 dcmjpeg dcmdata dcmnet dcmimgle dcmimage dcmtls ; do \
	    cp $(prefix)/bin/lib$$i.dll /tmp/$(PACKAGE)-$(VERSION)/bin ; \
	done
	$(shell) $(STRIP) -g -S -d --strip-debug --strip-unneeded -x -X --remove-section=.comment --remove-section=.note /tmp/$(PACKAGE)-$(VERSION)/bin/*.exe
	$(shell) $(STRIP) -g -S -d --strip-debug --strip-unneeded -x -X --remove-section=.comment --remove-section=.note /tmp/$(PACKAGE)-$(VERSION)/bin/*.dll
	-mv /tmp/$(PACKAGE)-$(VERSION)/bin/* /tmp/$(PACKAGE)-$(VERSION)
	-rm -Rf /tmp/$(PACKAGE)-$(VERSION)/bin
	-rm -Rf /tmp/$(PACKAGE)-$(VERSION)/lib
	-rm -f /tmp/$(PACKAGE)-$(VERSION)/share/icons/hicolor/icon-theme.cache
	-rm -Rf /tmp/$(PACKAGE)-$(VERSION)/share/applications
	-mv /tmp/$(PACKAGE)-$(VERSION)/share/icons/hicolor/48x48/apps/aeskulap.ico /tmp/$(PACKAGE)-$(VERSION)
	cd /tmp && zip -r -9 $(PACKAGE)-$(VERSION).zip $(PACKAGE)-$(VERSION)
	-rm -Rf /tmp/$(PACKAGE)-$(VERSION)
	mv /tmp/$(PACKAGE)-$(VERSION).zip .

push-dist-win32: dist-win32
	scp $(PACKAGE)-$(VERSION).zip pipelka@www.bms-austria.com:/home/pipelka/public_html/aeskulap/win32
