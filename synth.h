#ifndef H_synth
#define H_synth

#include "waves.h"

enum Wave {
    SINE = 0,
    SQUARE,
    TRIANGLE,
    SAWTOOTH,
    NOISE,
};

class Synth {

    public:
        Synth();
        
        uint16_t nSamples;

        void begin();

        void noteOff(byte channel, byte note, byte velocity);
        void noteOn(byte channel, byte note, byte velocity);

        uint8_t step(uint16_t count);
        void play(uint16_t freq);
        void stop();
        void loop();

        void setVolume(uint8_t vol);
        void setWave(uint8_t wav);

        uint8_t  curVolume;

    private:
        uint32_t tPeriod;
        uint8_t  *samples;
        uint8_t  dutyCycle;
        uint8_t  volume;
        uint8_t  wave;

};

Synth* global_synth_int();

#endif