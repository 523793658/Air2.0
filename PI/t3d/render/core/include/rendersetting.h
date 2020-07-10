
#ifndef INCLUDE_RENDERSETTING_H
#define INCLUDE_RENDERSETTING_H

#include <pi_lib.h>
#include <renderformat.h>

typedef struct 
{
	PiBool	is_full_screen;				/* 默认值：FALSE */
	sint	left;						/* 默认值：0 */
	sint	top;						/* 默认值：0 */
	uint32	width;						/* 默认值：0 */
	uint32	height;						/* 默认值：0 */
	RenderFormat color_fmt;				/* 默认值：RF_ABGR8 */
	RenderFormat depth_stencil_fmt;		/* 默认值：RF_D16 */
	uint32 sample_count;				/* 默认值：1 */
	uint32 sample_quality;				/* 默认值：0 */
	uint32 sync_interval;				/* 默认值：0 */
	uint32 motion_frames;				/* 默认值：0 */
	PiBool is_hdr;						/* 默认值：FALSE */
	PiBool is_fft_lens_effects;			/* 默认值：FLASE */
	PiBool is_ppaa;						/* 默认值：FALSE */
	PiBool is_gamma;					/* 默认值：FALSE */	
	PiBool is_color_grading;			/* 默认值：FALSE */
}PiRenderSetting;

#endif	/* INCLUDE_RENDERSETTING_H */