#include <stdio.h>
#include "repra.h"
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

int main() {

    printf("INICIANDO ENGINE\n");
    RepraEngine* engine = repra_create( 512 );
    RepraHandle* handle = repra_loop_engine();
    if ( engine == NULL ) return -1;
    get_mp3_data( "/home/rey/Documentos/ReprAPI/tests/test_mp3.mp3");
    // repra_play ( "/home/rey/Documentos/ReprAPI/tests/test_wav6.wav" );
    
    //  repra_play ( "/home/rey/Documentos/ReprAPI/tests/test_wav5.wav" );
    
    //  sleep( 3 );
    
    
    //   for ( int i = 0 ; i < 2; i ++ ) {
    //      printf("EL MAIN : %d\n", i );
    //      //usleep( 1000 );
    //      sleep ( 1 );
    //      repra_play ( "/home/rey/Documentos/ReprAPI/tests/test_wav6.wav" );
        
    //  }
    //  repra_play ( "/home/rey/Documentos/ReprAPI/tests/test_wav2.wav" );
    // printf("ESPERANDO a que termine la musica para cerrar...\n");
    repra_wait(handle);
    repra_destroy(engine);
    printf("Engine DESTRUIDO\n");

    return 0;
}
