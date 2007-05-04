icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = aeskulap.png aeskulap.ico
svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = aeskulap.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
	        echo "*** Icon cache not updated.  After install, run this:"; \
	        echo "***   $(gtk_update_icon_cache)"; \
	fi

imagesdir = $(datadir)/aeskulap/images
images_DATA = \
	grid-16.png \
	grid-1.png \
	grid-2h.png \
	grid-2v.png \
	grid-4.png \
	series-1x1.png \
	series-2x1.png \
	series-2x2.png \
	series-3x2.png \
	series-3x3.png \
	series-4x4.png \
	stock-layers-24.png \
	stock-tool-measure-22.png \
	stock-tool-scale-22.png \
	stock-tool-eraser-22.png \
	cursor_pan.png \
	cursor_windowlevel.png \
	cursor_zoom.png \
	filmholes-big-left.png \
	filmholes-big-right.png \
	start-here.png \
	stock-tool-color-picker-22.png

EXTRA_DIST = \
	README \
	$(icon_DATA) \
	$(svgicon_DATA) \
	$(images_DATA)
