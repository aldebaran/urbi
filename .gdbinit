# These lines are tricks to work with the source files of the gdb extensions
# instead of the installed files.  This gives opportunity to develop new
# extensions for the debugger without making new installs.
python urbi_source_dir = ''
python libport_source_dir = 'sdk-remote/libport/'
source sdk-remote/libport/share/gdb/python/libport.so-gdb.py
source share/gdb/python/libuobject.so-gdb.py

### These lines are useful for testing gdb extensions.
# set breakpoint pending on
# break urbi::object::system_backtrace
# set breakpoint pending off
# set args -e 'for(var i: [1]) backtrace;' -i
# run
# urbi-stack

