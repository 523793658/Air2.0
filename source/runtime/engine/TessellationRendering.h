#pragma once
#include "EngineMininal.h"
#include "RHI.h"
namespace Air
{
	class MaterialInterface;
	class VertexFactoryType;


	ENGINE_API bool requiresAdjacencyInformation(std::shared_ptr<MaterialInterface> material, const VertexFactoryType* vertexFactoryType, ERHIFeatureLevel::Type inFeatureLevel);
}