#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <locale.h>
#ifdef MAX_PATH
#define M_MAX_PATH MAX_PATH
#else
#define M_MAX_PATH 1024
#endif

#include "Move.h"

namespace urbi
{
  inline float ffloatpart(float a) {return a-(int)a;}

  static char * dirname(char * src) {
    static char result[M_MAX_PATH];
    if (!strchr(src, '/')) {
      strcpy(result,".");
      return result;
    }
    strcpy(result, src);
    int p;
    for (p=strlen(result)-1;result[p]!='/';p--)
      ;
    result[p]=0;
    return result;
  }

  int Move::initialize(UClient * client,
		       bool uploadFiles,
		       const char * configFile, bool enableInterrupt)
  {
    setlocale( LC_ALL, "C" ); //this is required to parse our configuration file correctly
    walks.clear();
    turns.clear();
    robot = client;
    robot->makeUniqueTag(tag);
    robot->makeUniqueTag(execTag);
    if (enableInterrupt) {
      interruptConnection = new UClient(robot->getServerName());
    }
    else
      interruptConnection = 0;
    strcat(tag,"mov");
    FILE * cf=fopen(configFile,"r");
    if (!cf) {
      fprintf(stderr,"Failed to open main configuration file %s\n",configFile);
      return 1;
    }
    char configPath[M_MAX_PATH];
    char filePath[M_MAX_PATH];
    strcpy(configPath, configFile);
    char * path=dirname(configPath);
    char line[1024];
    int idx=0;

    while (fgets(line,1024,cf)) {
      char type[64];
      char name[M_MAX_PATH];
      LoadedFile lf;
      if (line[0]=='\n' || line[0]==0)
	continue;
      if (line[strlen(line)-1]=='\n')
	line[strlen(line)-1]=0;
      int pos=0;
      while (line[pos]==' ')
	pos++;
      if (line[pos]==0)
	continue; //empty line
      static const char format[]="%s%s%f%f%f";
      int s=sscanf(line,format,type,name,&lf.value,&lf.speed, &lf.precision);
      if ( (s>=1) && (type[0]=='#'))
	continue;
      if ( (s>=1) && (type[0]==0))
	continue;
      if (s!=5) {
	fprintf(stderr,"parse error at line %s, skipping\n",line);
	continue;
      }

      sprintf(lf.name,"mov%d",idx++);

      if (uploadFiles) {
	if (name[0]!='/') {
	  //not an absolute path, happen dirname as a prefix
	  strcpy(filePath,path);
	  strcat(filePath,"/");
	  strcat(filePath,name);

	}
	else strcpy(filePath,name);
	FILE * cmdf=fopen(filePath,"r");
	if (!cmdf)
	  {
	    fprintf(stderr,"cannot open file %s, skipping\n",name);
	    continue;
	  }

	struct stat st;
	stat(filePath,&st);
	long filelength=st.st_size;
	char * buffer=(char *)malloc(filelength+200);
	//read the file
	int left=filelength;
	while (left) {
	  int rc=fread(buffer,1,left,cmdf);
	  if (!rc) {fprintf(stderr,"error reading file %s\n",name); return 3;}
	  left-=rc;
	}
	sprintf(&buffer[filelength],"%s :ping;",tag);
	filelength+=strlen(tag)+strlen(" :ping;");
	fclose(cmdf);
	char block[101];
	block[100]=0;
	int offset = 0;

	robot->send("%s = \"%s: {\";", lf.name, execTag);
	while (filelength > offset) {
	  //std::cerr << offset<<" / "<<filelength<<std::endl;
	  //usleep(200000);
	  //std::cerr << block<<std::endl;
	  strncpy(block, &buffer[offset],100);
	  offset +=100;
	  robot->send("%s = %s + \"%s\";", lf.name, lf.name, block);
	}
	robot->send("%s = %s + \"};\";",lf.name, lf.name );
	free(buffer);
      }

      if (tolower(type[0])=='w')
	walks.push_back(lf);
      else
	turns.push_back(lf);

    }
    fclose(cf);

    //register our callback
    robot->setCallback(*this, &Move::moveEnd, tag);
    //fill the properties structs by parsing our loadedfiles.
    pwalk.minSpeed=pwalk.resolution=pwalk.precision=300000000;
    pwalk.maxSpeed=0;
    for (std::list<LoadedFile>::iterator it=walks.begin();it!=walks.end();it++) {
      if (it->speed>pwalk.maxSpeed) pwalk.maxSpeed=it->speed;
      if (it->speed<pwalk.minSpeed) pwalk.minSpeed=it->speed;
      if (it->value<pwalk.resolution) {pwalk.resolution=it->value; pwalk.precision=it->precision;}
    }
    pturn.minSpeed=pturn.resolution=pturn.precision=300000000;
    pturn.maxSpeed=0;
    for (std::list<LoadedFile>::iterator it=turns.begin();it!=turns.end();it++)
      {
	if (it->speed>pturn.maxSpeed) pturn.maxSpeed=it->speed;
	if (it->speed<pturn.minSpeed) pturn.minSpeed=it->speed;
	if (it->value<pturn.resolution) {pturn.resolution=it->value; pturn.precision=it->precision;}
      }
    moving=0;
    return 0;
  };

