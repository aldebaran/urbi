include $(top_srcdir)/flags.mk

# Path to envdir from top/install_prefix
topenvdir=$(PACKAGE_BRAND)/core/$(URBI_HOST)/$(URBI_ENV)

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

SUBDIRS =
if BINDINGS
 SUBDIRS += swig
endif
SUBDIRS += . #tests

## --------- ##
## uobject.  ##
## --------- ##

include $(top_srcdir)/build-aux/svn-externals.mk
uobject_srcdir = $(srcdir)/uobject
include uobject/uobject.mk
urbiinclude_HEADERS = $(uobject_headers)

## ------------ ##
## liburbi.la.  ##
## ------------ ##

env_LTLIBRARIES = liburbi.la

liburbi_la_SOURCES =				\
$(uobject_sources)				\
package-info.cc					\
uabstractclient.cc				\
ucallbacks.cc					\
uclient.cc					\
uconversion.cc					\
umain.cc					\
uobject.cc					\
usyncclient.cc					\
usystem.cc					\
uvar.cc

liburbi_la_LIBADD = $(LIBADD)

# We install twice liburbi: once for application using it to
# communicate with a server, and once for remote-components (which
# also need to communicate with a kernel).  Formerly we had two
# libraries: liburbi and libkernel-remote.  The latter used to be the
# former plus umain.cc.  It also was a problem with Libtool which was
# shooting in its foot in -j2 when linking two libraries using the
# same sub libraries: it created concurrently twice the same directory
# with the library files in it.
#
# So now we have a single library, but to users of liburbi do not want
# to use umake so the lib should be in a std location (libdir), and
# remote component creators should deal with umake which wants the
# libraries in its own tree (envdir).
#
# Therefore we have a single library that we install twice.
#
# Note that the kernel itself seems to do something similar in its
# install-exec-hook, which I don't entirely understand.  If something
# is wrong here, fetch inspiration there.
install-exec-hook:
	$(mkdir_p) $(DESTDIR)$(libdir)
	cd $(DESTDIR)$(libdir) && $(LN_S) -f $(DESTDIR)../$(topenvdir)/liburbi.la
	cd $(DESTDIR)$(libdir) && $(LN_S) -f $(DESTDIR)../$(topenvdir)/liburbi.a
