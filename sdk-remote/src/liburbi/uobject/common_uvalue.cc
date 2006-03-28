/*! \file common_uvalue.cc
 *******************************************************************************

 File: common_uvalue.cc\n
 Implementation of the UValue class and other linked classes

 This file is part of LIBURBI\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */
#include <stdio.h>
#include "uobject.h" 
#include <sstream>
using namespace urbi;

//////////////////////
//// UValue Parsing
//////////////////////

void unescape(string & data) {
  int src=0, dst=0;
  while (data[src]) {
    if (data[src]!='\\')
      data[dst]=data[src];
    else {
      switch(data[++src]) {
      case 'n':
	data[dst]='\n';
	break;
      case '\\':
	data[dst]='\\';
	break;
      case '"':
	data[dst]='"';
	break;
      default:
	data[dst]=data[src];
	break;
      }
    }
    src++;
    dst++;
  }
  data[dst] = 0;
  
}
void unescape(char * data) {
  char* src = data;
  char * dst = data;
  while (*src) {
    if (*src != '\\') 
      *dst = *src;
    
    else {
      switch (*(++src)) {
      case 'n':
	*dst = '\n';
	break;
      case '\\':
	*dst='\\';
	break;
      case '"':
	*dst = '"';
	break;
      default:
	*dst = *src;
      };
    }
    src++;
    dst++;
  }
  *dst = 0;
}



int UValue::parse(char * message, int pos, std::list<BinaryData> bins, std::list<BinaryData>::iterator &binpos) {
  while (message[pos]==' ')
    pos++;
  if (message[pos] == '"') {
    //string
    type = DATA_STRING;
    //get terminating '"'
    int p=pos+1;
    while (message[p] && message[p]!='"') {
      if (message[p]=='\\')
	p++;
      p++;
    }
    if (!message[p])
      return -p; //parse error

    stringValue = new string(message+pos+1, p-pos-1);
    unescape(*stringValue);
    return p+1;
  }

  if (message[pos] == '[') { 
    //list message
    type = DATA_LIST;
    list = new UList();
    pos++;
    while (message[pos]==' ') pos++;
    while (message[pos]) {
      while (message[pos]==' ') pos++;
      UValue *v = new UValue();
      int p = v->parse(message, pos, bins, binpos);
      if (p<0)
	return p;
      list->array.push_back(v);
      pos = p;
      while (message[pos]==' ') pos++;
      //expect , or rbracket
      if (message[pos]==']')
	break;
      if (message[pos]!=',')
	return -pos;
      pos++;
    }
    
    if (message[pos]!=']')
      return -pos;
    return pos+1;
  }

  //OBJ a [x:12, y:4]
  if (!strncmp(message+pos, "OBJ ", 4)) {
    //obj message
    pos+=4;
    type = DATA_OBJECT;
    object = new UObjectStruct();

    //parse object name
    while (message[pos]==' ')
      pos++;
    
    int p = pos;
    while (message[p] && message[p]!=' ')
      p++;
    if (!message[p])
      return -p; //parse error
    object->refName = string(message+pos, p-pos);
    pos=p;

    
    while (message[pos]==' ')
      pos++;
    if (message[pos]!='[')
      return -pos;
    pos++;
    
    while (message[pos]) {
      while (message[pos]==' ') pos++;
      //parse name
      int p = pos;
      while (message[p] && message[p]!=':')
	p++;
      if (!message[p])
	return -p; //parse error
      p++;
      UNamedValue nv;
      nv.name = string(message+pos, p-pos-1);
      pos=p;
      while (message[pos]==' ') pos++;
      UValue *v = new UValue();
      p = v->parse(message, pos, bins, binpos);
      if (p<0)
	return p;
      nv.val = v;
      object->array.push_back(nv);
      pos = p;
      while (message[pos]==' ') pos++;
      //expect , or rbracket
      if (message[pos]==']')
	break;
      if (message[pos]!=',')
	return -pos;
      pos++;
    }
    
    if (message[pos]!=']')
      return -pos;
    return pos+1;
  }


      

  if (!strncmp(message+pos,"BIN ",4)) {
    //binary message: delegate
    type = DATA_BINARY;
    binary = new UBinary();
    pos +=4;
    //parsing type
    int p = binary->parse(message, pos, bins, binpos);
    return p;
  }
  
  //last attempt: double
  int p;
  int count = sscanf(message+pos, "%lf%n", &val, &p);
  if (!count) 
    return -pos;
  type = DATA_DOUBLE;
  pos +=p;
  return pos;

}


