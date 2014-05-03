//
// beep_testing.cpp
//
// French Horn Bot
// Copyright (c) Fletcher Porter 2014
//

#include "hello_world-sdl.h"

void glissando(double start, double end, double time);

double volume_scale(double x) {
  double y = x**2 / 16;
  return 0.5;
}

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_AUDIO);

  //setenv(SDL_AUDIODRIVER, alsa, 1);

  int duration = 1000;
  double Hz = 220;
  double volume = 0.5;

  Beeper b;
  b.beep(Hz / 2, duration, volume_scale(1.0));
  //b.beep(0, 20);
	 b.beep(Hz, duration, volume_scale(2.0));
  //b.beep(0, 20);
	 b.beep(Hz * 2, duration, volume_scale(3.0));
  //b.beep(0, 20);
	 b.beep(Hz * 3, duration, volume_scale(4.0));
  //b.beep(20, 20);
  b.wait();

  return 0;
}
