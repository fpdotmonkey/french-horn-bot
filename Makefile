
CXXFLAGS=-std=c++1y -g
LIBS=-lSDL2

foo: foo.o
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

foo.o: foo.cpp hello_world-sdl.h


clean:
	rm -rf *.o hello