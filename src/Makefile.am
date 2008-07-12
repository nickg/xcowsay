bin_PROGRAMS = xcowsay
bin_SCRIPTS = xcowfortune

AM_CFLAGS = $(XCOWSAY_CFLAGS) 
LDADD = $(XCOWSAY_LIBS)

xcowsay_SOURCES = xcowsay.c display_cow.c display_cow.h floating_shape.h \
	floating_shape.c settings.h settings.c Cowsay_glue.h xcowsayd.h \
	xcowsayd.c config_file.h config_file.c i18n.h bubblegen.c

EXTRA_DIST = xcowfortune

localedir = $(datadir)/locale

# Add -DSHOW_FPS here to show FPS in title bar
DEFS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(pkgdatadir)\" @DEFS@