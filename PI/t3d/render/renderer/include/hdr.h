#ifndef INCLUDE_HDR_H
#define INCLUDE_HDR_H

#include <renderer.h>

const static char* RS_LIGHT_PASS_VS = "default.vs";
const static char* RS_LIGHT_PASS_FS = "light_pass.fs";

const static char* RS_TONE_MAPPING_VS = "default.vs";
const static char* RS_TONE_MAPPING_FS = "tone_mapping.fs";

const static char* RS_DOWN_SAMPLE_VS = "default.vs";
const static char* RS_DOWN_SAMPLE_FS = "down_sample.fs";

const static char* RS_MIPMAP_VS = "default.vs";
const static char* RS_MIPMAP_FS = "mipmap.fs";

const static char* RS_LUMINANCE_ACCUMULATION_VS = "default.vs";
const static char* RS_LUMINANCE_ACCUMULATION_FS = "luminance_accumulation.fs";

PI_BEGIN_DECLS

PiRenderer* PI_API pi_hdr_new_with_name(char* name);

PiRenderer* PI_API pi_hdr_new();


void PI_API pi_hdr_deploy(PiRenderer* renderer, char* input_name, char* output_name);

void PI_API pi_hdr_free(PiRenderer* renderer);

void PI_API pi_hdr_set_min_tone_luminance(PiRenderer* renderer, float tone_luminance);

void PI_API pi_hdr_set_gaussian_scalar(PiRenderer* renderer, float scale);

void PI_API pi_hdr_set_exposure(PiRenderer* renderer, float exposure);

void PI_API pi_hdr_set_bloom_brightness_threshold(PiRenderer* renderer, float threshold);

void PI_API pi_hdr_set_bloom_scale(PiRenderer* renderer, float scale);

PI_END_DECLS

#endif /* INCLUDE_HDR_H */