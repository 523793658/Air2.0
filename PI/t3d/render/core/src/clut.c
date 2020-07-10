#include "pi_lib.h"
#include "clut.h"

PiColorLookUpTable* PI_API pi_clut_load(byte* data, uint32 size)
{
	PiColorLookUpTable* clut = pi_new0(PiColorLookUpTable, 1);
	char* data_temp = pi_malloc(size);
	char* index = data_temp;
	char* str = data_temp;
	PiVector* strs = pi_vector_new();
	pi_vector_set_capacity(strs, 32800);
	uint32 lines = 0;
	uint i;
	uint write_index = 0;
	PiBool is_data = FALSE;
	
	pi_memcpy_inline(data_temp, data, size);
	while ((index - data_temp) < (int)size && *index != '\0')
	{
		if (*index == '\r' && (*(index + 1) == '\n'))
		{
			*index = '\0';
			++lines;
			pi_vector_push(strs, str);
			str = index + 2;
			index++;
		}

		else if (*index == '\n')
		{
			*index = '\0';
			++lines;
			pi_vector_push(strs, str);
			str = index + 1;
		}
		index++;
	}
	for (i = 0; i < lines; i++)
	{
		str = pi_vector_get(strs, i);
		if (is_data)
		{
			*(clut->data + write_index) = strtof(str, &str);
			++write_index;

			*(clut->data + write_index) = strtof(str + 1, &str);
			++write_index;

			*(clut->data + write_index) = strtof(str + 1, &str);
			++write_index;

			*(clut->data + write_index) = 1.0f;
			++write_index;
		}
		else
		{
			if (pi_str_start_with(str, "LUT_3D_SIZE"))
			{
				int index = pi_str_char_index(str, ' ') + 1;
				int64 size;
				pi_str_parse_number(str + index, &size);
				clut->size = (uint)size;
				clut->data = pi_new(float, clut->size * clut->size * clut->size * 4);
				clut->data_size = clut->size * clut->size * clut->size * 4 * sizeof(float);
			}
			else if (pi_str_equal(str, "#LUT data points", TRUE))
			{
				is_data = TRUE;
			}
		}
	}
	pi_vector_free(strs);
	pi_free(data_temp);

	return clut;
}

void PI_API pi_clut_free(PiColorLookUpTable* clut)
{
	if (clut->data != NULL)
	{
		pi_free(clut->data);
	}
	pi_free(clut);
}

uint PI_API pi_clut_get_size(PiColorLookUpTable* clut)
{
	return clut->size;
}