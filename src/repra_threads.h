#ifndef REPRA_THREADS_H
#define REPRA_THREADS_H

#include<pthread.h>
#include<stdbool.h>
#include<stdatomic.h>
#include "engine_internals.h"
#include "repra.h"
#include "backend_alsa.h"

#include<stdlib.h>

void* _audio_thread_internal ( void* data);
RepraHandle* repra_loop_engine();
void repra_wait ( RepraHandle* handle ) ;

extern volatile bool engine_running;
extern volatile bool is_secure_off;
#endif