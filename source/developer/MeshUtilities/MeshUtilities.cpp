#include "MeshUtilities.h"
#include "Math/MathUtility.h"
#include "Modules/ModuleManager.h"
#include "RawMesh.h"
namespace Air
{

	static float getComparisonThreshold(MeshBuildSettings const & buildSettings)
	{
		return buildSettings.bRemoveDegenerates ? THRESH_POINTS_ARE_SAME : 0.0f;
	}

	struct IndexAndZ
	{
		float mZ;
		int32 mIndex;
		IndexAndZ() {}
		IndexAndZ(int32 inIndex, float3 v)
		{
			mZ = 0.30f * v.x + 0.33f * v.y + 0.37 * v.z;
			mIndex = inIndex;
		}
	};

	struct CompareIndexAndZ
	{
		FORCEINLINE bool operator()(IndexAndZ const & A, IndexAndZ const & B) const{ return A.mZ < B.mZ; }
	};

	inline bool pointsEqual(const float3& v1, const float3& v2, float comparisonThreshold)
	{
		return Math::abs(v1.x - v2.x) <= comparisonThreshold
			&& Math::abs(v1.y - v2.y) <= comparisonThreshold
			&& Math::abs(v1.z - v2.z) <= comparisonThreshold;
	}

	inline bool normalsEqual(const float3& v1, const float3& v2)
	{
		const float epsilon = THRESH_NORMALS_ARE_SAME;

		return Math::abs(v1.x - v2.x) <= epsilon
			&& Math::abs(v1.y - v2.y) <= epsilon
			&& Math::abs(v1.z - v2.z) <= epsilon;
	}

	inline bool uvsEqual(const float2& uv1, const float2& uv2)
	{
		const float epsilon = 1.0f / 1024.0f;
		return Math::abs(uv1.x - uv2.x) <= epsilon && Math::abs(uv1.y - uv2.y) <= epsilon;
	}

	static void findOverlappingCorners(std::multimap<int32, int32>& outOverlappingCorners, const TArray<float3>& inVertices, const TArray<uint32>& inIndices, float comparisonThreshold)
	{
		const int32 NumWedges = inIndices.size();
		TArray<IndexAndZ> vertIndexAndZ;
		vertIndexAndZ.reserve(NumWedges);
		for (int32 wedgetIndex = 0; wedgetIndex < NumWedges; wedgetIndex++)
		{
			new(vertIndexAndZ)IndexAndZ(wedgetIndex, inVertices[inIndices[wedgetIndex]]);
		}
		vertIndexAndZ.sort(CompareIndexAndZ());

		for (int32 i = 0; i < vertIndexAndZ.size(); i++)
		{
			for (int32 j = i + 1; j < vertIndexAndZ.size(); j++)
			{
				if (Math::abs(vertIndexAndZ[j].mZ - vertIndexAndZ[i].mZ) > comparisonThreshold)
					break;

				const float3& positionA = inVertices[inIndices[vertIndexAndZ[i].mIndex]];
				const float3& positionB = inVertices[inIndices[vertIndexAndZ[j].mIndex]];
				if (pointsEqual(positionA, positionB, comparisonThreshold))
				{
					outOverlappingCorners.emplace(vertIndexAndZ[i].mIndex, vertIndexAndZ[j].mIndex);
					outOverlappingCorners.emplace(vertIndexAndZ[j].mIndex, vertIndexAndZ[i].mIndex);
				}
			}
		}
	}

	static void findOverlappingCorners(
		std::multimap<int32, int32>& outOverlappingCorners,
		RawMesh const & rawMesh,
		float comparisonThreshold
	)
	{
		findOverlappingCorners(outOverlappingCorners, rawMesh.mVertexPositions, rawMesh.mWedgeIndices, comparisonThreshold);
	}

	static void computeTangents(
		const TArray<float3>& inVertices,
		const TArray<uint32>& inIndices,
		const TArray<float2>& inUVs,
		const TArray<uint32>& smoothingGroupIndices,
		std::multimap<int32, int32> const & overlappingConners,
		TArray<float3>& outTangentX,
		TArray<float3>& outTangentY,
		TArray<float3>& outTangentZ,
		const uint32 tangentOptions
	)
	{

	}

