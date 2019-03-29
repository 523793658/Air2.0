#pragma once
#include "Classes/Components/ShapeComponent.h"
#include "Classes/Materials/Material.h"
namespace Air
{
	class ENGINE_API CubeComponent : public ShapeComponent
	{
		GENERATED_RCLASS_BODY(CubeComponent, ShapeComponent)
	public:
	

	protected:
		virtual void _genericGeometryData() override;

	protected:
		float mSize;
	public:
		Color mShapeColor;

	private:
		struct CubeMesh : CustomMesh
		{
			CubeMesh();
		};

		static CubeMesh* mCubeMesh;

	private:
		friend class CubeSceneProxy;

	};
}