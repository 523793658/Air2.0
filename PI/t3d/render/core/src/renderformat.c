#include <renderformat.h>

static uint64 s_rf_impl[] = {
	RFIMPL_UNKNOWN,
	RFIMPL_A8 ,
	RFIMPL_ARGB4 ,
	RFIMPL_R8 ,
	RFIMPL_SIGNED_R8 ,
	RFIMPL_GR8 ,
	RFIMPL_SIGNED_GR8 ,
	RFIMPL_BGR8 ,
	RFIMPL_SIGNED_BGR8 ,
	RFIMPL_ARGB8 ,
	RFIMPL_ABGR8 ,
	RFIMPL_SIGNED_ABGR8 ,
	RFIMPL_A2BGR10 ,
	RFIMPL_SIGNED_A2BGR10 ,
	RFIMPL_R8UI ,
	RFIMPL_R8I ,
	RFIMPL_GR8UI ,
	RFIMPL_GR8I ,
	RFIMPL_BGR8UI ,
	RFIMPL_BGR8I ,
	RFIMPL_ABGR8UI ,
	RFIMPL_ABGR8I ,
	RFIMPL_A2BGR10UI ,
	RFIMPL_A2BGR10I ,
	RFIMPL_R16 ,
	RFIMPL_SIGNED_R16 ,
	RFIMPL_GR16 ,
	RFIMPL_SIGNED_GR16 ,
	RFIMPL_BGR16 ,
	RFIMPL_SIGNED_BGR16 ,
	RFIMPL_ABGR16 ,
	RFIMPL_SIGNED_ABGR16 ,
	RFIMPL_R32 ,
	RFIMPL_SIGNED_R32 ,
	RFIMPL_GR32 ,
	RFIMPL_SIGNED_GR32 ,
	RFIMPL_BGR32 ,
	RFIMPL_SIGNED_BGR32 ,
	RFIMPL_ABGR32 ,
	RFIMPL_SIGNED_ABGR32 ,
	RFIMPL_R16UI ,
	RFIMPL_R16I ,
	RFIMPL_GR16UI ,
	RFIMPL_GR16I ,
	RFIMPL_BGR16UI ,
	RFIMPL_BGR16I ,
	RFIMPL_ABGR16UI ,
	RFIMPL_ABGR16I ,
	RFIMPL_R32UI ,
	RFIMPL_R32I ,
	RFIMPL_GR32UI ,
	RFIMPL_GR32I ,
	RFIMPL_BGR32UI ,
	RFIMPL_BGR32I ,
	RFIMPL_ABGR32UI ,
	RFIMPL_ABGR32I ,
	RFIMPL_R16F ,
	RFIMPL_GR16F ,
	RFIMPL_B10G11R11F ,
	RFIMPL_BGR16F ,
	RFIMPL_ABGR16F ,
	RFIMPL_R32F ,
	RFIMPL_GR32F ,
	RFIMPL_BGR32F ,
	RFIMPL_ABGR32F ,
	RFIMPL_BC1 ,
	RFIMPL_SIGNED_BC1 ,
	RFIMPL_BC2 ,
	RFIMPL_SIGNED_BC2 ,
	RFIMPL_BC3 ,
	RFIMPL_SIGNED_BC3 ,
	RFIMPL_BC4 ,
	RFIMPL_SIGNED_BC4 ,
	RFIMPL_BC5 ,
	RFIMPL_SIGNED_BC5 ,
	RFIMPL_BC6 ,
	RFIMPL_SIGNED_BC6 ,
	RFIMPL_BC7 ,
	RFIMPL_D16 ,
	RFIMPL_D24 ,
	RFIMPL_D24S8 ,
	RFIMPL_D32F ,
	RFIMPL_ARGB8_SRGB ,
	RFIMPL_ABGR8_SRGB ,
	RFIMPL_BC1_SRGB ,
	RFIMPL_BC2_SRGB ,
	RFIMPL_BC3_SRGB ,
	RFIMPL_BC4_SRGB ,
	RFIMPL_BC5_SRGB ,
	RFIMPL_BC7_SRGB ,
	RFIMPL_INTZ,
	RFIMPL_NULL
};

