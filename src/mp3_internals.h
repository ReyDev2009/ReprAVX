#ifndef MP3_INTERNALS_H
#define MP3_INTERNALS_H

#include "repra.h"
#include "alloc.h"

typedef struct {
    int version;          
    int layer;            
    int error_protection; 
    int bitrate_index;    
    int samplerate_index; 
    int padding_bit;      
    int private_bit;      
    int channel_mode;     
    int mode_extension;   
    int copyright;        
    int original;         
    int emphasis;         
    
    // Calculados
    int bitrate;          
    int samplerate;       
    int frame_size;       
} MP3Header;

typedef struct {
    const uint32_t* data;
    int bit_pos;
} BitStream;

typedef struct {
    uint16_t main_data_begin;
    uint8_t private_bits;
    uint8_t scfsi [ 2 ] [ 4 ];
    GranuleInfo gr[ 2 ] [ 2 ];
} SideInfo;

typedef struct {
    uint16_t part2_3_length;    // 12 bits
    uint16_t big_values;        // 9 bits
    uint8_t  global_gain;       // 8 bits
    uint8_t  scalefac_compress; // 4 bits
    uint8_t  window_switching_flag; // 1 bit

    // Campos si window_switching_flag == 1
    uint8_t  block_type;       // 2 bits
    uint8_t  mixed_block_flag; // 1 bit
    uint8_t  table_select[2];  // 2 * 5 bits
    uint8_t  subblock_gain[3]; // 3 * 3 bits

    // Campos si window_switching_flag == 0
    uint8_t  table_select_long[3]; // 3 * 5 bits
    uint8_t  region0_count;        // 4 bits
    uint8_t  region1_count;        // 3 bits

    uint8_t  preflag;            // 1 bit
    uint8_t  scalefac_scale;     // 1 bit
    uint8_t  count1table_select; // 1 bit
} GranuleInfo;

const int samplerates_table[4][4] = {
    {11025, 12000, 8000, 0},  // 0 = MPEG-2.5
    {0, 0, 0, 0},             // 1 = Reservado (Error si cae aquí)
    {22050, 24000, 16000, 0}, // 2 = MPEG-2
    {44100, 48000, 32000, 0}  // 3 = MPEG-1
};

// [Es MPEG-1? (0=No, 1=Sí)][Layer (0 a 3)][Bitrate Index (0 a 15)]
// El Layer en el header es: 1=Layer III, 2=Layer II, 3=Layer I
const int bitrates_table[2][4][16] = {
    // Índice 0: MPEG-2 y MPEG-2.5 (Comparten tabla de bitrates)
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // Layer 0 (Reservado)
        {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0}, // Layer III (MP3)
        {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0}, // Layer II
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0}  // Layer I
    },
    // Índice 1: MPEG-1
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // Layer 0 (Reservado)
        {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0}, // Layer III (MP3)
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0}, // Layer II
        {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0}  // Layer I
    }
};

#endif