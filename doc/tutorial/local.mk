pdf_DATA += tutorial/urbi-tutorial.pdf
urbi_tutorial_sources = $(call ls_files,tutorial/*.tex)
urbi_tutorial_deps = $(urbi_tutorial_sources) # $(call ls_files,tutorial/*.sty)

# texi2pdf does not like directories that do not exists.  Don't depend
# on the directory, as only its existence matters, not its time
# stamps.
tutorial/urbi-tutorial.pdf: tutorial/.stamp $(urbi_tutorial_deps)

tutorial/.stamp:
	-mkdir tutorial
	test -f $@ || touch $@
