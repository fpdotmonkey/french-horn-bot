#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

#include "tones.hpp"

int main(int argc, char **argv) {
  SDL_Init(SDL_INIT_AUDIO);

  for (int i = 1; i < argc; ++i) {
    auto song = read_file(argv[i]);
    Beeper b;

    for (const tone& n : song) {
      b . beep (n . hz, n . duration, n.attack);
    }

    b.wait();
  }

  return 0;
}
