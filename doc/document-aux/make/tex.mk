# Make sure we don't become promoted as default target.
all:
.PHONY: all

share_style_dir = $(share_dir)/styles
share_bib_dir = $(share_dir)/bib
ChangeLog ?= ChangeLog

# Verbosity of texi2dvi (see the section on silent rules in Automake's
# manual).
AM_DEFAULT_VERBOSITY ?= 1
texi2dvi_verbose = $(texi2dvi_verbose_$(V))
texi2dvi_verbose_ = $(texi2dvi_verbose_$(AM_DEFAULT_VERBOSITY))
texi2dvi_verbose_0 = >$@.log 2>&1

TEXINDYFLAGS = --module makeindex

TEXI2DVI = max_print_line=32000 $(share_bin_dir)/texi2dvi
TEXI2DVIFLAGS = --tidy --build-dir=tmp.t2d --batch $(texi2dvi_verbose)

TEXLOGANALYZER = $(share_bin_dir)/texloganalyser
TEXLOGANALYZERFLAGS = -w

# Find the style files, the bibliography files, and the figures.
# Arguably, we should always use qualified names, but this is annoying
# (at least for style files, it is acceptable for bib files).  We do
# it nevertheless for figures, because experience has already bitten
# me (AD).  So use \includegraphics{figs/lrde-big} instead of
# {lrde-big}.
TEXI2DVIFLAGS += -I $(share_style_dir)// -I $(share_bib_dir) -I $(share_dir)

TEXI2PDF = $(TEXI2DVI) --pdf
TEXI2PDFFLAGS = $(TEXI2DVIFLAGS)

TEXI2HTML = $(TEXI2DVI) --html
TEXI2HTMLFLAGS = $(TEXI2DVIFLAGS)

TEXI2TEXT = $(TEXI2DVI) --text
TEXI2TEXTFLAGS = $(TEXI2DVIFLAGS)

TEXI2INFO = $(TEXI2DVI) --info
TEXI2INFOFLAGS = $(TEXI2DVIFLAGS)

share_tex_dependencies =				\
  $(STYLES)						\
  $(wildcard $(share_style_dir)/* $(share_bib_dir)/*)


## ------- ##
## *.tex.  ##
## ------- ##

%.dvi: %.tex $(share_tex_dependencies)
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)rm -f $@ $@.log
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2DVI) $(TEXI2DVIFLAGS) -o $@ $<; then			\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi

%.html: %.tex $(share_tex_dependencies)
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)rm -f $@ $@.log
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2HTML) $(TEXI2HTMLFLAGS) -o $@ $<; then		\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi

%.info: %.tex $(share_tex_dependencies)
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)rm -f $@ $@.log
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2INFO) $(TEXI2INFOFLAGS) -o $@ $<; then		\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi

%.pdf: %.tex $(share_tex_dependencies)
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)rm -f $@ $@.log
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2PDF) $(TEXI2PDFFLAGS) -o $@ $<; then			\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi
CLEANFILES += $(pdf_DATA:=.log)


%.txt: %.tex $(share_tex_dependencies)
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)rm -f $@ $@.log
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2TEXT) $(TEXI2TEXTFLAGS) -o $@ $<; then		\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi


## ------- ##
## *.ltx.  ##
## ------- ##

%.dvi: %.ltx $(share_tex_dependencies)
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)rm -f $@ $@.log
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2DVI) $(TEXI2DVIFLAGS) -o $@ $<; then			\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi

%.info: %.ltx $(share_tex_dependencies)
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)rm -f $@ $@.log
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2HTML) $(TEXI2HTMLFLAGS) -o $@ $<; then		\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi

%.html: %.ltx $(share_tex_dependencies)
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)rm -f $@ $@.log
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2HTML) $(TEXI2HTMLFLAGS) -o $@ $<; then		\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi

%.pdf: %.ltx $(share_tex_dependencies)
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)rm -f $@ $@.log
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2PDF) $(TEXI2PDFFLAGS) -o $@ $<; then			\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi

%.txt: %.ltx $(share_tex_dependencies)
	$(AM_V_GEN)$(ENSURE_TARGET_DIR)
	$(AM_V_at)rm -f $@ $@.log
	$(AM_V_at)							\
	$(if $(TEX_ENVIRONMENT), export $(TEX_ENVIRONMENT);)		\
	if $(TEXI2TEXT) $(TEXI2TEXTFLAGS) -o $@ $<; then		\
	  test ! -f $@.log ||						\
	    $(TEXLOGANALYZER) $(TEXLOGANALYZERFLAGS) $@.log >&2;	\
	else								\
	  sta=$$?;							\
	  test ! -f $@.log ||						\
	    cat >&2 $@.log;						\
	  exit $$sta;							\
	fi




## --------- ##
## rev.sty.  ##
## --------- ##

rev.sty: $(ChangeLog)
	$(AM_V_GEN) if svn --version >/dev/null 2>&1; then	\
	  $(share_bin_dir)/generate-rev-sty $< >$@;		\
	elif test -f $@; then					\
	  touch $@;						\
	else							\
	  echo '@newcommand{@SvnRev}{}' | tr '@' '\\' >$@;	\
	fi

CLEANFILES += rev.sty

tex-mostlyclean:
	rm -rf tmp.t2d
.PHONY: tex-mostlyclean
# mostlyclean-local is an Automake special target.
mostlyclean-local: tex-mostlyclean
.PHONY: mostlyclean-local
