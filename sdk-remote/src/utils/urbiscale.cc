#include <cstdlib>
#include "libport/cstdio"
#include <vector>

#include "urbi/uclient.hh"

inline float fabs(float f)
{
  if (f > 0)
    return f;
  return f * -1.0;
}

enum UType
{
  TYPE_BOOL,
  TYPE_ANGLE,
  TYPE_NORM
};

union UJointValue
{
  float angle;
  float normalized;
  bool boolean;
};

struct UCommand
{
  int timestamp;
  short id;
  UJointValue value;
};

struct UDev
{
  char* name;
  short id;
  UType type;
};
UDev* devices;
int devCount;

int parseHeader(FILE *f, FILE * of)
{
  char buff[4];
  if (fread(buff, 4, 1, f) != 1)
    return 1;
  if (fwrite(buff, 4, 1, of) != 1)
    return 2;
  if (strncmp(buff, "URBI", 4))
    return 3;
  if (fread(&devCount, 4, 1, f) != 1)
    return 4;
  if (fwrite(&devCount, 4, 1, of) != 1)
    return 5;

  for (int i = 0; i < devCount; ++i)
  {
    char device[256];
    int pos = 0;
    int a = 0;
    int r = EOF;
    do {
      if ((r = fgetc(f)) == EOF)
	return 6;
      device[pos++] = r;
    }
    while (device[pos - 1]);

    if (fwrite(device, strlen(device) + 1, 1, of) != 1)
      return 7;
    if (fread(&a, 2, 1, f) != 1)
      return 8;
    if (fwrite(&a, 2, 1, of) != 1)
      return 9;
    int type;
    if ((type = fgetc(f)) == EOF)
      return 10;
    fputc(type, of);
  }
  return 0;
}

int main(int argc, char* argv[])
{
  //cut static part of an urbi file
  if (argc < 4)
  {
    printf("usage %s infile outfile factor [interpolate]\n"
	   "\ttime-scale the urbi file by given factor (>1 means slow-down) \n"
	   "\tInterpolation will only work with integer factors\n", argv[0]);
    exit(1);
  }
  FILE* inf;
  FILE* ouf;

  if (STREQ(argv[1], "-"))
    inf = stdin;
  else
    inf = fopen(argv[1], "r");
  if (!inf)
  {
    printf("error opening file\n");
    exit(2);
  }
  if (STREQ(argv[2], "-"))
    ouf = stdout;
  else
    ouf = fopen(argv[2], "w");
  if (!ouf)
  {
    printf("error opening file\n");
    exit(2);
  }
  if (int a = parseHeader(inf, ouf))
  {
    printf("error parsing header: %d\n", a);
    exit(3);
  }

  float scale = 2;
  int interpolate = 0;
  if (argc > 4)
    interpolate = strtol(argv[4], NULL, 0);
  printf("interpolation set to %d\n", interpolate);
  //if no move greater than atresh cut the line
  sscanf(argv[3], "%f", &scale);
  UCommand uc;
  int starttime = -1;
  std::vector<UCommand> lastuc (devCount);
  std::vector<UCommand> nextuc (devCount);
  for (int i = 0; i < devCount; ++i)
  {
    lastuc[i].timestamp = -1;
  }
  for (int i = 0; i < devCount; ++i)
  {
    nextuc[i].timestamp =- 1;
  }

  int inittime = 0;
  bool first_time = true;
  while (fread(&uc, sizeof (UCommand), 1, inf) == 1)
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
	lastuc[uc.id].timestamp = (int) (((float)(inittime) - ((float)starttime)) * scale);
	fprintf(stderr, "miss: adding command for id %d at time %d\n",
		uc.id, lastuc[uc.id].timestamp);
	//go on processing uc
      }
      else
      {
	uc.timestamp = (int) (((float)(uc.timestamp) - ((float)starttime)) * scale);
	fwrite(&uc, sizeof (UCommand), 1, ouf);
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
	  for (int dev = 0; dev < devCount; ++dev)
	  {
	    if (nextuc[dev].timestamp == -1)
	      continue;
	    UCommand suc = lastuc[dev];
	    suc.timestamp = (lastuc[dev].timestamp * ((int)scale-step) + nextuc[dev].timestamp * step) / (int)scale;
	    suc.value.angle = (lastuc[dev].value.angle * (float)((int)scale-step) + nextuc[dev].value.angle * (float)step) / (float)((int)scale);
	    fwrite(&suc, sizeof (UCommand), 1, ouf);
	  }
	}
	//send the last
	for (int dev = 0; dev < devCount; ++dev)
	{
	  if (nextuc[dev].timestamp == -1)
	    continue;
	  fwrite(&nextuc[dev], sizeof (UCommand), 1, ouf);
	  lastuc[dev] = nextuc[dev];
	  nextuc[dev].timestamp = -1;
	}
      }
      nextuc[uc.id] = uc;
    }
    else
    {
      fwrite(&uc, sizeof (UCommand), 1, ouf);
    }
  }
  //send the last chunk
  if (interpolate)
    for (int dev = 0; dev < devCount; ++dev)
    {
      if (nextuc[dev].timestamp == -1)
	continue;
      fwrite(&nextuc[dev], sizeof (UCommand), 1, ouf);
    }

  fclose(inf);
  fclose(ouf);
}
