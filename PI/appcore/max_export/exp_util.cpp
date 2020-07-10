#include "piexp.h"
#include "util.h"
#include "modstack.h"

#include "cs\bipexp.h"
#include "iiksys.h"

#include <list>

cJSON *PIExp::_buildSKJson(const std::string &absPath)
{
	std::string path = absPath + m_strName + ".obj";
	TCHAR wc[MAX_FILE_PATH];
	std::size_t l;
	mbstowcs_s(&l, wc, MAX_FILE_PATH, path.c_str(), MAX_FILE_PATH);

	TCHAR szFilePath[MAX_FILE_PATH];
	WIN32_FIND_DATA findFileData;
	wcscpy_s(szFilePath, wc);
	wcscat_s(szFilePath, L"\\*.sk");

	cJSON *pAnims = cJSON_CreateArray();

	HANDLE hFind = ::FindFirstFile(szFilePath, &findFileData);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		return pAnims;
	}

	do
	{
		char mb[MAX_FILE_PATH];
		std::size_t l;
		wcstombs_s(&l, mb, MAX_FILE_PATH, findFileData.cFileName, MAX_FILE_PATH);
		std::string strName = mb;
		strName = strName.substr(0, strName.find_first_of('.'));

		//和自己重名的不处理，那个不是动画 是主骨骼
		if (m_strName == strName)
		{
			continue;
		}

		cJSON *pAnim = cJSON_CreateObject();
		cJSON_AddStringToObject(pAnim, "name", strName.c_str());
		cJSON_AddStringToObject(pAnim, "file", (m_strResPath + m_strName + ".obj/" + std::string(mb)).c_str());
		cJSON_AddItemToArray(pAnims, pAnim);
	} while (FindNextFile(hFind, &findFileData));

	return pAnims;
}

void PIExp::_exportModelToFile(const std::string &strFileName, INode *node, std::vector<INode *> *outMeshNodes, TimeValue t, bool recursion, bool onlyModel, float *aabb)
{
	FILE *fp = fopen(strFileName.c_str(), "wb");

	//写文件头
	fwrite(MODEL_VERSION, sizeof(char), sizeof(MODEL_VERSION) - 1, fp);
	//获取size的偏移
	UINT32 modelSizeOffset = ftell(fp);

	//获取subMesh数量的偏移
	UINT32 size = 0;
	fwrite(&size, sizeof(UINT32), 1, fp);
	UINT32 subMeshNumOffset = ftell(fp);
	UINT32 numSubMesh = 0;
	fwrite(&numSubMesh, sizeof(UINT32), 1, fp);

	std::vector<INode *> *pMeshNodes = outMeshNodes;
	std::vector<INode *> meshNodes;
	if (pMeshNodes == NULL)
	{
		pMeshNodes = &meshNodes;
	}

	if(!_exportModelData(node, t, recursion, fp, pMeshNodes, onlyModel, aabb))
	{
		fclose(fp);
		return;
	}

	//写subMesh数量
	numSubMesh = pMeshNodes->size();
	fseek(fp, subMeshNumOffset, SEEK_SET);
	fwrite(&numSubMesh, sizeof(UINT32), 1, fp);
	fseek(fp, 0, SEEK_END);

	//写size
	UINT32 totalSize = ftell(fp);
	fseek(fp, modelSizeOffset, SEEK_SET);
	fwrite(&totalSize, sizeof(UINT32), 1, fp);
	fclose(fp);
}

