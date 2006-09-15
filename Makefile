################################################################################
#
# URBI - (c) Jean-Christophe Baillie, 2004-2005 
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
################################################################################
# usage: call with environnment variables OS, NETWORK and PARSER
BISON?=bison -v # It MUST be version 1.875d 
FLEX?=flex    # It MUST be version 2.5.4a 
LIBS=-lm
OPTIM?=-O2
OBJEXT ?= .o

################################################################################

ifeq (ww$(OBJEXT),ww)
  echo fatal error
  false;
endif

KERNEL_SOURCES= uconnection.cc \
             ughostconnection.cc \
             userver.cc \
             ucommand.cc \
             ucommandqueue.cc \
             ustring.cc \
             ufunction.cc \
             uvariable.cc \
             uqueue.cc \
             uexpression.cc \
             uvariablelist.cc \
             unamedparameters.cc \
             uvalue.cc \
             uobj.cc \
	     ugroup.cc \
             uproperty.cc \
             uvariablename.cc \
             ubinary.cc \
	     ubinder.cc \
             ucallid.cc \
	     uobject/uobject.cc \
	     uobject/uvar.cc \
	     uobject/common_uvalue.cc \
	     memorymanager/memorymanager.cc \
	     ufloat.cc


KERNEL_FILES=$(KERNEL_SOURCES:.cc=$(OBJEXT))


CPPFLAGS+= -I./network/$(NETWORK) -I./parser/$(PARSER) -I. -I./uobject 
#CPPFLAGS+=-DENABLE_BLOCKMEMMNGR -DFLOAT_DOUBLE
CPPFLAGS+=-DFLOAT_DOUBLE

CXXFLAGS+= -Wno-deprecated\
        $(OPTIM) 


.PHONY:: all install clean

all: build/buildnumber parser network build/libkernelurbi-$(NETWORK)-$(PARSER).a

include network/$(NETWORK)/Makefile.defs
include parser/$(PARSER)/Makefile.defs


NETWORK_SRC?=$(wildcard network/$(NETWORK)/*.cpp)  $(wildcard network/$(NETWORK)/*.cc)
NETWORK_OBJS?=$(NETWORK_SRC:.cc=$(OBJEXT))
PARSER_SRC?=$(wildcard parser/$(PARSER)/*.cpp)  $(wildcard parser/$(PARSER)/*.cc)

PARSER_OBJS?=$(PARSER_SRC:.cc=$(OBJEXT))

ifeq ($(OS),macosx)
	LDFLAGS := $(LDFLAGS) -r -all_load
else
	LDFLAGS := $(LDFLAGS) -r -whole-archive
endif 

CPPFLAGS += -DOS=$(OS)
################################################################################

build/buildnumber: $(KERNEL_SOURCES) parser/$(PARSER)/*.y parser/$(PARSER)/*.l
	LANG=en && svn info | sed -e "s/é/e/" | grep Revision | sed -e "s/Revision: //" > build/buildnumber 
	echo '"' `cat build/buildnumber` '"'  > buildversion.h

network: $(NETWORK_OBJS)

parser: $(PARSER_OBJS)

build/libkernelurbi-$(NETWORK)-$(PARSER).a: $(KERNEL_FILES) $(PARSER_OBJS) $(NETWORK_OBJS) 
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o userver$(OBJEXT) -c userver.cc
	$(LD) $(LDFLAGS)  -o build/libkernelurbi-$(NETWORK)-$(PARSER)-$(OS).a $^
#	$(MKLIB) build/libkernelurbi-$(NETWORK)-$(PARSER)-$(OS).a $^


%$(OBJEXT): %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%$(OBJEXT): %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%$(OBJEXT): %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

depend::
	makedepend -- $(CXXFLAGS) -- *.cc *.c

clean:: $(PARSER)-clean $(NETWORK)-clean
	makedepend
	rm parser/$(PARSER)/*$(OBJEXT)
	rm network/$(NETWORK)/*$(OBJEXT)
	rm -f uobject/*$(OBJEXT) memorymanager/*$(OBJEXT) *$(OBJEXT) *.elf *.snap.cc *.bak *~ utoken.yy.* ugrammar.tab.* *.a

################################################################################
# DO NOT DELETE
