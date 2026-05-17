#include "repra_threads.h"


volatile bool engine_running = true;
volatile bool is_secure_off = true;
void* _audio_thread_internal ( void* data) {
    while ( engine_running ) {
        is_secure_off = ( bool ) repra_update_engine ( 1024 );
    }
    return NULL;
}

RepraHandle* repra_loop_engine() {
    
    RepraHandle *h = malloc ( sizeof ( RepraHandle ));
    pthread_create( &h -> thread , NULL, _audio_thread_internal , h);
    return h;
}

void repra_wait ( RepraHandle* handle ) {
    if ( handle ) {
        while ( !is_secure_off ) sleep ( 1 );
        engine_running = false;
        pthread_join( handle -> thread , NULL );
    }
}



