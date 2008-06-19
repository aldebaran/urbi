## -------- ##
## papers.  ##
## -------- ##
papersdir = /lrde/doc/lrde/papers

install: papers-install
papers-install: $(papers_DATA)
	if test -n '$($(@:-install=_DATA))'; then			\
	  install -m 644 $($(@:-install=_DATA)) $($(@:-install=dir));	\
	fi

## ------- ##
## dload.  ##
## ------- ##
dloaddir = /lrde/dload/papers

install: dload-install
dload-install: $(dload_DATA)
	if test -n '$($(@:-install=_DATA))'; then			\
	  install -m 644 $($(@:-install=_DATA)) $($(@:-install=dir));	\
	fi


.PHONY: install dload-install papers-install
