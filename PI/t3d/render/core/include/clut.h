#ifndef _CLUT_H_
#define _CLUT_H_


typedef struct  
{
	uint size;
	uint data_size;
	float* data;
}PiColorLookUpTable;

PI_BEGIN_DECLS
PiColorLookUpTable* PI_API pi_clut_load(byte* data, uint32 size);

uint PI_API pi_clut_get_size(PiColorLookUpTable* clut);

void PI_API pi_clut_free(PiColorLookUpTable* clut);

PI_END_DECLS

#endif