SUBDIRS = .

AM_CPPFLAGS = -I$(top_srcdir)/src/liburbi/swig
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS) -fpic

bin_SCRIPTS =				\
		sample.cs		\
		sample.java		\
		sample.lua		\
		sample.php		\
		sample.pl		\
		sample.py		\
		sample.rb

EXTRA_DIST = liburbi.i

if BINDING_CSHARP
	lib_LTLIBRARIES += liburbiCSharp.la
	liburbiCSharp_la_SOURCES = liburbi_csharp.cc
endif

if BINDING_JAVA
	lib_LTLIBRARIES += liburbiJava.la
	liburbiJava_la_SOURCES = liburbi_java.cc
	liburbiJava_la_CXXFLAGS = -I/usr/local/jdk1.5.0_08/include \
				  -I/usr/local/jdk1.5.0_08/include/linux
endif

if BINDING_LUA
	lib_LTLIBRARIES += liburbiLua.la
	liburbiLua_la_SOURCES = liburbi_lua.cc
	liburbiLua_la_CXXFLAGS = -I/usr/include/lua5.1
endif

if BINDING_PERL
	lib_LTLIBRARIES += liburbiPerl.la
	liburbiPerl_la_SOURCES = liburbi_perl.cc
	liburbiPerl_la_CXXFLAGS = -I/usr/lib/perl/5.8.7/CORE
endif

if BINDING_PHP
	lib_LTLIBRARIES += liburbiPhp.la
	liburbiPhp_la_SOURCES = liburbi_php.cc
	liburbiPhp_la_CXXFLAGS =		\
	    -I/usr/include/php4			\
	    -I/usr/include/php4/main		\
	    -I/usr/include/php4/regex		\
	    -I/usr/include/php4/TSRM		\
	    -I/usr/include/php4/Zend
endif

if BINDING_PYTHON
	lib_LTLIBRARIES += liburbiPython.la
	liburbiPython_la_SOURCES = liburbi_python.cc swig_python.hh
	liburbiPython_la_CXXFLAGS = -I/usr/include/python2.4
endif

if BINDING_RUBY
	lib_LTLIBRARIES += liburbiRuby.la
	liburbiRuby_la_SOURCES = liburbi_ruby.cc swig_ruby.hh
	liburbiRuby_la_CXXFLAGS = -I/usr/lib/ruby/1.8/i486-linux
endif


liburbi_csharp.cc: liburbi.i
	$(SWIG) -csharp $(SWIG_ARGS) -o $@ $<

liburbi_java.cc: liburbi.i
	$(SWIG) -java $(SWIG_ARGS) -o $@ $<

liburbi_lua.cc: liburbi.i
	$(SWIG) -lua $(SWIG_ARGS) -o $@ $<

liburbi_perl.cc: liburbi.i
	$(SWIG) -perl $(SWIG_ARGS) -o $@ $<

liburbi_php.cc: liburbi.i
	$(SWIG) -php4 $(SWIG_ARGS) -o $@ $<

liburbi_python.cc: liburbi.i swig_python.h
	$(SWIG) -python $(SWIG_ARGS) -o $@ $<

liburbi_ruby.cc: liburbi.i swig_ruby.h
	$(SWIG) -ruby $(SWIG_ARGS) -o $@ $<