BOOL PIExp::_exportModelData(INode *node, TimeValue t, bool recursion, FILE *modelFP, std::vector<INode *> *outMeshNodes, bool onlyModel, float aabb[6])
{
	if (recursion)
	{
		for (int c = 0; c < node->NumberOfChildren(); c++)
		{
			if(!_exportModelData(node->GetChildNode(c), t, true, modelFP, outMeshNodes, onlyModel, aabb))
			{
				return FALSE;
			}
		}
	}

	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID)
	{
		return TRUE; // Safety net. This shouldn't happen.
	}

	//如果不能够被HIDE_OBJECTS隐藏,说明不是geometry object,不用处理
	if (node->IsHidden(HIDE_OBJECTS) == 0 || node->IsHidden(HIDE_BONEOBJECTS))
	{
		return TRUE;
	}

	BOOL needDel;
	TriObject *tri = GetTriObjectFromNode(node, t, needDel);
	if (!tri)
	{
		return TRUE;
	}

	//导出材质
	if (!onlyModel)
	{
		ExportMaterial(node, outMeshNodes->size());
	}

	BOOL hasBumpTex = GetExportTangent();

	Mesh *mesh = &tri->GetMesh();

	if (outMeshNodes != NULL)
	{
		outMeshNodes->push_back(node);
	}

	mesh->buildNormals();
	//mesh->DeleteIsoVerts();若使用这个函数, 会在获取normal信息时的崩溃, 可能是在getRVertPtr这个函数里.
	mesh->DeleteIsoMapVerts();

	std::vector<PIVertexData> piVertexDatas;
	std::vector<int> piFaces;
	if(!_genPiVertexData(mesh, node, node->GetObjTMAfterWSM(t), piVertexDatas, piFaces))
	{
		return FALSE;
	}

	//写模型数据
	INT8 op = 4;
	int numIndices = mesh->getNumFaces() * 3;
	INT8 indexSize = 4;

	INT8 usage = 0;
	INT8 stage = 0;
	int numVertices = 0;
	UINT32 numVertDataOffset = 0;
	INT8 numVertData = 0;

	//写模型数据
	fwrite(&op, sizeof(INT8), 1, modelFP);

	int numFaces = mesh->getNumFaces();
	//索引数据
	numIndices = numFaces * 3;

	fwrite(&numIndices, sizeof(int), 1, modelFP);
	fwrite(&indexSize, sizeof(INT8), 1, modelFP);
	UINT32 *indicesData = new UINT32[numIndices];
	for (int i = 0; i < numFaces; i++)
	{
		indicesData[i * 3] = piFaces[i * 3];
		indicesData[i * 3 + 1] = piFaces[i * 3 + 1];
		indicesData[i * 3 + 2] = piFaces[i * 3 + 2];
	}
	fwrite(indicesData, sizeof(UINT32), numIndices, modelFP);
	delete []indicesData;

	numVertices = piVertexDatas.size();
	fwrite(&numVertices, sizeof(UINT32), 1, modelFP);
	numVertDataOffset = ftell(modelFP);
	numVertData = 0;
	fwrite(&numVertData, sizeof(INT8), 1, modelFP);

	int numMapChannels = piVertexDatas[0].uvcoords.size();
	bool hasBoneAss = _getSkin(node) != NULL;

	//顶点数据准备
	float *pPos = new float[numVertices * 3];
	float *pNormal = new float[numVertices * 3];
	float *pTangent = NULL;
	float *pColor = new float[numVertices * 4];
	std::vector<float *> texcoords;
	float *pBlendWeights = NULL;
	UINT16 *pBlendIndices = NULL;
	UINT16 minBlendIndex = 2000, maxBlendIndex = 0;
	if (hasBoneAss)
	{
		pBlendWeights = new float[numVertices * 4];
		pBlendIndices = new UINT16[numVertices * 4];
		memset(pBlendWeights, 0, sizeof(float) * numVertices * 4);
		memset(pBlendIndices, 0, sizeof(UINT16) * numVertices * 4);
	}

	if (hasBumpTex)
	{
		pTangent = new float[numVertices * 4];
	}

	for (int i = 0; i < numMapChannels; ++i)
	{
		float *pTexcoord = new float[numVertices * 2];
		texcoords.push_back(pTexcoord);
	}

	for (UINT32 i = 0; i < piVertexDatas.size(); ++i)
	{
		pPos[i * 3] = piVertexDatas[i].pos.x;
		pPos[i * 3 + 1] = piVertexDatas[i].pos.y;
		pPos[i * 3 + 2] = piVertexDatas[i].pos.z;

		if (aabb != NULL)
		{
			if (piVertexDatas[i].pos.x < aabb[0])
			{
				aabb[0] = piVertexDatas[i].pos.x;
			}
			if (piVertexDatas[i].pos.y < aabb[1])
			{
				aabb[1] = piVertexDatas[i].pos.y;
			}
			if (piVertexDatas[i].pos.z < aabb[2])
			{
				aabb[2] = piVertexDatas[i].pos.z;
			}
			if (piVertexDatas[i].pos.x > aabb[3])
			{
				aabb[3] = piVertexDatas[i].pos.x;
			}
			if (piVertexDatas[i].pos.y > aabb[4])
			{
				aabb[4] = piVertexDatas[i].pos.y;
			}
			if (piVertexDatas[i].pos.z > aabb[5])
			{
				aabb[5] = piVertexDatas[i].pos.z;
			}
		}

		pNormal[i * 3] = piVertexDatas[i].normal.x;
		pNormal[i * 3 + 1] = piVertexDatas[i].normal.y;
		pNormal[i * 3 + 2] = piVertexDatas[i].normal.z;

		if (pTangent != NULL)
		{
			pTangent[i * 4] = piVertexDatas[i].tangent.x;
			pTangent[i * 4 + 1] = piVertexDatas[i].tangent.y;
			pTangent[i * 4 + 2] = piVertexDatas[i].tangent.z;
			pTangent[i * 4 + 3] = piVertexDatas[i].tangent.w;
		}

		pColor[i * 4] = piVertexDatas[i].color.x;
		pColor[i * 4 + 1] = piVertexDatas[i].color.y;
		pColor[i * 4 + 2] = piVertexDatas[i].color.z;
		pColor[i * 4 + 3] = 1.0;

		for (int j = 0; j < numMapChannels; ++j)
		{
			texcoords[j][i * 2] = piVertexDatas[i].uvcoords[j].x;
			texcoords[j][i * 2 + 1] = 1.0f - piVertexDatas[i].uvcoords[j].y;
		}

		if (hasBoneAss)
		{
			UINT32 numBoneAss = piVertexDatas[i].bongAssigns.size();
			assert(numBoneAss <= 4);
			float weightSum = 0.0f;
			for (UINT32 k = 0; k < numBoneAss; ++k)
			{
				pBlendWeights[i * 4 + k] = piVertexDatas[i].bongAssigns[k].weight;
				weightSum += piVertexDatas[i].bongAssigns[k].weight;
				pBlendIndices[i * 4 + k] = piVertexDatas[i].bongAssigns[k].boneID;

				minBlendIndex = min(piVertexDatas[i].bongAssigns[k].boneID, minBlendIndex);
				maxBlendIndex = max(maxBlendIndex, piVertexDatas[i].bongAssigns[k].boneID);
			}
			if (abs(weightSum - 1.0f) > 0.01)
			{
				for (UINT32 k = 0; k < numBoneAss; ++k)
				{
					pBlendWeights[i * 4 + k] /= weightSum;
				}
			}
		}
	}

	//写顶点位置
	numVertData += 1;
	usage = 1;
	stage = 0;
	fwrite(&usage, sizeof(INT8), 1, modelFP);
	fwrite(&stage, sizeof(INT8), 1, modelFP);
	fwrite(pPos, sizeof(float), numVertices * 3, modelFP);
	delete []pPos;

	//写纹理坐标
	usage = 5;
	stage = 0;
	for (int i = 0; i < numMapChannels; ++i)
	{
		fwrite(&usage, sizeof(INT8), 1, modelFP);
		fwrite(&stage, sizeof(INT8), 1, modelFP);
		fwrite(texcoords[i], sizeof(float), numVertices * 2, modelFP);
		delete [] texcoords[i];
		stage += 1;
		numVertData += 1;
	}
	texcoords.clear();

	//写法线
	numVertData += 1;
	usage = 2;
	stage = 0;
	fwrite(&usage, sizeof(INT8), 1, modelFP);
	fwrite(&stage, sizeof(INT8), 1, modelFP);
	fwrite(pNormal, sizeof(float), numVertices * 3, modelFP);
	delete []pNormal;

	//写切线
	if (pTangent != NULL)
	{
		numVertData += 1;
		usage = 9;
		stage = 0;
		fwrite(&usage, sizeof(INT8), 1, modelFP);
		fwrite(&stage, sizeof(INT8), 1, modelFP);
		fwrite(pTangent, sizeof(float), numVertices * 4, modelFP);
		delete []pTangent;
	}

	//写颜色
	if (node->GetCVertMode())
	{
		numVertData += 1;
		usage = 3;
		stage = 1;
		fwrite(&usage, sizeof(INT8), 1, modelFP);
		fwrite(&stage, sizeof(INT8), 1, modelFP);
		fwrite(pColor, sizeof(float), numVertices * 4, modelFP);
		delete []pColor;
	}

	//写骨骼混合权重
	if (pBlendWeights != NULL)
	{
		numVertData += 1;
		usage = 7;
		stage = 0;
		fwrite(&usage, sizeof(INT8), 1, modelFP);
		fwrite(&stage, sizeof(INT8), 1, modelFP);
		fwrite(pBlendWeights, sizeof(float), numVertices * 4, modelFP);
		delete []pBlendWeights;
	}

	//写骨骼混合索引
	if (pBlendIndices != NULL)
	{
		numVertData += 1;
		usage = 8;
		stage = 0;
		fwrite(&usage, sizeof(INT8), 1, modelFP);
		fwrite(&stage, sizeof(INT8), 1, modelFP);
		fwrite(&minBlendIndex, sizeof(UINT16), 1, modelFP);
		minBlendIndex = maxBlendIndex - minBlendIndex + 1;
		fwrite(&minBlendIndex, sizeof(UINT16), 1, modelFP);
		fwrite(pBlendIndices, sizeof(UINT16), numVertices * 4, modelFP);
		delete []pBlendIndices;
	}

	//重定位到写之前, 写顶点数据的数量
	fseek(modelFP, numVertDataOffset, SEEK_SET);
	fwrite(&numVertData, sizeof(INT8), 1, modelFP);
	fseek(modelFP, 0, SEEK_END);

	if (needDel)
	{
		delete tri;
	}
	return TRUE;
}

