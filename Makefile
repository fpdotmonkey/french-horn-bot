CXX=clang++
CXXFLAGS=$(shell sdl-config --cflags)
LDFLAGS=
LIBS=$(shell sdl-config --static-libs)

all: hello_world-sdl

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

hello_world-sdl: hello_world-sdl.o
	$(CXX) -o $@ $< $(LDFLAGS) $(LIBS)

clean:
	rm -fr hello_world-sdl hello_world-sdl.o