	static void computeTangents(
		RawMesh& rawMesh,
		std::multimap<int32, int32> const & overlappingCorners,
		uint32 tangetnOptions
	)
	{
		const float comparisonThreshold = (tangetnOptions & ETangentOptions::IgnoreDegenerateTriangles) ? THRESH_POINTS_ARE_SAME : 0.0f;
		computeTangents(rawMesh.mVertexPositions, rawMesh.mWedgeIndices, rawMesh.mWedgeTexCoords[0], rawMesh.mFaceSmoothingMasks, overlappingCorners, rawMesh.mWedgeTangentX, rawMesh.mWedgeTangentY, rawMesh.mWedgeTangentZ, tangetnOptions);
	}

	static int32 computeNumTexCoords(RawMesh const & rawMesh, int32 maxSupportedTexCoords)
	{
		int32 numWedges = rawMesh.mWedgeIndices.size();
		int32 numTexCoords = 0;
		for (int32 texcoordIndex = 0; texcoordIndex < MAX_MESH_TEXTURE_COORDS; ++texcoordIndex)
		{
			if (rawMesh.mWedgeTexCoords[texcoordIndex].size() != numWedges)
			{
				break;
			}
			numTexCoords++;
		}
		return Math::min(numTexCoords, maxSupportedTexCoords);
	}


	class MeshUtilities : public IMeshUtilities
	{
	public:

		virtual bool buildStaticMesh(StaticMeshRenderData& outRenderData, TArray<StaticMeshSourceModel>& sourceModels, const StaticMeshLODGroup& LODGroup) override;
	private:

		void cacheOptimizeVertexAndIndexBuffer(
			TArray<StaticMeshBuildVertex>& vertices,
			TArray<TArray<uint32>>& perSectionIndices,
			TArray<int32>& wedgeMap
		);


		void cacheOptimizeIndexBuffer(TArray<uint32>& indices);

		virtual void buildStaticMeshVertexAndIndexBuffers(TArray<StaticMeshBuildVertex>& outVertices, TArray<TArray<uint32>>& outPerSectionIndices, TArray<int32>& outWedgeMap, const RawMesh& rawMesh, const std::multimap<int32, int32>& overlappingCorners, const TMap<uint32, uint32>& materialToSectionMapping, float comparisonThreshold, float3 buildScale) override;

		friend class StaticMeshUtilityBuilder;
	private:
		IMeshReduction * mStaticMeshReduction;
	};


	class StaticMeshUtilityBuilder
	{
	public:
		StaticMeshUtilityBuilder() :mStage(EStage::Uninit), mNumValidLODs(0){}

		bool gatherSourceMeshesPerLOD(TArray<StaticMeshSourceModel>& sourceModels, IMeshReduction* meshReduction)
		{
			BOOST_ASSERT(mStage == EStage::Uninit);
			for (int32 LODIndex = 0; LODIndex < sourceModels.size(); ++LODIndex)
			{
				StaticMeshSourceModel& srcModel = sourceModels[LODIndex];
				RawMesh& rawMesh = *new(mLODMeshes)RawMesh;
				std::multimap<int32, int32>& overlappingCorners = *new(mLODOverlappingCorners)std::multimap<int32, int32>;

				if (!srcModel.mRawMeshBulkData->isEmpty())
				{
					srcModel.mRawMeshBulkData->loadRawMesh(rawMesh);
					if (!rawMesh.isValidOrFixable())
					{
						AIR_LOG(logMeshUtilities, Error, TEXT("Raw mesh is corrupt for LOD%d."), LODIndex);
						return false;
					}
					mLODBuildSettings[LODIndex] = srcModel.mBuildttings;
					float comparisonThreshold = getComparisonThreshold(mLODBuildSettings[LODIndex]);
					int32 numWegets = rawMesh.mWedgeIndices.size();

					findOverlappingCorners(overlappingCorners, rawMesh, comparisonThreshold);

					bool bRecomputeNormals = srcModel.mBuildttings.bRecomputeNormals || rawMesh.mWedgeTangentY.size() != numWegets;
					bool bRecomputeTangents = srcModel.mBuildttings.bRecomputeTangents || rawMesh.mWedgeTangentX.size() != numWegets || rawMesh.mWedgeTangentZ.size() != numWegets;
					if (bRecomputeTangents)
					{
						rawMesh.mWedgeTangentX.empty(numWegets);
						rawMesh.mWedgeTangentX.addZeroed(numWegets);
						rawMesh.mWedgeTangentZ.empty(numWegets);
						rawMesh.mWedgeTangentZ.addZeroed(numWegets);
					}

					if (bRecomputeNormals)
					{
						rawMesh.mWedgeTangentY.empty(numWegets);
						rawMesh.mWedgeTangentY.addZeroed(numWegets);
					}

					{
						uint32 tangentOptions = ETangentOptions::BlendOverlappingNormals;
						if (srcModel.mBuildttings.bRemoveDegenerates)
						{
							tangentOptions |= ETangentOptions::IgnoreDegenerateTriangles;
						}

						if (srcModel.mBuildttings.bUseMikkTSpace && (srcModel.mBuildttings.bRecomputeNormals || srcModel.mBuildttings.bRecomputeTangents))
						{

						}
						else
						{
							computeTangents(rawMesh, overlappingCorners, tangentOptions);
						}
					}

					BOOST_ASSERT(rawMesh.mWedgeTangentX.size() == numWegets);
					BOOST_ASSERT(rawMesh.mWedgeTangentY.size() == numWegets);
					BOOST_ASSERT(rawMesh.mWedgeTangentZ.size() == numWegets);
					hasRawMesh[LODIndex] = true;

				}
				else if (LODIndex > 0 && meshReduction)
				{

				}
			}
			BOOST_ASSERT(mLODMeshes.size() == sourceModels.size());
			BOOST_ASSERT(mLODOverlappingCorners.size() == sourceModels.size());

			if (mLODMeshes.size() == 0 || mLODMeshes[0].mWedgeIndices.size() == 0)
			{
				return false;
			}

			mStage = EStage::Gathered;
			return true;
		}

