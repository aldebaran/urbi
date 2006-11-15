## ---------------------- ##
## libkernet for bsdnet.  ##
## ---------------------- ##

dist_libkernel_la_SOURCES +=			\
network/bsdnet/connection.hh			\
network/bsdnet/connection.cc			\
network/bsdnet/network.hh			\
network/bsdnet/network.cc

# Kludge to install userver.h.
bsdnetdir = $(kernelincludedir)/network/bsdnet
bsdnet_HEADERS = 				\
network/bsdnet/connection.hh			\
network/bsdnet/network.hh
