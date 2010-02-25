                                                -*- outline -*-

This package contains the Urbi Software Development Kit.

* Installation

The package is relocatable, i.e., it does not need to be put at a
specific location, nor does it need special environment variables to
be set.  It is not necessary to be a super-user to install it.  The
root of the package, denoted by URBI_ROOT hereafter, is the absolute
name of the directory which contains the package.

** GNU/Linux and Mac OS X

Decompress the package where you want to install it:

  $ rm -rf URBI_ROOT
  $ cd /tmp
  $ tar zxf .../urbi-sdk-2.0-linux-x86-gcc4.tar.gz
  $ mv urbi-sdk-2.0-linux-x86-gcc4 URBI_ROOT

This directory, URBI_ROOT, should contain "bin", "FAQ.txt" and so
forth.  Do not move things around inside this directory.  In order to
have an easy access to the Urbi programs, set up your PATH:

  $ export PATH="\var{urbi\_root}/bin:$PATH"

To check that the package is functional:

  # Check that urbi is properly set up.
  $ urbi --version

  # Check that urbi-launch is properly installed.
  $ urbi-launch --version
  # Check that urbi-launch can find its dependencies.
  $ urbi-launch -- --version

  # Check that Urbi can compute.
  $ urbi -e '1+2*3; shutdown;'
  [00000175] 7

** Windows

Decompress the zip file wherever you want.  Execute the script
"urbi.bat", located at the root of the uncompressed package. It should
open a terminal with an interactive Urbi session.


* Documentation

The following documents will guide you to install and run Urbi.

** README.txt
These notes.

** RELEASE-NOTES.txt
A list of changes since the previous releases.

** share/doc/urbi-sdk/urbi-sdk.pdf
The complete Urbi SDK documentation.  It includes:

*** A tutorial about urbiscript

*** A chapter on the installation

*** Frequently Asked Questions

*** The urbiscript reference manual
the language, the standard libraries, and the tools.

** REPORTING-BUGS.txt
How to report bugs about Urbi SDK.
