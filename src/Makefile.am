AM_CFLAGS = $(XCOWSAY_CFLAGS) 
LDADD = $(XCOWSAY_LIBS)

libexec_PROGRAMS = xcowsay_applet

libexecdir = $(libdir)/gnome-applets

xcowsay_applet_SOURCES = display_cow.c display_cow.h floating_shape.h \
	floating_shape.c settings.h settings.c Cowsay_glue.h xcowsayd.h \
	xcowsayd.c config_file.h config_file.c i18n.h applet.c

localedir = $(datadir)/locale

DEFS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(pkgdatadir)\" @DEFS@

serverdir = $(libdir)/bonobo/servers
server_files = CowsayApplet.server

$(server_files): $(server_files).in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

EXTRA_DIST = $(server_files).in

install-data-local: $(server_files)
	mkdir -p $(serverdir)
	$(INSTALL_DATA) $(server_files) $(serverdir)
