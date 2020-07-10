#include <gl_convert.h>
#include <gl_interface.h>

uint gl_polygon_get(PolygonMode mode)
{
	uint r = GL_FILL;
	switch (mode)
	{
	case PM_POINT:
		r = GL_POINT;
		break;
	case PM_LINE:
		r = GL_LINE;
		break;
	case PM_FILL:
		r = GL_FILL;
		break;
	default:
		PI_ASSERT(FALSE, "invalid mode = %d", mode);
		break;
	}
	return r;
}

uint gl_compare_func_get(CompareFunction func)
{
	uint r = GL2_EQUAL;
	switch (func)
	{
	case CF_ALWAYSFAIL:
		r = GL2_NEVER;
		break;
	case CF_ALWAYSPASS:
		r = GL2_ALWAYS;
		break;
	case CF_LESS:
		r = GL2_LESS;
		break;
	case CF_LESSEQUAL:
		r = GL2_LEQUAL;
		break;
	case CF_EQUAL:
		r = GL2_EQUAL;
		break;
	case CF_NOTEQUAL:
		r = GL2_NOTEQUAL;
		break;
	case CF_GREATEREQUAL:
		r = GL2_GEQUAL;
		break;
	case CF_GREATER:
		r = GL2_GREATER;
		break;
	default:
		PI_ASSERT(FALSE, "invalid func = %d", func);
		break;
	}
	return r;
}

uint gl_stencil_op_get(StencilOperation op)
{
	uint r = GL2_KEEP;;
	switch (op)
	{
	case SOP_KEEP:
		r = GL2_KEEP;
		break;
	case SOP_ZERO:
		r = GL2_ZERO;
		break;
	case SOP_REPLACE:
		r = GL2_REPLACE;
		break;
	case SOP_INCR:
		r = GL2_INCR;
		break;
	case SOP_DECR:
		r = GL2_DECR;
		break;
	case SOP_INVERT:
		r = GL2_INVERT;
		break;
	case SOP_INCR_WRAP:
		r = GL2_INCR_WRAP;
		break;
	case SOP_DECR_WRAP:
		r = GL2_DECR_WRAP;
		break;
	default:
		PI_ASSERT(FALSE, "invalid op = %d", op);
		break;
	}
	return r;
}

uint gl_blend_op_get(BlendOperation bo)
{
	uint r = GL2_FUNC_ADD;
	switch (bo)
	{
	case BOP_ADD:
		r = GL2_FUNC_ADD;
		break;
	case BOP_SUB:
		r = GL2_FUNC_SUBTRACT;
		break;
	case BOP_REV_SUB:
		r = GL2_FUNC_REVERSE_SUBTRACT;
		break;
	case BOP_MIN:
        r = GL3_MIN;
		break;
	case BOP_MAX:
		r = GL3_MAX;
		break;
	default:
		PI_ASSERT(FALSE, "invalid bo = %d", bo);
		break;
	}
	return r;
}

uint gl_blend_factor_get(BlendFactor factor)
{
	uint r = BF_ZERO;
	switch (factor)
	{
	case BF_ZERO:
		r = GL2_ZERO;
		break;
	case BF_ONE:
		r = GL2_ONE;
		break;
	case BF_SRC_ALPHA:
		r = GL2_SRC_ALPHA;
		break;
	case BF_DST_ALPHA:
		r = GL2_DST_ALPHA;
		break;
	case BF_INV_SRC_ALPHA:
		r = GL2_ONE_MINUS_SRC_ALPHA;
		break;
	case BF_INV_DST_ALPHA:
		r = GL2_ONE_MINUS_DST_ALPHA;
		break;
	case BF_SRC_COLOR:
		r = GL2_SRC_COLOR;
		break;
	case BF_DST_COLOR:
		r = GL2_DST_COLOR;
		break;
	case BF_INV_SRC_COLOR:
		r = GL2_ONE_MINUS_SRC_COLOR;
		break;
	case BF_INV_DST_COLOR:
		r = GL2_ONE_MINUS_DST_COLOR;
		break;
	case BF_SRC_ALPHA_SAT:
		r = GL2_SRC_ALPHA_SATURATE;
		break;
	default:
		PI_ASSERT(FALSE, "invalid factor = %d", factor);
		break;
	}
	return r;
}

