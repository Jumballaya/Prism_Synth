#include "waves.h"

uint8_t sine_wave(int t, uint16_t nSamples, uint8_t dutyCycle) {
    return 128 + (127 * sin(2 * PI * t / nSamples));
}

uint8_t triangle_wave(int t, uint16_t nSamples, uint8_t dutyCycle) {
    uint16_t nDuty = (dutyCycle * nSamples) / 100;
    if (t < nDuty) {
        return 255 * (double)t / nDuty; //Rise
    } else {
        return 255 * (1 - (double)(t - nDuty) / (nSamples - nDuty)); //Fall
    }
}

uint8_t square_wave(int t, uint16_t nSamples, uint8_t dutyCycle) {
    uint16_t nDuty = (dutyCycle * nSamples) / 100;
    return t < nDuty ? 255 : 0;
}

uint8_t sawtooth_wave(int t, uint16_t nSamples, uint8_t dutyCycle) {
    uint16_t nDuty = (dutyCycle * nSamples) / 100;
    if ((2 * t) < nDuty) {
        nDuty = nDuty - (2 * t);
    }
    return (127 - (127 / (t / nDuty))) + 128;
}