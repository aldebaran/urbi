// The purpose of this empty file is simply to activate the linking of
// libkernel, which contains main and everything.  Some old
// architectures (AIX 3.2) used to dislike empty compilation units, in
// which case just introduce some dummy variable or function.

// Microsoft compiler does not allow main to be in a library.
// So we define one here.

#include <urbi/uobject.hh>

int main(int argc, const char* argv[]){
  return urbi::main(argc, argv);
  }

