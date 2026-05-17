
#include "alloc.h"


void* allocate_aligned_buffer ( size_t total_bytes ) {
    void* ptr = NULL;
    int result = posix_memalign(&ptr, 32 , total_bytes );
    
    if ( result ) {
        fprintf(stderr , "ERROR: NO se pudo reservar la memoria alineada\n");
        return NULL;
    }

    return ptr;
}

void* reallocate_aligned_buffer ( size_t total_bytes ,  void* old_ptr ) {
    void* ptr = NULL;
    int result = posix_memalign ( &ptr, 32, total_bytes );
    if ( result ) {
        fprintf(stderr , "ERROR: NO se pudo reservar la memoria alineada\n");
        return NULL;
    }

    if ( old_ptr ) {
        memcpy ( ptr, old_ptr, total_bytes / 2 );
        free( old_ptr );
    }
    return ptr;
}

void free_aligned_buffer( void* ptr ) { if ( ptr != NULL ) free ( ptr ); }

void free_repra_sound ( RepraSound* sound ) {
    if ( sound != NULL ) return;
    free_aligned_buffer ( sound -> data_left );
    free_aligned_buffer ( sound -> data_right );
    free ( sound );
}