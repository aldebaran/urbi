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

CXXFLAGS+= -Wno-deprecated\
        $(OPTIM) \
        -I.

KERNEL_FILES=udevice.o \
             uconnection.o \
             ughostconnection.o \
             userver.o \
             ucommand.o \
             ucommandqueue.o \
             umixqueue.o \
             ustring.o \
             ufunction.o \
             uvariable.o \
             uqueue.o \
             uexpression.o \
             uvariablelist.o \
             unamedparameters.o \
             uvalue.o \
             ugroup.o \
             uproperty.o \
             uvariablename.o \
             ubinary.o \
             ucallid.o

NETWORK_SRC=$(wildcard network/$(NETWORK)/*.cpp)  $(wildcard network/$(NETWORK)/*.cc)

NETWORK_OBJS=$(NETWORK_SRC:.*=.o);

PARSER_SRC=$(wildcard parser/$(PARSER)/*.cpp)  $(wildcard parser/$(PARSER)/*.cc)

PARSER_OBJS=$(PARSER_SRC:.*=.o);


################################################################################

.PHONY:: all install clean

all:: buildnumber network parser build/libkernelurbi-$(NETWORK).a

buildnumber: $(KERNEL_FILES) $(PARSER_SRC) $(NETWORK_SRC)
	expr 1 + $(cat build/buildnumber) > build/buildnumber
	sed -e "s/build.*\"/build $(cat build/buildnumber) \"/" > tmp && mv tmp version.h

network: $(NETWORK_OBJS)

parser: $(PARSER_OBJS)

libkernelurbi-$(NETWORK)-$(PARSER).a: $(KERNEL_FILES)  $(NETWORK_OBJS) $(PARSER_OBJS)
	$(CXX) $(CXXFLAGS) -o userver.o -c userver.cc
	$(LD) -r -whole-archive -o build/libkernelurbi-$(NETWORK)-$(PARSER).a $^

parser/bison/ugrammar.tab.cc: parser/bison/ugrammar.y
	cd parser/bison/ && $(BISON) --defines ugrammar.y
	mv parser/bison/ugrammar.tab.c parser/bison/ugrammar.tab.cc

parser/bison/utoken.yy.cc: parser/bison/utoken.l parser/bison/ugrammar.tab.cc
	$(FLEX) -+ utoken.l
	sed -e 's/class istream;/#include <istream.h>/' parser/bison/lex.yy.cc > parser/bison/utoken.yy.cc
	rm -f parser/bison/lex.yy.cc

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: %.c
	$(CC) $(CXXFLAGS) -o $@ -c $<

depend::
	makedepend -- $(CXXFLAGS) -- *.cc *.c

clean::
	makedepend
	rm -f *.o *.elf *.snap.cc *.bak *~ utoken.yy.* ugrammar.tab.* *.a

################################################################################
# DO NOT DELETE
