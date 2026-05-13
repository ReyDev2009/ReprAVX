#include "engine_internals.h"
#include "wav_internals.h"
#include "alloc.h"
#include "loader.h"
#include "repra.h"



float* repra_get_left_channel(RepraSound* sound) {
    return sound->data_left;
}

float* repra_get_right_channel(RepraSound* sound) {
    return sound->data_right;
}

uint32_t repra_get_total_samples(RepraSound* sound) {
    return sound->total_samples;
}

int repra_play ( const char* filepath ) {
    RepraSound* sound =  repra_load_wav( filepath );
    if ( sound == NULL ) return -1;
    for ( int i = 0 ; i < MAX_ACTIVE_SOUNDS ; i ++ ){
        if ( active_slots [ i ].active ) continue;
        active_slots[ i ].sound = sound;
        active_slots[ i ].current_sample = 0;
        active_slots[ i ].active = true;
        is_secure_off = false;
        
        return i;
    }
    return -1;
}


void read_data_by_bits_per_sample ( float* left, float* right , WavHeader* header , FILE* file ) {
    uint32_t bytes_per_sample = header->bits_per_sample / 8;
    uint32_t num_samples = header -> data_size / ( header -> num_channels * bytes_per_sample);
    uint16_t channels = header -> num_channels;
    uint32_t bps = header -> bits_per_sample;
    uint16_t format = header -> audio_format;

    //printf("bps %d", bps);
    void* tmp_buffer = malloc ( header -> chunk_size );
    if ( !tmp_buffer ) return;
    fread ( tmp_buffer, 1, header -> chunk_size , file );

    if ( bps == 16 ) {
        int16_t* data = ( int16_t* )tmp_buffer;
        for ( uint32_t i = 0 ; i < num_samples ; i ++ ) {
            left [ i ] = ( float ) data[ i * channels ] / P15;
            right [ i ] = ( channels > 1 ) ? ( float )  data [ i * channels + 1 ] / P15 : left [ i ];
        }
    }
    else if ( bps == 24 ) {
        uint8_t* data = ( uint8_t* ) tmp_buffer;
        for ( uint32_t i = 0 ; i < num_samples ;  i ++ ) {
            int32_t bpx = i * channels * 3;
            int32_t l_32 =  ( data [ bpx ] << 8 )|
                            ( data [ bpx + 1 ] << 16 )|
                            ( data [ bpx + 2 ] << 24 );
            left [ i ] = ( float ) l_32 / P31;
            right [ i ] = ( channels > 1 ) ? ( float ) ( ( data [ bpx + 3 ] << 8 )|
            ( data [ bpx + 4 ] << 16 )|
            ( data [ bpx + 5 ] << 24 ) ) / P31 :
            left [ i ];
            
            
        }
    }
    else if ( bps == 32 ) {
        if ( format == 3 ) {
            float* data = ( float* )tmp_buffer;
            for ( int i = 0 ; i < num_samples ; i ++ ) {
                left [ i ] = data [ i * channels ];
                right [ i ] = ( channels > 1 ) ? data [ i * channels + 1 ] : left [ i ];
            }
        }
        else {
            int32_t* data = ( int32_t* ) tmp_buffer;
            for ( int i = 0 ; i < num_samples ;i ++ ) {
                left [ i ] = ( float ) data [ i * channels ] / P31;
                right [ i ] = ( channels > 1 ) ? 
                                        ( float ) data [ i * channels + 1 ] / P31:
                                        left [ i ];
            }
        }
    }

    free ( tmp_buffer );

}


RepraSound* repra_load_wav ( const char* filepath ) {
    FILE* file = fopen(filepath , "rb");
    if ( !file ) return NULL;

    WavHeader wavheader;
    if (fread( &wavheader, sizeof( WavHeader ), 1, file ) < 1 ) {
        fclose(file);
        return NULL;
    }

    if ( wavheader.bits_per_sample != 16 && wavheader.bits_per_sample != 24 && wavheader.bits_per_sample != 32  ) {
        fclose( file );
        return NULL;
    }
    
    uint32_t bytes_per_sample = wavheader.bits_per_sample / 8;
    uint32_t num_sample = wavheader.data_size / ( bytes_per_sample * wavheader.num_channels );
    uint32_t safe_sample = ( num_sample + 7 ) & ~7;
    
    RepraSound* sound = malloc ( sizeof ( RepraSound ));
    if ( !sound ) {
        fprintf( stderr , "ERROR: No se pudo reservar la memoria para el sonido ");
        return NULL;
    }
    
    sound -> total_samples = safe_sample;
    sound ->sample_rate = wavheader.sample_rate;

    sound -> data_left = allocate_aligned_buffer( num_sample * sizeof ( float ));
    sound -> data_right = allocate_aligned_buffer ( num_sample * sizeof ( float ));
    sound -> bits_per_sample = wavheader.bits_per_sample;

    read_data_by_bits_per_sample ( 
        sound -> data_left, 
        sound ->data_right,
        &wavheader,
        file
    );

    fclose(file);
    return sound;
    
    
}