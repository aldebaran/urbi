/**
   Send a binary file to the URBI server, to be saved in a variable.
*/
#include "urbi/uclient.hh"
#include <sys/types.h>
#include "libport/sys/stat.h"
#include "libport/windows.hh"

int main(int argc, char * argv[])
{
  if (argc < 4 )
    {
      printf("usage: %s robotname variablename filename ['BIN headers']\n"
	     "       %s rname varname fname headers varname fname headers...\n"	     "\t send the content of a file or files to the URBI server, to be saved in a variable\n"
	     "\t example: %s myrobot sounds.hello ~/sounds/hello.wav WAV\n",argv[0], argv[0], argv[0]);
      return -1;
    }

  

  urbi::UClient uc(argv[1]);
  if (uc.error())
    exit(2);
  int argp = 2;
  while (argp+1 <argc) {
    char  * varname = argv[argp];
    const char * headers= "";
    if (argp+2<argc)
      headers=argv[argp+2];
    FILE *f;
    if (STREQ(argv[argp+1],"-"))
      f = stdin;
    else
      f = fopen(argv[argp+1],"r");
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
      stat(argv[argp+1],&st);
      buffer = static_cast<char *> (malloc (st.st_size));
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
      //std::cerr <<"read "<<pos<<" bytes from "<<argv[argp+1]<<std::endl;
    }

    else
    {
      int sz=10000;
      pos = 0;
      buffer = static_cast<char *> (malloc (sz));
      while (true)
      {
	if (sz-pos < 500)
	{
	  sz += 10000;
	  buffer = static_cast<char *> (realloc (buffer,sz));
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
    
    uc.sendBin(buffer, pos, "%s = BIN %d %s;", varname, pos, headers);
    argp+=3;
  }
    sleep(1);
}
