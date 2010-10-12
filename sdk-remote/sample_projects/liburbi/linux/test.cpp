#include <urbi/uclient.hh>

using namespace urbi;

int
main()
{
  UClient robotC;
  robotC.send( "motors on;" );
  robotC.send( "play(\"bark.wav\");" );
  robotC.send( "headPan = 0;" );
  robotC.send( "neck=0;" );
  urbi::execute();
}