uint gl_tex_addr_get(TexAddressMode mode)
{
	uint r = GL2_REPEAT;
	switch (mode)
	{
	case TAM_WRAP:
		r = GL2_REPEAT;
		break;
	case TAM_MIRROR:
		r = GL2_MIRRORED_REPEAT;
		break;
	case TAM_CLAMP:
		r = GL2_CLAMP_TO_EDGE;
		break;
	case TAM_BORDER:
		if(gl_Self_GetInterfaceType() == RIT_OPENGL)
		{
			r = GL_CLAMP_TO_BORDER;
		}
		else
		{/* GLES 和 WebGL都不支持border过滤，所以用edge代替 */
			r = GL2_CLAMP_TO_EDGE;
		}
		break;
	default:
		PI_ASSERT(FALSE, "invalid mode = %d", mode);
		break;
	}
	return r;
}

void gl_tex_filter_get(TexFilterOp filter, uint *min_filter, uint *mag_filter)
{
	if (filter & TFOE_MIN_LINEAR)
	{
		if (filter & TFOE_MIP_LINEAR)
		{
			*min_filter = GL2_LINEAR_MIPMAP_LINEAR;
		}
		else if(filter & TFOE_MIP_POINT)
		{
			*min_filter = GL2_LINEAR_MIPMAP_NEAREST;
		} else
		{
			*min_filter = GL2_LINEAR;
		}
	}
	else
	{
		if (filter & TFOE_MIP_LINEAR)
		{
			*min_filter = GL2_NEAREST_MIPMAP_LINEAR;
		}
		else if(filter & TFOE_MIP_POINT)
		{
			*min_filter = GL2_NEAREST_MIPMAP_NEAREST;
		}
		else
		{
			*min_filter = GL2_NEAREST;
		}
	}

	if (filter & TFOE_MAG_LINEAR)
	{
		*mag_filter = GL2_LINEAR;
	}
	else
	{
		*mag_filter = GL2_NEAREST;
	}
	if (filter & TFOE_ANISOTROPIC)
	{
		*mag_filter = GL2_LINEAR;
		*min_filter = GL2_LINEAR_MIPMAP_LINEAR;
	}
}

uint gl_primitive_get(EGeometryType type)
{
	uint32 gl_type = GL2_TRIANGLES;

	switch (type)
	{
	case EGOT_POINT_LIST:
		gl_type = GL2_POINTS;
		break;
	case EGOT_LINE_LIST:
		gl_type = GL2_LINES;
		break;
	case EGOT_LINE_STRIP:
		gl_type = GL2_LINE_STRIP;
		break;
	case EGOT_TRIANGLE_STRIP:
		gl_type = GL2_TRIANGLE_STRIP;
		break;
	case EGOT_TRIANGLE_FAN:
		gl_type = GL2_TRIANGLE_FAN;
		break;
	case EGOT_TRIANGLE_LIST:
		gl_type = GL2_TRIANGLES; 
		break;
	default:
		PI_ASSERT(FALSE, "invalid type = %d", type);
		break;
	}
	return gl_type;
}

uint gl_tex_target_get(TextureType type, uint array_size)
{
	uint target = GL2_TEXTURE_2D;
	
	switch (type)
	{
	case TT_2D:
		if (array_size > 1)
		{
			target = GL3_TEXTURE_2D_ARRAY;
		}
		else
		{
			target = GL2_TEXTURE_2D;
		}
		break;

	case TT_3D:
		target = GL3_TEXTURE_3D;
		break;

	case TT_CUBE:
		target = GL2_TEXTURE_CUBE_MAP;
		break;

	default:
		pi_log_print(LOG_ERROR, "invalid type and array_size");
		break;
	}
	return target;
}

