all: umain.o

umain.o: umain.cc
	g++ -o umain.o -I../src/liburbi/uobject -Wno-deprecated -c umain.cc

clean:
	rm -f *.o
