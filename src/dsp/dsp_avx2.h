

void dsp_preprare_alsa_buffer ( float* left, float* right , int16_t* alsa_buff , uint32_t start_sample , uint32_t total_samples);
void dsp_fill ( float* buffer, float value, size_t size );
void dsp_add_buffer( float* out_buffer, float* in_buffer , uint32_t size);
void dsp_normalize_buffer ( float* mixer_buffer , int num_samples );