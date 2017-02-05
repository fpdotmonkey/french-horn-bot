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
    BeepObject& bo = beeps.front(); //take a note from the queue and put in bo

    int samplesToDo = std::min(i + bo.samplesLeft, length);
    bo.samplesLeft -= samplesToDo - i; //tells bo how much longer it has to play

    while (i < samplesToDo) {//this all says how high the sine wave ought to be.Frequency adjusts when you set the height
      double volume = bo.volume * bo.attack(bo.t, bo.duration);
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

typedef std::function<double(double, double)> AttackFun;


void Beeper::beep(double freq,
                  double duration,
                  double volume,
                  AttackFun attack) {
  // BeepObject bo;
  // bo.freq = freq;
  // bo.samplesLeft = duration * FREQUENCY / 1000;
  // bo.volume = volume;

  SDL_LockAudio(); //protects the callback function
  beeps.push(BeepObject { freq,
              duration,
              volume,
              int(duration * FREQUENCY),
              attack });
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


//
// This attack function is completely linear and only works for
// attacks that scale linearly with note length.  Some organs may have
// this characteristic
//
// The "attack" is an initial increase of volume, generally, but not
// stictly, over the ultimate sustain volume.  "decay" is the drop
// from the attack volume down to the sustain volume.  "sustain" is
// the volume that characterizes the note.  It however doesn't have to
// be the majority of the note's length in time.  The "release" is the
// drop in volume from "sustain" down to 0.
//
//
// note_descriptor contains:
//    double t_attack
//    double l_attack
//    double t_decay
//    double t_sustain
//    double l_sustain
//    double t_release
//
// The time descriptors (t_*) represent the percent of the note that
// element takes up.  Therefore, 
//    t_attack + t_decay + t_sustain + t_release = 1
//
// The relative volume descriptors (l like the symbol for sound intesity
// level) should be between 0 and 1
// 
AttackFun attack_new (std::vector<double> note_descriptor) {
    std::cout << "PING" << std::endl;
    double t0 = 0.0;
    double t1 = note_descriptor[0]; // t_attack
    double t2 = note_descriptor[2]; // t_decay
    double t3 = note_descriptor[3]; // t_sustain
    double t4 = note_descriptor[5]; // t_release
    if (t0 + t1 + t2 + t3 + t4 != 0) {
        std::cout
            << "ERROR!! Time discriptors for attack do not add up to 1.0"
            << std::endl;
    }

    double l1 = note_descriptor[1]; // l_attack
    double l2 = note_descriptor[4]; // l_sustain
    if (l1 > 1 || l2 > 1) {
        std::cout
            << "ERROR!! Relative volume descriptors must be less than 1.0"
            << std::endl;
    }
    
    return [t0, t1, t2, t3, t4, l1, l2]
        (double t, double total_time) -> double {
        
        double attack_rate  = (l1 - 0) / (t1 - t0);
        double decay_rate   = (l2 - l1) / (t2 - t1);
        double sustain_rate = (l2 - l2) / (t3 - t2); // = 0 ==> const vol
        double release_rate  = (0 - l2) / (t4 - t3);

        double t_rel = t / total_time;
        
        if (t_rel < t1) // attack
            return attack_rate * t_rel;
        else if (t_rel < t2) // decay
            return attack_rate * t1 + decay_rate * t_rel;
        else if (t_rel < t3) // sustain
            return attack_rate * t1 + decay_rate * t2;
        else
            return attack_rate * t1 + decay_rate * t2 + release_rate * t_rel;
    };
}



/*
AttackFun attack (double attack_time,
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
    }
    else if (t < t2) {
      return attack_vol -  attack_difference * (t - t1) / (t2 - t1);
    }
    else if (t < t3) {
      return initial_vol;
    }
    else {
      double remaining = total_time - t;
      double d = remaining / fade_out_time;
      return initial_vol * d * d;
    }
  };
  }*/

AttackFun const_vol(double x) {
  return [x](double, double) { return x; };
}


typedef std::map<std::string, AttackFun> AttackMap;

struct NamedAttack {
  std::string name;
  AttackFun fun;
};

NamedAttack articulation(std::string line) {  // TODO: make this
                                              // function work for a
                                              // general attack function
  char *str;
  str = (char *)alloca(line.size() + 1);
  memcpy(str, line.c_str(), line.size() + 1);
  char * data = strtok(str, "  ");

  double attack_a, attack_b, attack_c, attack_d, attack_e, attack_f;

  char * name = data;

  NamedAttack named_attack;
  // std::cout << "PING @ articulation" << std::endl;
  // if (strcmp(data, "") == 0) {
  //   std::cout << "no attack" << std::endl;
  //   named_attack = { "ZZZ", const_vol(0.3) };
  //   return named_attack;
  // }

  std::cout << "ON" << std::endl;

  if (strcmp(data, "ZZZ") == 0) {
    data = strtok(NULL, "  ");
    attack_a = atof(data);
    data = strtok(NULL, "  ");
    attack_b = atof(data);
    data = strtok(NULL, "  ");
    attack_c = atof(data);
    data = strtok(NULL, "  ");
    attack_d = atof(data);
    data = strtok(NULL, "  ");
    attack_e = atof(data);
    data = strtok(NULL, "  ");
    attack_f = atof(data);

    std::cout << "PONG" << std::endl;

    std::vector<double> attackVector  {attack_a,
            attack_b,
            attack_c,
            attack_d,
            attack_e,
            attack_f };
    
    named_attack = {name, attack_new( attackVector ) };
  }
  else {
    named_attack = { "ZZZ", const_vol(0.3) };
  }
  
  return named_attack;
}

std::vector<NamedAttack> get_attacks(std::string file) {
    std::cout << "RALLY" << std::endl;
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
        std::cout << "ERROR UNABLE TO OPEN FILE-open_attack" << std::endl;
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
  // std::cout << "PING @ parse_string" << std::endl;
  
  if (strcmp(data, "REST") != 0) {
    // std::cout << "PING @ beginning of parse_string condition" << std::endl;
    double notef = char2note(data);
    data = strtok(NULL, "  ");
    int octave = atoi(data);
    tone.hz = note(notef, octave);
    data = strtok(NULL, "  ");
    tone.duration = atof(data);
    data = strtok(NULL, "  ");
    tone.volume = atof(data);
    data = strtok(NULL, "  ");
    
    if (data == NULL) {
        //std::cout << "Unspecified attack.  Default: ZZZ" << std::endl;
        tone.attack = const_vol(0.3);
    }
    else {
        //std::cout << data << std::endl;
      
        std::string attack (data);
        // attack << data;
        // std::cout << "PING @ before tone.attack assigned in parse_string" << std::endl;
        tone.attack = get_articulation(attack, "NEW_ATTACKS.fha");
    }
  }
  else {
      //std::cout << "REST" << std::endl;
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
      //std::cout << "READING" << std::endl;
      while (std::getline(song, line)) {
          vsong.push_back(parse_string(line));
          //std::cout << "..." << std::endl;
    }
    song.close();
    //std::cout << "FINISHED" << std::endl;
    return vsong;
  }
  else {
      std::cout << "ERROR: UNABLE TO OPEN FILE" << file << std::endl;
    return vsong;
  }
}
