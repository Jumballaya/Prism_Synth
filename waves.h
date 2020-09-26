#ifndef H_waves
#define H_waves

#include <Arduino.h>

uint8_t sine_wave(int t, uint16_t nSamples, uint8_t dutyCycle);
uint8_t triangle_wave(int t, uint16_t nSamples, uint8_t dutyCycle);
uint8_t square_wave(int t, uint16_t nSamples, uint8_t dutyCycle);
uint8_t sawtooth_wave(int t, uint16_t nSamples, uint8_t dutyCycle);

#endif