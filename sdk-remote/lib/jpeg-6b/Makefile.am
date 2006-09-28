AM_CPPFLAGS = -I.

lib_LTLIBRARIES = libjpeg.la
env_LTLIBRARIES = libjpegx.la
libjpeg_la_SOURCES = \
	jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
        jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c \
        jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c \
        jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c \
        jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c \
        jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c \
        jfdctint.c jidctflt.c jidctfst.c jidctint.c jidctred.c jquant1.c \
        jquant2.c jutils.c jmemmgr.c


libjpeg_la_SOURCES += 							   \
cderror.h       jerror.h        jmorecfg.h      jversion.h		   \
cdjpeg.h        jdct.h          jinclude.h      jpegint.h       transupp.h \
jchuff.h        jdhuff.h        jmemsys.h       jpeglib.h

# We use the stock memory allocator.
libjpeg_la_SOURCES += jmemnobs.c

# FIXME: We used a prebuild config.h.  This is certainly wrong...
libjpeg_la_SOURCES += jconfig.h

libjpegx_la_SOURCES = libjpeg_la_SOURCES

EXTRA_DIST =							\
coderules.doc   install.doc     libjpeg.doc     usage.doc	\
filelist.doc    jconfig.doc     structure.doc   wizard.doc
