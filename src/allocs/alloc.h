#ifndef REPRA_ALLOC_H
#define REPRA_ALLOC_H

#include "engine_internals.h"

#include<stdlib.h>
#include<stddef.h>
#include<string.h>

void* allocate_aligned_buffer ( size_t total_bytes );
void* reallocate_aligned_buffer ( size_t total_bytes ,  void* old_ptr );
void free_aligned_buffer ( void* ptr );
void free_repra_sound( RepraSound* sound);
#endif