#include <vector>

#include "urbi/uclient.hh"
#include "parse-header.hh"

void
usage (const char* name, int status)
{
  printf("usage %s "
	 "infile outfile [jointid] [startval] [direction] [numcycle]\n"
	 "\tDetect and extract cycles in an urbi recorded file\n"
	 "\tJointid is the joint used to detect cycles (0-based value,"
	 " see urbirecord.cpp for id/name correspondance)\n"
	 "\tStartval is the value that will mark the beginning of a cycle"
	 " when reached by the joint, in the direction defined by the"
	 " 'direction' parameter\n"
	 "\tnumcycle is the number of the cycle that will be written to"
	 " 'outfile'\n"
	 "\tSet startval to '-' for 'first value seen'\n", name);
  if (status)
    exit(status);
}

int main(int argc, char * argv[])
{
  FILE* inf = 0;
  FILE* ouf = 0;
  urbi::reverse_cycle_scale_getopt (argc, argv, 3, &inf, &ouf);

  int joint = 0;
  int wantedcycle = 2;
  float startval = 0.0;
  bool init = false; //not found yet startval on joint
  bool gotSign = false;
  bool gotLastVal = false;
  bool cyclesgn = false;

  if (argc > 3)
    sscanf(argv[3], "%d", &joint);
  if (argc > 4)
  {
    if (STREQ(argv[4], "-"))
      init = true;
    else
      sscanf(argv[4], "%f", &startval);
  }
  if (argc > 5)
  {
    int v = 0;
    sscanf(argv[5], "%d", &v);
    if (v != 0)
    {
      gotSign = true;
      cyclesgn = (v < 0);
    }
  }
  if (argc > 6)
    sscanf(argv[6], "%d", &wantedcycle);

  float lastval = 0.0;
  std::vector<urbi::UCommand> buff (urbi::devCount);
  int cycle = 0;

  int buffTime = 0;

  int basetime = 0;
  for (int i = 0; i < urbi::devCount; ++i)
    buff[i].timestamp = 0;
  //read and handle by block of commands with same timestamp.
  //init:
  urbi::UCommand uc;
  fread(&uc, sizeof (urbi::UCommand), 1, inf);
  buffTime = uc.timestamp;
  buff[uc.id] = uc;
  while (true)
  {
    int ok = fread(&uc, sizeof (urbi::UCommand), 1, inf);
    if (ok && !basetime)
      basetime = uc.timestamp;
    if (ok && buffTime == 0)
      buffTime = uc.timestamp;
    if (ok && uc.timestamp == buffTime)
    {
      buff[uc.id] = uc;
      continue;
    }

    if (init)
    {
      //initialize asap<-now

      if (buff[joint].timestamp == 0)
      {
	//cant do anything
	for (int i = 0; i < urbi::devCount; ++i)
	  buff[i].timestamp = 0;
	buff[uc.id] = uc;
	buffTime = 0;
	continue;
      }

      startval = buff[joint].value.angle;
      gotSign = false;
      init = false;
      gotLastVal = true;
      lastval = startval;
      ++cycle;
      fprintf(stderr, "cycle %d starts at %d\n", cycle, buffTime - basetime);
    }


    if (gotLastVal
	&& (!gotSign || (cyclesgn ^ (lastval<startval)))
	&& ((lastval < startval && buff[joint].value.angle >= startval)
	    || (lastval > startval && buff[joint].value.angle <= startval)))
    {
      cyclesgn = (lastval > startval);
      gotSign = true;
      ++cycle;
      fprintf(stderr, "cycle %d starts at %d\n", cycle, buffTime - basetime);
    }

    if (buff[joint].timestamp != 0)
    {
      lastval = buff[joint].value.angle;
      gotLastVal = true;
    }
    if (cycle == wantedcycle)
      for (int i = 0; i < urbi::devCount; ++i)
	if (buff[i].timestamp!=0)
	{
	  buff[i].timestamp -= basetime;
	  fwrite(&buff[i], sizeof (urbi::UCommand), 1, ouf);
	}

    //flush buffer
    for (int i = 0; i < urbi::devCount; ++i)
      buff[i].timestamp = 0;
    buff[uc.id] = uc;
    buffTime = 0;
    if (!ok)
      break;
  }
  fclose(inf);
  fclose(ouf);
}
