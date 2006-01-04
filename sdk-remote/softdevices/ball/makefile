all: ball

ball: ball.h ball.cc
	(cd .. && make) 
	g++ -o ball.o -I../../src/liburbi/uobject -Wno-deprecated -c ball.cc
	g++ -o ball ball.o ../umain.o ../../src/liburbi/uobject/libuobject.a -lurbi -pthread

clean:
	rm -f *.o ball
