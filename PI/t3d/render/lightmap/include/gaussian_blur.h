#ifndef _Include_Gaussian_Blur_Renderer_H_
#define _Include_Gaussian_Blur_Renderer_H_

PI_BEGIN_DECLS

PiRenderer* PI_API pi_gaussian_blur_renderer_new();

void PI_API pi_gaussian_blur_renderer_deploy(PiRenderer* renderer, char* input_name, char* output_name);

void PI_API pi_gaussian_blur_renderer_free(PiRenderer* renderer);
PI_END_DECLS
#endif