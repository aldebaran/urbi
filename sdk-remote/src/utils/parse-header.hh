#include "libport/cstdio"
#include <cstdlib>

void usage(const char* name, int status);

namespace urbi
{
  inline float
  fabs (float f )
  {
    if (f>0)
      return f;
    else
      return f*(-1.0);
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
    char * name;
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
      do {
	int r = fgetc(f);
	if (r == EOF)
	  return 6;
	device[pos++] = r;
      }
      while (device[pos - 1]);

      if (fwrite(device, strlen(device) + 1, 1, of) != 1)
	return 7;

      int a = 0;
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

  void
  reverse_cycle_scale_getopt (int argc, char * argv[], int min_argc,
			      FILE** in, FILE** out)
  {
    //cut static part of an urbi file
    if (argc < min_argc)
      usage(argv[0], 1);

    FILE* inf;
    if (STREQ(argv[1],"-"))
      inf=stdin;
    else
      inf=fopen(argv[1],"r");
    if (!inf)
    {
      printf("error opening file\n");
      std::exit(2);
    }

    FILE* ouf;
    if (STREQ(argv[2],"-"))
      ouf=stdout;
    else
      ouf=fopen(argv[2],"w");
    if (!ouf)
    {
      printf("error opening file\n");
      std::exit(2);
    }
    if (int a = parseHeader(inf,ouf))
    {
      printf("error parsing header: %d\n",a);
      std::exit(3);
    }
    *in = inf;
    *out = ouf;
  }

} // namespace urbi
