SUBDIRS = po src 

dist_pkgdata_DATA = cow_small.png cow_med.png cow_large.png
EXTRA_DIST = config.rpath m4/ChangeLog cow.svg xcowsay.6
man_MANS = xcowsay.6

ACLOCAL_AMFLAGS = -I m4

TESTS_ENVIRONMENT = \
	BUILD_DIR=$(top_builddir) \
	SRC_DIR=$(top_srcdir)

TESTS = test.sh
