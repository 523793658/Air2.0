#pragma once
#include "Classes/Components/ShapeComponent.h"
#include "Classes/Materials/Material.h"
namespace Air
{
	class ENGINE_API SphereComponent : public ShapeComponent
	{
		GENERATED_RCLASS_BODY(SphereComponent, ShapeComponent)
	public:
		

	protected:
		virtual void _genericGeometryData() override;

	protected:
		float mSphereRadius;
	public:
		Color mShapeColor;

	private:
		struct SphereMesh : CustomMesh
		{
			SphereMesh();
		};

		static SphereMesh* mSphereMesh;

	private:
		friend class SphereSceneProxy;

	};
}