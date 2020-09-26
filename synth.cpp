#include <Arduino.h>
#include <MIDI.h>
#include "synth.h"

#define ISR_CYCLE 16
#define MAX_VOLUME 32

MIDI_CREATE_DEFAULT_INSTANCE();

static unsigned long timer = 0;

struct ADSREnvelope {

    // Times in millis
    unsigned long attackTime;
    unsigned long decayTime;
    unsigned long releaseTime;

    // Amplitude modifiers from 0 - 1.0
    double ampStart;
    double ampSustain;

    // Times in millis
    unsigned long triggerOnTime;
    unsigned long triggerOffTime;

    // Check to see if note is actively pressed
    bool isNoteOn;

    ADSREnvelope() {
        attackTime = 200;
        decayTime = 100;
        releaseTime = 200;
        ampStart = 1.0;
        ampSustain = 0.8;
        triggerOnTime = 0;
        triggerOffTime = 0;
        isNoteOn = false;
    }

    double getAmplitude() {
        // Amplitude ranges from 0.0 - 1.0
        // It will be used to scale the volume
        double amplitude = 0.0;
        unsigned long lifetime = timer - triggerOnTime;

        if (isNoteOn) {
            // Attack
            if (lifetime <= attackTime) {
                amplitude = ((double)lifetime / (double)attackTime) * ampStart;
            }

            // Decay
            if (lifetime > attackTime && lifetime <= (attackTime + decayTime)) {
                amplitude = (((double)(lifetime - attackTime) / (double)decayTime) * (ampStart - ampSustain)) + ampSustain;
            }

            // Sustain
            if (lifetime > (attackTime + decayTime)) {
                amplitude = ampSustain;
            }
        } else {
            // Release
            amplitude = ((double)(timer - triggerOffTime) / (double)releaseTime) * (0.0 - ampSustain) + ampSustain;
        }

        if (amplitude < 0.001) {
            amplitude = 0.0;
        }

        return amplitude;
    }

    void noteOn() {
        triggerOnTime = timer;
        isNoteOn = true;
    }

    void noteOff() {
        triggerOnTime = timer;
        isNoteOn = false;
    }
};

Synth *synth;
ADSREnvelope envelope;

void handleNoteOn(byte channel, byte note, byte velocity) {
    synth->noteOn(channel, note, velocity);
}

void handleNoteOff(byte channel, byte note, byte velocity) {
    synth->noteOff(channel, note, velocity);
}

Synth* global_synth_int() {
    synth = new Synth();
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    synth->begin();
    return synth;
}

Synth::Synth() {
    samples = (uint8_t *)malloc(0);
    dutyCycle = 50;
    volume = 32;
    curVolume = 0;
    wave = Wave::SINE;
}

int mtfreq(int midi) {
    double exp = (midi - 69.0) / 12.0;
    return pow(2, exp) * 440.0;
}

void Synth::begin() {
    pinMode(13, OUTPUT);
    noInterrupts();
    DDRB |= (1 << PB3) + (1 << PB0);				  //OC2A, Pin 11
    TCCR2A = (1 << WGM21) + (1 << WGM20);			  //Fast PWM
    TCCR2A |= (0 << COM2A0) + (1 << COM2A1);		  //Set OC2A on compare match, clear OC2A at BOTTOM,(inverting mode).
    TCCR2B = (0 << CS22) + (0 << CS21) + (1 << CS20); //No Prescaling
    TIMSK2 = (1 << TOIE2);
    interrupts();
}

void Synth::noteOff(byte channel, byte note, byte velocity) {
    digitalWrite(13, LOW);
    stop();
}

void Synth::noteOn(byte channel, byte note, byte velocity) {
    digitalWrite(13, HIGH);
    int hz = mtfreq(note);
    play(hz);
}

// Set up nSamples, tPeriod and the actual buffered samples
void Synth::play(uint16_t freq) {
    envelope.noteOn();
    curVolume = 32.0;
    tPeriod = 1E6 / freq;
    nSamples = tPeriod / ISR_CYCLE;
    realloc(samples, nSamples);
    for (int i = 0; i < nSamples; i++) {
        uint8_t sample;
        switch (wave) {
            case Wave::SQUARE:
                sample = square_wave(i, nSamples, dutyCycle);
                break;
            case Wave::TRIANGLE:
                sample = triangle_wave(i, nSamples, dutyCycle);
                break;
            case Wave::SAWTOOTH:
                sample = sawtooth_wave(i, nSamples, dutyCycle);
                break;
            default:
                sample = sine_wave(i, nSamples, dutyCycle);
        }
        samples[i] = sample;
    }
}

void Synth::stop() {
    curVolume = 0;
    envelope.noteOff();
}

uint8_t Synth::step(uint16_t count) {
	return (samples[count]) * (envelope.getAmplitude()) * (volume / MAX_VOLUME);
}

void Synth::loop() {
    MIDI.read();
}

void Synth::setVolume(uint8_t vol) {
    volume = vol;
}

void Synth::setWave(uint8_t wav) {
    wave = wav;
}


// Interrupt
// Output the sound on OCR2A
ISR(TIMER2_OVF_vect) {
	static uint16_t count = 0;
    
	OCR2A = synth->step(count);

    timer += 16;
    if (count < synth->nSamples - 1) {
		count++;
	} else {
        count = 0;
    }
}
