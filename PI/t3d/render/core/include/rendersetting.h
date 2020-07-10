
#ifndef INCLUDE_RENDERSETTING_H
#define INCLUDE_RENDERSETTING_H

#include <pi_lib.h>
#include <renderformat.h>

typedef struct 
{
	PiBool	is_full_screen;				/* Ĭ��ֵ��FALSE */
	sint	left;						/* Ĭ��ֵ��0 */
	sint	top;						/* Ĭ��ֵ��0 */
	uint32	width;						/* Ĭ��ֵ��0 */
	uint32	height;						/* Ĭ��ֵ��0 */
	RenderFormat color_fmt;				/* Ĭ��ֵ��RF_ABGR8 */
	RenderFormat depth_stencil_fmt;		/* Ĭ��ֵ��RF_D16 */
	uint32 sample_count;				/* Ĭ��ֵ��1 */
	uint32 sample_quality;				/* Ĭ��ֵ��0 */
	uint32 sync_interval;				/* Ĭ��ֵ��0 */
	uint32 motion_frames;				/* Ĭ��ֵ��0 */
	PiBool is_hdr;						/* Ĭ��ֵ��FALSE */
	PiBool is_fft_lens_effects;			/* Ĭ��ֵ��FLASE */
	PiBool is_ppaa;						/* Ĭ��ֵ��FALSE */
	PiBool is_gamma;					/* Ĭ��ֵ��FALSE */	
	PiBool is_color_grading;			/* Ĭ��ֵ��FALSE */
}PiRenderSetting;

#endif	/* INCLUDE_RENDERSETTING_H */