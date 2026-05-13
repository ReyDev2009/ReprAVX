
#include "repra.h"
#include "engine_internals.h"
#include "dsp_avx2.h"
#include "alloc.h"
#include "backend_alsa.h"

snd_pcm_t *handle;


int init_alsa() {
    int err;
    
    if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Error abriendo PCM: %s\n", snd_strerror(err));
        return -1;
    }
    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca( &params );
    snd_pcm_hw_params_any( handle , params );
    snd_pcm_hw_params_set_access( handle , params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format( handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels( handle, params, 2 );
    unsigned int val = 44100;
    snd_pcm_hw_params_set_rate_near( handle, params , &val, 0 );

    snd_pcm_uframes_t period_size = 1024;
    snd_pcm_hw_params_set_period_size( handle, params, period_size , 0 );

    snd_pcm_uframes_t buffer_size = period_size * 4;
    snd_pcm_hw_params_set_buffer_size( handle, params, buffer_size );

    if ((err = snd_pcm_hw_params(handle, params)) < 0) {
        printf("Error configurando HW: %s\n", snd_strerror(err));
        return -1;
    }
    snd_pcm_prepare(handle);
    return 0;
}


void repra_fill_alsa_buffer( RepraSound* sound , int16_t* out_buffer , uint32_t start_sample, uint32_t num_samples) {
    for ( uint32_t i = 0 ; i < num_samples ; i ++ ) {
        uint32_t idx = i + start_sample;
        float left = sound -> data_left [ idx ] * P15;
        out_buffer [ i * 2 ] = ( int16_t )left;
        float right = sound -> data_right [ idx ] * P15;
        out_buffer [ i * 2 + 1 ] = ( int16_t )right;

    }
}

void repra_play_sound_alsa ( RepraSound* sound ) {
        if (!sound) {
            printf("DEBUG: El puntero de sonido es NULL\n");
            return;
        }
        uint32_t current_sample = 0;
        uint32_t chunk_size = 1024;

        int16_t* alsa_buff = allocate_aligned_buffer( chunk_size * 2 * sizeof ( int16_t ));

        while ( current_sample < sound -> total_samples ) {
            uint32_t to_write = chunk_size;
            if ( current_sample + to_write > sound -> total_samples )  {
                to_write = sound -> total_samples - current_sample;
            }

            dsp_preprare_alsa_buffer( sound -> data_left , sound -> data_right , alsa_buff, current_sample , to_write );
            //repra_fill_alsa_buffer( sound, alsa_buff , current_sample , to_write );
            
            snd_pcm_sframes_t frames = snd_pcm_writei( handle, alsa_buff, to_write);


            if ( frames == -EPIPE ) {
                printf("DEBUG: XRUN, La tarjeta se quedo sin datos, reiniciando...");
                snd_pcm_prepare(handle);
            }
            else if ( frames < 0 ) printf("DEBUG: Error de ALSA: %s\n", snd_strerror(frames));
            else current_sample += frames;
            
        }
        snd_pcm_drain(handle);
        
        free( alsa_buff );

}



int repra_update_engine ( uint32_t chunk_size) {
    static float* master_l = NULL;
    static float* master_r = NULL;
    static int16_t* alsa_buff = NULL;


    if ( !master_l ) {
        master_l = allocate_aligned_buffer( chunk_size * sizeof( float ));
        master_r = allocate_aligned_buffer ( chunk_size * sizeof( float ));
        alsa_buff = allocate_aligned_buffer( 2 * chunk_size * sizeof( int16_t));
    }

    dsp_fill(master_l, 0.0f, chunk_size);
    dsp_fill(master_r, 0.0f, chunk_size);

    int is_playing = 0;

    for ( int i = 0 ; i < MAX_ACTIVE_SOUNDS ; i ++ ) {
        if ( !active_slots [ i ].active ) continue;
        is_playing = 1;
        MixerSlot* slot = &active_slots[ i ];

        uint32_t samples_to_mix = chunk_size;
        
        if ( slot -> current_sample + samples_to_mix > slot -> sound -> total_samples) {
            samples_to_mix = slot -> sound -> total_samples - slot -> current_sample;
            
        }
        
        dsp_add_buffer( 
            master_l , 
            &slot -> sound -> data_left [ slot -> current_sample ],
            samples_to_mix
        );

        dsp_add_buffer( 
            master_r , 
            &slot -> sound -> data_right [ slot -> current_sample ],
            samples_to_mix
        );

        
        
        
        slot -> current_sample += samples_to_mix;
        if ( slot -> current_sample >= slot ->sound -> total_samples ) slot->active = 0;
    }
    
    if ( is_playing ) {
        
        dsp_normalize_buffer( master_l , chunk_size );
        dsp_normalize_buffer( master_r , chunk_size );
        dsp_preprare_alsa_buffer ( master_l, master_r, alsa_buff , 0 , chunk_size );
        
        snd_pcm_writei(handle, alsa_buff, chunk_size );
        return 0;
    }

    usleep(1000);
    return 1;
    
}




