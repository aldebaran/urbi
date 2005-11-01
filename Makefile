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

KERNEL_FILES=ugrammar.tab.o \
             utoken.yy.o \
	     udevice.o \
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

################################################################################

.PHONY:: all install clean

all:: libkernelurbi.a

libkernelurbi.a: $(KERNEL_FILES)
	./addnb
	$(CXX) $(CXXFLAGS) -o userver.o -c userver.cc
	$(LD) -r -whole-archive -o $@ $^

ugrammar.tab.cc: ugrammar.y
	$(BISON) --defines ugrammar.y
	mv ugrammar.tab.c ugrammar.tab.cc

utoken.yy.cc: utoken.l ugrammar.tab.cc
	$(FLEX) -+ utoken.l
	sed -e 's/class istream;/#include <istream.h>/' lex.yy.cc > utoken.yy.cc
	rm -f lex.yy.cc

%.o: %.cc
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
