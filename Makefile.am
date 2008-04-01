AUTOMAKE_OPTIONS        = foreign
ACLOCAL_AMFLAGS         = -I autotools
AM_MAKEFLAGS            = -s

pkglib_LTLIBRARIES	= profileserv.la
profileserv_la_SOURCES	= profileserv.c profileserv_help.c

noinst_HEADERS		= profileserv.h

profileserv_la_LDFLAGS	= -module -avoid-version

EXTRA_DIST		= autotools/ccdv.c autotools/shtool RELNOTES

dist_doc_DATA		= README.ProfileServ


include $(top_srcdir)/autotools/rules.mk
