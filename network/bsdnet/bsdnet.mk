## ---------------------- ##
## libkernet for bsdnet.  ##
## ---------------------- ##

dist_libkernel_la_SOURCES +=			\
network/bsdnet/Connection.h			\
network/bsdnet/Connection.cc			\
network/bsdnet/network.h			\
network/bsdnet/network.cc

# Kludge to install userver.h.
bsdnetdir = $(kernelincludedir)/network/bsdnet
bsdnet_HEADERS = 				\
network/bsdnet/Connection.h			\
network/bsdnet/network.h