  void Move::interrupt(bool notifyEndMove) {
    if (!interruptConnection)
      return;
    if (moving <=0)
      return;
    moving = -2;

    interruptConnection->send("stop %s;", execTag);
    if (notifyEndMove)
      robot->send("%s: ping;", tag);
  }

  int Move::walk(float &distance, float precision, const char * tag) {
    float absoluteprecision=fabs(distance * precision);
    int length=walks.size();
    float values[length/2][2];
    int move[length];
    int bestmove[length];
    int direction[length];
    int bestnummoves=-1;
    float currentval=0;
    std::list<LoadedFile>::iterator it=walks.begin();
    std::list<LoadedFile>::iterator it2=it;
    for (int i=0;i<length/2;i++,it2++)
      continue;
    //std::cerr << "values: ";
    for (int i=0;i<length/2;i++,it++,it2++)
      {
	values[i][0]=it->value;
	values[i][1]=it2->value*(-1); //the minus is in the negative value in move
	//std::cerr << values[i][0] << " " << values[i][1] <<" , ";
	move[i]=0;
      }
    //std::cerr <<endl;
    direction[0]=(distance>0)?0:1;
    move[0]= (distance>0)?-1:1;
    int index=0;
    while (1) {
      if (index==length/2-1) {
	//calculate best value for move[index]
	float dist=distance-currentval;
	int cnt= (int) (dist/values[index][direction[index]]); // -12/5 = -2 en division entiere.
	float rest=ffloatpart(dist/values[index][direction[index]]);
	if (rest>0.5) cnt++;
	if (rest<-0.5) cnt--;
	move[index]=cnt;
	currentval+=(float)cnt*values[index][direction[index]];
	/*
	  std::cerr << "eval ";
	  for (int k=0;k<length/2;k++) std::cerr << move[k] <<" ";
	  std::cerr << "(" <<currentval <<")";
	  std::cerr <<endl;
	*/
	//calculate nummoves
	int nummoves=0;
	for (int i=0;i<=index;i++) nummoves+=abs(move[i]);
	if (fabs(currentval-distance)<absoluteprecision && (nummoves<bestnummoves || bestnummoves==-1) ) {
	  bestnummoves=nummoves;
	  for (int k=0;k<length/2;k++)
	    bestmove[k]=move[k];
	}
	//get back one or more level
	do {
	  currentval -= (float)move[index]*values[index][direction[index]];
	  move[index]=0;
	  index--;
	}
	while ( index>=0
		&& (
		    (currentval>distance && direction[index]==0)
		    ||
		    (currentval<distance && direction[index]==1)
		    ));
	if (index<0) break;
	//DEBUG:recalculate currentval
	/*
	  std::cerr << "cv: "<<currentval;
	  currentval=0;
	  for (int k=0;k<length/2;k++) currentval+=(float)move[k]*values[k][direction[k]];
	  std::cerr << " " <<currentval <<std::endl;
	*/
      }

      move[index]+= ((direction[index]==0)?1:-1);
      if (move[index]!=0)
	currentval+=values[index][direction[index]]*(float)((direction[index]==0)?1:-1);
      if (currentval>distance)
	direction[index+1]=1;
      else
	direction[index+1]=0;
      move[index+1]= ((direction[index+1]==0)?-1:1);
      index++;
    }

    //end, apply best match

    if (tag) strcpy(usertag,tag);
    else
      strcpy(usertag, "notag");
    if (bestnummoves==-1 || bestnummoves==0) {
      distance=0;
      moving = 0;
      robot->send("%s: ping;", this->tag);
      return 0;
    }
    char command[1024];
    float realmove=0;
    command[0]=0;
    moving=0;
    it=walks.begin();
    it2=it;
    sequence.clear();
    for (int i=0;i<length/2;i++,it2++);
    for (int i=0;i<length/2;i++,it++,it2++) {
      char * name= (bestmove[i]>0)?it->name:it2->name;
      realmove += bestmove[i]*values[i][(bestmove[i]>0)?0:1];
      for (int j=0;j<abs(bestmove[i]);j++) {
	moving++;
	if (moving<4)
	  sprintf(&command[strlen(command)],"exec(%s);", name);
	else
	  sequence.push_back((std::string)name);
      }
    }
    robot->send(command);
    distance=realmove;
    return 0;
  }
  int Move::turn(float &distance, float precision, const char * tag) {
    float absoluteprecision=fabs(distance * precision);
    int length=turns.size();
    float values[length/2][2];
    int move[length];
    int bestmove[length];
    int direction[length];
    int bestnummoves=-1;
    float currentval=0;
    std::list<LoadedFile>::iterator it=turns.begin();
    std::list<LoadedFile>::iterator it2=it;
    for (int i=0;i<length/2;i++,it2++);
    // std::cerr << "values: ";
    for (int i=0;i<length/2;i++,it++,it2++) {
      values[i][0]=it->value;
      values[i][1]=it2->value*(-1); //the minus is in the negative value in move
      //std::cerr << values[i][0] << " " << values[i][1] <<" , ";
      move[i]=0;
    }
    //std::cerr <<endl;
    direction[0]=(distance>0)?0:1;
    move[0]= (distance>0)?-1:1;
    int index=0;
    while (1) {
      if (index==length/2-1) {
	//calculate best value for move[index]
	float dist=distance-currentval;
	int cnt= (int)(dist/values[index][direction[index]]); // -12/5 = -2 en division entiere.
	float rest=ffloatpart(dist/values[index][direction[index]]);
	if (rest>0.5) cnt++;
	if (rest<-0.5) cnt--;
	move[index]=cnt;
	currentval+=(float)cnt*values[index][direction[index]];
	/*
	  std::cerr << "eval ";
	  for (int k=0;k<length/2;k++) std::cerr << move[k] <<" ";
	  std::cerr << "(" <<currentval <<")";
	  std::cerr <<endl;
	*/
	//calculate nummoves
	int nummoves=0;
	for (int i=0;i<=index;i++) nummoves+=abs(move[i]);
	if ((fabs(currentval-distance)<absoluteprecision) && ((nummoves<bestnummoves) || (bestnummoves==-1)) ) {
	  bestnummoves=nummoves;
	  for (int k=0;k<length/2;k++)
	    bestmove[k]=move[k];
	}
	//get back one or more level
	do {
	  currentval -= (float)move[index]*values[index][direction[index]];
	  move[index]=0;
	  index--;
	}
	while ( index>=0
		&& (
		    (currentval>distance && direction[index]==0)
		    ||
		    (currentval<distance && direction[index]==1)
		    ));
	if (index<0) break;
	//DEBUG:recalculate currentval
	/*
	  std::cerr << "cv: "<<currentval;
	  currentval=0;
	  for (int k=0;k<length/2;k++) currentval+=(float)move[k]*values[k][direction[k]];
	  std::cerr << " " <<currentval <<std::endl;
	*/
      }

      move[index]+= ((direction[index]==0)?1:-1);
      if (move[index]!=0) currentval+=values[index][direction[index]]*(float)((direction[index]==0)?1:-1);
      if (currentval>distance) direction[index+1]=1;
      else direction[index+1]=0;
      move[index+1]= ((direction[index+1]==0)?-1:1);
      index++;
    }

    //end, apply best match

    if (tag) strcpy(usertag,tag);
    else
      strcpy(usertag, "notag");
    if (bestnummoves==-1 || bestnummoves==0) {
      distance=0;
      moving = 0;
      robot->send("%s: ping;", this->tag);
      return 0;
    }
    char command[1024];
    float realmove=0;
    command[0]=0;
    moving=0;
    it=turns.begin();
    it2=it;
    sequence.clear();
    for (int i=0;i<length/2;i++,it2++);
    // std::cerr << "choice:" ;
    for (int i=0;i<length/2;i++,it++,it2++) {
      char * name= (bestmove[i]>0)?it->name:it2->name;
      realmove += bestmove[i]*values[i][(bestmove[i]>0)?0:1];
      //	std::cerr << bestmove[i] << " ";
      for (int j=0;j<abs(bestmove[i]);j++) {
	moving++;
	if (moving<4)
	  sprintf(&command[strlen(command)],"exec(%s);", name);
	else
	  sequence.push_back((std::string)name);
      }
    }
    //  std::cerr <<endl;
    robot->send(command);
    distance=realmove;
    return 0;
  }

  UCallbackAction Move::moveEnd(const UMessage &msg)
  {
    moving--;
    std::cerr << "movend "<< moving << std::endl;
    if (moving >0 && !sequence.empty()) {
      std::string s=sequence.front();
      sequence.pop_front();
      msg.client.send("exec(%s);",s.c_str());
    }
    if (moving <= 0
	&& usertag[0])
      robot->notifyCallbacks(UMessage(*robot, msg.timestamp, usertag,
				      "***end move", std::list<urbi::BinaryData>()));
    return URBI_CONTINUE;
  }
} // namespace urbi
