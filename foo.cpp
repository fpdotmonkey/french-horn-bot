#include <stdlib.h>
#include <string>
#include <iostream>
#include "hello_world-sdl.h"
#include <vector>

int main() {
  SDL_Init(SDL_INIT_AUDIO);

   auto song = read_file("mary.fhb");
  std::cout << song[4] . hz << std::endl;
  Beeper b;

  for (const tone& n : song) {
      b . beep (n . hz, n . duration, n.attack);
  }

  b.wait();

  return 0;
}
