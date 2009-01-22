pdf_DATA += specs/urbi-specs.pdf
urbi_specs_sources = $(call ls_files,specs/*.tex)
urbi_specs_deps = $(urbi_specs_sources) $(call ls_files,*.sty)

# texi2pdf does not like directories that do not exist.  Don't depend
# on the directory, as only its existence matters, not its time
# stamps.
specs/urbi-specs.pdf: specs/.stamp $(urbi_specs_deps)

specs/.stamp:
	-mkdir specs
	test -f $@ || touch $@
