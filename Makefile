all: foo

CXXFLAGS=-std=c++1y -g
LIBS=-lSDL2

foo: foo.o tones.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

foo.o: foo.cpp tones.hpp
tones.o: tones.cpp tones.hpp

clean:
	rm -rf *.o hello