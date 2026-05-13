#ifndef WAV_INTERNAL_H
#define WAV_INTERNAL_H

#include<stdint.h>
#pragma pack(push , 1 )


typedef struct {
    char     chunk_id[4];
    uint32_t chunk_size;
    char     format[4];
    char     fmt_id[4];
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char     data_id[4];
    uint32_t data_size;
} WavHeader;
#pragma pack(pop)




#endif