		bool reduceLODs(TArray<StaticMeshSourceModel>& sourceModels, const StaticMeshLODGroup& LODGroup, IMeshReduction* meshReduction, bool & bOutWasReduced)
		{
			BOOST_ASSERT(mStage == EStage::Gathered);

			for (int32 lodIndex = 0; lodIndex < sourceModels.size(); ++lodIndex)
			{
				const StaticMeshSourceModel& srcModel = sourceModels[lodIndex];
				MeshReductionSettings reductionSettings = LODGroup.getSettings(srcModel.mReductionSettings, lodIndex);
				mLODMaxDeviation[mNumValidLODs] = 0.0f;
				if (lodIndex != mNumValidLODs)
				{
					mLODBuildSettings[mNumValidLODs] = mLODBuildSettings[lodIndex];
					mLODOverlappingCorners[mNumValidLODs] = mLODOverlappingCorners[lodIndex];
				}
				if (meshReduction && (reductionSettings.mPercentTriangles < 1.0f || reductionSettings.mMaxDeviation > 0.0f))
				{

				}
				if (mLODMeshes[mNumValidLODs].mWedgeIndices.size() > 0)
				{
					mNumValidLODs++;
				}
			}

			if (mNumValidLODs < 1)
			{
				return false;
			}
			mStage = EStage::Reduce;
			return true;
		}


