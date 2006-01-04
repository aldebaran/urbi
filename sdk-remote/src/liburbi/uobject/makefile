all: libuobject.a

libuobject.a: uobject.template.h uobject.cc uvar.cc uobjectdata.cc uobjectdata.h usharedexternal.h
	./template_autogen.pl uobject.template.h > uobject.h
	g++ -g -o uobject.o -I.. -Wno-deprecated -c uobject.cc
	g++ -g -o uobjectdata.o -I.. -Wno-deprecated -c uobjectdata.cc
	g++ -g -o uvar.o -Wno-deprecated -c uvar.cc
	ar -r libuobject.a uobject.o uobjectdata.o uvar.o

clean:
	rm -f *.o *.a
