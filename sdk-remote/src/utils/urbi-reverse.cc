/*
 * Copyright (C) 2005-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cstdlib>
#include <libport/cstdio>

#include "urbi/uclient.hh"
#include "parse-header.hh"

// FIXME: those return value should not be ignored
static size_t ignore;

void
usage (const char* name, int status)
{
  printf("usage %s infile outfile \nreverse the urbi file\n", name);
  if (status)
    exit(status);
}

int
main(int argc, char * argv[])
{
  FILE* inf = 0;
  FILE* ouf = 0;
  urbi::reverse_cycle_scale_getopt (argc, argv, 3, &inf, &ouf);

  long endheader=ftell(inf);

  urbi::UCommand uc;
  int starttime=-1;
  fseek(inf,sizeof (urbi::UCommand)*(-1),SEEK_END);
  while (ftell(inf) != endheader)
  {
    if (fread(&uc,sizeof (urbi::UCommand),1,inf)!=1)
    {
      printf("error reading from file\n");
      exit(1);
    }
    fseek(inf,sizeof (urbi::UCommand)*(-2),SEEK_CUR);
    if (starttime==-1)
      starttime=uc.timestamp;
    uc.timestamp = starttime-uc.timestamp;
    ignore = fwrite(&uc,sizeof (urbi::UCommand),1,ouf);
  }

  fclose(inf);
  fclose(ouf);
}
