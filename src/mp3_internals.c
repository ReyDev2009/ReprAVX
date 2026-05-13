#include "mp3_internals.h"
#include<sys/stat.h>
#include "repra.h"

int parse_header ( uint32_t h , MP3Header* header ) {
    if ( ( h >> 21 ) != 0x7FF ) return -1;

    header->version          = (h >> 19) & 0x3;
    header->layer            = (h >> 17) & 0x3;
    header->error_protection = (h >> 16) & 0x1;
    header->bitrate_index    = (h >> 12) & 0xF;
    header->samplerate_index = (h >> 10) & 0x3;
    header->padding_bit      = (h >> 9) & 0x1;
    header->private_bit      = (h >> 8) & 0x1;
    header->channel_mode     = (h >> 6) & 0x3;
    header->mode_extension   = (h >> 4) & 0x3;
    header->copyright        = (h >> 3) & 0x1;
    header->original         = (h >> 2) & 0x1;
    header->emphasis         = (h >> 0) & 0x3;
    

    if (header->version == 1 || header->layer == 0 || 
        header->bitrate_index == 0 || header->bitrate_index == 15 || 
        header->samplerate_index == 3) {
        return -2; // Header corrupto
    }


    bool is_mpeg1 = ( header -> version == 3 ) ? 1 : 0;

    header->samplerate = samplerates_table [ header -> version ] [ header -> samplerate_index ];
    header->bitrate = bitrates_table [ is_mpeg1 ] [ header -> layer ][ header -> bitrate_index ] * 1000;


    if ( header -> layer == 1 ) {
        int coef = ( header -> version == 3 ) ? 144 : 72;
        header -> frame_size = ( coef * header->bitrate ) / header->samplerate + header->padding_bit;
    }
    else return -3;

    return 0;
}

uint32_t read_32be( const uint8_t* data) {
    return ( uint32_t ) ( data [ 0 ] << 24 )|
           ( uint32_t ) ( data [ 1 ] << 16 )|
           ( uint32_t ) ( data [ 2 ] << 8 )|
           ( uint32_t ) ( data [ 3 ] );
}

long get_filesize( const char* file){
    struct stat st;
    if ( stat ( file, &st ) == 0 ) {
        long size = st.st_size;
        return size;
    }
    return -1;
}


MP3Map* get_mp3_data ( const char* filepath ) {
    FILE* file = fopen ( filepath , "rb");
    long filesize = get_filesize( filepath );
    
    
    uint8_t* raw_data = allocate_aligned_buffer ( filesize );
    fread ( raw_data , 1, filesize , file );
    fclose( file );
    
    uint32_t* frame_offsets = allocate_aligned_buffer ( sizeof ( uint32_t ) * 10000);
    size_t count = 10000;
    int total_frames = 0;
    size_t pos = 0;

    
    
    while ( pos < filesize - 4 ) {
        uint32_t h = read_32be( &raw_data [ pos ] );

        MP3Header header;
        if ( parse_header( h, &header) == 0) {

            if ( total_frames >= count - 1) {
                count *= 2;
                frame_offsets = reallocate_aligned_buffer ( count , frame_offsets );
            }

            frame_offsets [ total_frames++ ] = pos;
            pos += header.frame_size;
            
        }
        else pos++;

    }
    
    printf("Frames: %d\n", total_frames );
    MP3Map map = { frame_offsets, total_frames };
    return &map;
}

uint32_t get_bits( BitStream* bs , uint32_t num_bits ) {
    int byte_pos = bs ->bit_pos >> 3;
    int bit_offset = bs -> bit_pos & 7;

    uint64_t window = __builtin_bswap64( *(uint64_t* )&bs->data[ byte_pos ] );
    window <<= bit_offset;
    uint32_t result = ( uint32_t ) ( window >> ( 64 - num_bits ));
    bs->bit_pos += num_bits;
    return result;
}

SideInfo* extract_side_info ( BitStream *bs ) {
    SideInfo si;
    si.main_data_begin = get_bits( bs, 9 );
    si.private_bits = get_bits ( bs, 3 );


    for ( int ch = 0; ch < 2; ch ++ ) {
        for ( int sc = 0; sc < 4 ; sc ++ ) {
            si.scfsi[ ch ] [ sc ] = get_bits(bs, 1 );
        }
    }

    for ( int gr = 0 ; gr < 2 ; gr ++ ) {
        for ( int ch = 0 ; ch < 2; ch ++ ) {
            GranuleInfo *gi = &si.gr [ gr ] [ ch ];

            gi->part2_3_length = get_bits ( bs , 12 );
            gi->big_values = get_bits( bs, 9 );
            gi->global_gain = get_bits ( bs, 8 );
            gi->scalefac_compress = get_bits ( bs , 4 );
            gi->window_switching_flag = get_bits( &bs, 1 );

            if ( gi->window_switching_flag ) {
                gi->block_type = get_bits( bs, 2 );
                gi->mixed_block_flag = get_bits( bs, 1 );
                for ( int i = 0 ; i < 2 ; i ++ ) {
                    gi->table_select [ i ] = get_bits ( bs, 5 );
                }
                for ( int i = 0 ; i < 3 ; i ++ ) {
                    gi->subblock_gain [ i ] = get_bits ( bs, 5 );
                }
                gi->region0_count = (gi->block_type == 2) ? 8 : 7;
                gi->region1_count = 20 - gi->region0_count;
            }
            else{
                for ( int i = 0 ; i < 3; i ++ ) {
                    gi->table_select_long[ i ] = get_bits( bs, 5 );
                }
                gi->region0_count = get_bits ( bs, 4 );
                gi->region1_count = get_bits ( bs, 3 );
                gi->block_type = 0;
            }
            gi->preflag = get_bits(bs, 1);
            gi->scalefac_scale = get_bits(bs, 1);
            gi->count1table_select = get_bits(bs, 1);
            
        }
    }

    return &si;
    
}


int proccess_mp3 ( const char* filepath , uint8_t* raw_data) {
    MP3Map* mp3map = get_mp3_data  ( filepath );
    uint8_t main_data_buffer[ 131072 ];
    int buffer_ptr = 0;

    for ( int frame = 0 ; frame < mp3map->count ; frame ++ ) {
        int pos = mp3map->offsets[ frame ];
        MP3Header header;
        uint32_t h = read_32be( &raw_data [ pos ]);
        parse_header ( h, &header);
        BitStream bs = {.bit_pos = 0, .data = &raw_data [ pos + 4 ]};
        SideInfo* si = extract_side_info ( &bs );

        int side_info_size = bs.bit_pos >> 3;
        size_t main_data_in_frame = header.frame_size - side_info_size - 4;
        
        memcpy ( &main_data_buffer [ buffer_ptr ] , raw_data [ pos + side_info_size + 4] , main_data_in_frame );
        uint32_t exceded_mask = ( buffer_ptr + main_data_in_frame ) >> 16;
        memmove ( &main_data_buffer[ 0 ], &main_data_buffer [ 65536 ], 512 * exceded_mask);
        buffer_ptr = ( buffer_ptr + main_data_in_frame ) & 0xFFFF;
        
        BitStream huffman_bs;
        huffman_bs.data = &main_data_buffer [ buffer_ptr - main_data_in_frame - si->main_data_begin ];
        huffman_bs.bit_pos = 0;
        
    }
}
