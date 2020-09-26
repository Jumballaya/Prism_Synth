#include "synth.h"

Synth *MainSynth;
void setup() {
    MainSynth = global_synth_int();
    MainSynth->setWave(Wave::SAWTOOTH);
}

void loop() {
    MainSynth->loop();
}