static RenderFormat _get_format(uint64 impl)
{
	RenderFormat i;
	for (i = 0; i < RF_NUM; ++i)
	{
		if(s_rf_impl[i] == impl)
		{
			break;
		}
	}
	return i < RF_NUM ? i : RFIMPL_UNKNOWN;
}

uint64 PI_API pi_renderformat_get_impl(RenderFormat format)
{
	return s_rf_impl[format];
}

ElementChannel PI_API pi_renderformat_get_channel(RenderFormat format, uint64 index)
{
	return s_rf_impl[format] >> (4 * index) & 0xF;
}

RenderFormat PI_API pi_renderformat_set_channel(RenderFormat format, uint64 index, ElementChannel ch)
{
	uint64 r = s_rf_impl[format];
	r &= ~(0xFULL << (4 * index));
	r |= ch << (4 * index);
	return _get_format(r);
}

uint8 PI_API pi_renderformat_get_size(RenderFormat format, uint64 index)
{
	return (s_rf_impl[format] >> (16 + 6 * index)) & 0x3F;
}

RenderFormat PI_API pi_renderformat_set_size(RenderFormat format, uint64 index, uint8 size)
{
	uint64 r = s_rf_impl[format];
	r &= ~(0x3FULL << (16 + 6 * index));
	r |= (size << (16 + 6 * index));
	return _get_format(r);
}

ElementChannelType PI_API pi_renderformat_get_type(RenderFormat format, uint64 index)
{
	return (s_rf_impl[format] >> (40 + 4 * index)) & 0xF;
}

RenderFormat PI_API pi_renderformat_set_type(RenderFormat format, uint64 index, ElementChannelType type)
{
	uint64 r = s_rf_impl[format];
	r &= ~(0xFULL << (40 + 4 * index));
	r |= (type << (40 + 4 * index));
	return _get_format(r);
}

PiBool PI_API pi_renderformat_is_float_format(RenderFormat format)
{
	ElementChannelType type = pi_renderformat_get_type(format, 0);
	return (type == ECT_FLOAT);
}

PiBool PI_API pi_renderformat_is_compressed_format(RenderFormat format)
{
	ElementChannel ch = pi_renderformat_get_channel(format, 0);
	return (ch == EC_BC);
}

PiBool PI_API pi_renderformat_is_depth_format(RenderFormat format)
{
	ElementChannel ch = pi_renderformat_get_channel(format, 0);
	return (ch == EC_D);
}

PiBool PI_API pi_renderformat_is_stencil_format(RenderFormat format)
{
	ElementChannel ch = pi_renderformat_get_channel(format, 1);
	return (ch == EC_S);
}

PiBool PI_API pi_renderformat_is_srgb(RenderFormat format)
{
	return (ECT_UNORM_SRGB == pi_renderformat_get_type(format, 0));
}

PiBool PI_API pi_renderformat_is_signed(RenderFormat format)
{
	return (ECT_SNORM == pi_renderformat_get_type(format, 0));
}

uint8 PI_API pi_renderformat_get_numbits(RenderFormat format)
{
	uint8 r = 0;
	if (pi_renderformat_is_compressed_format(format))
	{
		uint8 size = pi_renderformat_get_size(format, 0);
		switch (size)
		{
		case 1:
		case 4:
			r = 16;
			break;
		case 2:
		case 3:
		case 5:
			r = 32;
			break;
		default:
			PI_ASSERT(FALSE, "invalid format, format = %p", format);
			break;
		}
	}
	else
	{
		r = pi_renderformat_get_size(format, 0)
			+ pi_renderformat_get_size(format, 1)
			+ pi_renderformat_get_size(format, 2)
			+ pi_renderformat_get_size(format, 3);
	}
	return r;
}

uint8 PI_API pi_renderformat_get_numbytes(RenderFormat format)
{
	return pi_renderformat_get_numbits(format) / 8;
}

