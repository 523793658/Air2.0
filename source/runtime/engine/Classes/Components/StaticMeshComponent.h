#pragma once
#include "Classes/Components/MeshComponent.h"
#include "Classes/Engine/StaticMesh.h"
#include "ObjectGlobals.h"
namespace Air
{
	class ENGINE_API StaticMeshComponent : public MeshComponent
	{
		GENERATED_RCLASS_BODY(StaticMeshComponent, MeshComponent)
	public:
		

		virtual PrimitiveSceneProxy* createSceneProxy() override;

		RStaticMesh* getStaticMesh()const
		{
			return mStaticMesh.get();
		}

		virtual bool setStaticMesh(class RStaticMesh* newMesh);

		std::shared_ptr<class RStaticMesh> mStaticMesh;
	};
}