		bool generateRenderingMeshes(MeshUtilities& meshUtilities, StaticMeshRenderData& outRenderData, TArray<StaticMeshSourceModel>& inOutModels)
		{
			outRenderData.allocateLODResource(mNumValidLODs);
			for (int32 lodIndex = 0; lodIndex < mNumValidLODs; ++lodIndex)
			{
				StaticMeshLODResources& LODModel = outRenderData.mLODResources[lodIndex];
				RawMesh& rawMesh = mLODMeshes[lodIndex];
				LODModel.mMaxDeviation = mLODMaxDeviation[lodIndex];

				TArray<StaticMeshBuildVertex> vertices;
				TArray<TArray<uint32>> perSectionIndices;
				TMap<uint32, uint32> materialToSectionMapping;

				TArray<uint32> materialIndices;

				for (const int32 materialIndex : rawMesh.mFaceMaterialIndices)
				{
					materialIndices.addUnique(materialIndex);
				}

				for (int32 index = 0; index < materialIndices.size(); index++)
				{
					const int32 materialIndex = materialIndices[index];
					StaticMeshSection* section = new(LODModel.mSections)StaticMeshSection();
					section->mMaxVertexIndex = materialIndex;
					materialToSectionMapping.emplace(materialIndex, index);
					new(perSectionIndices)TArray<uint32>;
				}

				{
					TArray<int32> tempWedgeMap;
					TArray<int32>& wedgeMap = (lodIndex == 0 && inOutModels[0].mReductionSettings.mPercentTriangles >= 1.0f) ? outRenderData.mWedgeMap : tempWedgeMap;
					float comparisionThreshold = getComparisonThreshold(mLODBuildSettings[lodIndex]);
					meshUtilities.buildStaticMeshVertexAndIndexBuffers(vertices, perSectionIndices, wedgeMap, rawMesh, mLODOverlappingCorners[lodIndex], materialToSectionMapping, comparisionThreshold, mLODBuildSettings[lodIndex].mBuildScale3D);
					BOOST_ASSERT(wedgeMap.size() == rawMesh.mWedgeIndices.size());

					if (rawMesh.mWedgeIndices.size() < 100000 * 3)
					{
						meshUtilities.cacheOptimizeVertexAndIndexBuffer(vertices, perSectionIndices, wedgeMap);
						BOOST_ASSERT(wedgeMap.size() == rawMesh.mWedgeIndices.size());
					}
				}

				BOOST_ASSERT(vertices.size() != 0);

				int32 numTexCoords = computeNumTexCoords(rawMesh, MAX_STATIC_TEXCOORDS);
				LODModel.mVertexBuffer.setUseHighPrecisionTangentBasis(mLODBuildSettings[lodIndex].bUseHighPrecisionTangentBasis);
				LODModel.mVertexBuffer.setUseFullPrecisionUVs(mLODBuildSettings[lodIndex].bUseFullPrecisionUVs);
				LODModel.mVertexBuffer.init(vertices, numTexCoords);
				LODModel.mPositionVertexBuffer.init(vertices);
				LODModel.mColorVertexBuffer.init(vertices);

				TArray<uint32> combinedIndices;
				bool bNeeds32BitIndices = false;
				for (int32 sectionIndex = 0; sectionIndex < LODModel.mSections.size(); sectionIndex++)
				{
					StaticMeshSection& section = LODModel.mSections[sectionIndex];
					TArray<uint32> const & sectionIndices = perSectionIndices[sectionIndex];
					section.mFirstIndex = 0; 
					section.mNumTriangles = 0;
					section.mMinVertexIndex = 0;
					section.mMaxVertexIndex = 0;

					if (sectionIndices.size())
					{
						section.mFirstIndex = combinedIndices.size();
						section.mNumTriangles = sectionIndices.size() / 3;
						combinedIndices.addUninitialized(sectionIndices.size());
						uint32* destPtr = &combinedIndices[section.mFirstIndex];
						uint32 const * srcPtr = sectionIndices.getData();

						section.mMinVertexIndex = *srcPtr;
						section.mMaxVertexIndex = *srcPtr;

						for (int32 index = 0; index < sectionIndices.size(); index++)
						{
							uint32 vertIndex = *srcPtr++;

							bNeeds32BitIndices |= (vertIndex > std::numeric_limits<uint16>::max());

							section.mMinVertexIndex = Math::min<uint32>(vertIndex, section.mMinVertexIndex);
							section.mMaxVertexIndex = Math::max<uint32>(vertIndex, section.mMaxVertexIndex);
							*destPtr++ = vertIndex;
						}
					}
				}

				LODModel.mIndexBuffer.setIndices(combinedIndices, bNeeds32BitIndices ? EIndexBufferStride::Force32Bit : EIndexBufferStride::Force16Bit);



			}

			Box boundingBox(0);
			PositionVertexBuffer& basePositionVertexBuffer = outRenderData.mLODResources[0].mPositionVertexBuffer;
			for (int32 vertexIndex = 0; vertexIndex < basePositionVertexBuffer.getNumVertices(); vertexIndex++)
			{
				boundingBox += basePositionVertexBuffer.vertexPosition(vertexIndex);
			}

			boundingBox.getCenterAndExtents(outRenderData.mBounds.mOrigin, outRenderData.mBounds.mBoxExtent);

			outRenderData.mBounds.mSphereRadius = 0.0f;
			for (uint32 vertexIndex = 0; vertexIndex < basePositionVertexBuffer.getNumVertices(); vertexIndex++)
			{
				outRenderData.mBounds.mSphereRadius = Math::max<float>((basePositionVertexBuffer.vertexPosition(vertexIndex) - outRenderData.mBounds.mOrigin).size(), outRenderData.mBounds.mSphereRadius);
			}

			mStage = EStage::GenerateRendering;
			return true;
		}



	private:
		enum class EStage
		{
			Uninit,
			Gathered,
			Reduce,
			GenerateRendering,
			ReplaceRaw,
		};

		EStage mStage;
		int32 mNumValidLODs;
		TindirectArray<RawMesh> mLODMeshes;
		TindirectArray<std::multimap<int32, int32>> mLODOverlappingCorners;

