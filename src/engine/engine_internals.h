#ifndef ENGINE_INTERNALS_H
#define ENGINE_INTERNALS_H

#include "repra.h"

struct RepraEngine {
    float* left_buffer;
    float* right_buffer;
    size_t buffer_size;
    float master_gain;
};

struct RepraSound{
    float* data_left;
    float* data_right;
    uint32_t total_samples;
    uint32_t sample_rate;
    uint32_t bits_per_sample;
};



#endif