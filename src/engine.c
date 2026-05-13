#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H
#include "engine_internals.h"
#include "alloc.h"
#include "repra.h"
#include "backend_alsa.h"
#include "repra_threads.h"

MixerSlot active_slots [ MAX_ACTIVE_SOUNDS ] = {0};
int playing = 0;


RepraEngine* repra_create(size_t buffer_size) {
    RepraEngine* engine = malloc ( sizeof ( RepraEngine ));
    if ( !engine ) { 
        fprintf(stderr , "ERROR: Ha ocurrido un error al resevrar memoria para el Engine ");
        return NULL;
    }

    engine -> left_buffer = allocate_aligned_buffer( buffer_size * sizeof ( float ));
    engine -> right_buffer = allocate_aligned_buffer( buffer_size * sizeof ( float ));
    engine -> buffer_size = buffer_size;
    engine -> master_gain = 1.0f;
    if ( init_alsa() < 0 ) {
        fprintf( stderr , "ERROR: Alsa no se ha inciado correctamente\n");
        return NULL;
    }
    
    
    return engine;
}

void repra_destroy ( RepraEngine* engine ) {
    if ( engine == NULL ) return;
    engine_running = false;
    free_aligned_buffer( engine -> left_buffer );
    free_aligned_buffer( engine -> right_buffer );
    free ( engine );
}



#endif