#include<immintrin.h>
#include<stddef.h>
#include "repra.h"
#include "dsp_avx2.h"

void dsp_fill ( float* buffer, float value, size_t size ) {
    __m256 v_value = _mm256_set1_ps( value );
    for ( int i = 0; i < size; i += 8 ) { _mm256_store_ps(&buffer[ i ], v_value ); }
}

void dsp_add_buffer( float* out_buffer, float* in_buffer , uint32_t size) {
    uint32_t i = 0;
    uint32_t safe_size = size & ~7;
    
    for ( ; i < safe_size ; i += 8 ) {
        __m256 out_v = _mm256_load_ps( &out_buffer [ i ]);
        __m256 in_v = _mm256_load_ps( &in_buffer [ i ]);
        __m256 res = _mm256_add_ps( in_v, out_v );
        _mm256_store_ps( &out_buffer[ i ] , res );
    }

    for ( ; i < size; i ++ ) { 
        out_buffer [ i ] += in_buffer [ i ]; 
    }
}

void dsp_normalize_buffer ( float* mixer_buffer , int num_samples ) {
    __m256 v_max = _mm256_setzero_ps ();
    __m256 sign_mask = _mm256_set1_ps(-0.0f);
    int i = 0;
    for ( ; i < num_samples ; i += 8 ) {
        __m256 data = _mm256_load_ps( &mixer_buffer[ i ]);
        __m256 abs_data = _mm256_andnot_ps ( sign_mask , data );
        v_max = _mm256_max_ps( abs_data , v_max);
    }


    __m128 high = _mm256_extractf128_ps( v_max , 1 );
    __m128 low = _mm256_castps256_ps128( v_max );
    __m128 res = _mm_max_ps( low, high );
    res = _mm_max_ps( res , _mm_movehl_ps( res , res ));
    res = _mm_max_ps( res , _mm_shuffle_ps( res , res , _MM_SHUFFLE( 1, 1 , 1, 1 )));


    float peak = _mm_cvtss_f32(res);

    

    if ( peak > 1.0f ) {
        float scale = 0.9f / peak;
        __m256 scale_v = _mm256_set1_ps( scale );
        
        int i = 0;

        for ( ; i < num_samples ; i += 8 ) {
            __m256 data = _mm256_load_ps ( &mixer_buffer [ i ] );
            __m256 res = _mm256_mul_ps ( scale_v , data );
            _mm256_store_ps ( &mixer_buffer [ i ], res );
        }

        for ( ; i < num_samples ; i ++ ) { mixer_buffer [ i ] *= scale; }
    }
}



void dsp_preprare_alsa_buffer ( float* left, float* right , int16_t* alsa_buff , uint32_t start_sample , uint32_t total_samples) {
    __m128 v_scale = _mm_set1_ps ( P15 );
    uint32_t i = 0;
    uint32_t safe_sample = total_samples & ~3;
    for ( ; i < safe_sample; i += 4 ) {
        uint32_t idx = i + start_sample;
        __m128 f_left = _mm_load_ps( &left [ idx ] );
        __m128 f_right = _mm_load_ps ( &right [ idx ] );

        __m128i i_left = _mm_cvtps_epi32( _mm_mul_ps ( f_left , v_scale ) );
        __m128i i_right = _mm_cvtps_epi32( _mm_mul_ps ( f_right , v_scale ) );

        __m128i lo = _mm_unpacklo_epi32 ( i_left , i_right );
        __m128i hi = _mm_unpackhi_epi32 ( i_left , i_right );

        __m128i final = _mm_packs_epi32 ( lo, hi );
        _mm_store_si128 ( ( __m128i* )&alsa_buff[ i * 2 ] , final );
        
    }

    for ( ; i < total_samples ; i ++ ) {
        uint32_t idx = i + start_sample;
        alsa_buff [ i * 2 ] = ( int16_t)left [ idx ] * P15;
        alsa_buff [ i * 2 + 1 ] = ( int16_t)right [ idx ] * P15;
    }
   
}
