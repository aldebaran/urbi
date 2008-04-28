#include <cstdlib>
#include <libport/cstdio>
#include <vector>

#include "urbi/uclient.hh"
#include "parse-header.hh"

void
usage (const char* name, int status)
{
  printf("usage %s infile outfile factor [interpolate]\n"
	 "\ttime-scale the urbi file by given factor (>1 means slow-down) \n"
	 "\tInterpolation will only work with integer factors\n", name);
  if (status)
    exit(status);
}

int
main(int argc, char * argv[])
{
  FILE* inf = 0;
  FILE* ouf = 0;
  urbi::reverse_cycle_scale_getopt (argc, argv, 4, &inf, &ouf);

  float scale = 2;
  int interpolate = 0;
  if (argc > 4)
    interpolate = strtol(argv[4], NULL, 0);
  printf("interpolation set to %d\n", interpolate);
  //if no move greater than atresh cut the line
  sscanf(argv[3], "%f", &scale);
  urbi::UCommand uc;
  int starttime = -1;
  std::vector<urbi::UCommand> lastuc (urbi::devCount);
  std::vector<urbi::UCommand> nextuc (urbi::devCount);
  for (int i = 0; i < urbi::devCount; ++i)
    lastuc[i].timestamp = -1;
  for (int i = 0; i < urbi::devCount; ++i)
    nextuc[i].timestamp =- 1;


  int inittime = 0;
  bool first_time = true;
  while (fread(&uc, sizeof (urbi::UCommand), 1, inf) == 1)
  {
    if (starttime == -1)
      starttime = uc.timestamp;
    if (interpolate && (lastuc[uc.id].timestamp == -1))
    {
      if (first_time == true)
      {
	inittime = uc.timestamp;
	first_time = false;
      }
      fprintf(stderr, "first command for %d at %d   (inittime %d)\n",
	      uc.id, uc.timestamp, inittime);
      if (inittime != uc.timestamp)
      {
	// trouble, missed some commands at start
	lastuc[uc.id] = uc;
	lastuc[uc.id].timestamp =
	  (int) (((float)(inittime) - ((float)starttime)) * scale);
	fprintf(stderr, "miss: adding command for id %d at time %d\n",
		uc.id, lastuc[uc.id].timestamp);
	//go on processing uc
      }
      else
      {
	uc.timestamp =
	  (int) (((float)(uc.timestamp) - ((float)starttime)) * scale);
	fwrite(&uc, sizeof (urbi::UCommand), 1, ouf);
	lastuc[uc.id] = uc;
	continue;
      }
    }
    uc.timestamp = (int) (((float)(uc.timestamp) - ((float)starttime)) * scale);
    if (interpolate)
    {
      if (nextuc[uc.id].timestamp != -1)
      {
	//flush all!
	fprintf(stderr, "flush, %d -1 steps\n", (int) scale);
	for (int step = 1; step < (int)scale; ++step)
	{
	  //step 0 already sent, last step will be sent after
	  for (int dev = 0; dev < urbi::devCount; ++dev)
	  {
	    if (nextuc[dev].timestamp == -1)
	      continue;
	    urbi::UCommand suc = lastuc[dev];
	    suc.timestamp = (lastuc[dev].timestamp * ((int)scale-step) + nextuc[dev].timestamp * step) / (int)scale;
	    suc.value.angle = (lastuc[dev].value.angle * (float)((int)scale-step) + nextuc[dev].value.angle * (float)step) / (float)((int)scale);
	    fwrite(&suc, sizeof (urbi::UCommand), 1, ouf);
	  }
	}
	//send the last
	for (int dev = 0; dev < urbi::devCount; ++dev)
	{
	  if (nextuc[dev].timestamp == -1)
	    continue;
	  fwrite(&nextuc[dev], sizeof (urbi::UCommand), 1, ouf);
	  lastuc[dev] = nextuc[dev];
	  nextuc[dev].timestamp = -1;
	}
      }
      nextuc[uc.id] = uc;
    }
    else
    {
      fwrite(&uc, sizeof (urbi::UCommand), 1, ouf);
    }
  }
  //send the last chunk
  if (interpolate)
    for (int dev = 0; dev < urbi::devCount; ++dev)
    {
      if (nextuc[dev].timestamp == -1)
	continue;
      fwrite(&nextuc[dev], sizeof (urbi::UCommand), 1, ouf);
    }

  fclose(inf);
  fclose(ouf);
}
