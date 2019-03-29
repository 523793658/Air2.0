#include "PrimitiveSceneProxy.h"
#include "MeshBatch.h"
#include "Classes/Components/CubeComponent.h"
#include "Classes/Materials/Material.h"
#include "SimpleReflection.h"
#include "LocalVertexFactory.h"
namespace Air
{
	CubeComponent::CubeMesh::CubeMesh()
	{
		
		mIndexBuffer.mIndices.addUninitialized(36);
		mVertexBuffer.mVertices.addUninitialized(24);

		TArray<int32>& mIndices = mIndexBuffer.mIndices;
		
		Color color(255, 255, 255, 255);
		float3 position[24]{
			{-0.5, -0.5, -0.5},
			{0.5, -0.5, -0.5},
			{-0.5, 0.5, -0.5},
			{0.5, 0.5, -0.5},

			{0.5, -0.5, -0.5},
			{0.5, -0.5, 0.5},
			{0.5, 0.5, -0.5},
			{0.5, 0.5, 0.5},

			{0.5, -0.5, 0.5},
			{-0.5, -0.5, 0.5},
			{0.5, 0.5, 0.5},
			{-0.5, 0.5, 0.5},

			{-0.5, -0.5, 0.5},
			{-0.5, -0.5, -0.5},
			{-0.5, 0.5, 0.5},
			{-0.5, 0.5, -0.5},

			{ -0.5, 0.5, -0.5 },
			{ 0.5, 0.5, -0.5 },
			{ -0.5, 0.5, 0.5 },
			{ 0.5, 0.5, 0.5 },

			{ -0.5, -0.5, 0.5 },
			{ 0.5, -0.5, 0.5 },
			{ -0.5, -0.5, -0.5 },
			{ 0.5, -0.5, -0.5 }
		};
		float3 normals[6] = 
		{
			{0.0f, 0.0f, -1.0f},
			{1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f},
			{-1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, -1.0f, 0.0f}
		};

		float3 tangents[6] =
		{
			{1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f},
			{-1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, -1.0f},
			{1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f}
		};

		float2 uvs[4] = 
		{
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},
			{1.0f, 1.0f}
		};
		//0
		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				mVertexBuffer.mVertices[i * 4 + j].mPosition = position[i * 4 + j];
				mVertexBuffer.mVertices[i * 4 + j].mColor = color;
				mVertexBuffer.mVertices[i * 4 + j].mNormal = normals[i];
				mVertexBuffer.mVertices[i * 4 + j].mTangent = tangents[i];
				mVertexBuffer.mVertices[i * 4 + j].mTextureCoordinate = uvs[j];
			}
			
			mIndices[i * 6 + 0] = i * 4;
			mIndices[i * 6 + 1] = i * 4 + 1;
			mIndices[i * 6 + 2] = i * 4 + 2;

			mIndices[i * 6 + 3] = i * 4 + 1;
			mIndices[i * 6 + 4] = i * 4 + 3;
			mIndices[i * 6 + 5] = i * 4 + 2;

		}
	}

	CubeComponent::CubeMesh* CubeComponent::mCubeMesh = new CubeComponent::CubeMesh();


	CubeComponent::CubeComponent(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mSize = 1.0f;
		mShapeColor = Color(255, 0, 0, 255);

	}

	void CubeComponent::_genericGeometryData()
	{
		mMesh = mCubeMesh;
	}

	DECLARE_SIMPLER_REFLECTION(CubeComponent);
}