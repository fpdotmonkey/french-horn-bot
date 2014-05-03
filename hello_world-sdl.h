#include <SDL.h>
#include <SDL_audio.h>
#include <queue>
#include <cmath>
#include <stdlib.h>

const int AMPLITUDE = 28000;
const int FREQUENCY = 44100;

struct BeepObject
{
  double freq;
  int samplesLeft;
  double volume; // a number between 0 and 1
};

class Beeper
{
private:
  double v = 0; //v for velocity
  std::queue<BeepObject> beeps;
public:
  Beeper();
  ~Beeper();
  void beep(double freq, int duration, double volume);
  void generateSamples(Sint16 *stream, int length);
  void wait();
};

void audio_callback(void*, Uint8*, int);

Beeper::Beeper()
{
  SDL_AudioSpec desiredSpec; 
  // SDL_AudioSpec - descibes the format of some audio data
  desiredSpec.freq = FREQUENCY;
  desiredSpec.format = AUDIO_S16SYS;
  desiredSpec.channels = 1;
  desiredSpec.samples = 2048;
  desiredSpec.callback = audio_callback;
  desiredSpec.userdata = this;

  SDL_AudioSpec obtainedSpec;

  // you might want to look for errors here
  SDL_OpenAudio(&desiredSpec, &obtainedSpec);
  //opens the audio device with parametes &desiredSpec and writes the hardware parameters to &obtainedSpec

  // start play audio
  SDL_PauseAudio(0);
}

Beeper::~Beeper()
{
  SDL_CloseAudio();
  //turns the sound off
}

void Beeper::generateSamples(Sint16 *stream, int length)
{
  int i = 0;
  while (i < length) { //while the note ought to be playing,

    if (beeps.empty()) { //if there are no notes to play,
      while (i < length) {
	stream[i] = 0; //play silence.
	i++;
      }
      return;
    } //otherwise,
    BeepObject& bo = beeps.front(); //put a note from the queue and put in bo

    int samplesToDo = std::min(i + bo.samplesLeft, length);
    bo.samplesLeft -= samplesToDo - i; //tells bo how much longer it has to play

    while (i < samplesToDo) {//this all says how high the sine wave ought to be.Frequency adjusts when you set the height
      stream[i] = AMPLITUDE * bo.volume * std::sin(v * 2 * M_PI / FREQUENCY);
      i++;
      v += bo.freq;
    }

    if (bo.samplesLeft == 0) {
      beeps.pop(); //delete the note
    }
  }
}

void Beeper::beep(double freq, int duration, double volume)
{
  BeepObject bo;
  bo.freq = freq;
  bo.samplesLeft = duration * FREQUENCY / 1000;
  bo.volume = volume;

  SDL_LockAudio(); //protects the callback function
  beeps.push(bo); //puts bo in the queue beeps
  SDL_UnlockAudio(); //undoes SDL_LockAudio()
}

void Beeper::wait()
{
  int size;
  do {
    SDL_Delay(20); //wait 20ms
    SDL_LockAudio();
    size = beeps.size();
    SDL_UnlockAudio();
  } while (size > 0);

}

void audio_callback(void *_beeper, Uint8 *_stream, int _length)
{
  Sint16 *stream = (Sint16*) _stream;
  int length = _length / 2;
  Beeper* beeper = (Beeper*) _beeper;

  beeper->generateSamples(stream, length);
}

/*int main(int argc, char* argv[])
{
  SDL_Init(SDL_INIT_AUDIO);

  //setenv(SDL_AUDIODRIVER, alsa, 1);

  int duration = 1000;
  double Hz = 220;
  int amplitude = 28000;

  Beeper b;
  b.beep(Hz / 2, duration, amplitude / 100);
  //b.beep(0, 20);
  b.beep(Hz, duration, amplitude / 10);
  //b.beep(0, 20);
  b.beep(Hz * 2, duration, amplitude);
  //b.beep(0, 20);
  b.beep(Hz * 3, duration, amplitude * 10);
  //b.beep(20, 20);
  b.wait();

  return 0;
  }*/
