#ifndef _PhysX_Convert_H_
#define _PhysX_Convert_H_
#pragma once
#include "pi_renderdata.h"
#include "UserRenderVertexBufferDesc.h"

using namespace nvidia;

VertexSemantic apex_semantic_to_pi_semantic(RenderVertexSemantic::Enum semantic);

EGeometryType apex_geometery_to_pi_geometery(RenderPrimitiveType::Enum type);

void apex_vertex_format_to_pi_format(RenderDataFormat::Enum format, EVertexType* type, uint32 * component_num);
#endif