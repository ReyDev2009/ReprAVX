#ifndef BACKEND_ALSA_INTERNAL
#define BACKEND_ALSA_INTERNAL
#include "repra.h"
#include<alsa/asoundlib.h>

int init_alsa();
void repra_fill_alsa_buffer( RepraSound* sound , int16_t* out_buffer , uint32_t start_sample, uint32_t num_samples);
void repra_play_sound_alsa ( RepraSound* sound );
int repra_update_engine ( uint32_t chunk_size);

#endif