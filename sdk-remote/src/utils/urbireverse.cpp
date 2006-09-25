#include <uclient.h>
#include <stdlib.h>
#include <stdio.h>
inline float fabs(float f ) {if (f>0) return f; else return f*(-1.0);}
enum UType {
  TYPE_BOOL,
  TYPE_ANGLE,
  TYPE_NORM
};
union UJointValue {
  float angle;
  float normalized;
  bool boolean;
};
struct UCommand {
  int timestamp;
  short id;
  UJointValue value;
};

struct UDev {
  char * name;
  short id;
  UType type;
};
UDev* devices;
int devCount;

int parseHeader(FILE *f,FILE * of) {
  char buff[4];
  if (fread(buff,4,1,f)!=1) return 1;
  if (fwrite(buff,4,1,of)!=1) return 2;
  if (strncmp(buff,"URBI",4)) return 3;
  if (fread(&devCount,4,1,f)!=1) return 4;
  if (fwrite(&devCount,4,1,of)!=1) return 5;
  for (int i=0;i<devCount;i++) {
	char device[256];
	int pos=0;
	int a;
	do {
	  if ((device[pos++]=fgetc(f))==EOF) return 6;
	}
	while (device[pos-1]);
	if (fwrite(device,strlen(device)+1,1,of)!=1) return 7;
	if (fread(&a,2,1,f)!=1) return 8;
	if (fwrite(&a,2,1,of)!=1) return 9;
	int type;
	if ( (type=fgetc(f)) ==EOF) return 10;
	fputc(type,of);
  }
  return 0;
}

int main(int argc, char * argv[]) {
  //cut static part of an urbi file
  if (argc<3) {
	printf("usage %s infile outfile \nreverse the urbi file\n",argv[0]);
	exit(1);
  }
  FILE * inf;
  FILE * ouf;
  if (!strcmp(argv[1],"-")) inf=stdin;
  else inf=fopen(argv[1],"r");
  if (!inf) {printf("error opening file\n");exit(2);}
  if (!strcmp(argv[2],"-")) ouf=stdout;
  else ouf=fopen(argv[2],"w");
  if (!ouf) {printf("error opening file\n");exit(2);}
  if (int a=parseHeader(inf,ouf)) {printf("error parsing header: %d\n",a); exit(3);}
  long endheader=ftell(inf);
  
  UCommand uc;
  int starttime=-1;
  fseek(inf,sizeof(UCommand)*(-1),SEEK_END);
  while (ftell(inf)!=endheader) {
	if (fread(&uc,sizeof(UCommand),1,inf)!=1) {printf("error reading from file\n");exit(1);}
	fseek(inf,sizeof(UCommand)*(-2),SEEK_CUR);
	if (starttime==-1) starttime=uc.timestamp;
	uc.timestamp = starttime-uc.timestamp;
	fwrite(&uc,sizeof(UCommand),1,ouf);
  }
  
  fclose(inf);
  fclose(ouf);
}