		float mLODMaxDeviation[MAX_STATIC_MESH_LODS];
		MeshBuildSettings mLODBuildSettings[MAX_STATIC_MESH_LODS];
		bool hasRawMesh[MAX_STATIC_MESH_LODS];
	};


	bool MeshUtilities::buildStaticMesh(StaticMeshRenderData& outRenderData, TArray<StaticMeshSourceModel>& sourceModels, const StaticMeshLODGroup& LODGroup)
	{
		StaticMeshUtilityBuilder builder;
		if (!builder.gatherSourceMeshesPerLOD(sourceModels, mStaticMeshReduction))
		{
			return false;
		}

		bool bWasReduced = false;
		if (!builder.reduceLODs(sourceModels, LODGroup, mStaticMeshReduction, bWasReduced))
		{
			return false;
		}

		return builder.generateRenderingMeshes(*this, outRenderData, sourceModels);
	}

	void MeshUtilities::cacheOptimizeVertexAndIndexBuffer(TArray<StaticMeshBuildVertex>& vertices, TArray<TArray<uint32>>& perSectionIndices, TArray<int32>& wedgeMap)
	{
		TArray<StaticMeshBuildVertex> originalVertices = vertices;
		TArray<int32> indexCache;
		indexCache.addUninitialized(vertices.size());
		Memory::Memset(indexCache.getData(), INDEX_NONE, indexCache.size() * indexCache.getTypeSize());

		int32 nextAvailableIndex = 0; 
		for (int32 sectionIndex = 0; sectionIndex < perSectionIndices.size(); sectionIndex++)
		{
			TArray<uint32>& indices = perSectionIndices[sectionIndex];
			if (indices.size())
			{
				cacheOptimizeIndexBuffer(indices);
				TArray<uint32> originalIndices = indices;
				for (int32 index = 0; index < indices.size(); index++)
				{
					const int32 cachedIndex = indexCache[originalIndices[index]];
					if (cachedIndex == INDEX_NONE)
					{
						indices[index] = nextAvailableIndex;
						indexCache[originalIndices[index]] = nextAvailableIndex;
						nextAvailableIndex++;
					}
					else
					{
						indices[index] = cachedIndex;
					}

					vertices[indices[index]] = originalVertices[originalIndices[index]];
				}
			}
		}

		for (int32 i = 0; i < wedgeMap.size(); i++)
		{
			int32 mappedIndex = wedgeMap[i];
			if (mappedIndex != INDEX_NONE)
			{
				wedgeMap[i] = indexCache[mappedIndex];
			}
		}
	}

	//ÓÅ»¯Ë÷Òý
	void MeshUtilities::cacheOptimizeIndexBuffer(TArray<uint32>& indices)
	{
		
	}
	static inline float3 getPositionForWedge(RawMesh const & mesh, int32 wedgeIndex)
	{
		int32 vertexIndex = mesh.mWedgeIndices[wedgeIndex];
		return mesh.mVertexPositions[vertexIndex];
	}

	static StaticMeshBuildVertex buildStaticMeshVertex(RawMesh const & rawMesh, int32 wedgeindex, float3 buildScale)
	{
		StaticMeshBuildVertex vertex;
		vertex.mPosition = getPositionForWedge(rawMesh, wedgeindex) * buildScale;
		const Matrix scaleMatrix = ScaleMatrix(buildScale).inverse().getTransposed();
		vertex.TangentX = scaleMatrix.transformVector(rawMesh.mWedgeTangentX[wedgeindex]).getSafeNormal();
		vertex.TangentY = scaleMatrix.transformVector(rawMesh.mWedgeTangentY[wedgeindex]).getSafeNormal();
		vertex.TangentZ = scaleMatrix.transformVector(rawMesh.mWedgeTangentZ[wedgeindex]).getSafeNormal();

		if (rawMesh.mWedgeColors.isValidIndex(wedgeindex))
		{
			vertex.mColor = rawMesh.mWedgeColors[wedgeindex];
		}
		else
		{
			vertex.mColor = Color::White;
		}

		int32 numTexCoords = Math::min<int32>(MAX_MESH_TEXTURE_COORDS, MAX_STATIC_TEXCOORDS);
		for (int32 i = 0; i < numTexCoords; i++)
		{
			if (rawMesh.mWedgeTexCoords[i].isValidIndex(wedgeindex))
			{
				vertex.UVs[i] = rawMesh.mWedgeTexCoords[i][wedgeindex];
			}
			else
			{
				vertex.UVs[i] = float2::zero();
			}
		}
		return vertex;
	}