void gl_tex_format_get(RenderFormat fmt, uint *internal_fmt, uint *gl_fmt, uint *gl_type)
{
    uint ifmt = 0, gfmt = 0, gtype = 0;
    uint64 impl = pi_renderformat_get_impl(fmt);
    switch (impl)
    {
    case RFIMPL_A8:
        ifmt = GL2_ALPHA;
        gfmt = GL2_ALPHA;
        gtype = GL2_UNSIGNED_BYTE;
        break;
    case RFIMPL_R8:
        if (gl_Self_IsTextureRG())
        {
            ifmt = GL3_R8;
            gfmt = GL3_RED;
            gtype = GL2_UNSIGNED_BYTE;
        }
        break;
    case RFIMPL_SIGNED_R8:
        if (gl_Self_IsTextureRG())
        {
            ifmt = GL3_R8;
            gfmt = GL3_RED;
            gtype = GL2_BYTE;
        }
        break;
    case RFIMPL_GR8:
        if (gl_Self_IsTextureRG())
        {
            ifmt = GL3_RG8;
            gfmt = GL3_RG;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_SIGNED_GR8:
        if (gl_Self_IsTextureRG())
        {
            ifmt = GL3_RG8;
            gfmt = GL3_RG;
            gtype = GL2_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BGR8:
		ifmt = GL2_RGB;
        gfmt = GL2_RGB;
        gtype = GL2_UNSIGNED_BYTE;
        break;
    case RFIMPL_ABGR8:
		ifmt = GL2_RGBA;
        gfmt = GL2_RGBA;
        gtype = GL2_UNSIGNED_BYTE;
        break;
    case RFIMPL_A2BGR10:
        ifmt = GL3_RGB10_A2;
        gfmt = GL2_RGBA;
        gtype = GL3_UNSIGNED_INT_2_10_10_10_REV;
        break;
    case RFIMPL_SIGNED_A2BGR10:
        ifmt = GL3_RGB10_A2;
        gfmt = GL2_RGBA;
        if (gl_Self_IsVersion3())
        {
            gtype = GL3_INT_2_10_10_10_REV;
        }
        else
        {
            gtype = GL3_UNSIGNED_INT_2_10_10_10_REV;
        }
        break;
    case RFIMPL_R8UI:
        if (gl_Self_IsVersion3())
        {
            ifmt = GL3_R8UI;
            gfmt = GL3_RED_INTEGER;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_R8I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_R8I;
            gfmt = GL3_RED_INTEGER;
            gtype = GL2_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_GR8UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RG8UI;
            gfmt = GL3_RG_INTEGER;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_GR8I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RG8I;
            gfmt = GL3_RG_INTEGER;
            gtype = GL2_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BGR8UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGB8UI;
            gfmt = GL3_RGB_INTEGER;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BGR8I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGB8I;
            gfmt = GL3_RGB_INTEGER;
            gtype = GL2_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_ABGR8UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGBA8UI;
            gfmt = GL3_RGBA_INTEGER;
            gtype = GL_UNSIGNED_INT_8_8_8_8;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_ABGR8I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGBA8I;
            gfmt = GL3_RGBA_INTEGER;
            gtype = GL_UNSIGNED_INT_8_8_8_8;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_R16:
    case RFIMPL_SIGNED_R16:
    case RFIMPL_GR16:
    case RFIMPL_SIGNED_GR16:
    case RFIMPL_BGR16:
    case RFIMPL_SIGNED_BGR16:
    case RFIMPL_ABGR16:
	case RFIMPL_SIGNED_ABGR16:
        PI_ASSERT(FALSE, "unsupported format = %d", impl);
        break;
    case RFIMPL_R16UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_R16UI;
            gfmt = GL3_RED_INTEGER;
            gtype = GL2_UNSIGNED_SHORT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_R16I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_R16I;
            gfmt = GL3_RED_INTEGER;
            gtype = GL2_SHORT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_GR16UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RG16UI;
            gfmt = GL3_RG_INTEGER;
            gtype = GL2_UNSIGNED_SHORT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_GR16I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RG16I;
            gfmt = GL3_RG_INTEGER;
            gtype = GL2_SHORT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BGR16UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGB16UI;
            gfmt = GL3_RGB_INTEGER;
            gtype = GL2_UNSIGNED_SHORT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BGR16I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGB16I;
            gfmt = GL3_RGB_INTEGER;
            gtype = GL2_SHORT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_ABGR16UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGBA16UI;
            gfmt = GL3_RGBA_INTEGER;
            gtype = GL2_UNSIGNED_SHORT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_ABGR16I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGBA16I;
            gfmt = GL3_RGBA_INTEGER;
            gtype = GL2_SHORT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_R32UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_R32UI;
            gfmt = GL3_RED_INTEGER;
            gtype = GL2_UNSIGNED_INT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_R32I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_R32I;
            gfmt = GL3_RED_INTEGER;
            gtype = GL2_INT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_GR32UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RG32UI;
            gfmt = GL3_RG_INTEGER;
            gtype = GL2_UNSIGNED_INT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_GR32I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RG32I;
            gfmt = GL3_RG_INTEGER;
            gtype = GL2_INT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BGR32UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGB32UI;
            gfmt = GL3_RGB_INTEGER;
            gtype = GL2_UNSIGNED_INT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BGR32I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGB32I;
            gfmt = GL3_RGB_INTEGER;
            gtype = GL2_INT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_ABGR32UI:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGBA32UI;
            gfmt = GL3_RGBA_INTEGER;
            gtype = GL2_UNSIGNED_INT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_ABGR32I:
        if (gl_Self_IsTextureInteger())
        {
            ifmt = GL3_RGBA32I;
            gfmt = GL3_RGBA_INTEGER;
            gtype = GL2_INT;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_R16F:
        if (gl_Self_IsTextureRG())
        {
            ifmt = GL3_R16F;
            gfmt = GL3_RED;
            gtype = GL3_HALF_FLOAT;
        }
        break;
    case RFIMPL_GR16F:
        if (gl_Self_IsTextureRG())
        {
            ifmt = GL3_RG16F;
            gfmt = GL3_RG;
            gtype = GL3_HALF_FLOAT;
        }
        break;
    case RFIMPL_B10G11R11F:
        if (gl_Self_IsPackedFloat())
        {
            ifmt = GL3_R11F_G11F_B10F;
            gfmt = GL2_RGB;
            gtype = GL3_UNSIGNED_INT_10F_11F_11F_REV;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BGR16F:
        ifmt = GL3_RGB16F;
        gfmt = GL2_RGB;
        gtype = GL3_HALF_FLOAT;
        break;
    case RFIMPL_ABGR16F:
        ifmt = GL3_RGBA16F;
        gfmt = GL2_RGBA;
        gtype = GL3_HALF_FLOAT;
        break;
    case RFIMPL_R32F:
        if (gl_Self_IsTextureRG())
        {
            ifmt = GL3_R32F;
            gfmt = GL3_RED;
            gtype = GL2_FLOAT;
        }
        break;
    case RFIMPL_GR32F:
        if (gl_Self_IsTextureRG())
        {
            ifmt = GL3_RG32F;
            gfmt = GL3_RG;
            gtype = GL2_FLOAT;
        }
        break;
    case RFIMPL_BGR32F:
        ifmt = GL3_RGB32F;
        gfmt = GL2_RGB;
        gtype = GL2_FLOAT;
        break;
    case RFIMPL_ABGR32F:
        ifmt = GL3_RGBA32F;
        gfmt = GL2_RGBA;
        gtype = GL2_FLOAT;
        break;
    case RFIMPL_BC1:
        if (gl_Self_IsTextureCompressionS3tc())
        {
            ifmt = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            gfmt = GL2_RGBA;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BC2:
        if (gl_Self_IsTextureCompressionS3tc())
        {
            ifmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            gfmt = GL2_RGBA;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BC3:
        if (gl_Self_IsTextureCompressionS3tc())
        {
            ifmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            gfmt = GL2_RGBA;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BC4:
        if (gl_Self_IsTextureCompressionLatc())
        {
            ifmt = GL_COMPRESSED_LUMINANCE_LATC1_EXT;
            gfmt = GL_COMPRESSED_LUMINANCE;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BC5:
        if (gl_Self_IsTextureCompressionLatc())
        {
            ifmt = GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
            gfmt = GL_COMPRESSED_LUMINANCE_ALPHA;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_SIGNED_BC4:
        if (gl_Self_IsTextureCompressionLatc())
        {
            ifmt = GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT;
            gfmt = GL_COMPRESSED_LUMINANCE;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_SIGNED_BC5:
        if (gl_Self_IsTextureCompressionLatc())
        {
            ifmt = GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT;
            gfmt = GL_COMPRESSED_LUMINANCE_ALPHA;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_D16:
		if(gl_Self_IsVersion3())
		{
			ifmt = GL2_DEPTH_COMPONENT16;
		}
		else
		{
			ifmt = GL2_DEPTH_COMPONENT;
		}
		gfmt = GL2_DEPTH_COMPONENT;
        gtype = GL2_UNSIGNED_SHORT;
        break;
	case RFIMPL_D24:
		if(gl_Self_IsVersion3())
		{
			ifmt = GL3_DEPTH_COMPONENT24;
		}
		else
		{
			PI_ASSERT(FALSE, "D24 isn't support");
		}
		gfmt = GL2_DEPTH_COMPONENT;
		gtype = GL2_UNSIGNED_INT;
		break;
    case RFIMPL_D24S8:
        if (gl_Self_IsPackedDepthStencil())
        {
            ifmt = GL3_DEPTH24_STENCIL8;
            gfmt = GL3_DEPTH_STENCIL;
            gtype = GL3_UNSIGNED_INT_24_8;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_D32F:
        ifmt = GL3_DEPTH_COMPONENT32F;
        gfmt = GL2_DEPTH_COMPONENT;	
        gtype = GL2_FLOAT;
        break;
    case RFIMPL_ABGR8_SRGB:
        if (gl_Self_IsTextureSRGB())
        {
            ifmt = GL3_SRGB8_ALPHA8;
            gfmt = GL2_RGBA;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BC4_SRGB:
        if (gl_Self_IsTextureSRGB())
        {
            ifmt = GL_COMPRESSED_SLUMINANCE;
            gfmt = GL2_LUMINANCE;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
    case RFIMPL_BC5_SRGB:
        if (gl_Self_IsTextureSRGB())
        {
            ifmt = GL_COMPRESSED_SLUMINANCE_ALPHA;
            gfmt = GL2_LUMINANCE_ALPHA;
            gtype = GL2_UNSIGNED_BYTE;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        break;
	/*case RFIMPL_ARGB8_SRGB:
		if (gl_Self_IsTextureSRGB())
		{
			ifmt = GL3_SRGB8_ALPHA8;
			gfmt = GL_BGRA;
			gtype = GL_UNSIGNED_INT_8_8_8_8_REV;
		}
		else
		{
			PI_ASSERT(FALSE, "unsupported format = %d", impl);
		}
		break;
	case RFIMPL_BC1_SRGB:
		if (gl_Self_IsTextureSRGB())
		{
			ifmt = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
			gfmt = GL_BGRA;
			gtype = GL2_UNSIGNED_BYTE;
		}
		else
		{
			PI_ASSERT(FALSE, "unsupported format = %d", impl);
		}
		break;
	case RFIMPL_BC2_SRGB:
		if (gl_Self_IsTextureSRGB())
		{
			ifmt = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
			gfmt = GL_BGRA;
			gtype = GL2_UNSIGNED_BYTE;
		}
		else
		{
			PI_ASSERT(FALSE, "unsupported format = %d", impl);
		}
		break;
	case RFIMPL_BC3_SRGB:
		if (gl_Self_IsTextureSRGB())
		{
			ifmt = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
			gfmt = GL_BGRA;
			gtype = GL2_UNSIGNED_BYTE;
		}
		else
		{
			PI_ASSERT(FALSE, "unsupported format = %d", impl);
		}
		break;
	case RFIMPL_ARGB8:
		ifmt = GL3_RGBA8;
		gfmt = GL_BGRA;
		gtype = GL_UNSIGNED_INT_8_8_8_8_REV;
		break;
	case RFIMPL_ARGB4:
		ifmt = GL_RGBA4;
		gfmt = GL_BGRA;
		gtype = GL_UNSIGNED_SHORT_4_4_4_4_REV;
		break;*/
    default:
        PI_ASSERT(FALSE, "invalid vertex format = %d", impl);
    }

    if(internal_fmt != NULL)
        *internal_fmt = ifmt;
    if(gl_fmt != NULL)
        *gl_fmt = gfmt;
    if(gl_type != NULL)
        *gl_type = gtype;
}

void gl_vertex_format_get(RenderFormat fmt, uint *gl_type, PiBool *is_normalized)
{
    uint gtype = 0;
    PiBool is_n = FALSE;
    uint64 impl = pi_renderformat_get_impl(fmt);
    switch (impl)
    {
    case RFIMPL_A8:
    case RFIMPL_R8:
    case RFIMPL_GR8:
    case RFIMPL_BGR8:
    case RFIMPL_ARGB8:
    case RFIMPL_ABGR8:
        gtype = GL2_UNSIGNED_BYTE;
        is_n = TRUE;
        break;
    case RFIMPL_R8UI:
    case RFIMPL_GR8UI:
    case RFIMPL_BGR8UI:
    case RFIMPL_ABGR8UI:
        gtype = GL2_UNSIGNED_BYTE;
        is_n = FALSE;
        break;
    case RFIMPL_SIGNED_R8:
    case RFIMPL_SIGNED_GR8:
    case RFIMPL_SIGNED_BGR8:
    case RFIMPL_SIGNED_ABGR8:
        gtype = GL2_BYTE;
        is_n = TRUE;
        break;
    case RFIMPL_R8I:
    case RFIMPL_GR8I:
    case RFIMPL_BGR8I:
    case RFIMPL_ABGR8I:
        gtype = GL2_BYTE;
        is_n = FALSE;
        break;
    case RFIMPL_A2BGR10:
        gtype = GL3_UNSIGNED_INT_2_10_10_10_REV;
        is_n = TRUE;
        break;
    case RFIMPL_SIGNED_A2BGR10:
        if (gl_Self_IsVertexType_2_10_10_10_rev())
        {
            gtype = GL3_INT_2_10_10_10_REV;
        }
        else
        {
            gtype = GL3_UNSIGNED_INT_2_10_10_10_REV;
        }
        is_n = TRUE;
        break;
    case RFIMPL_R16:
    case RFIMPL_GR16:
    case RFIMPL_BGR16:
    case RFIMPL_ABGR16:
        gtype = GL2_UNSIGNED_SHORT;
        is_n = TRUE;
        break;
    case RFIMPL_R16UI:
    case RFIMPL_GR16UI:
    case RFIMPL_BGR16UI:
    case RFIMPL_ABGR16UI:
        gtype = GL2_UNSIGNED_SHORT;
        is_n = FALSE;
        break;
    case RFIMPL_SIGNED_R16:
    case RFIMPL_SIGNED_GR16:
    case RFIMPL_SIGNED_BGR16:
    case RFIMPL_SIGNED_ABGR16:
        gtype = GL2_SHORT;
        is_n = TRUE;
        break;
    case RFIMPL_R16I:
    case RFIMPL_GR16I:
    case RFIMPL_BGR16I:
    case RFIMPL_ABGR16I:
        gtype = GL2_SHORT;
        is_n = FALSE;
        break;
    case RFIMPL_R32UI:
    case RFIMPL_GR32UI:
    case RFIMPL_BGR32UI:
    case RFIMPL_ABGR32UI:
        gtype = GL2_UNSIGNED_INT;
        is_n = FALSE;
        break;
    case RFIMPL_R32I:
    case RFIMPL_GR32I:
    case RFIMPL_BGR32I:
    case RFIMPL_ABGR32I:
        gtype = GL2_INT;
        is_n = FALSE;
        break;
    case RFIMPL_R16F:
    case RFIMPL_GR16F:
    case RFIMPL_BGR16F:
    case RFIMPL_ABGR16F:
        if (gl_Self_IsTextureRG())
        {
            gtype = GL3_HALF_FLOAT;
        }
        else
        {
            gtype = GL2_FLOAT;
        }
        is_n = FALSE;
        break;
    case RFIMPL_B10G11R11F:
        if (gl_Self_IsPackedFloat())
        {
            gtype = GL3_UNSIGNED_INT_10F_11F_11F_REV;
        }
        else
        {
            PI_ASSERT(FALSE, "unsupported format = %d", impl);
        }
        is_n = FALSE;
        break;
    case RFIMPL_R32F:
    case RFIMPL_GR32F:
    case RFIMPL_BGR32F:
    case RFIMPL_ABGR32F:
        gtype = GL2_FLOAT;
        is_n = FALSE;
        break;
    default:
        PI_ASSERT(FALSE, "invalid vertex format = %d", impl);
    }

    if(gl_type != NULL)
        *gl_type = gtype;
    if(is_normalized != NULL)
        *is_normalized = is_n;
}

uint gl_vertex_type_get(EVertexType type)
{
	uint r = GL2_FLOAT;
	switch(type)
	{
	case EVT_BYTE:
		r = GL2_BYTE;
		break;
	case EVT_UNSIGNED_BYTE:
		r = GL2_UNSIGNED_BYTE;
		break;
	case EVT_SHORT:
		r = GL2_SHORT;
		break;
	case EVT_UNSIGNED_SHORT:
		r = GL2_UNSIGNED_SHORT;
		break;
	case EVT_INT:
		r = GL2_INT;
		break;
	case EVT_UNSIGNED_INT:
		r = GL2_UNSIGNED_INT;
		break;
	case EVT_FLOAT:
		r = GL2_FLOAT;
		break;
	default:
		PI_ASSERT(FALSE, "invalid vertex type = %d", type);
		break;
	}
	return r;
}

uint gl_tex_face_get(TextureCubeFace face)
{
	uint glCubeFace = GL2_TEXTURE_CUBE_MAP_POSITIVE_X;
	
	switch(face)
	{
	case CF_POSITIVE_X:
		glCubeFace = GL2_TEXTURE_CUBE_MAP_POSITIVE_X;
		break;
	case CF_NEGATIVE_X:
		glCubeFace = GL2_TEXTURE_CUBE_MAP_NEGATIVE_X;
		break;
	case CF_POSITIVE_Y:
		glCubeFace = GL2_TEXTURE_CUBE_MAP_POSITIVE_Y;
		break;
	case CF_NEGATIVE_Y:
		glCubeFace = GL2_TEXTURE_CUBE_MAP_NEGATIVE_Y;
		break;
	case CF_POSITIVE_Z:
		glCubeFace = GL2_TEXTURE_CUBE_MAP_POSITIVE_Z;
		break;
	case CF_NEGATIVE_Z:
		glCubeFace = GL2_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		break;
	default:
		break;
	}
	return glCubeFace;
}

uint gl_buffer_usage_get(EBufferUsage usage)
{
	uint r = GL2_DYNAMIC_DRAW;
	switch (usage)
	{
	case EVU_STREAM_DRAW:
		r = GL2_STREAM_DRAW;
		break;
	case EVU_STREAM_READ:	
		r = GL3_STREAM_READ;
		break;
	case EVU_STREAM_COPY:
		r = GL3_STREAM_COPY;
		break;
	case EVU_STATIC_DRAW:
		r = GL2_STATIC_DRAW;
		break;
	case EVU_STATIC_READ:
		r = GL3_STATIC_READ;
		break;
	case EVU_STATIC_COPY:
		r = GL3_STATIC_COPY;
		break;
	case EVU_DYNAMIC_DRAW:
		r = GL2_DYNAMIC_DRAW;
		break;
	case EVU_DYNAMIC_READ:
		r = GL3_DYNAMIC_READ;
		break;
	case EVU_DYNAMIC_COPY:
		r = GL3_DYNAMIC_COPY;
		break;
	default:
		break;
	}
	return r;
}