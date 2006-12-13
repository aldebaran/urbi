/**
   Send a binary file to the URBI server, to be saved in a variable.
*/
#include "urbi/uclient.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include "libport/windows.h"

int main(int argc, char * argv[])
{
  if (argc < 4 || argc >5)
    {
      printf("usage: %s robotname variablename filename ['BIN headers']\n"
	     "\t send the content of a file to the URBI server, to be saved in a variable\n"
	     "\t example: %s myrobot sounds.hello ~/sounds/hello.wav WAV\n",argv[0], argv[0]);
      return -1;
    }

  char * headers= "";
  if (argc==5)
    headers=argv[4];

  urbi::UClient uc(argv[1]);
  if (uc.error())
    exit(2);

  FILE *f;
  if (!strcmp(argv[3],"-"))
    f = stdin;
  else
    f = fopen(argv[3],"r");
  if (!f)
    {
      printf("error opening file\n");
      exit(3);
    }

  char * buffer;
  int pos;
  //read the whole file in memory
  if (f!=stdin)
  {
    struct stat st;
    stat(argv[3],&st);
    buffer = (char *)malloc(st.st_size);
    if (!buffer)
    {
      printf("not enough memory\n");
      return -2;
    }

    pos=0;
    while (true)
    {
      int r = fread(buffer + pos, 1, st.st_size-pos, f);
      if (r<=0)
	break;
      pos +=r;
    }
  }

  else
  {
    int sz=10000;
    pos = 0;
    buffer = (char *)malloc(sz);
    while (true)
    {
      if (sz-pos < 500)
      {
	sz += 10000;
	buffer = (char *)realloc(buffer,sz);
	if (!buffer)
	{
	  printf("not enough memory\n");
	  return -2;
	}
	int l = fread(buffer + pos, 1, sz-pos, f);
	if (l<=0)
	  break;
	pos +=l;
      }
    }
  }

  uc.sendBin(buffer, pos, "%s = BIN %d %s;", argv[2], pos, headers);
  sleep(1);
}
