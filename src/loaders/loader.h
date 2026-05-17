#ifndef REPRA_LOADER_H
#define REPRA_LOADER_H

#include "engine_internals.h"
#include "repra.h"
#include "repra_threads.h"

#include<stdint.h>

typedef struct {
    float* data;
    uint32_t length;
    uint32_t rate;
    uint16_t channels;
}RawSound;

int repra_play( const char* filepath );
float* repra_get_left_channel(RepraSound* sound);


#endif