#include <stdlib.h>
#include <string>
#include <iostream>
#include "hello_world-sdl.h"
#include <vector>

int main() {
  SDL_Init(SDL_INIT_AUDIO);

  std::string foo = "0 1 2 3 4 5/0";
  char *str;
  str = (char *)alloca(foo.size() + 1);
  memcpy(str, foo.c_str(), foo.size() + 1);
  char * data = strtok(str, "  ");

  int x = sizeof(data) / sizeof(char);

  std::cout << x << std::endl << std::endl;
  std::cout << data << std::endl;
  std::cout << strtok(NULL, "  ") << std::endl;
  std::cout << strtok(NULL, "  ") << std::endl;
  std::cout << strtok(NULL, "  ") << std::endl;
  std::cout << strtok(NULL, "  ") << std::endl;
  std::cout << strtok(NULL, "  ") << std::endl;
  std::cout << strtok(NULL, "  ") << std::endl;

  //  auto song = read_file("mary.fhb");
  // std::cout << song[4] . hz << std::endl;
  // Beeper b;

  // for (const tone& n : song) {
  //     b . beep (n . hz, n . duration, n.attack);
  // }

  // b.wait();

  return 0;
}
