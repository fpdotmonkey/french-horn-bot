#include <cmath>
#include <stdlib.h>
#include <fstream>
#include <cstring>
#include <map>
#include <iostream>

#include "tones.hpp"

const int AMPLITUDE = 28000;
const int FREQUENCY = 44100;
const double DT = 1.0 / double(FREQUENCY);

const double A = 440;

double noteFrequency(int halfSteps) {
  return pow(pow(2.0, 1.0/12.0), halfSteps) * A;
}

const double Bb = noteFrequency(1);
const double B = noteFrequency(2);
const double C = noteFrequency(3);
const double Db = noteFrequency(4);
const double D = noteFrequency(5);
const double Eb = noteFrequency(6);
const double E = noteFrequency(7);
const double F = noteFrequency(8);
const double Gb = noteFrequency(9);
const double G = noteFrequency(10);
const double Ab = noteFrequency(11);
const double REST = 0;

void BeepObject::IncTime () { t += DT; }


double note(const double note, int octave) {
  double pitch = note * pow(2, octave - 4);
  return pitch;
}

double char2note(char * note) {
  if (strcmp(note, "A") == 0) return A;
  else if (strcmp(note, "Bb") == 0) return Bb;
  else if (strcmp(note, "B") == 0) return B;
  else if (strcmp(note, "C") == 0) return C;
  else if (strcmp(note, "Db") == 0) return Db;
  else if (strcmp(note, "D") == 0) return D;
  else if (strcmp(note, "Eb") == 0) return Eb;
  else if (strcmp(note, "E") == 0) return E;
  else if (strcmp(note, "F") == 0) return F;
  else if (strcmp(note, "Gb") == 0) return Gb;
  else if (strcmp(note, "G") == 0) return G;
  else return Ab;
}

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
      double volume = bo.volume(bo.t, bo.duration);
      stream[i] = AMPLITUDE * volume * std::sin(v * 2 * M_PI / FREQUENCY);
      i++;
      v += bo.freq;
      bo . IncTime();
    }

    if (bo.samplesLeft == 0) {
      beeps.pop(); //delete the note
    }
  }
}

void Beeper::beep(double freq, double duration, std::function<double(double, double)> volume)
{
  // BeepObject bo;
  // bo.freq = freq;
  // bo.samplesLeft = duration * FREQUENCY / 1000;
  // bo.volume = volume;

  SDL_LockAudio(); //protects the callback function
  beeps.push(BeepObject { freq, duration, int(duration * FREQUENCY), volume });
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

std::function<double(double)> campled(std::function<double(double)> f) {
  return [f](double x) -> double {
    double y = f(x);
    return std::max (std::min (1.0, y), 0.0);
  };
}

std::function<double(double, double)> attack (double attack_time,
                                              double fade_out_time,
                                              double attack_vol,
                                              double initial_vol) {
  double t0 = 0.0;
  double t1 = attack_time / 2.0;
  double t2 = attack_time;
  double attack_difference = attack_vol - initial_vol;

  return [t0, t1, t2, attack_difference,
          fade_out_time, attack_vol, initial_vol]
    (double t, double total_time) -> double {
    double t3 = total_time - fade_out_time;
    if (t < t1) {
      return attack_vol * std::sqrt(t / t1);
    } else if (t < t2) {
      return attack_vol -  attack_difference * (t - t1) / (t2 - t1);
    } else if (t < t3) {
      return initial_vol;
    } else {
      double remaining = total_time - t;
      double d = remaining / fade_out_time;
      return initial_vol * d * d;
    }
  };
}

std::function<double(double, double)> const_vol(double x) {
  return [x](double, double) { return x; };
}


typedef std::function<double(double, double)> AttackFun;
typedef std::map<std::string, AttackFun> AttackMap;

struct NamedAttack {
  std::string name;
  AttackFun fun;
};

NamedAttack articulation(std::string line) {
  char *str;
  str = (char *)alloca(line.size() + 1);
  memcpy(str, line.c_str(), line.size() + 1);
  char * data = strtok(str, "  ");

  double attack_a, attack_b, attack_c, attack_d;

  char * name = data;
  data = strtok(NULL, "  ");
  attack_a = atof(data);
  data = strtok(NULL, "  ");
  attack_b = atof(data);
  data = strtok(NULL, "  ");
  attack_c = atof(data);
  data = strtok(NULL, "  ");
  attack_d = atof(data);

  NamedAttack named_attack { name, attack(attack_a, attack_b, attack_c, attack_d) };
  return named_attack;
}

std::vector<NamedAttack> get_attacks(std::string file) {
  std::string line;
  std::ifstream fha;
  std::vector<NamedAttack> articulations;
  fha.open (file, std::ios::in);
  if (fha.is_open()) {
    while (std::getline(fha, line)) {
      articulations.push_back(articulation(line));
    }
    fha.close();

    return articulations;
  }
  else {
    std::cout << "ERROR UNABLE TO OOPEN FILE" << std::endl;
    return articulations;
  }
}

AttackFun get_articulation(std::string articulation, std::string file) {
  AttackFun Fun;
  for (int i = 0; i < get_attacks(file).size(); ++i) {
    if (articulation == get_attacks(file).at(i).name)
      Fun = get_attacks(file).at(i).fun;
  }

  return Fun;
}

tone parse_string(std::string line) {
  tone tone;
  char *str;
  str = (char *)alloca(line.size() + 1);
  memcpy(str, line.c_str(), line.size() + 1);
  char * data = strtok(str, "  ");
  
  if (strcmp(data, "REST") != 0) {
    double notef = char2note(data);
    data = strtok(NULL, "  ");
    int octave = atoi(data);
    tone.hz = note(notef, octave);
    data = strtok(NULL, "  ");
    tone.duration = atof(data);
    data = strtok(NULL, "  ");
    tone.volume = atof(data);
    data = strtok(NULL, "  ");
    std::cout << data << "\n";
    std::string attack (data);
    // attack << data;
    tone.attack = get_articulation(attack, "ATTACKS.fha");
  }
  else {
    tone.hz = 0;
    tone.volume = 0;
    data = strtok(NULL, "  ");
    tone.duration = atof(data);
    tone.attack = const_vol(tone.volume);
  }
  // std::cout << tone.hz << std::endl;
  // std::cout << tone.duration << std::endl;
  // std::cout << tone.volume << std::endl << std::endl;

  return tone;
}

std::vector<tone> read_file(std::string file) { //file is of format name.fhb
  std::cout << "START" << std::endl;
  std::string line;
  std::ifstream song;
  std::vector<tone> vsong;
  song.open (file, std::ios::in);
  if (song.is_open()) {
    std::cout << "READING" << std::endl;
    while (std::getline(song, line)) {
      vsong.push_back(parse_string(line));
      std::cout << "..." << std::endl;
    }
    song.close();
    std::cout << "FINISHED" << std::endl;
    return vsong;
  }
  else {
    std::cout << "ERROR: UNABLE TO OPEN FILE" << std::endl;
    return vsong;
  }
}
