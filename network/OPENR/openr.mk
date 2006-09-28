## --------------------- ##
## libkernet for openr.  ##
## --------------------- ##

dist_libkernel_la_SOURCES +=			\
network/OPENR/aiboconnection.h			\
network/OPENR/aiboconnection.cc

libkernel_la_CPPFLAGS +=				\
	-I$(OPEN_R_SDK)/OPEN_R/include/R4000	\
	-I$(OPEN_R_SDK)/OPEN_R/include/MCOOP	\
	-I$(OPEN_R_SDK)/OPEN_R/include


# Kludge to install userver.h.
openrdir = $(kernelincludedir)/network/OPENR
openr_HEADERS = network/OPENR/aiboconnection.h
