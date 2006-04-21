#ifndef UCONVERSION_H
#define UCONVERSION_H
#include "uobject.h"
using namespace urbi;
typedef unsigned char byte; 


/// Image format conversion functions.
int convertRGBtoYCrCb  (const byte* source, int sourcelen, byte* dest);
int convertYCrCbtoRGB  (const byte* source, int sourcelen, byte* dest);
int convertJPEGtoYCrCb (const byte* source, int sourcelen, byte* dest, 
                       int &size);
int convertJPEGtoRGB   (const byte* source, int sourcelen, byte* dest, 
                       int &size);

int convertRGBtoJPEG(const byte* source, int w, int h, byte* dest, 
                       int &size, int quality);
int convertYCrCbtoJPEG(const byte* source, int w, int h, byte* dest, 
                       int &size, int quality);


//sound format conversion functions.
int convert(const USound &source, USound &destination);


//image format conversion. JPEG compression not impletmented.
int convert(const UImage &source, UImage & destination);



#endif
