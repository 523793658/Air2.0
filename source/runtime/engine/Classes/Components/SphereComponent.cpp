#include "PrimitiveSceneProxy.h"
#include "MeshBatch.h"
#include "Classes/Components/SphereComponent.h"
#include "Classes/Materials/Material.h"
#include "SimpleReflection.h"
#include "LocalVertexFactory.h"
namespace Air
{
	SphereComponent::SphereMesh::SphereMesh()
	{
		int n = 20;
		int rows = n * 2 + 1;
		int cels = n * 4 + 1;
		int i, j;
		float tcdx = 1.0f / (rows - 1);
		float tcdy = 1.0f / (cels - 1);
		float dgy = PI / (rows - 1);
		float dgx = 2 * PI / (cels - 1);
		mIndexBuffer.mIndices.addUninitialized((rows - 1) * (cels - 1) * 2 * 3);
		mVertexBuffer.mVertices.addUninitialized(rows * cels);

		TArray<int32>& mIndices = mIndexBuffer.mIndices;
		for (i = 0; i < rows; i++)
		{
			for (j = 0; j < cels; j++)
			{
				DynamicMeshVertex& vertex = mVertexBuffer.mVertices[i * cels + j];

				float3& v = vertex.mPosition;
				PackedNormal& normal = vertex.mNormal;
				v.x = sin((i - n)* dgy) * 0.5f;
				v.y = cos((i - n) * dgy) * sin(j * dgx) * 0.5f;
				v.z = cos((i - n) * dgy) * cos(j * dgx) * 0.5f;
				normal.set(v);
				vertex.mTangent.set(float3(1, 0, 0));
				if (i < rows - 1 && j < cels - 1)
				{
					mIndices[(i * (cels - 1) + j) * 6] = (i * cels + j);
					mIndices[(i * (cels - 1) + j) * 6 + 2] = (i * cels + j + 1);
					mIndices[(i * (cels - 1) + j) * 6 + 1] = ((i + 1) * cels + j + 1);

					mIndices[(i * (cels - 1) + j) * 6 + 3] = (i * cels + j);
					mIndices[(i * (cels - 1) + j) * 6 + 5] = ((i + 1) * cels + j + 1);
					mIndices[(i * (cels - 1) + j) * 6 + 4] = ((i + 1) * cels + j);
				}
			}
		}
	}

	SphereComponent::SphereMesh* SphereComponent::mSphereMesh = new SphereComponent::SphereMesh();

	SphereComponent::SphereComponent(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mSphereRadius = 32.0f;
		mShapeColor = Color(255, 0, 0, 255);

	}

	void SphereComponent::_genericGeometryData()
	{
		mMesh = mSphereMesh;
	}

	DECLARE_SIMPLER_REFLECTION(SphereComponent);
}