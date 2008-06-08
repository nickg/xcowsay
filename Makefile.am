SUBDIRS = po debian src 

dist_pkgdata_DATA = cow_small.png cow_med.png cow_large.png \
	cow_panel_icon.png
EXTRA_DIST = config.rpath m4/ChangeLog cow.svg xcowsay.6 cow_icon.png \
	cow_panel_icon.png

ACLOCAL_AMFLAGS = -I m4

pixmaps_dir = $(datadir)/pixmaps
icon = $(srcdir)/cow_icon.png

install-data-local: $(icon)
	mkdir -p $(pixmaps_dir)
	$(INSTALL_DATA) $(icon) $(pixmaps_dir)
