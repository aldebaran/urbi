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
  if (argc<3) {
	printf("usage %s infile outfile [jointid] [startval] [direction] [numcycle]\n"
		   "\tDetect and extract cycles in an urbi recorded file\n"
		   "\tJointid is the joint used to detect cycles (0-based value, see urbirecord.cpp for id/name correspondance)\n"
		   "\tStartval is the value that will mark the beginning of a cycle when reached by the joint, in the direction defined by the 'direction' parameter\n"
		   "\tnumcycle is the number of the cycle that will be written to 'outfile'\n"
		   "\tSet startval to '-' for 'first value seen'\n",argv[0]); 
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
   
  int joint=0;
  int wantedcycle=2;
  float startval;
  bool init=false; //not found yet startval on joint
  bool gotSign=false, gotLastVal=false;
  bool cyclesgn;
  if (argc>3) sscanf(argv[3],"%d",&joint);
  if (argc>4)  {
	if (!strcmp(argv[4],"-")) init=true;
	else sscanf(argv[4],"%f",&startval);
  }
  if (argc>5) {
	int v;
	sscanf(argv[5],"%d",&v);
	if (v!=0) {
	  gotSign=true;
	  cyclesgn=(v<0);
	}
  }
  if (argc>6) sscanf(argv[6],"%d",&wantedcycle);
  UCommand uc;

  float lastval;
  UCommand buff[devCount];
  int cycle=0;

  int buffTime=0;
  
  int basetime=0;
  for (int i=0;i<devCount;i++) {buff[i].timestamp=0;}
  //read and handle by block of commands with same timestamp.
  //init:
  fread(&uc,sizeof(UCommand),1,inf);
  buffTime=uc.timestamp;
  buff[uc.id]=uc;
  while (1) {

	int ok=fread(&uc,sizeof(UCommand),1,inf);
	if (ok && !basetime) basetime=uc.timestamp;
	if (ok && buffTime==0) buffTime=uc.timestamp;
	if (ok && uc.timestamp==buffTime) {buff[uc.id]=uc;continue;}
	
	if (init) {
	  //initialize asap<-now

	  if (buff[joint].timestamp==0) {
		//cant do anything
		for (int i=0;i<devCount;i++) buff[i].timestamp=0;
		buff[uc.id]=uc;
		buffTime=0;
		continue;
	  }

	  startval=buff[joint].value.angle;
	  gotSign=false;
	  init=false;
	  gotLastVal=true;
	  lastval=startval;
	  cycle++;
	  fprintf(stderr,"cycle %d starts at %d\n",cycle, buffTime-basetime);
	}

	
	if (gotLastVal && 
		((!gotSign) ||   ( cyclesgn ^ (lastval<startval))) &&
		(   (lastval<startval && buff[joint].value.angle>=startval) ||
		    (lastval>startval && buff[joint].value.angle<=startval) )) {
	  cyclesgn = (lastval>startval);
	  gotSign=true;
	  cycle++;
	  fprintf(stderr,"cycle %d starts at %d\n",cycle, buffTime-basetime);
	}

	if (buff[joint].timestamp!=0) {
	  lastval=buff[joint].value.angle;
	  gotLastVal=true;
	}
	if (cycle==wantedcycle) 
	  for (int i=0;i<devCount;i++)
		if (buff[i].timestamp!=0) {buff[i].timestamp-=basetime;fwrite(&buff[i],sizeof(UCommand),1,ouf); }
	   	
	//flush buffer
	for (int i=0;i<devCount;i++) buff[i].timestamp=0;
	buff[uc.id]=uc;
	buffTime=0;
	if (!ok) break;
  }
  fclose(inf);
  fclose(ouf);
}