std::ostream & operator <<(std::ostream &s, const UValue &v) {
  switch( v.type) {
    case DATA_DOUBLE:
      s<< v.val;
      break;
    case DATA_STRING:
      s<< '"'<<*v.stringValue<<'"';
      break;
    case DATA_BINARY:
      s<<"BIN "<<v.binary->size<<" "<<v.binary->message<<";";
      s.write((char *)v.binary->data, v.binary->size);
      break;
    case DATA_LIST:
      {
	s<<"[";
  	int sz = v.list->size();
    	int p = 0;
	for (int i=0; i<sz;i++) {
	  s << (*v.list)[i];
	  if (i != sz-1)
	    s<< " , ";
	}
	s<< "]";
      }
      break;
    case DATA_OBJECT:
      {
	s<<"OBJ "<<v.object->refName<<" [";
	int sz = v.object->size();
	int p = 0;
	for (int i=0; i<sz;i++) {
	  s << (*v.object)[i].name << ":";
	  s<< (*v.object)[i].val;
	  if (i != sz-1)
	    s<< " , ";
	}
	s<< "]";
      }  
      break;      
  }
  return s;
}


int UBinary::parse(const char * message, int pos, list<BinaryData> bins, list<BinaryData>::iterator &binpos) {
  while (message[pos]==' ') pos++;
  //find end of header
 
  if( binpos == bins.end()) //no binary data available
    return -1;
  
  //validate size
  int ps,psize;
  int count = sscanf(message+pos,"%d%n",&psize,&ps);
  if (count!=1)
    return -pos;
  if (psize != binpos->size) {
    std::cerr <<"bin size inconsistency\n";
    return -pos;
  }
  pos +=ps;
  size = psize;
  data = malloc(psize);
  memcpy(data, binpos->data, size);
  binpos++;


 int p = pos;
  while (message[p] && message[p]!='\n')
    p++;
  if (!message[p])
    return -p; //parse error
  this->message = string(message+pos, p-pos);
  p++;

  //trying to parse header to find type
  char type[64];
  memset(type, 0, 64);
  int p1, p2, p3, p4, p5;
  count = sscanf(message+pos,"%63s %d %d %d %d", type, &p2, &p3, &p4, &p5);
  //DEBUG fprintf(stderr,"%s:  %d %s %d %d\n", message, p1, type, p2, p3);
  if (!strcmp(type, "jpeg")) {
    this->type = BINARY_IMAGE;
    image.size = size;
    image.width = p2;
    image.height = p3;
    image.imageFormat = IMAGE_JPEG;
    return p;
  }
 
  if (!strcmp(type, "YCbCr")) {
    this->type = BINARY_IMAGE;
    image.size = size;
    image.width = p2;
    image.height = p3;
    image.imageFormat = IMAGE_YCbCr;
    return p;
  }

  if (!strcmp(type, "rgb")) {
    this->type = BINARY_IMAGE;
    image.size = size;
    image.width = p2;
    image.height = p3;
    image.imageFormat = IMAGE_RGB;
    return p;
  }

  if (!strcmp(type, "raw")) {
    this->type = BINARY_SOUND;
    sound.soundFormat = SOUND_RAW;
    sound.size = size;
    sound.channels = p2;
    sound.rate = p3;
    sound.sampleSize = p4;
    sound.sampleFormat = (USoundSampleFormat) p5;
    return p;
  }

  if (!strcmp(type, "wav")) {
    this->type = BINARY_SOUND;
    sound.soundFormat = SOUND_WAV;
    sound.size = size;
    sound.channels = p2;
    sound.rate = p3;
    sound.sampleSize = p4;
    sound.sampleFormat = (USoundSampleFormat) p5;
    return p;
  }

  //unknown binary
  this->type = BINARY_UNKNOWN;
  return p;
 }

void UBinary::buildMessage() {
  message = getMessage();
}

string UBinary::getMessage() const {
  std::ostringstream str;
  if (type == BINARY_IMAGE) {
    switch( image.imageFormat) {
      case IMAGE_RGB:
	str<<"rgb ";
	break;
      case IMAGE_JPEG:
	str<<"jpeg ";
	break;
      case IMAGE_YCbCr:
	str<<"YCbCr";
	break;
      default:
	str<<"unknown ";
	break;
    };
    str<<image.width<<" "<<image.height;
  }
  if (type == BINARY_SOUND) {
    switch (sound.soundFormat) {
      case SOUND_RAW:
	str<<"raw ";
	break;
      case SOUND_WAV:
	str << "wav ";
	break;
      default:
	str << "unknown ";
	break;	
    };
    str<<sound.channels<<" "<<sound.rate<<" "<<sound.sampleSize<<" "<<sound.sampleFormat;
  
  }

  return str.str();
 }

//! Class UValue implementation
UValue::UValue() : type(DATA_VOID), storage(0) {}


UValue::UValue(UFloat v) : val(v), type(DATA_DOUBLE)  {}
UValue::UValue(int v) : val(v), type(DATA_DOUBLE)  {}

