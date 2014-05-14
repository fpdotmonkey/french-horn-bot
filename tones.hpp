#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>


#include <string>
#include <functional>
#include <vector>
#include <queue>

struct BeepObject
{
  double freq, duration;
  int samplesLeft;
  std::function<double(double, double)> volume; //volume at time (in seconds) = t
  double t { 0.0 };

  void IncTime (); 
};

class Beeper
{
private:
  double v = 0; //v for velocity
  std::queue<BeepObject> beeps;
public:
  Beeper();
  ~Beeper();
  void beep(double freq, double duration, std::function<double(double, double)>);

  void generateSamples(Sint16 *stream, int length);
  void wait();
};

struct tone {
  double hz, duration, volume;
  std::function<double(double, double)> attack;
};

std::vector<tone> read_file(std::string file);
