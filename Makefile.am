bin_PROGRAMS = xcowsay
bin_SCRIPTS = xcowat xcowfortune

AM_CFLAGS = $(XCOWSAY_CFLAGS) -DDATADIR=\"$(pkgdatadir)\"
LDADD = $(XCOWSAY_LIBS)

xcowsay_SOURCES = xcowsay.c display_cow.c display_cow.h floating_shape.h \
	floating_shape.c settings.h settings.c Cowsay_glue.h xcowsayd.h \
	xcowsayd.c

dist_pkgdata_DATA = cow_small.png cow_med.png cow_large.png
EXTRA_DIST = cow.svg xcowat xcowfortune
