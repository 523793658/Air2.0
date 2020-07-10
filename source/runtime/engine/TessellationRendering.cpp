#include "TessellationRendering.h"
#include "MaterialShared.h"
#include "VertexFactory.h"
#include "Classes/Materials/MaterialInterface.h"
#include "Classes/Materials/Material.h"
namespace Air
{
	bool materialRequiresAdjacencyInformation_RenderingThread(std::shared_ptr<MaterialInterface> material, const VertexFactoryType* vertexFactoryType, ERHIFeatureLevel::Type inFeatureLevel)
	{
		BOOST_ASSERT(isInRenderingThread());
		bool vertexFactorySupportsTesselation = !vertexFactoryType || (vertexFactoryType && vertexFactoryType->supportsTessellationShaders());
		if (RHISupportsTessellation(GShaderPlatformForFeatureLevel[inFeatureLevel]) && vertexFactorySupportsTesselation && material)
		{
			MaterialRenderProxy* materialRenderProxy = material->getRenderProxy();
			if (materialRenderProxy)
			{
				const FMaterial* materialResource = materialRenderProxy->getMaterial(inFeatureLevel);
				if (materialResource)
				{
					EMaterialTessellationMode tessellationMode = materialResource->getTessellationMode();
					bool bEnableCrackFreeDisplacement = materialResource->isCrackFreeDisplacementEnabled();
					return tessellationMode == MTM_PNTriangles || (tessellationMode == MTM_FlatTessellation && bEnableCrackFreeDisplacement);
				}
			}
		}
		return false;
	}

	bool materialSettingsRequiresAdjacencyInformation_GameThread(std::shared_ptr<MaterialInterface> material, const VertexFactoryType* vertexFactoryType, ERHIFeatureLevel::Type inFealtureLevel)
	{
		BOOST_ASSERT(isInGameThread());
		bool vertexFactorySupoortsTesselation = !vertexFactoryType || (vertexFactoryType && vertexFactoryType->supportsTessellationShaders());
		if (RHISupportsTessellation(GShaderPlatformForFeatureLevel[inFealtureLevel]) && vertexFactorySupoortsTesselation && material)
		{
			std::shared_ptr<RMaterial> baseMaterial = material->getMaterial();
			BOOST_ASSERT(baseMaterial);
			EMaterialTessellationMode tesselationMode = (EMaterialTessellationMode)baseMaterial->mD3D11TessellationMode;
			bool bEnableCrackFreeDisplacement = baseMaterial->bEnableCrackFreeDisplacemenet;
			return tesselationMode == MTM_PNTriangles || (tesselationMode == MTM_FlatTessellation && bEnableCrackFreeDisplacement);
		}
		return false;
	}


	bool requiresAdjacencyInformation(std::shared_ptr<MaterialInterface> material, const VertexFactoryType* vertexFactoryType, ERHIFeatureLevel::Type inFeatureLevel)
	{
		if (isInRenderingThread())
		{
			return materialRequiresAdjacencyInformation_RenderingThread(material, vertexFactoryType, inFeatureLevel);
		}
		else if (isInGameThread())
		{
			return materialSettingsRequiresAdjacencyInformation_GameThread(material, vertexFactoryType, inFeatureLevel);
		}
		else
		{
			bool vertexFactorySupportsTesselation = !vertexFactoryType || (vertexFactoryType->supportsTessellationShaders());
			if (RHISupportsTessellation(GShaderPlatformForFeatureLevel[inFeatureLevel]) && vertexFactorySupportsTesselation && material)
			{
				MaterialInterface::TMicRecursionGuard recursionGuard;
				auto baseMaterial = material->getMaterial_Concurrent(recursionGuard);
				BOOST_ASSERT(baseMaterial);
				EMaterialTessellationMode tessellationMode = (EMaterialTessellationMode)baseMaterial->mD3D11TessellationMode;
				bool bEnableCrackFreeDisplacement = baseMaterial->bEnableCrackFreeDisplacemenet;
				return tessellationMode == MTM_PNTriangles || (tessellationMode == MTM_FlatTessellation && bEnableCrackFreeDisplacement);
			}
		}
		return false;
	}
}