RenderFormat PI_API pi_renderformat_make_srgb(RenderFormat format)
{
	ElementChannelType type;
	
	type = pi_renderformat_get_type(format, 0);
	if (type == ECT_UNORM)
	{
		format = pi_renderformat_set_type(format, 0, ECT_UNORM_SRGB);
	}
	
	if (!pi_renderformat_is_compressed_format(format))
	{
		uint i;
		for(i = 1; i < 4; ++i)
		{
			type = pi_renderformat_get_type(format, i);
			if (type == ECT_UNORM)
			{
				format = pi_renderformat_set_type(format, i, ECT_UNORM_SRGB);
			}
		}
	}
	return format;
}

RenderFormat PI_API pi_renderformat_make_non_srgb(RenderFormat format)
{
	uint32 i;
	ElementChannelType type;
	
	for(i = 0; i < 4; ++i)
	{
		type = pi_renderformat_get_type(format, i);
		if (type == ECT_UNORM_SRGB)
		{
			format = pi_renderformat_set_type(format, i, ECT_UNORM);
		}
	}
	return format;
}

RenderFormat PI_API pi_renderformat_make_signed(RenderFormat format)
{
	uint i;
	ElementChannelType type;
	
	for(i = 0; i < 4; ++i)
	{
		type = pi_renderformat_get_type(format, i);
		if (type == ECT_UNORM)
		{
			format = pi_renderformat_set_type(format, i, ECT_SNORM);
		}
		else if (type == ECT_UINT)
		{
			format = pi_renderformat_set_type(format, i, ECT_SINT);
		}
	}
	return format;
}

RenderFormat PI_API pi_renderformat_make_unsigned(RenderFormat format)
{
	uint i;
	ElementChannelType type;

	for(i = 0; i < 4; ++i)
	{
		type = pi_renderformat_get_type(format, i);
		if (type == ECT_SNORM)
		{
			format = pi_renderformat_set_type(format, i, ECT_UNORM);
		}
		else if(type == ECT_SINT)
		{
			format = pi_renderformat_set_type(format, i, ECT_UINT);
		}
	}
	
	return format;
}

uint8 PI_API pi_renderformat_get_numdepthbits(RenderFormat format)
{
	uint8 r = 0;
	ElementChannel ch = pi_renderformat_get_channel(format, 0);
	if (ch == EC_D)
	{
		r = pi_renderformat_get_size(format, 0);
	}
	return r;
}

uint8 PI_API pi_renderformat_get_numstencilbits(RenderFormat format)
{
	uint8 r = 0;
	ElementChannel ch = pi_renderformat_get_channel(format, 1);
	if (ch == EC_S)
	{
		r = pi_renderformat_get_size(format, 1);
	}
	return r;
}

uint32 PI_API pi_renderformat_get_numcomponents(RenderFormat format)
{
	uint32 r = 0;
	if (pi_renderformat_is_compressed_format(format))
	{
		uint32 size = pi_renderformat_get_size(format, 0);
		switch (size)
		{
		case 1:
		case 2:
		case 3:
			r = 4;
			break;
		case 4:
			r = 1;
			break;
		case 5:
			r = 2;
			break;
		default:
			PI_ASSERT(FALSE, "invalid format = %p", format);
			break;
		}
	}
	else
	{
		r = (pi_renderformat_get_size(format, 0) != 0)
			+ (pi_renderformat_get_size(format, 1) != 0)
			+ (pi_renderformat_get_size(format, 2) != 0)
			+ (pi_renderformat_get_size(format, 3) != 0);
	}
	return r;
}

uint32 PI_API pi_renderformat_get_component_bpps(RenderFormat format)
{
	uint32 r, r_01, r_23;
	uint32 size_0 = pi_renderformat_get_size(format, 0);
	uint32 size_1 = pi_renderformat_get_size(format, 1);
	uint32 size_2 = pi_renderformat_get_size(format, 2);
	uint32 size_3 = pi_renderformat_get_size(format, 3);
	
	r_01 = MAX(size_0, size_1);
	r_23 = MAX(size_2, size_3);
	r = MAX(r_01, r_23);
	return r;
}