UValue::UValue(char * v) : stringValue(new string(v)), type(DATA_STRING)  {}
UValue::UValue(const string &v) : type(DATA_STRING), stringValue(new string(v)) {}
 
UValue::UValue(const UBinary &b) : type(DATA_BINARY){
  binary = new UBinary(b); 
}
UValue::UValue(const UList &l) : type(DATA_LIST){
  list = new UList(l); 
}
UValue::UValue(const UObjectStruct &o) : type(DATA_OBJECT){
  object = new UObjectStruct(o); 
}


UValue::~UValue() {
  switch(type) {
  case DATA_STRING:
    if (stringValue)
      delete stringValue;
    break;
  case DATA_BINARY:
    if (binary)
      delete binary;
    break;
  case DATA_LIST:
    if (list)
      delete list;
    break;
  case DATA_OBJECT:
    if (object)
      delete object;
  }
}

UValue::operator UFloat () const {

  UFloat v;
  switch( type) {
  case DATA_DOUBLE:

    return val;
    break;
    
  case DATA_STRING: 
    {
      std::istringstream tstr(*stringValue);
      tstr >> v;
      return v;
    }
    break;
  };

  return UFloat(0);
};


UValue::operator string() const {
   switch( type) {
   case DATA_DOUBLE: 
     {
       std::ostringstream tstr;
       tstr << val;
       return tstr.str();
     }
     break;
   case DATA_STRING:
     return *stringValue;
     break;
   };
};

UValue::operator UBinary() const {
  if (type != DATA_BINARY)
    return UBinary();
  else 
    return *binary;
}


UValue& UValue::operator= (const UValue& v)
{ //TODO: optimize
  if (this == &v) return *this;
  switch(type) {
  case DATA_STRING:
    if (stringValue)
      delete stringValue;
  case DATA_BINARY:
    if (binary)
      delete binary;
    break;
  case DATA_LIST:
    if (list)
      delete list;
    break;
  case DATA_OBJECT:
    if (object)
      delete object;
  }
  
  type = v.type;
  switch (type) {  
  case DATA_DOUBLE:
    val = v.val;
    break;
  case DATA_STRING:
    stringValue = new string(*v.stringValue);
    break;
  case DATA_BINARY:
    binary = new UBinary(*v.binary); 
    break;
  case DATA_LIST:
    list = new UList(*v.list);
    break;
  case DATA_OBJECT:
    object = new UObjectStruct(*v.object);
    break;
  };
  return *this;
}


UValue::UValue(const UValue &v) {
  type = DATA_VOID;
  (*this) = v;
}


UBinary::UBinary() {
  data = 0;
}

UBinary::~UBinary() {
  if (data)
    free(data);
}

UBinary::UBinary(const UBinary &b) {
  type = BINARY_NONE;
  data = 0;
  (*this) = b;
}

UBinary & UBinary::operator = (const UBinary &b) {
  if (data)
    free(data);

  type = b.type;
  message = b.message;
  size = b.size;
  switch(type) {
  case BINARY_IMAGE:
    image = b.image;
    break;
  case BINARY_SOUND:
    sound = b.sound;
    break;
  }
  data = malloc(size);
  memcpy(data, b.data, b.size);
}


UList::UList() : offset(0) {}

UList::UList(const UList &b) : offset(0) {
  (*this) = b;
}

UList & UList::operator = (const UList &b) {

  offset = 0;
  for (int i=0;i<size();i++) //relax, its a vector
    delete array[i];
  array.clear();

  for (vector<UValue*>::const_iterator it= b.array.begin(); 
      it !=b.array.end();it++)
    array.push_back(new UValue(**it));
  offset = b.offset;
  
  return (*this);
}

UList::~UList() {
  offset=0;
  for (int i=0;i<size();i++) //relax, its a vector
    delete array[i];
  array.clear();
}




UObjectStruct::UObjectStruct() {}

UObjectStruct::UObjectStruct(const UObjectStruct &b) {
  (*this) = b;
}

UObjectStruct & UObjectStruct::operator = (const UObjectStruct &b) {
  for (int i=0;i<size();i++) //relax, its a vector
	delete array[i].val;
  array.clear();

  for (vector<UNamedValue>::const_iterator it= b.array.begin(); it != b.array.end();it++)
    array.push_back(UNamedValue(it->name, new UValue(*(it->val))));

  return (*this);
}

UObjectStruct::~UObjectStruct() {
 for (int i=0;i<size();i++) //relax, its a vector
	delete array[i].val;
  array.clear();
}


UValue & UObjectStruct::operator [](string s) {
  for (int i=0;i<size();i++)
	if (array[i].name==s)
	  return *array[i].val;
  static UValue n;
  return n;
}




