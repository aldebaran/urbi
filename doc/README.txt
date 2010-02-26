                                                -*- outline -*-

This package contains the Urbi Software Development Kit.

  Table of Contents of this file:
   * Installation
   * Documentation
   * Frequently Asked Questions
   * Reporting Bugs

* Installation

The package is relocatable, i.e., it does not need to be put at a
specific location, nor does it need special environment variables to
be set.  It is not necessary to be a super-user to install it.  The
root of the package, denoted by <urbi-root> hereafter, is the absolute
name of the directory which contains the package.

** GNU/Linux and Mac OS X

Decompress the package where you want to install it:

  $ rm -rf <urbi-root>
  $ cd /tmp
  $ tar zxf .../urbi-sdk-2.0-linux-x86-gcc4.tar.gz
  $ mv urbi-sdk-2.0-linux-x86-gcc4 <urbi-root>

This directory, <urbi-root>, should contain "bin", "FAQ.txt" and so
forth.  Do not move things around inside this directory.  In order to
have an easy access to the Urbi programs, set up your PATH:

  $ export PATH="<urbi-root>/bin:$PATH"

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

* Frequently Asked Questions

See the chapter "Frequently Asked Questions" in
share/doc/urbi-sdk/urbi-sdk.pdf.

* Reporting Bugs

Bug reports should be sent to "kernel-bugs@lists.gostai.com", it will
be addressed as fast as possible.  Please, be sure to read the FAQ,
and to have checked that no more recent release fixed your issue.

Each bug report should contain a self-contained example, which can be
tested by our team. Using self-contained code, that is code which does
not depend on other code, helps ensuring that we will be able to
duplicate the problem and analyze it promptly. It will also helps us
integrating the code snippet into our non-regression test suite so
that the bug does not reappear in the future.

If it appears that your report identifies a bug in the Urbi kernel or
its dependencies, we will prepare a fix to be integrated in a later
release. If the bug takes some time to fix, we may provide you with a
workaround so that your developments are not delayed.

In your bug report, make sure that you indicate the Urbi version you
are using (use "urbi --version" to check it out) and whether this bug
is blocking you or not. Also, please keep
"kernel-bugs@lists.gostai.com" in copy of all your correspondence, and
do not reply individually to a member of our team as this may slow
down the handling of the report.
