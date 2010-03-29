# To make this package available through apt, you first need to install
# dpkg-scanpackages (dpkg-dev) and checkinstall.

all: deb
install:
	# Empty on purpose
deb:
	echo "Meta-package for urbi kernel dependencies." > description-pak
	checkinstall --install=no --showinstall=no --pkgname=urbi-deps  \
		     --pkgversion=2.0  --maintainer=kernel@lists.gostai.com \
			 --nodoc                                                \
		     --requires="autoconf, automake, bc, ccache, coreutils, \
				 cvs, g++-4.2, gettext, git-core, graphviz,         \
				 help2man, imagemagick, libboost-dev,               \
				 python-yaml, python2.6, socat, tex4ht,             \
				 texinfo, texlive-base, texlive-latex-extra,        \
				 valgrind, gnuplot, transfig, flex, libxext-dev,    \
                                 x11proto-xext-dev"
	rm description-pak
	dpkg-scanpackages -m . /dev/null | gzip -9c > Packages.gz

