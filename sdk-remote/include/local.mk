## Copyright (C) 2008-2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

ucallbacks_hh = include/urbi/ucallbacks.hh

urbiinclude_HEADERS =				\
  $(ucallbacks_hh)				\
  include/urbi/customuvar.hh                    \
  include/urbi/customuvar.hxx                   \
  include/urbi/exit.hh				\
  include/urbi/export.hh			\
  include/urbi/fwd.hh				\
  include/urbi/input-port.hh			\
  include/urbi/input-port.hxx			\
  include/urbi/kernel-version.hh		\
  include/urbi/package-info.hh			\
  include/urbi/socket.hh			\
  include/urbi/qt_umain.hh			\
  include/urbi/uabstractclient.hh		\
  include/urbi/uabstractclient.hxx		\
  include/urbi/ubinary.hh			\
  include/urbi/ubinary.hxx			\
  include/urbi/ublend-type.hh			\
  include/urbi/uclient.hh			\
  include/urbi/ucontext-factory.hh		\
  include/urbi/ucontext-impl.hh			\
  include/urbi/ucontext-impl.hxx		\
  include/urbi/ucontext.hh			\
  include/urbi/ucontext.hxx			\
  include/urbi/uconversion.hh			\
  include/urbi/uexternal.hh			\
  include/urbi/uevent.hh			\
  include/urbi/uevent.hxx			\
  include/urbi/uimage.hh			\
  include/urbi/umain.hh				\
  include/urbi/umessage.hh			\
  include/urbi/umessage.hxx			\
  include/urbi/uobject-hub.hh			\
  include/urbi/uobject.hh			\
  include/urbi/uobject.hxx			\
  include/urbi/uprop.hh				\
  include/urbi/uproperty.hh			\
  include/urbi/urbi-root.hh			\
  include/urbi/usound.hh			\
  include/urbi/ustarter.hh			\
  include/urbi/ustarter.hxx			\
  include/urbi/usyncclient.hh			\
  include/urbi/utable.hh			\
  include/urbi/utag.hh				\
  include/urbi/utimer-callback.hh		\
  include/urbi/uvalue.hh			\
  include/urbi/uvalue.hxx			\
  include/urbi/uvalue-serialize.hh		\
  include/urbi/uvar.hh				\
  include/urbi/uvar.hxx                         \
  include/urbi/version-check.hh

nodist_urbiinclude_HEADERS =			\
  include/urbi/revision.hh


## -------------------- ##
## urbi/ucallbacks.hh.  ##
## -------------------- ##

EXTRA_DIST +=				\
  include/template_autogen.pl		\
  $(ucallbacks_hh).template
MAINTAINERCLEANFILES += $(ucallbacks_hh)

# We used to generate this file in builddir and not ship it, but it
# required to adjust -I paths to find it in builddir.  Since this was
# the only such file, it is easier to put it in srcdir (and ship it to
# comply with the Automake model).
$(srcdir)/$(ucallbacks_hh): $(ucallbacks_hh).template include/template_autogen.pl
	$(AM_V_GEN)
	$(AM_V_at)rm -f $@.tmp $@
	$(AM_V_at)$(mkdir_p) $$(dirname "$@.tmp")
	$(AM_V_at)$(srcdir)/include/template_autogen.pl \
	  $(srcdir)/$(ucallbacks_hh).template >$@.tmp
# Avoid accidental edition.
	$(AM_V_at)chmod a-w $@.tmp
	$(AM_V_at)$(top_srcdir)/build-aux/bin/move-if-change --color $@.tmp $@
	$(AM_V_at)touch $@

