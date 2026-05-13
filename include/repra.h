#ifndef REPRA_H
#define REPRA_H

#include <stddef.h>
#include<stdio.h>
#include<stdint.h>
#include<pthread.h>
#include<stdbool.h>

#define P15 32767.0f
#define P16 65536.0f
#define P31 2147483648.0f

#define MAX_ACTIVE_SOUNDS 16

typedef struct RepraEngine RepraEngine;
typedef struct RepraSound RepraSound;

typedef struct {
    RepraSound* sound;
    uint32_t current_sample;
    bool active;
} MixerSlot;


typedef struct {
    RepraSound* sound;
} AudioThreadArgs;


extern MixerSlot active_slots[ MAX_ACTIVE_SOUNDS ] ;
extern int playing;

typedef struct RepraHandle{
    pthread_t thread;
} RepraHandle;

typedef struct MP3Map{
    uint32_t* offsets;
    int count;
} MP3Map;

typedef struct MP3Map MP3Map;

extern MP3Map get_mp3_data ( const char* filepath );
RepraHandle* repra_play_sound_async ( RepraSound* sound );
RepraHandle* repra_loop_engine();
void repra_wait ( RepraHandle* handle );

RepraEngine* repra_create(size_t buffer_size);
void repra_destroy(RepraEngine* engine);
RepraSound* repra_load_wav ( const char* filepath );

float* repra_get_left_channel(RepraSound* sound);
float* repra_get_right_channel(RepraSound* sound);
uint32_t repra_get_total_samples(RepraSound* sound);

void dsp_apply_gain( float* buffer, float gain , size_t size );

void repra_play_sound_alsa ( RepraSound* sound );
int repra_play( const char* filepath );



#endif