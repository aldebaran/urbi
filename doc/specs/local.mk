pdf_DATA += specs/urbi-specs.pdf
urbi_specs_sources = $(call ls_files,specs/*.tex)
urbi_specs_deps = $(urbi_specs_sources) $(call ls_files,*.sty)

EXTRA_DIST += $(urbi_specs_sources) $(urbi_specs_deps)

# texi2pdf does not like directories that do not exist.  Don't depend
# on the directory, as only its existence matters, not its time
# stamps.
specs/urbi-specs.pdf: specs/.stamp $(urbi_specs_deps)

specs/.stamp:
	-mkdir specs
	test -f $@ || touch $@
