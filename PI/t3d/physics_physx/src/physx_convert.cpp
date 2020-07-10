#include "physx_convert.h"

VertexSemantic apex_semantic_to_pi_semantic(RenderVertexSemantic::Enum semantic)
{
	VertexSemantic r = EVS_TEXCOORD_6;
	switch (semantic)
	{
	case nvidia::apex::RenderVertexSemantic::POSITION:
		r = EVS_POSITION;
		break;
	case nvidia::apex::RenderVertexSemantic::NORMAL:
		r = EVS_NORMAL;
		break;
	case nvidia::apex::RenderVertexSemantic::TANGENT:
		r = EVS_TANGENT;
		break;
	case nvidia::apex::RenderVertexSemantic::BINORMAL:
		r = EVS_BINORMAL;
		break;
	case nvidia::apex::RenderVertexSemantic::COLOR:
		r = EVS_DIFFUSE;
		break;
	case nvidia::apex::RenderVertexSemantic::TEXCOORD0:
		r = EVS_TEXCOORD_0;
		break;
	case nvidia::apex::RenderVertexSemantic::TEXCOORD1:
		r = EVS_TEXCOORD_1;
		break;
	case nvidia::apex::RenderVertexSemantic::TEXCOORD2:
		r = EVS_TEXCOORD_2;
		break;
	case nvidia::apex::RenderVertexSemantic::TEXCOORD3:
		r = EVS_TEXCOORD_3;
		break;
	case nvidia::apex::RenderVertexSemantic::BONE_INDEX:
		r = EVS_BLEND_INDICES;
		break;
	case nvidia::apex::RenderVertexSemantic::BONE_WEIGHT:
		r = EVS_BLEND_WEIGHTS;
		break;
	case nvidia::apex::RenderVertexSemantic::DISPLACEMENT_TEXCOORD:
		r = EVS_SPECULAR;
		break;
	case nvidia::apex::RenderVertexSemantic::DISPLACEMENT_FLAGS:
		r = EVS_TEXCOORD_4;
		break;
	case nvidia::apex::RenderVertexSemantic::CUSTOM:
		r = EVS_TEXCOORD_5;
		break;
	default:
		break;
	}
	return r;
}

EGeometryType apex_geometery_to_pi_geometery(RenderPrimitiveType::Enum type)
{
	EGeometryType piType = EGOT_UNKOWN;
	switch (type)
	{
	case nvidia::apex::RenderPrimitiveType::TRIANGLES:
		piType = EGOT_TRIANGLE_LIST;
		break;
	case nvidia::apex::RenderPrimitiveType::TRIANGLE_STRIP:
		piType = EGOT_TRIANGLE_STRIP;
		break;
	case nvidia::apex::RenderPrimitiveType::LINES:
		piType = EGOT_LINE_LIST;
		break;
	case nvidia::apex::RenderPrimitiveType::LINE_STRIP:
		piType = EGOT_LINE_STRIP;
		break;
	case nvidia::apex::RenderPrimitiveType::POINTS:
		piType = EGOT_POINT_LIST;
		break;
	default:
		break;
	}
	return piType;
}


void apex_vertex_format_to_pi_format(RenderDataFormat::Enum format, EVertexType* type, uint32 * component_num)
{
	*type = EVT_UNSPECIFIED;
	switch (format)
	{
	case nvidia::apex::RenderDataFormat::UBYTE1:
		*type = EVT_UNSIGNED_BYTE;
		*component_num = 1;
		break;
	case nvidia::apex::RenderDataFormat::UBYTE2:
		*type = EVT_UNSIGNED_BYTE;
		*component_num = 2;
		break;
	case nvidia::apex::RenderDataFormat::UBYTE3:
		*type = EVT_UNSIGNED_BYTE;
		*component_num = 3;
		break;
	case nvidia::apex::RenderDataFormat::UBYTE4:
		*type = EVT_UNSIGNED_BYTE;
		*component_num = 4;
		break;
	case nvidia::apex::RenderDataFormat::USHORT1:
		*type = EVT_UNSIGNED_SHORT;
		*component_num = 2;
		break;
	case nvidia::apex::RenderDataFormat::USHORT2:
		*type = EVT_UNSIGNED_SHORT;
		*component_num = 2;
		break;
	case nvidia::apex::RenderDataFormat::USHORT3:
		*type = EVT_UNSIGNED_SHORT;
		*component_num = 4;
		break;
	case nvidia::apex::RenderDataFormat::USHORT4:
		*type = EVT_UNSIGNED_SHORT;
		*component_num = 4;
		break;
	case nvidia::apex::RenderDataFormat::SHORT1:
		*type = EVT_SHORT;
		*component_num = 1;
		break;
	case nvidia::apex::RenderDataFormat::SHORT2:
		*type = EVT_SHORT;
		*component_num = 2;
		break;
	case nvidia::apex::RenderDataFormat::SHORT3:
		*type = EVT_SHORT;
		*component_num = 3;
		break;
	case nvidia::apex::RenderDataFormat::SHORT4:
		*type = EVT_SHORT;
		*component_num = 4;
		break;
	case nvidia::apex::RenderDataFormat::UINT1:
		*type = EVT_UNSIGNED_INT;
		*component_num = 1;
		break;
	case nvidia::apex::RenderDataFormat::UINT2:
		*type = EVT_UNSIGNED_INT;
		*component_num = 2;
		break;
	case nvidia::apex::RenderDataFormat::UINT3:
		*type = EVT_UNSIGNED_INT;
		*component_num = 3;
		break;
	case nvidia::apex::RenderDataFormat::UINT4:
		*type = EVT_UNSIGNED_INT;
		*component_num = 4;
		break;
	case nvidia::apex::RenderDataFormat::FLOAT1:
		*type = EVT_FLOAT;
		*component_num = 1;
		break;
	case nvidia::apex::RenderDataFormat::FLOAT2:
		*type = EVT_FLOAT;
		*component_num = 2;
		break;
	case nvidia::apex::RenderDataFormat::FLOAT3:
		*type = EVT_FLOAT;
		*component_num = 3;
		break;
	case nvidia::apex::RenderDataFormat::FLOAT4:
		*type = EVT_FLOAT;
		*component_num = 4;
		break;
	default:
		break;
	}
}
