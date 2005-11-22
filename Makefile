################################################################################
#
# URBI - (c) Jean-Christophe Baillie, 2004 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
################################################################################

BISON?=bison -v # It MUST be version 1.875d 
FLEX?=flex    # It MUST be version 2.5.4a 
LIBS=-lm
OPTIM?=-O2

################################################################################



KERNEL_SOURCES=udevice.cc \
             uconnection.cc \
             ughostconnection.cc \
             userver.cc \
             ucommand.cc \
             ucommandqueue.cc \
             umixqueue.cc \
             ustring.cc \
             ufunction.cc \
             uvariable.cc \
             uqueue.cc \
             uexpression.cc \
             uvariablelist.cc \
             unamedparameters.cc \
             uvalue.cc \
             ugroup.cc \
             uproperty.cc \
             uvariablename.cc \
             ubinary.cc \
             ucallid.cc \
             ugroupdevice.cc

KERNEL_FILES=$(KERNEL_SOURCES:.cc=.o)


CPPFLAGS+= -I./network/$(NETWORK) -I./parser/$(PARSER) -I.
CXXFLAGS+= -Wno-deprecated\
        $(OPTIM) 


.PHONY:: all install clean

all: build/buildnumber parser network build/libkernelurbi-$(NETWORK)-$(PARSER).a



include network/$(NETWORK)/Makefile.defs
include parser/$(PARSER)/Makefile.defs



NETWORK_SRC?=$(wildcard network/$(NETWORK)/*.cpp)  $(wildcard network/$(NETWORK)/*.cc)



NETWORK_OBJS?=$(NETWORK_SRC:.cc=.o)



PARSER_SRC?=$(wildcard parser/$(PARSER)/*.cpp)  $(wildcard parser/$(PARSER)/*.cc)



PARSER_OBJS?=$(PARSER_SRC:.cc=.o)


################################################################################

build/buildnumber: $(KERNEL_SOURCES)
	expr 1 + `cat build/buildnumber` > build/tbuildnumber && mv -f build/tbuildnumber  build/buildnumber
	echo '"' `cat build/buildnumber` '"'  > buildversion.h

network: $(NETWORK_OBJS)

parser: $(PARSER_OBJS)

build/libkernelurbi-$(NETWORK)-$(PARSER).a: $(KERNEL_FILES) $(PARSER_OBJS) $(NETWORK_OBJS) 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o userver.o -c userver.cc
	$(LD) -r -whole-archive -o build/libkernelurbi-$(NETWORK)-$(PARSER).a $^


%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

depend::
	makedepend -- $(CXXFLAGS) -- *.cc *.c

clean:: $(PARSER)-clean $(NETWORK)-clean
	makedepend
	rm parser/$(PARSER)/*.o
	rm network/$(NETWORK)/*.o
	rm -f *.o *.elf *.snap.cc *.bak *~ utoken.yy.* ugrammar.tab.* *.a

################################################################################
# DO NOT DELETE
