bin_PROGRAMS = xcowsay
bin_SCRIPTS = xcowfortune xcowdream xcowthink

GTK3_CHECK = -DGTK_DISABLE_DEPRECATED -DGTK_DISABLE_SINGLE_INCLUDES -DGSEAL_ENABLE
#GTK3_CHECK += -DGDK_DISABLE_DEPRECATED

AM_CFLAGS = $(XCOWSAY_CFLAGS) -Wall $(GTK3_CHECK)
LDADD = $(XCOWSAY_LIBS)

xcowsay_SOURCES = xcowsay.c display_cow.c display_cow.h floating_shape.h \
	floating_shape.c settings.h settings.c Cowsay_glue.h xcowsayd.h \
	xcowsayd.c config_file.h config_file.c i18n.h bubblegen.c

EXTRA_DIST = xcowfortune xcowdream xcowthink
