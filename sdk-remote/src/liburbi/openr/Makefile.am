CXX		= $(OPEN_R_SDK)/bin/mipsel-linux-g++
CC		= $(OPEN_R_SDK)/bin/mipsel-linux-gcc
STRIP		= $(OPEN_R_SDK)/bin/mipsel-linux-strip
MKBIN		= $(OPEN_R_SDK)/OPEN_R/bin/mkbin
STUBGEN		= $(OPEN_R_SDK)/OPEN_R/bin/stubgen2
MKBINFLAGS	= -p $(OPEN_R_SDK)
LIBS		= -L$(OPEN_R_SDK)/OPEN_R/lib -lObjectComm -lOPENR -lInternet -lantMCOOP
# library (.a) file creation command
AR_OPENR= $(OPEN_R_SDK)/bin/mipsel-linux-ar rc
# second step in .a creation (use "touch" if not needed)
RANLIB_OPENR= $(OPEN_R_SDK)/bin/mipsel-linux-ranlib


LIBJPEGDIR	= ../../../lib/jpeg-6b
LIBJPEG		= $(LIBJPEGDIR)/libjpeg.mips.a

CXXFLAGS+= \
	-O2 \
	-I. \
	-I$(OPEN_R_SDK)/OPEN_R/include/R4000 \
	-I$(OPEN_R_SDK)/OPEN_R/include/MCOOP \
	-I$(OPEN_R_SDK)/OPEN_R/include \
	-I$(LIBJPEGDIR)	\
	-I$(LIBURBIDIR)

SOURCES =					\
  uclient.cc					\
  aiboconnection.cc				\
  uqueue.cc					\
  ../uabstractclient.cpp

OBJA = $(SOURCES:.cc=.mips.o)
OBJ = $(OBJA:.cpp=.mips.o)

#
# When OPENR_DEBUG is defined, OSYSDEBUG() is available.
#
CXXFLAGS += -DOPENR_DEBUG



all: liburbi.mips.a

liburbi.mips.a: $(OBJ) $(LIBJPEG) 
	$(RM) liburbi.mips.a
	cp ../../../lib/jpeg-6b/libjpeg.mips.a ./liburbi.mips.a
	$(AR_OPENR) liburbi.mips.a  $(OBJ)
	$(RANLIB_OPENR) liburbi.mips.a

%.mips.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $^

%.mips.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $^

%.mips.o: %.c
	$(CC) $(CXXFLAGS) -o $@ -c $^


$(LIBJPEG):
	$(RM) $(LIBJPEG)
	make -C $(LIBJPEGDIR) -f makefile.aibo

clean-local:
	make -C $(LIBJPEGDIR) -f makefile.aibo clean
	rm -f *.mips.o *.BIN *.elf *.snap.cc *~ ../*.mips.o

install-exec-local:  liburbi.mips.a
	$(mkinstalldirs) $(bindir)
	echo "cp $(datadir)/Makefile.aibo ./ && make -f  ./Makefile.aibo prepare" '$$*' >  $(bindir)/make.openr
	chmod 755  $(bindir)/make.openr
	$(mkinstalldirs) $(libdir)
	$(INSTALL) liburbi.mips.a  $(libdir)
	$(mkinstalldirs) $(datadir)
	$(INSTALL) Makefile.aibo  CLIENT.ocf stub.cfg CLIENT.h CLIENT.cc $(datadir)
	$(mkinstalldirs) $(includedir)/openr
	$(INSTALL) uclient.h aiboconnection.h uconnection.h uqueue.h utypes.h $(includedir)/openr
	$(INSTALL) ../uabstractclient.h  $(includedir)

uninstall-local:
	$(rm) $(libdir)/liburbi.mips.a
	$(rm) $(datadir)/Makefile.aibo
	$(rm) $(bindir)/make.openr