	static bool areVerticesEqual(
		StaticMeshBuildVertex const & a,
		StaticMeshBuildVertex const & b,
		float comparisonThreshold
	)
	{
		if (!pointsEqual(a.mPosition, b.mPosition, comparisonThreshold) ||
			!normalsEqual(a.TangentX, b.TangentX) ||
			!normalsEqual(a.TangentY, b.TangentY) ||
			!normalsEqual(a.TangentZ, b.TangentZ) || a.mColor != b.mColor)
		{
			return false;
		}

		for (int32 uvIndex = 0; uvIndex < MAX_STATIC_TEXCOORDS; uvIndex++)
		{
			if (!uvsEqual(a.UVs[uvIndex], b.UVs[uvIndex]))
			{
				return false;
			}
		}
		return true;
	}


	void MeshUtilities::buildStaticMeshVertexAndIndexBuffers(TArray<StaticMeshBuildVertex>& outVertices, TArray<TArray<uint32>>& outPerSectionIndices, TArray<int32>& outWedgeMap, const RawMesh& rawMesh, const std::multimap<int32, int32>& overlappingCorners, const TMap<uint32, uint32>& materialToSectionMapping, float comparisonThreshold, float3 buildScale)
	{
		TMap<int32, int32> finalVerts;

		TArray<int32> dupVerts;

		int32 numFaces = rawMesh.mWedgeIndices.size() / 3;

		for (int32 faceIndex = 0; faceIndex < numFaces; faceIndex++)
		{
			int32 vertexIndices[3];
			float3 cornerPostiions[3];
			for (int32 cornerIndex = 0; cornerIndex < 3; cornerIndex++)
			{
				cornerPostiions[cornerIndex] = getPositionForWedge(rawMesh, faceIndex * 3 + cornerIndex);
			}
			if (pointsEqual(cornerPostiions[0], cornerPostiions[1], comparisonThreshold) || pointsEqual(cornerPostiions[1], cornerPostiions[2], comparisonThreshold) || pointsEqual(cornerPostiions[0], cornerPostiions[2], comparisonThreshold))
			{
				for (int32 cornerIndex = 0; cornerIndex < 3; cornerIndex++)
				{
					outWedgeMap.add(INDEX_NONE);
				}
				continue;
			}
			for (int32 cornerIndex = 0; cornerIndex < 3; cornerIndex++)
			{
				int32 wedgeIndex = faceIndex * 3 + cornerIndex;
				StaticMeshBuildVertex thisVertex = buildStaticMeshVertex(rawMesh, wedgeIndex, buildScale);

				dupVerts.reset();
				auto range = overlappingCorners.equal_range(wedgeIndex);
				for (auto& it = range.first; it != range.second; it++)
				{
					int32 i = 0;
					for (i = dupVerts.size() - 1; i >= 0; i--)
					{
						if (dupVerts[i] <= it->second)
						{
							break;
						}
					}
					dupVerts.insert(it->second, i + 1);
				}

				int32 index = INDEX_NONE;
				for (int32 k = 0; k < dupVerts.size(); k++)
				{
					if (dupVerts[k] >= wedgeIndex)
					{
						break;
					}

					auto& location = finalVerts.find(dupVerts[k]);
					if (location != finalVerts.end() && areVerticesEqual(thisVertex, outVertices[location->second], comparisonThreshold))
					{
						index = location->second;
						break;
					}
				}
				if (index == INDEX_NONE)
				{
					index = outVertices.add(thisVertex);
					finalVerts.emplace(wedgeIndex, index);
				}
				vertexIndices[cornerIndex] = index;
			}

			if (vertexIndices[0] == vertexIndices[1]
				|| vertexIndices[1] == vertexIndices[2]
				|| vertexIndices[0] == vertexIndices[2])
			{
				for (int32 cornerIndex = 0; cornerIndex < 3; cornerIndex++)
				{
					outWedgeMap.add(INDEX_NONE);
				}
				continue;
			}

			uint32 sectionIndex = 0;
			sectionIndex = materialToSectionMapping.findChecked(rawMesh.mFaceMaterialIndices[faceIndex]);
			TArray<uint32>& sectionIndices = outPerSectionIndices[sectionIndex];
			for (int32 cornerIndex = 0; cornerIndex < 3; cornerIndex++)
			{
				sectionIndices.add(vertexIndices[cornerIndex]);
				outWedgeMap.add(vertexIndices[cornerIndex]);
			}
		}
	}
}

IMPLEMENT_MODULE(MeshUtilities, MeshUtilities);