bool PIExp::_hasVertexAnim(std::vector<INode *> &meshNodes)
{
	Interval animRange = m_ip->GetAnimRange();

	for (UINT32 i = 0; i < meshNodes.size(); ++i)
	{
		Control *c = meshNodes[i]->GetTMController();
		Control *pc = c->GetPositionController();
		Control *rc = c->GetRotationController();
		Control *sc = c->GetScaleController();
		Tab<TimeValue> keyTimes;
		pc->GetKeyTimes(keyTimes, animRange, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
		rc->GetKeyTimes(keyTimes, animRange, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
		sc->GetKeyTimes(keyTimes, animRange, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
		if (keyTimes.Count() > 0)
		{
			return true;
		}
	}

	return false;
}

void PIExp::_exportVertexAnimInfo(cJSON *rootJSON, std::vector<INode *> &meshNodes)
{
	//写json
	cJSON *vertexAnim = cJSON_CreateObject();
	cJSON_AddItemToObject(rootJSON, "vertexAnim", vertexAnim);

	cJSON *frames = cJSON_CreateArray();
	cJSON_AddItemToObject(vertexAnim, "frames", frames);

	Interval animRange = m_ip->GetAnimRange();

	int duration = -1;

	for (UINT32 i = 0; i < meshNodes.size(); ++i)
	{
		cJSON *subModelFrames = cJSON_CreateArray();
		cJSON_AddItemToArray(frames, subModelFrames);

		Control *c = meshNodes[i]->GetTMController();
		Control *pc = c->GetPositionController();
		Control *rc = c->GetRotationController();
		Control *sc = c->GetScaleController();
		Tab<TimeValue> keyTimes;
		pc->GetKeyTimes(keyTimes, animRange, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
		rc->GetKeyTimes(keyTimes, animRange, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
		sc->GetKeyTimes(keyTimes, animRange, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);

		if (m_nExportedAnimInterval > 0)
		{
			for (int i = animRange.Start(); i < animRange.End(); i += m_nExportedAnimInterval * 160)
			{
				keyTimes.Append(1, &i);
			}
		}

		keyTimes.Sort(_compare_func);

		int keyTime = -1;
		for (int j = 0; j < keyTimes.Count(); j++)
		{
			//忽略相同的
			if (keyTime == keyTimes[j])
			{
				continue;
			}

			keyTime = keyTimes[j];
			float keyTimef = (float)keyTimes[j] / (float)GetTicksPerFrame() / (float)GetFrameRate();
			//帧时间
			INT32 frameKeyTime = (INT32)(0.5f + keyTimef * 1000);

			//写json
			cJSON *frame = cJSON_CreateObject();
			cJSON_AddItemToArray(subModelFrames, frame);
			cJSON_AddNumberToObject(frame, "time", frameKeyTime);
			char nameSuffix[MAX_PATH];
			sprintf(nameSuffix, "_%d_%d", i, frameKeyTime);
			cJSON_AddStringToObject(frame, "meshFile", (m_strResPath + m_strName + ".obj/" + m_strName + nameSuffix + ".model").c_str());

			//生成模型文件
			_exportModelToFile(m_strName + ".obj/" + m_strName + nameSuffix + ".model", meshNodes[i], NULL, keyTime, false, true, NULL);
		}

		//更新最大持续时间
		float fduration = (float)keyTime / (float)GetTicksPerFrame() / (float)GetFrameRate();
		duration = max(duration, (INT32)(0.5f + fduration * 1000));
	}

	//写持续时间
	cJSON_AddNumberToObject(vertexAnim, "duration", duration);
}

BOOL PIExp::_genPiVertexData(Mesh *mesh, INode *node, const Matrix3 &tm, std::vector<PIVertexData> &outVertexDatas, std::vector<int> &outFaces)
{
	//使用一张查找表来排查重复的顶点
	std::map<int, std::vector<SearchVertexData>> searchTable;

	_getHierarchyRoots();

	BOOL hasError = FALSE;

	ISkin *skin = _getSkin(node);

	int numFaces = mesh->getNumFaces();
	//每个面
	for (int faceI = 0; faceI < numFaces; ++faceI)
	{
		//每个面的每个顶点
		int piFace[3];
		BOOL error = FALSE;
		PIExp::PITangentData tangent_data = _genFaceTangent(mesh, faceI, tm, &error);
		//Point4 faceTangent = tangent_data.tangent;

		if(error)
		{
			hasError = TRUE;
		}

		for (int localVertexI = 0; localVertexI < 3; ++localVertexI)
		{
			SearchVertexData svd;

			svd.tangent_data = tangent_data;

			//相当于tDestColorIndex1 
			int vertexIndex = mesh->faces[faceI].v[localVertexI];

			Face *f = &mesh->faces[faceI];

			DWORD vert = f->getVert(localVertexI);

			Point3 pos = tm * mesh->verts[vert];

			Point3 color(1.0f, 1.0f, 1.0f);


			if (mesh->numCVerts != 0)
			{
				TVFace tface = mesh->vcFace[faceI];

				//tSrcColorIndex1 
				int tSrcColor = tface.getTVert(localVertexI);
				color = mesh->vertCol[tSrcColor];
			}


			


			svd.color = color;

			Point3 normal = GetVertexNormal(mesh, faceI, mesh->getRVertPtr(vert));
			svd.normal = normal;

			//每个纹理通道, 从1开始, 0是color vertex
			for (int ch = 1; ch < MAX_MESHMAPS - 1; ++ch)
			{
				if (mesh->mapSupport(ch))
				{
					DWORD idx = mesh->mapFaces(ch)[faceI].t[localVertexI];
					UVVert tv = mesh->mapVerts(ch)[idx];
					svd.uvcoords.push_back(Point2(tv.x, tv.y));
				}
			}

			//去查找表里面查找重复顶点
			bool isNewVertex = false;
			int realVertexNum = 0;
			int res = _doesVertexExist(searchTable, vertexIndex, svd.normal, svd.color, svd.uvcoords, svd.tangent_data);

			if (res == 0)
			{
				isNewVertex = true;
			}
			else
			{
				realVertexNum = res;
				isNewVertex = false;

				//融合tangent
				Point4 tangent = outVertexDatas[realVertexNum - 1].tangent;
				Point3 normalized_tangent(tangent.x + tangent_data.tangent.x, tangent.y + tangent_data.tangent.y, tangent.z + tangent_data.tangent.z);
				normalized_tangent.Unify();
				outVertexDatas[realVertexNum - 1].tangent.x = normalized_tangent.x;
				outVertexDatas[realVertexNum - 1].tangent.y = normalized_tangent.y;
				outVertexDatas[realVertexNum - 1].tangent.z = normalized_tangent.z;
			}

			if (isNewVertex)
			{
				realVertexNum = outVertexDatas.size() + 1;

				//添加进查找表
				svd.realVertexIndex = realVertexNum;
				searchTable[vertexIndex].push_back(svd);

				PIVertexData pvd;
				pvd.pos.x = pos.x;
				pvd.pos.y = pos.z;
				pvd.pos.z = -pos.y;

				pvd.color.x = color.x;
				pvd.color.y = color.y;
				pvd.color.z = color.z;

				pvd.normal.x = normal.x;
				pvd.normal.y = normal.z;
				pvd.normal.z = -normal.y;

				pvd.tangent = tangent_data.tangent;

				pvd.uvcoords = svd.uvcoords;

				if (skin != NULL)
				{
					ISkinContextData *skinData = skin->GetContextInterface(node);

					// loop through all the vertices, writing out skinning data as we go
					int skinnedVertexCount = skinData->GetNumPoints();

					if (skinnedVertexCount > 0)
					{
						// grab the bone indices for this vertex
						int vertexBoneInfluences = skinData->GetNumAssignedBones(vertexIndex);
						if(vertexBoneInfluences >4)
						{
							char buffer[100];
							sprintf_s(buffer, "第%d个顶点绑定的骨骼数量不正确", vertexIndex);
							
							MessageBoxA(NULL, buffer, "提示", MB_OK);
							return FALSE;
						}

						if (vertexBoneInfluences > 0)
						{
							for (int j = 0; j < vertexBoneInfluences; j++)
							{
								INode *boneNode = skin->GetBone(skinData->GetAssignedBone(vertexIndex, j));
								BoneAssign ba;
								ba.boneID = 0;
								ba.weight = 0.0f;
								//boneNode 可能为NULL
								if (boneNode != NULL)
								{
									const MCHAR *boneName = boneNode->GetName();
									ba.boneID = _getBoneIndex(boneName);
									ba.weight = skinData->GetBoneWeight(vertexIndex, j);
								}
								else
								{
									//m_pStream.Printf(_T("\t bone name is NULL, assigned bone index is %d\n"), skinData->GetAssignedBone(vertexIndex, j));
								}

								pvd.bongAssigns.push_back(ba);
							}
						}
					}
				}

				outVertexDatas.push_back(pvd);
			}
			piFace[localVertexI] = realVertexNum - 1;
		}
		outFaces.push_back(piFace[0]);
		outFaces.push_back(piFace[1]);
		outFaces.push_back(piFace[2]);
	}
	if(hasError)
	{
		char buffer[100];
		sprintf_s(buffer, "模型中存在未展开的UV，可能导致法线贴图表现不正确");
		MessageBoxA(NULL, buffer, "重要提示", MB_OK);
	}

	return TRUE;
}

PIExp::PITangentData PIExp::_genFaceTangent(Mesh *mesh, int faceIdx, const Matrix3 &tm, BOOL* error)
{
	PITangentData t;
	Face *f = &mesh->faces[faceIdx];
	t.smGroup = f->smGroup;

	int vertexIndex = mesh->faces[faceIdx].v[0];
	DWORD vert = f->getVert(0);
	Point3 pos0 = tm * mesh->verts[vert];

	float temp = pos0.z;
	pos0.z = -pos0.y;
	pos0.y = temp;

	vertexIndex = mesh->faces[faceIdx].v[1];
	vert = f->getVert(1);
	Point3 pos1 = tm * mesh->verts[vert];

	temp = pos1.z;
	pos1.z = -pos1.y;
	pos1.y = temp;

	vertexIndex = mesh->faces[faceIdx].v[2];
	vert = f->getVert(2);
	Point3 pos2 = tm * mesh->verts[vert];

	temp = pos2.z;
	pos2.z = -pos2.y;
	pos2.y = temp;

	float u0, v0, u1, v1, u2, v2;
	//每个纹理通道, 从1开始, 0是color vertex
	for (int ch = 1; ch < MAX_MESHMAPS - 1; ++ch)
	{
		if (mesh->mapSupport(ch))
		{
			DWORD idx = mesh->mapFaces(ch)[faceIdx].t[0];
			UVVert tv = mesh->mapVerts(ch)[idx];
			u0 = tv.x;
			v0 = tv.y;

			idx = mesh->mapFaces(ch)[faceIdx].t[1];
			tv = mesh->mapVerts(ch)[idx];
			u1 = tv.x;
			v1 = tv.y;

			idx = mesh->mapFaces(ch)[faceIdx].t[2];
			tv = mesh->mapVerts(ch)[idx];
			u2 = tv.x;
			v2 = tv.y;
			break;
		}
	}

	//拿到了pos0 pos1 pos2 和 uv0 uv1 uv2, 下面开始计算切线
	Point3 edgeA = pos1 - pos0;
	Point3 edgeB = pos2 - pos0;
	Point3 normal = Normalize(edgeA ^ edgeB);

	float deltaV1 = v1 - v0;
	float deltaV2 = v2 - v0;
	float deltaU1 = u1 - u0;
	float deltaU2 = u2 - u0;
	float div = (deltaU1 * deltaV2 - deltaU2 * deltaV1);

	float a = deltaV2 / div;
	float b = -deltaV1 / div;
	float c = -deltaU2 / div;
	float d = deltaU1 / div;

	if(div == 0)
	{
		*error = TRUE;
	}


	Point3 tangent = Normalize(edgeA * a + edgeB * b);
	Point3 binormal = Normalize(edgeA * c + edgeB * d);

	// binormal是向下的， 因为我们是OpenGL是右手坐标系，binormal应该向上
	bool parity = DotProd(tangent ^ -binormal, normal) < 0.0f;

	Point4 face_tangent(tangent.x, tangent.y, tangent.z, parity ? -1.0f : 1.0f);

	//return face_tangent;
	t.tangent = face_tangent;
	t.binormal = binormal;
	return t;
}

void PIExp::_exportKeyframes(FILE *fp, INode *thisNode, Tab<TimeValue> &keyTimes, Interval &interval)
{
	int i;
	int keyTime = -1;
	int start = interval.Start();
	int end = interval.End();

	Control *c = thisNode->GetTMController();
	Control *pc = thisNode->GetParentNode()->GetTMController();
	bool isRoot = _isRootNode(thisNode);

	const TCHAR *tc = thisNode->GetName();

	Matrix3 initTM = thisNode->GetNodeTM(0);
	Matrix3 ptm;
	if (isRoot)
	{
		initTM = initTM * Inverse(RotateXMatrix(PI / 2.0f));
	}
	else
	{
		ptm = thisNode->GetParentTM(0);
		initTM = initTM * Inverse(ptm);
	}

	//写帧数
	INT32 numFramesOffset = ftell(fp);
	INT16 numFrames = 0;
	fwrite(&numFrames, sizeof(INT16), 1, fp);

	for (i = 0; i < keyTimes.Count(); i++)
	{
		// only operate within the supplied keyframe time range
		if (keyTimes[i] < start)
		{
			continue;
		}
		if (keyTimes[i] > end)
		{
			break;
		}

		// ignore key times we've already processed
		if (keyTimes[i] != keyTime)
		{
			keyTime = keyTimes[i];
			float keyTimef = (float)(keyTimes[i] - start) / (float)GetTicksPerFrame() / (float)GetFrameRate();
			//帧时间
			INT32 frameKeyTime = (INT32)(0.5f + keyTimef * 1000);
			fwrite(&frameKeyTime, sizeof(INT32), 1, fp);

			/*-- First, rotation which depends on initial transformation
			Tform = d.transform ;
			-- if this is the root bone
			if (isRoot d) then (
			mparent = matrix3 1 ;
			-- if flipYZ == true
			if (flipYZ) then
			Tform = flipYZTransform Tform ;
			)
			else
			mparent = d.parent.transform ;

			-- computes rotation
			mref = initTform*mparent ;
			Tform = Tform*inverse(mref) ;

			-- rotation part is saved.
			rot = Tform.rotation as angleaxis ;
			angle = - degToRad (rot.angle) ; -- don't know why there must be this minus :((((((*/

			//获取父tm
			Matrix3 tm = thisNode->GetNodeTM(keyTime);

			if (isRoot)
			{
				ptm.IdentityMatrix();
				tm = tm * Inverse(RotateXMatrix(PI / 2.0f));
			}
			else
			{
				ptm = thisNode->GetParentTM(keyTime);
			}

			//获取旋转分量
			Matrix3 mref = initTM * ptm;
			tm = tm * Inverse(mref);
			AngAxis aa(tm);

			//获取平移分量
			tm = thisNode->GetNodeTM(keyTime);
			tm = tm * Inverse(ptm);
			if (isRoot)
			{
				tm = tm * Inverse(RotateXMatrix(PI / 2.0f));
			}

			Point3 trans = tm.GetTrans();
			trans -= initTM.GetTrans();

			Matrix3 sclMat(1);
			AffineParts parts;
			decomp_affine(tm, &parts);
			ApplyScaling(sclMat, ScaleValue(parts.k * parts.f, parts.u));
			Point3 scale(sclMat.GetRow(1)[1], sclMat.GetRow(0)[0], sclMat.GetRow(2)[2]);

			//写trans, rotate, scale
			fwrite(&trans.x, sizeof(float), 1, fp);
			fwrite(&trans.y, sizeof(float), 1, fp);
			fwrite(&trans.z, sizeof(float), 1, fp);

			float angle = -aa.angle;
			fwrite(&angle, sizeof(float), 1, fp);
			fwrite(&aa.axis.x, sizeof(float), 1, fp);
			fwrite(&aa.axis.y, sizeof(float), 1, fp);
			fwrite(&aa.axis.z, sizeof(float), 1, fp);

			

			INT8 hasScale = GetExportSkeletonScale() && _isScale(&scale);
			fwrite(&hasScale, sizeof(INT8), 1, fp);
			if (hasScale == 1)
			{
				fwrite(&scale.x, sizeof(float), 1, fp);
				fwrite(&scale.y, sizeof(float), 1, fp);
				fwrite(&scale.z, sizeof(float), 1, fp);
			}

			numFrames += 1;
		}
	}

	fseek(fp, numFramesOffset, SEEK_SET);
	fwrite(&numFrames, sizeof(INT16), 1, fp);
	fseek(fp, 0, SEEK_END);
}

INT8 PIExp::_isScale(Point3 *scale)
{
	INT8 result = 0;
	Point3 identity(1.0, 1.0, 1.0);
	if (abs(scale->x - identity.x) > SCALE_PRECISION || abs(scale->y - identity.y) > SCALE_PRECISION || abs(scale->z - identity.z) > SCALE_PRECISION)
	{
		result = 1;
	}
	return result;
}

AngAxis PIExp::_toAngleAxis(const Quat &q)
{
	float angle;
	Point3 axis;
	AngAxis result;
	float sqrtLength = q.x * q.x + q.y * q.y + q.z * q.z;
	if (sqrtLength > 0.0)
	{
		angle = (acos(q.w) * PI / 90);
		float fInvLength = 1.0f / sqrt(sqrtLength);
		axis.x = q.x * fInvLength;
		axis.y = q.y * fInvLength;
		axis.z = q.z * fInvLength;
		result = AngAxis(axis, angle);
	}
	else
	{
		angle = 0.0f;
		axis = Point3(1.0f, 0.0f, 0.0f);
		result = AngAxis(axis, angle);
	}

	return result;
}

Matrix3 PIExp::_flipYZTransform(Matrix3 tm)
{
	Point3 axis1(1.0f, 0.0f, 0.0f);
	Point3 axis2(0.0f, 0.0f, 1.0f);
	Point3 axis3(0.0f, -1.0f, 0.0f);
	Point3 t(0.0f, 0.0f, 0.0f);

	Matrix3 m(axis1, axis2, axis3, t);

	return tm * Inverse(m);
}

bool PIExp::_isPartOfModifier(INode *node)
{
	if (node == NULL)
	{
		return false;
	}

	ISkin *skin = _getSkin(node);

	if (skin == NULL)
	{
		return false;
	}

	for (int i = 0; i < skin->GetNumBones(); ++i)
	{
		INode *boneNode = skin->GetBone(i);
		if (wcscmp(node->GetName(), boneNode->GetName()) == 0)
		{
			return true;
		}
	}

	return false;
}

void PIExp::_getHierarchyRoots()
{
	std::map< std::wstring, int >::iterator iter = m_boneIndexMap.begin();
	for (; iter != m_boneIndexMap.end(); ++iter)
	{
		INode *boneNode = m_ip->GetINodeByName(iter->first.c_str());

		while (boneNode->GetParentNode() != NULL &&
			(_isBoneObj(boneNode->GetParentNode()) || _isBipedObj(boneNode->GetParentNode()) || _isPartOfModifier(boneNode->GetParentNode())))
		{
			boneNode = boneNode->GetParentNode();
		}

		int trouve = 0;

		for (UINT32 j = 0; j < m_rootBoneNames.size(); ++j)
		{
			if (wcscmp(m_rootBoneNames[j].c_str(), boneNode->GetName()) == 0)
			{
				trouve = 1;
				break;
			}
		}

		if (trouve == 0)
		{
			if (_isBoneObj(boneNode) || _isBipedObj(boneNode) || _isPartOfModifier(boneNode))
			{
				m_rootBoneNames.push_back(boneNode->GetName());
			}
		}
	}
}

void PIExp::_getSubMeshs(INode *node, int keyTime, std::vector<INode *> *outMeshNodes)
{
	for (int c = 0; c < node->NumberOfChildren(); c++)
	{
		_getSubMeshs(node->GetChildNode(c), keyTime, outMeshNodes);
	}

	ObjectState os = node->EvalWorldState(keyTime);
	if (!os.obj || os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID)
	{
		return; // Safety net. This shouldn't happen.
	}

	//如果不能够被HIDE_OBJECTS隐藏,说明不是geometry object,不用处理
	if (node->IsHidden(HIDE_OBJECTS) == 0)
	{
		return;
	}

	if (outMeshNodes != NULL)
	{
		outMeshNodes->push_back(node);
	}
}

bool PIExp::_isRootNode(INode *node)
{
	if (node == NULL)
	{
		return false;
	}

	//return node->GetParentNode() == NULL;
	for (UINT i = 0; i < m_rootBoneNames.size(); ++i)
	{
		if (wcscmp(node->GetName(), m_rootBoneNames[i].c_str()) == 0)
		{
			return true;
		}
	}

	return false;
}

ISkin *PIExp::_getSkin(INode *node)
{
	if (node == NULL)
	{
		return NULL;
	}

	Object *oRef = node->GetObjectRef();

	if (oRef == NULL)
	{
		return NULL;
	}

	if (oRef->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject *dObj = (IDerivedObject *)oRef;
		Modifier *oMod = dObj->GetModifier(0);

		if (oMod == NULL)
		{
			return NULL;
		}

		if (oMod->ClassID() == SKIN_CLASSID)
		{
			ISkin *skin = (ISkin *)oMod->GetInterface(I_SKIN);
			return skin;
		}
	}
	return NULL;
}

bool PIExp::_isPelvis(INode *node)
{
	return wcscmp(node->GetName(), _T("Bip01")) == 0;
}

bool PIExp::_isBoneObj(INode *node)
{
	if (node == NULL)
	{
		return false;
	}

	Object *obj = node->EvalWorldState(0).obj;
	if (obj == NULL)
	{
		return false;
	}

	Class_ID cid = obj->ClassID();
	ULONG cidA = cid.PartA();

	return cidA == BONE_OBJ_CLASSID.PartA();
}

bool PIExp::_isBipedObj(INode *node)
{
	if (node == NULL)
	{
		return false;
	}

	Control *c = node->GetTMController();
	if (c == NULL)
	{
		return false;
	}

	Class_ID ccid = c->ClassID();
	ULONG ccidA = ccid.PartA();

	return ccidA == BIPSLAVE_CONTROL_CLASS_ID.PartA() ||
		ccidA == BIPBODY_CONTROL_CLASS_ID.PartA();
}

bool PIExp::_isHelperObj(INode *node)
{
	if (node == NULL)
	{
		return false;
	}

	return !!node->IsHidden(HIDE_HELPERS);
}
int PIExp::_getBoneIndex(const MCHAR *boneName)
{
	std::map< std::wstring, int >::const_iterator it = m_boneIndexMap.find(std::wstring(boneName));
	if (it == m_boneIndexMap.end())
	{
		m_boneIndexMap.insert(std::map< std::wstring, int >::value_type(std::wstring(boneName), m_nCurrentBoneIndex));
		return m_nCurrentBoneIndex++;
	}
	else
	{
		return it->second;
	}
}
void PIExp::_markMeshBones(INode* node, TimeValue t)
{
	for (int c = 0; c < node->NumberOfChildren(); c++){
		_markMeshBones(node->GetChildNode(c), t);
	}
	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID)
	{
		return; // Safety net. This shouldn't happen.
	}

	//如果不能够被HIDE_OBJECTS隐藏,说明不是geometry object,不用处理
	if (node->IsHidden(HIDE_OBJECTS) == 0 || node->IsHidden(HIDE_BONEOBJECTS))
	{
		return;
	}

	BOOL needDel;
	TriObject *tri = GetTriObjectFromNode(node, t, needDel);
	if (!tri)
	{
		return;
	}
	ISkin* skin = _getSkin(node);
	if (skin == NULL)
	{
		return;
	}
	std::vector<std::wstring> group;
	for (int i = 0; i < skin->GetNumBones(); i++)
	{
		group.push_back(skin->GetBone(i)->GetName());
	}

	m_boneGroups.push_back(std::make_pair(node->GetName(), group));
}

void PIExp::_markBone(INode *node, TimeValue t)
{
	if (m_skeleton_sort_type)
	{
		_markMeshBones(node, t);
		for (int i = m_boneGroups.size() - 1; i >= 0; i--)
		{
			for (int j = 0; j < m_boneGroups.size(); j++)
			{
				std::vector<std::wstring> &group1 = m_boneGroups[i].second;
				std::vector<std::wstring> &group2 = m_boneGroups[j].second;
				if (&group2 != &group1)
				{
					if (group1.size() <= group2.size())
					{
						int m;
						for (m = 0; m < group1.size(); m++)
						{
							wstring& s1 = group1[m];
							int n;
							for (n = 0; n < group2.size(); n++)
							{
								wstring &s2 = group2[n];
								if (s1.compare(s2.c_str()) == 0)
								{
									break;
								}
							}
							if (n == group2.size())
							{
								break;
							}
						}
						if (m == group1.size())
						{
							auto it = m_boneGroups.begin();
							std::advance(it, i);
							m_boneGroups.erase(it);
							break;
						}
					}
				}
			}
		}

		for (auto group : m_boneGroups)
		{
			int minIndex = 100000, maxIndex = 0;
			const wchar_t * minName, *maxName;
			for (auto name : group.second)
			{
				int index = _getBoneIndex(name.c_str());
				minIndex = min(minIndex, index);
				maxIndex = max(maxIndex, index);
			}
			if (maxIndex - minIndex + 1 > 60)
			{
				char buffer[255];
				sprintf_s(buffer, "模型:%s,与其他模型共用骨骼,骨骼跨度大于60", group.first.c_str());
				MessageBoxA(NULL, buffer, "提示", MB_OK);
			}
		}
	}
	_markBone(node);
}

void PIExp::_markBone(INode *node)
{
	for (int c = 0; c < node->NumberOfChildren(); c++)
	{
		_markBone(node->GetChildNode(c));
	}

	Object *obj = node->EvalWorldState(0).obj;
	if (obj == NULL)
	{
		return;
	}

	Class_ID cid = obj->ClassID();

	ULONG cidA = cid.PartA();
	if (cidA == BONE_CLASS_ID || cidA == DUMMY_CLASS_ID ||
		cidA == SKELOBJ_CLASS_ID.PartA() || cidA == BONE_OBJ_CLASSID.PartA())
	{
		Control *c = node->GetTMController();
		if (c != NULL)
		{
			Class_ID ccid = c->ClassID();

			ULONG ccidA = ccid.PartA();
			if (ccidA & PRS_CONTROL_CLASS_ID ||
				ccidA & BIPSLAVE_CONTROL_CLASS_ID.PartA() ||
				ccidA & BIPBODY_CONTROL_CLASS_ID.PartA() ||
				ccidA & FOOTPRINT_CLASS_ID.PartA())
			{

				if (node->GetName() != NULL && wcscmp(node->GetName(), _T("Bip01 Footsteps")) != 0) //Bip01 Footsteps这个骨骼没有用, 不导
				{
					if (_isHelperObj(node))
					{
						m_helperNames.push_back(node->GetName());
					}
					else
					{
						_getBoneIndex(node->GetName());
					}
				}
			}
		}
	}
}

int PIExp::_doesVertexExist(std::map<int, std::vector<SearchVertexData>> &searchTable, int vertexIndex, Point3 &normal, Point3 &color, std::vector<Point2> &uvcoords, PIExp::PITangentData &tangent_data)
{
	std::map<int, std::vector<SearchVertexData>>::iterator iter = searchTable.find(vertexIndex);
	if (iter == searchTable.end())
	{
		return 0;
	}

	for (UINT32 i = 0; i < iter->second.size(); ++i)
	{
		SearchVertexData &svd = iter->second[i];

		if (!svd.normal.Equals(normal))
		{
			continue;
		}

		if (!svd.color.Equals(color))
		{
			continue;
		}

		bool isEqual = true;
		for (UINT32 j = 0; j < svd.uvcoords.size(); ++j)
		{
			if (!svd.uvcoords[j].Equals(uvcoords[j]))
			{
				isEqual = false;
				break;
			}
		}

		if (!isEqual)
		{
			continue;
		}

		if (svd.tangent_data.tangent.w != tangent_data.tangent.w)
		{
			continue;
		}

		if(svd.tangent_data.smGroup != tangent_data.smGroup)
		{
			continue;
		}

		Point3 vrefUV(tangent_data.tangent.x + tangent_data.binormal.x, tangent_data.tangent.y + tangent_data.binormal.y, tangent_data.tangent.z + tangent_data.binormal.z);
		Point3 vRotHalf = vrefUV - svd.normal * DotProd(svd.normal, vrefUV);
		Point3 fvre(svd.tangent_data.tangent.x + svd.tangent_data.binormal.x, svd.tangent_data.tangent.y + svd.tangent_data.binormal.y, svd.tangent_data.tangent.z + svd.tangent_data.binormal.z);
		if(DotProd(fvre, vRotHalf) <= 0.0f)
		{
			continue;
		}
		return svd.realVertexIndex;
	}

	return 0;
}

bool PIExp::_hasBumpTex(INode *node)
{
	Mtl *mtl = node->GetMtl();

	if (mtl)
	{
		int mtlID = m_mtlList.GetMtlID(mtl);
		if (mtlID >= 0)
		{
			for (int i = 0; i < mtl->NumSubTexmaps(); i++)
			{
				Texmap *subTex = mtl->GetSubTexmap(i);
				if (subTex)
				{
					if (i == ID_BU)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}