//**************************************************************************
//* Export.cpp	- PI File Exporter
//***************************************************************************

#include "piexp.h"
#include "util.h"
#include "modstack.h"

#include "cs\bipexp.h"
#include "iiksys.h"

#include <list>
#include <assert.h>

/****************************************************************************

Global output [Scene info]

****************************************************************************/

void PIExp::ExportMainSkeleton()
{
	if (m_boneIndexMap.empty())
	{
		return;
	}

	//创建文件
	std::string strSkeletonName = m_strName + ".obj/" + m_strName + ".sk";
	m_pSkeletonStream = fopen(strSkeletonName.c_str(), "wb");

	//写文件头
	fwrite(SKELETOLN_VERSION, sizeof(char), sizeof(SKELETOLN_VERSION) - 1, m_pSkeletonStream);
	m_nSkeletonSizeOffset = ftell(m_pSkeletonStream);
	//写size,暂时是0
	UINT32 size = 0;
	fwrite(&size, sizeof(UINT32), 1, m_pSkeletonStream);

	//根骨骼,没有用,写0
	INT16 rootID = 0;
	fwrite(&rootID, sizeof(INT16), 1, m_pSkeletonStream);

	TimeValue start = m_ip->GetAnimRange().Start();

	//骨骼数量
	INT16 numBones = m_boneIndexMap.size();
	fwrite(&numBones, sizeof(INT16), 1, m_pSkeletonStream);

	std::map<std::wstring, int>::iterator iter = m_boneIndexMap.begin();
	for (; iter != m_boneIndexMap.end(); ++iter)
	{
		INode *node = m_ip->GetINodeByName(iter->first.c_str());

		//写骨骼id和名字
		INT16 boneID = iter->second;
		fwrite(&boneID, sizeof(INT16), 1, m_pSkeletonStream);

		setlocale(LC_ALL, "");
		char mb[BONE_NAME_MAX_LEN];
		std::size_t l;
		wcstombs_s(&l, mb, BONE_NAME_MAX_LEN, iter->first.c_str(), BONE_NAME_MAX_LEN);
		l = strlen(mb) + 1;
		fwrite(&l, sizeof(INT32), 1, m_pSkeletonStream);
		fwrite(mb, sizeof(char), l, m_pSkeletonStream);

		//写父骨骼ID,若没有则写自己的ID
		INT16 parentBoneID = boneID;
		if (!_isRootNode(node) && node->GetParentNode() != NULL && node->GetParentNode() != m_ip->GetRootNode())
		{
			parentBoneID = _getBoneIndex(node->GetParentNode()->GetName());
			//m_pStream.Printf(_T("bone name is %s, parent bone is %s\n"), iter->first.c_str(), node->GetParentNode()->GetName());
		}
		else
		{
			//m_pStream.Printf(_T(" root bone name is %s, parent bone is %s\n"), iter->first.c_str(), node->GetName());
		}
		fwrite(&parentBoneID, sizeof(INT16), 1, m_pSkeletonStream);

		ObjectState os = node->EvalWorldState(start);
		Object *obj = os.obj;
		SClass_ID scid = obj->SuperClassID();

		Matrix3 tm(node->GetNodeTM(start));
		Matrix3 ptm(node->GetParentTM(start));
		Control *tmc = node->GetTMController();

		Class_ID cid = tmc->ClassID();

		bool isRoot = _isRootNode(node);

		//算上父矩阵
		if (!isRoot)
		{
			tm = tm * Inverse(ptm);
		}

		//翻转yz
		if (isRoot)
		{
			tm = tm * Inverse(RotateXMatrix(PI / 2.0f));
		}

		Point3 pos = tm.GetTrans();

		AngAxis aa(tm);

		//写pos, rotate, scale
		fwrite(&pos.x, sizeof(float), 1, m_pSkeletonStream);
		fwrite(&pos.y, sizeof(float), 1, m_pSkeletonStream);
		fwrite(&pos.z, sizeof(float), 1, m_pSkeletonStream);

		float angle = -aa.angle;
		fwrite(&angle, sizeof(float), 1, m_pSkeletonStream);
		fwrite(&aa.axis.x, sizeof(float), 1, m_pSkeletonStream);
		fwrite(&aa.axis.y, sizeof(float), 1, m_pSkeletonStream);
		fwrite(&aa.axis.z, sizeof(float), 1, m_pSkeletonStream);

		INT8 hasScale = 0;
		fwrite(&hasScale, sizeof(INT8), 1, m_pSkeletonStream);
	}

	//写动画
	INT16 numAnim = 0;
	fwrite(&numAnim, sizeof(INT16), 1, m_pSkeletonStream);

	//写size
	UINT32 totalSize = ftell(m_pSkeletonStream);
	fseek(m_pSkeletonStream, m_nSkeletonSizeOffset, SEEK_SET);
	fwrite(&totalSize, sizeof(UINT32), 1, m_pSkeletonStream);

	fclose(m_pSkeletonStream);

	//写json文件
	cJSON *pSKJson = cJSON_CreateObject();
	std::string strMainSKRelPath = m_strResPath + strSkeletonName;
	cJSON_AddStringToObject(pSKJson, "skPath", strMainSKRelPath.c_str());
	cJSON *pAnims = _buildSKJson(m_strAbsPath);
	cJSON_AddItemToObject(pSKJson, "anims", pAnims);
	cJSON_AddStringToObject(pSKJson, "elementType", SKELETON_ELEMENT);

	cJSON_AddItemToObject(m_pJSONRoot, SKELETON_ELEMENT, pSKJson);

	//写helpers
	cJSON *pHelpers = cJSON_CreateObject();
	for (UINT32 i = 0; i < m_helperNames.size(); ++i)
	{
		INode *node = m_ip->GetINodeByName(m_helperNames[i].c_str());
		Matrix3 tm(node->GetNodeTM(start));
		Matrix3 ptm(node->GetParentTM(start));

		//父骨骼不是跟的话, 要跟着做一个rotate
		if (node->GetParentNode() != m_ip->GetRootNode())
		{
			tm = tm * Inverse(ptm);
			tm = RotateXMatrix(PI / 2.0f) * tm;
		}

		Point3 pos = tm.GetTrans();
		AngAxis aa(tm);

		//如果父骨骼是root, 那要翻转一下YZ
		if (node->GetParentNode() == m_ip->GetRootNode())
		{
			float y = pos.y;
			pos.y = pos.z;
			pos.z = -y;
		}

		cJSON *pHelper = cJSON_CreateObject();

		char mb[MAX_FILE_PATH];
		std::size_t l;
		wcstombs_s(&l, mb, MAX_FILE_PATH, node->GetParentNode()->GetName(), MAX_FILE_PATH);
		cJSON_AddStringToObject(pHelper, "parentBoneName", mb);

		cJSON *pHelperTran = cJSON_CreateObject();
		cJSON_AddNumberToObject(pHelperTran, "x", pos.x);
		cJSON_AddNumberToObject(pHelperTran, "y", pos.y);
		cJSON_AddNumberToObject(pHelperTran, "z", pos.z);
		cJSON_AddItemToObject(pHelper, "pos", pHelperTran);

		cJSON *pHelperRotate = cJSON_CreateObject();
		cJSON_AddNumberToObject(pHelperRotate, "angle", -aa.angle);
		cJSON_AddNumberToObject(pHelperRotate, "x", aa.axis.x);
		cJSON_AddNumberToObject(pHelperRotate, "y", aa.axis.y);
		cJSON_AddNumberToObject(pHelperRotate, "z", aa.axis.z);
		cJSON_AddItemToObject(pHelper, "rotate", pHelperRotate);

		wcstombs_s(&l, mb, MAX_FILE_PATH, m_helperNames[i].c_str(), MAX_FILE_PATH);
		cJSON_AddItemToObject(pHelpers, mb, pHelper);
	}

	cJSON_AddItemToObject(pSKJson, "helpers", pHelpers);
}

void PIExp::ExportAnimation(const char *name, int startFrame, int endFrame)
{
	int startTime = startFrame * GetTicksPerFrame();
	int endTime = endFrame * GetTicksPerFrame();
	//先记录骨骼
	_markBone(m_ip->GetRootNode(), startTime);

	_getHierarchyRoots();

	//复制基础骨骼文件的内容
	std::string strSkeletonName = m_strName + ".obj/" + m_strName + ".sk";
	FILE *pMainSkFP = fopen(strSkeletonName.c_str(), "rb");

	if (pMainSkFP == NULL)
	{
		MessageBox(m_ip->GetMAXHWnd(), _T("动作输出目录应该 含有 已生成的 *.obj/ 目录"), _T("错误"), MB_OK);
		return;
	}

	fseek(pMainSkFP, 0, SEEK_END);
	int lSize = ftell(pMainSkFP);
	rewind(pMainSkFP);
	char *buffer = (char *)malloc(sizeof(char) * lSize);
	fread(buffer, 1, lSize, pMainSkFP);
	fclose(pMainSkFP);

	strSkeletonName = m_strName + ".obj/" + name + ".sk";
	FILE *pAnimStream = fopen(strSkeletonName.c_str(), "wb");
	fwrite(buffer, sizeof(char), lSize, pAnimStream);
	free(buffer);

	int numAnimOffset = ftell(pAnimStream);
	numAnimOffset -= sizeof(INT16);
	fseek(pAnimStream, numAnimOffset, SEEK_SET);

	//写动画数量
	INT16 numAnim = 1;
	fwrite(&numAnim, sizeof(INT16), 1, pAnimStream);

	INT32 nameLen = strlen(name) + 1;
	fwrite(&nameLen, sizeof(INT32), 1, pAnimStream);
	fwrite(name, sizeof(char), nameLen, pAnimStream);



	INT32 playTime = endTime - startTime;
	float fPlayTime = playTime / (float)GetTicksPerFrame() / GetFrameRate();
	playTime = (INT32)(0.5f + fPlayTime * 1000.0f);

	//写动作时间
	fwrite(&playTime, sizeof(INT32), 1, pAnimStream);

	INT16 numTracks = m_boneIndexMap.size();
	fwrite(&numTracks, sizeof(INT16), 1, pAnimStream);

	Matrix3 initTM, bipedMasterTM0;
	IBipMaster *bip = 0;
	bipedMasterTM0.IdentityMatrix();
	std::map< std::wstring, int >::iterator iter = m_boneIndexMap.begin();
	for (; iter != m_boneIndexMap.end(); ++iter)
	{
		INode *thisNode = m_ip->GetINodeByName(iter->first.c_str());

		//写骨骼ID
		INT32 boneID = _getBoneIndex(thisNode->GetName());
		fwrite(&boneID, sizeof(INT16), 1, pAnimStream);

		Control *c = thisNode->GetTMController();
		Class_ID cid = c->ClassID();

		Tab<TimeValue> keyTimes;
		Interval interval(startTime, endTime);

		// must have at least a frame at the start...
		keyTimes.Append(1, &startTime);

		const TCHAR *tch = thisNode->GetName();

		// three-part controller for Biped root -- taking this cue from the old MaxScript exporter code
		if (_isRootNode(thisNode) && _isPelvis(thisNode))
		{
			// get the keys from the horiz, vert and turn controllers
			bip = GetBipMasterInterface(c);
			Control *biph = bip->GetHorizontalControl();
			Control *bipv = bip->GetVerticalControl();
			Control *bipr = bip->GetTurnControl();

			biph->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
			bipv->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
			bipr->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
		}
		else if (cid == BIPSLAVE_CONTROL_CLASS_ID)
		{
			// slaves just have keys, apparently
			c->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
		}
		else   //standard bones
		{
			if (cid.PartA() == PRS_CONTROL_CLASS_ID)
			{
				Control *pc = c->GetPositionController();
				Control *rc = c->GetRotationController();
				Control *sc = c->GetScaleController();
				pc->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
				rc->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
				sc->GetKeyTimes(keyTimes, interval, KEYAT_POSITION | KEYAT_ROTATION | KEYAT_SCALE);
				for (int k = 0; k < pc->NumKeys(); ++k)
				{
					TimeValue tv = pc->GetKeyTime(k);
					if (tv >= startTime && tv <= endTime)
					{
						keyTimes.Append(1, &tv);
					}
				}

				for (int k = 0; k < rc->NumKeys(); ++k)
				{
					TimeValue tv = rc->GetKeyTime(k);
					if (tv >= startTime && tv <= endTime)
					{
						keyTimes.Append(1, &tv);
					}
				}

				for (int k = 0; k < sc->NumKeys(); ++k)
				{
					TimeValue tv = sc->GetKeyTime(k);
					if (tv >= startTime && tv <= endTime)
					{
						keyTimes.Append(1, &tv);
					}
				}
			}
			else if (cid == IKCONTROL_CLASS_ID)
			{
				//m_pStream.Printf(_T("get ik key frame \n"));
			}
		}

		// ...and stick a frame at the end as well...it will get sorted out if it is redundant
		keyTimes.Append(1, &endTime);

		if (m_nExportedAnimInterval > 0)
		{
			for (int i = startTime; i < endTime; i += m_nExportedAnimInterval * 160)
			{
				keyTimes.Append(1, &i);
			}
		}

		// skip redundant key times here
		keyTimes.Sort(_compare_func);

		_exportKeyframes(pAnimStream, thisNode, keyTimes, interval);
	}

	//写size
	UINT32 totalSize = ftell(pAnimStream);
	fseek(pAnimStream, sizeof(SKELETOLN_VERSION) - 1, SEEK_SET);
	fwrite(&totalSize, sizeof(UINT32), 1, pAnimStream);

	fclose(pAnimStream);

	//找到json文件,添加anims域
	std::string strJsonName = m_strName + ".obj/" + m_strName + ".json";
	FILE *jsonFP = fopen(strJsonName.c_str(), "r");
	if (jsonFP == NULL)
	{
		return;
	}

	fseek(jsonFP, 0, SEEK_END);
	lSize = ftell(jsonFP);
	rewind(jsonFP);
	buffer = (char *)malloc(sizeof(char) * lSize);
	fread(buffer, 1, lSize, jsonFP);
	fclose(jsonFP);

	cJSON *pJson = cJSON_Parse(buffer);
	cJSON *pSkeletonJson = cJSON_GetObjectItem(pJson, SKELETON_ELEMENT);

	cJSON_DetachItemFromObject(pSkeletonJson, "anims");
	cJSON_DeleteItemFromObject(pSkeletonJson, "anims");

	cJSON *pAnims = _buildSKJson(m_strAbsPath);

	cJSON_AddItemToObject(pSkeletonJson, "anims", pAnims);

	std::string strResJSON = cJSON_Print(pJson);
	jsonFP = fopen(strJsonName.c_str(), "wb");
	fwrite(strResJSON.c_str(), sizeof(char), strResJSON.size(), jsonFP);
	fclose(jsonFP);
}

void PIExp::ExportVertexAnimation(const char *name, int startFrame, int endFrame)
{
	//找到json文件,添加anims域
	std::string strJsonName = m_strName + ".obj/" + m_strName + ".json";
	FILE *jsonFP = fopen(strJsonName.c_str(), "r");
	if (jsonFP == NULL)
	{
		return;
	}
	int lSize;
	char *buffer;

	fseek(jsonFP, 0, SEEK_END);
	lSize = ftell(jsonFP);
	rewind(jsonFP);
	buffer = (char *)malloc(sizeof(char) * lSize);
	fread(buffer, 1, lSize, jsonFP);
	fclose(jsonFP);

	//初始化mesh数据
	cJSON *pJsonRoot = cJSON_Parse(buffer);
	std::vector<INode *> meshNodes;
	std::string strModelName = m_strName + ".obj/" + m_strName + ".model";
	PreProcess(m_ip->GetRootNode(), m_nTotalNodeCount);
	_exportModelToFile(strModelName, m_ip->GetRootNode(), &meshNodes, 0, true, true, NULL);

	//初始化json节点
	cJSON *vertexAnimElement = cJSON_GetObjectItem(pJsonRoot, "vertexAnimElement");
	cJSON *vertexAnims;

	if (!vertexAnimElement)
	{
		vertexAnimElement = cJSON_CreateObject();
		vertexAnims = cJSON_CreateArray();
		cJSON_AddItemToObject(pJsonRoot, "vertexAnimElement", vertexAnimElement);
		cJSON_AddStringToObject(vertexAnimElement, "elementType", "vertexAnimElement");
		cJSON_AddItemToObject(vertexAnimElement, "anims", vertexAnims);
	}
	else
	{
		vertexAnims = cJSON_GetObjectItem(vertexAnimElement, "anims");
		for (INT32 i = 0; i < cJSON_GetArraySize(vertexAnims); ++i)
		{
			if (strcmp(cJSON_GetObjectItem(cJSON_GetArrayItem(vertexAnims, i), "name")->valuestring, name) == 0)
			{
				cJSON_DetachItemFromArray(vertexAnims, i);
				cJSON_DeleteItemFromArray(vertexAnims, i);
			}
		}
	}
	cJSON *anim = cJSON_CreateObject();
	cJSON_AddItemToArray(vertexAnims, anim);

	cJSON_AddStringToObject(anim, "name", name);

	//创建.ve动画文件
	std::string strVertexAnimName = m_strName + ".obj/" + name + ".veAnim";
	FILE *pAnimStream = fopen(strVertexAnimName.c_str(), "wb");
#if defined(_DEBUG)||defined(DEBUG)
	//创建导出log文件
	std::string strLogName = m_strName + ".obj/" + name + ".log";
	FILE *logStream = fopen(strLogName.c_str(), "w");
#endif
	//创建动画帧
	INT32 duration = -1;
	INT32 startTime = startFrame * GetTicksPerFrame();
	INT32 endTime = endFrame * GetTicksPerFrame();
	Interval animRange(startTime, endTime);

	UINT32 meshNodeSize = meshNodes.size();
	fwrite(&meshNodeSize, sizeof(UINT32), 1, pAnimStream);
#if defined(_DEBUG)||defined(DEBUG)
	char nodeSizeStr[20];
	sprintf(nodeSizeStr, "nodeSize = %d\n", meshNodeSize);
	fwrite(nodeSizeStr, sizeof(char) * strlen(nodeSizeStr), 1, logStream);
#endif
	cJSON *pNodeSize = cJSON_GetObjectItem(vertexAnimElement, "nodeCount");
	if (!pNodeSize)
	{
		cJSON_AddNumberToObject(vertexAnimElement, "nodeCount", meshNodeSize);
	}
	else
	{
		assert(pNodeSize->valueint == meshNodeSize);
	}

	for (UINT32 i = 0; i < meshNodeSize; ++i)
	{
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
			for (int k = animRange.Start(); k < animRange.End(); k += m_nExportedAnimInterval * 160)
			{
				keyTimes.Append(1, &k);
			}
		}

		INT32 keyTimesCount = keyTimes.Count();
		INT32 frameCount = keyTimesCount;
		UINT32 frameCountOffset = ftell(pAnimStream);
		fwrite(&frameCount, sizeof(INT32), 1, pAnimStream);
		keyTimes.Sort(_compare_func);

		int keyTime = -1;
		for (int j = 0; j < keyTimesCount; j++)
		{
			//忽略相同的
			if (keyTime == keyTimes[j])
			{
				--frameCount;
				continue;
			}
			keyTime = keyTimes[j];
			float keyTimef = (float)keyTimes[j] / (float)GetTicksPerFrame() / (float)GetFrameRate();
			//帧时间
			INT32 frameKeyTime = (INT32)(0.5f + keyTimef * 1000);
			fwrite(&frameKeyTime, sizeof(INT32), 1, pAnimStream);

#if defined(_DEBUG)||defined(DEBUG)
			char frameTimeStr[20];
			sprintf(frameTimeStr, "\tstartTime = %d\n", frameKeyTime);
			fwrite(frameTimeStr, sizeof(char) * strlen(frameTimeStr), 1, logStream);
#endif
			//model文件size 和 文件外的大小偏移
			UINT32 modelSize = 0;
			UINT32 preSizeOffset = ftell(pAnimStream);
			fwrite(&modelSize, sizeof(UINT32), 1, pAnimStream);

			//导出model文件
			//char nameSuffix[MAX_PATH];
			//sprintf(nameSuffix, "_%d_%d", i, frameKeyTime);
			//std::string strModelName =m_strResPath + m_strName + ".obj/" + name + nameSuffix + ".model";
			////fwrite(&frameKeyTime, sizeof(INT32), 1, pAnimStream);
			////fwrite(strModelName.c_str(), sizeof(char), strModelName.size(), pAnimStream);
			//_exportModelToFile(m_strName + ".obj/" + name + nameSuffix + ".model", meshNodes[i], NULL, keyTime, false, true);

			//开始写model信息
			//model文件起始偏移
			UINT32 headOffset = ftell(pAnimStream);
			//写文件头
			fwrite(MODEL_VERSION, sizeof(char), sizeof(MODEL_VERSION) - 1, pAnimStream);

			//获取model文件内的size的偏移
			UINT32 modelSizeOffset = ftell(pAnimStream);
			fwrite(&modelSize, sizeof(UINT32), 1, pAnimStream);

			//获取subMesh数量的偏移
			UINT32 subMeshNumOffset = ftell(pAnimStream);
			UINT32 numSubMesh = 0;
			fwrite(&numSubMesh, sizeof(UINT32), 1, pAnimStream);

			std::vector<INode *> outMeshNodes;
			std::vector<INode *> *pMeshNodes = &outMeshNodes;

			if(!_exportModelData(meshNodes[i], keyTime, true, pAnimStream, pMeshNodes, true, NULL)){
				fclose(jsonFP);
#if defined(_DEBUG)||defined(DEBUG)
				fclose(logStream);
#endif
				return;
			}

			//写subMesh数量
			numSubMesh = pMeshNodes->size();
			fseek(pAnimStream, subMeshNumOffset, SEEK_SET);
			fwrite(&numSubMesh, sizeof(UINT32), 1, pAnimStream);
			fseek(pAnimStream, 0, SEEK_END);

			//写modelSize
			UINT32 totalSize = ftell(pAnimStream) - headOffset;
			fseek(pAnimStream, preSizeOffset, SEEK_SET);
			fwrite(&totalSize, sizeof(UINT32), 1, pAnimStream);
			fseek(pAnimStream, modelSizeOffset, SEEK_SET);
			fwrite(&totalSize, sizeof(UINT32), 1, pAnimStream);

			fseek(pAnimStream, 0, SEEK_END);

			//UV贴图偏移
			for (int k = 0; k < numSubMesh; ++k)
			{
				Mtl *mtl = (*pMeshNodes)[k]->GetMtl();
				INT32 hasMtl = 0;

				if (mtl)
				{
					hasMtl = 1;
					fwrite(&hasMtl, sizeof(INT32), 1, pAnimStream);

					UINT32 numSubTexmaps = mtl->NumSubTexmaps();
					fwrite(&numSubTexmaps, sizeof(UINT32), 1, pAnimStream);

					for (int subMapIndex = 0; subMapIndex < numSubTexmaps; ++subMapIndex)
					{
						Texmap *subTex = mtl->GetSubTexmap(subMapIndex);
						INT32 hasSubTex = 0;
						if (subTex)
						{
							hasSubTex = 1;
							fwrite(&hasSubTex, sizeof(INT32), 1, pAnimStream);

							Matrix3 uvMat;
							//float matrix[4][4];
							subTex->GetUVTransform(uvMat);
							Point3 trans = uvMat.GetTrans();
							trans *= -1;
							Point3 scale(1.0, 1.0, 1.0);
							uvMat.PointTransform(scale);
							Quat rot(uvMat);

							fwrite(&trans.x, sizeof(float), 1, pAnimStream);
							fwrite(&trans.y, sizeof(float), 1, pAnimStream);
							fwrite(&trans.z, sizeof(float), 1, pAnimStream);

							fwrite(&scale.x, sizeof(float), 1, pAnimStream);
							fwrite(&scale.y, sizeof(float), 1, pAnimStream);
							fwrite(&scale.z, sizeof(float), 1, pAnimStream);

							fwrite(&rot.x, sizeof(float), 1, pAnimStream);
							fwrite(&rot.y, sizeof(float), 1, pAnimStream);
							fwrite(&rot.z, sizeof(float), 1, pAnimStream);
							fwrite(&rot.w, sizeof(float), 1, pAnimStream);

							/* 直接导出4*4矩阵
							/* Xpi = Xmax; Ypi = Zmax; Zpi = -Ymax;
							matrix[0][0] = uvMat.GetColumn(0).x;
							matrix[0][1] = uvMat.GetColumn(0).y;
							matrix[0][2] = uvMat.GetColumn(0).z;
							matrix[0][3] = uvMat.GetColumn(0).w;

							matrix[0][0] = uvMat.GetColumn(2).x;
							matrix[0][1] = uvMat.GetColumn(2).y;
							matrix[0][2] = uvMat.GetColumn(2).z;
							matrix[0][3] = uvMat.GetColumn(2).w;

							matrix[0][0] = -uvMat.GetColumn(1).x;
							matrix[0][1] = -uvMat.GetColumn(1).y;
							matrix[0][2] = -uvMat.GetColumn(1).z;
							matrix[0][3] = -uvMat.GetColumn(1).w;

							matrix[3][0] = 0.0f;
							matrix[3][1] = 0.0f;
							matrix[3][2] = 0.0f;
							matrix[3][3] = 1.0f;

							fwrite(matrix, sizeof(float), 16, pAnimStream);
							for(int rowIndex = 0; rowIndex < 4; ++rowIndex)
							{
							for(int columnIndex = 0; columnIndex < 4; ++columnIndex)
							{
							fwrite
							}
							}*/

						}
						else
						{
							fwrite(&hasSubTex, sizeof(INT32), 1, pAnimStream);
						}
					}
				}
				else
				{
					hasMtl = 0;
					fwrite(&hasMtl, sizeof(INT32), 1, pAnimStream);
				}
			}
		}//当前帧结束

		 //更新最大持续时间
		float fduration = (float)keyTime / (float)GetTicksPerFrame() / (float)GetFrameRate();
		duration = max(duration, (INT32)(0.5f + fduration * 1000));

		//写入帧数量
		fseek(pAnimStream, frameCountOffset, SEEK_SET);
		fwrite(&frameCount, sizeof(INT32), 1, pAnimStream);
		fseek(pAnimStream, 0, SEEK_END);
#if defined(_DEBUG)||defined(DEBUG)
		char frameCountStr[20];
		sprintf(frameCountStr, "frameCount = %d\n", frameCount);
		fwrite(frameCountStr, sizeof(char) * strlen(frameCountStr), 1, logStream);
#endif
	}
	fwrite(&duration, sizeof(float), 1, pAnimStream);
	fclose(pAnimStream);
#if defined(_DEBUG)||defined(DEBUG)
	fclose(logStream);
#endif
	cJSON_AddStringToObject(anim, "file", (m_strResPath + strVertexAnimName).c_str());

	std::string strResJSON = cJSON_Print(pJsonRoot);
	jsonFP = fopen(strJsonName.c_str(), "wb");
	fwrite(strResJSON.c_str(), sizeof(char), strResJSON.size(), jsonFP);
	fclose(jsonFP);
}

void PIExp::ExportUVAnimation(const char *name, int startFrame, int endFrame)
{
	std::string strJsonName = m_strName + ".obj/" + m_strName + ".json";
	FILE *jsonFP = fopen(strJsonName.c_str(), "r");
	if (jsonFP == NULL)
	{
		return;
	}
	int lSize;
	char *buffer;
	fseek(jsonFP, 0, SEEK_END);
	lSize = ftell(jsonFP);
	rewind(jsonFP);
	buffer = (char *)malloc(sizeof(char) * lSize);
	fread(buffer, 1, lSize, jsonFP);
	fclose(jsonFP);

	cJSON *pJsonRoot = cJSON_Parse(buffer);
	std::vector<INode *> meshNodes;
	std::string strModelName = m_strName + ".obj/" + m_strName + ".model";
	PreProcess(m_ip->GetRootNode(), m_nTotalNodeCount);
	_exportModelToFile(strModelName, m_ip->GetRootNode(), &meshNodes, 0, true, true, NULL);

	cJSON *uvAnimElement = cJSON_GetObjectItem(pJsonRoot, "uvAnimElement");
	cJSON *uvAnims;

	if (!uvAnimElement)
	{
		uvAnimElement = cJSON_CreateObject();
		uvAnims = cJSON_CreateArray();
		cJSON_AddItemToObject(pJsonRoot, "uvAnimElement", uvAnimElement);
		cJSON_AddStringToObject(uvAnimElement, "elementType", "uvAnimElement");
		cJSON_AddItemToObject(uvAnimElement, "anims", uvAnims);
	}
	else
	{
		uvAnims = cJSON_GetObjectItem(uvAnimElement, "anims");
		for (INT32 i = 0; i < cJSON_GetArraySize(uvAnims); i++)
		{
			if (strcmp(cJSON_GetObjectItem(cJSON_GetArrayItem(uvAnims, i), "name")->valuestring, name) == 0)
			{
				cJSON_DetachItemFromArray(uvAnims, i);
				cJSON_DeleteItemFromArray(uvAnims, i);
			}
		}
	}
	cJSON *anim = cJSON_CreateObject();
	cJSON_AddItemToArray(uvAnims, anim);
	cJSON_AddStringToObject(anim, "name", name);

	std::string strUVAnimName = m_strName + ".obj/" + name + ".uvAnim";
	FILE *pAnimStream = fopen(strUVAnimName.c_str(), "wb");
#if defined(_DEBUG) || defined(DEBUG)
	std::string strLogName = m_strName + ".obj/" + name + ".log";
	FILE *logStream = fopen(strLogName.c_str(), "w");
#endif
	INT32 duration = -1;
	INT32 startTime = startFrame * GetTicksPerFrame();
	INT32 endTime = endFrame * GetTicksPerFrame();
	Interval animRange(startTime, endTime);

	UINT32 meshNodeSize = meshNodes.size();
	fwrite(&meshNodeSize, sizeof(UINT32), 1, pAnimStream);
#if defined(_DEBUG) || defined(DEBUG)
	char nodeSizeStr[20];
	sprintf(nodeSizeStr, "nodeSize = %d\n", meshNodeSize);
	fwrite(nodeSizeStr, sizeof(char)*strlen(nodeSizeStr), 1, logStream);
#endif
	cJSON *pNodeSize = cJSON_GetObjectItem(uvAnimElement, "nodeCount");
	if (!pNodeSize)
	{
		cJSON_AddNumberToObject(uvAnimElement, "nodeCount", meshNodeSize);
	}
	else
	{
		assert(pNodeSize->valueint == meshNodeSize);
	}
	for (UINT32 i = 0; i < meshNodeSize; i++)
	{
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
			for (int k = animRange.Start(); k < animRange.End(); k += m_nExportedAnimInterval * 160)
			{
				keyTimes.Append(1, &k);
			}
		}
		INT32 keyTimesCount = keyTimes.Count();
		INT32 frameCount = keyTimesCount;
		UINT32 frameCountOffset = ftell(pAnimStream);
		fwrite(&frameCount, sizeof(INT32), 1, pAnimStream);
		keyTimes.Sort(_compare_func);

		int keyTime = -1;
		for (int j = 0; j < keyTimesCount; j++)
		{
			if (keyTime == keyTimes[j])
			{
				--frameCount;
				continue;
			}
			keyTime = keyTimes[j];
			float keyTimef = (float)keyTimes[j] / (float)GetTicksPerFrame() / (float)GetFrameRate();
			INT32 frameKeyTime = (INT32)(0.5f + keyTimef * 1000);
			fwrite(&frameKeyTime, sizeof(INT32), 1, pAnimStream);
#if defined(_DEBUG) || defined(DEBUG)
			char frameTimeStr[20];
			sprintf(frameTimeStr, "\tstartTime = %d\n", frameKeyTime);
			fwrite(frameTimeStr, sizeof(char)*strlen(frameTimeStr), 1, logStream);
#endif
			UINT32 modelSize = 0;
			UINT32 proSizeOffset = ftell(pAnimStream);
			fwrite(&modelSize, sizeof(UINT32), 1, pAnimStream);
			int numSubMesh = 0;
			std::vector<INode *> outMeshNodes;
			std::vector<INode *> *pMeshNodes = &outMeshNodes;
			_getSubMeshs(meshNodes[i], keyTime, pMeshNodes);
			numSubMesh = pMeshNodes->size();
			fwrite(&numSubMesh, sizeof(UINT32), 1, pAnimStream);
			for (int k = 0; k < numSubMesh; ++k)
			{
				Mtl *mtl = (*pMeshNodes)[k]->GetMtl();
				INT32 hasMtl = 0;
				if (mtl)
				{
					hasMtl = 1;
					fwrite(&hasMtl, sizeof(INT32), 1, pAnimStream);
					UINT32 numSubTexmaps = mtl->NumSubTexmaps();
					fwrite(&numSubTexmaps, sizeof(UINT32), 1, pAnimStream);
					for (int subMapIndex = 0; subMapIndex < numSubTexmaps; subMapIndex++)
					{
						Texmap *subTex = mtl->GetSubTexmap(subMapIndex);
						INT32 hasSubTex = 0;
						if (subTex)
						{
							hasSubTex = 1;
							fwrite(&hasSubTex, sizeof(INT32), 1, pAnimStream);
							Matrix3 uvMat;
							subTex->GetUVTransform(uvMat);
							Point3 trans = uvMat.GetTrans();
							trans *= -1;
							Point3 scale(1.0, 1.0, 1.0);
							uvMat.PointTransform(scale);
							Quat rot(uvMat);

							fwrite(&trans.x, sizeof(float), 1, pAnimStream);
							fwrite(&trans.y, sizeof(float), 1, pAnimStream);
							fwrite(&trans.z, sizeof(float), 1, pAnimStream);

							fwrite(&scale.x, sizeof(float), 1, pAnimStream);
							fwrite(&scale.y, sizeof(float), 1, pAnimStream);
							fwrite(&scale.z, sizeof(float), 1, pAnimStream);

							fwrite(&rot.x, sizeof(float), 1, pAnimStream);
							fwrite(&rot.y, sizeof(float), 1, pAnimStream);
							fwrite(&rot.z, sizeof(float), 1, pAnimStream);
							fwrite(&rot.w, sizeof(float), 1, pAnimStream);
						}
						else
						{
							fwrite(&hasSubTex, sizeof(INT32), 1, pAnimStream);
						}
					}
				}
				else
				{
					hasMtl = 0;
					fwrite(&hasMtl, sizeof(INT32), 1, pAnimStream);
				}
			}
		}
		float fduration = (float)keyTime / (float)GetTicksPerFrame() / (float)GetFrameRate();
		duration = max(duration, (INT32)(0.5f + fduration * 1000));
		fseek(pAnimStream, frameCountOffset, SEEK_SET);
		fwrite(&frameCount, sizeof(INT32), 1, pAnimStream);
		fseek(pAnimStream, 0, SEEK_END);
#if defined(_DEBUG) || defined(DEBUG)
		char frameCountStr[20];
		sprintf(frameCountStr, "frameCount = %d\n", frameCount);
		fwrite(frameCountStr, sizeof(char)*strlen(frameCountStr), 1, logStream);
#endif
	}
	fwrite(&duration, sizeof(float), 1, pAnimStream);
	fclose(pAnimStream);
#if defined(_DEBUG) || defined(DEBUG)
	fclose(logStream);
#endif
	cJSON_AddStringToObject(anim, "file", (m_strResPath + strUVAnimName).c_str());
	std::string strResJSON = cJSON_Print(pJsonRoot);
	jsonFP = fopen(strJsonName.c_str(), "wb");
	fwrite(strResJSON.c_str(), sizeof(char), strResJSON.size(), jsonFP);
	fclose(jsonFP);
}

/****************************************************************************

Mesh output

****************************************************************************/

Point3 PIExp::GetVertexNormal(Mesh *mesh, int faceNo, RVertex *rv)
{
	Face *f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals = 0;
	Point3 vertexNormal;

	// Is normal specified
	// SPCIFIED is not currently used, but may be used in future versions.
	if (rv->rFlags & SPECIFIED_NORMAL)
	{
		vertexNormal = rv->rn.getNormal();
	}
	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else if ((numNormals = rv->rFlags & NORCT_MASK) != 0 && smGroup)
	{
		// If there is only one vertex is found in the rn member.
		if (numNormals == 1)
		{
			vertexNormal = rv->rn.getNormal();
		}
		else
		{
			// If two or more vertices are there you need to step through them
			// and find the vertex with the same smoothing group as the current face.
			// You will find multiple normals in the ern member.
			for (int i = 0; i < numNormals; i++)
			{
				if (rv->ern[i].getSmGroup() & smGroup)
				{
					vertexNormal = rv->ern[i].getNormal();
				}
			}
		}
	}
	else
	{
		// Get the normal from the Face if no smoothing groups are there
		vertexNormal = mesh->getFaceNormal(faceNo);
	}

	return vertexNormal;
}

void PIExp::ExportMaterial(INode *node, int currSubMeshIdx)
{
	Mtl *mtl = node->GetMtl();

	if (mtl)
	{
		int mtlID = m_mtlList.GetMtlID(mtl);
		if (mtlID >= 0)
		{
			//写subMesh的材质数据
			DumpJSONMaterial(mtl, currSubMeshIdx);
		}
	}
}

void PIExp::DumpJSONMaterial(Mtl *mtl, int currSubMeshIdx)
{
	int i;
	std::string shadingEleName("");
	shadingEleName = SHADING_PHONG_ELEMENTS;

	//写光照element

	if (m_bIncrementExport && m_bElements[SHADING_PHONG_ELEMENTS] || !m_bIncrementExport)
	{
		cJSON *pShadingPhongEles = cJSON_GetObjectItem(m_pJSONRoot, shadingEleName.c_str());
		if (pShadingPhongEles == NULL)
		{
			pShadingPhongEles = cJSON_CreateArray();
			for (int i = 0; i < currSubMeshIdx; ++i)
			{
				cJSON_AddItemToArray(pShadingPhongEles, cJSON_CreateNull());
			}
			cJSON_AddItemToObject(m_pJSONRoot, shadingEleName.c_str(), pShadingPhongEles);
		}

		cJSON *pShadingPhong = cJSON_CreateObject();
		cJSON_AddItemToArray(pShadingPhongEles, pShadingPhong);
		cJSON_AddStringToObject(pShadingPhong, "elementType", shadingEleName.c_str());

		cJSON *pDiffuse = cJSON_CreateObject();
		cJSON_AddItemToObject(pShadingPhong, "diffuse", pDiffuse);
		cJSON_AddNumberToObject(pDiffuse, "r", mtl->GetDiffuse().r);
		cJSON_AddNumberToObject(pDiffuse, "g", mtl->GetDiffuse().g);
		cJSON_AddNumberToObject(pDiffuse, "b", mtl->GetDiffuse().b);

		cJSON *pSpecular = cJSON_CreateObject();
		cJSON_AddItemToObject(pShadingPhong, "specular", pSpecular);
		cJSON_AddNumberToObject(pSpecular, "r", mtl->GetSpecular().r * mtl->GetShinStr());
		cJSON_AddNumberToObject(pSpecular, "g", mtl->GetSpecular().g * mtl->GetShinStr());
		cJSON_AddNumberToObject(pSpecular, "b", mtl->GetSpecular().b * mtl->GetShinStr());

		cJSON_AddNumberToObject(pShadingPhong, "cullMode", 2);

		cJSON_AddNumberToObject(pShadingPhong, "depthFunc", 2);
		cJSON_AddBoolToObject(pShadingPhong, "depthEnable", true);
		cJSON_AddBoolToObject(pShadingPhong, "depthWriteEnable", true);

		cJSON_AddNumberToObject(pShadingPhong, "blendMode", 0);

		cJSON_AddNumberToObject(pShadingPhong, "reflectionIndex", 64);
		cJSON_AddNumberToObject(pShadingPhong, "glossiness", 64);

		cJSON_AddBoolToObject(pShadingPhong, "refraction", false);
		cJSON_AddBoolToObject(pShadingPhong, "refractionTransparentEnable", true);
		cJSON_AddNumberToObject(pShadingPhong, "refractionTransparent", 0.3);

		cJSON_AddBoolToObject(pShadingPhong, "environmentMapping", true);
	}

	for (i = 0; i < mtl->NumSubTexmaps(); i++)
	{
		Texmap *subTex = mtl->GetSubTexmap(i);
		float amt = 1.0f;
		if (subTex)
		{
			// If it is a standard material we can see if the map is enabled.
			if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
			{
				if (!((StdMat *)mtl)->MapEnabled(i))
				{
					continue;
				}
				amt = ((StdMat *)mtl)->GetTexmapAmt(i, 0);
			}
			DumpJSONTexture(mtl, subTex, i, currSubMeshIdx);
		}
	}
}

// For a standard material, this will give us the meaning of a map
// givien its submap id.
TCHAR *PIExp::GetMapID(Class_ID cid, int subNo)
{
	static TCHAR buf[50];

	if (cid == Class_ID(0, 0))
	{
		_tcscpy(buf, ID_ENVMAP);
	}
	else if (cid == Class_ID(DMTL_CLASS_ID, 0))
	{
		switch (subNo)
		{
		case ID_AM:
			_tcscpy(buf, ID_MAP_AMBIENT);
			break;
		case ID_DI:
			_tcscpy(buf, ID_MAP_DIFFUSE);
			break;
		case ID_SP:
			_tcscpy(buf, ID_MAP_SPECULAR);
			break;
		case ID_SH:
			_tcscpy(buf, ID_MAP_SHINE);
			break;
		case ID_SS:
			_tcscpy(buf, ID_MAP_SHINESTRENGTH);
			break;
		case ID_SI:
			_tcscpy(buf, ID_MAP_SELFILLUM);
			break;
		case ID_OP:
			_tcscpy(buf, ID_MAP_OPACITY);
			break;
		case ID_FI:
			_tcscpy(buf, ID_MAP_FILTERCOLOR);
			break;
		case ID_BU:
			_tcscpy(buf, ID_MAP_BUMP);
			break;
		case ID_RL:
			_tcscpy(buf, ID_MAP_REFLECT);
			break;
		case ID_RR:
			_tcscpy(buf, ID_MAP_REFRACT);
			break;
		}
	}
	else
	{
		_tcscpy(buf, ID_MAP_GENERIC);
	}

	return buf;
}

void PIExp::DumpJSONTexture(Mtl *mtl, Texmap *tex, int subNo, int currSubMeshIdx)
{
	if (!tex)
	{
		return;
	}

	// Is this a bitmap texture?
	// We know some extra bits 'n pieces about the bitmap texture
	if (tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00))
	{
		TSTR mapName = ((BitmapTex *)tex)->GetMapName();

		cJSON *pMapEles = NULL;
		std::string eleName("");
		if (subNo == ID_DI)
		{
			eleName = DIFFUSE_MAP_ELEMENTS;
		}
		else if (subNo == ID_SP)
		{
			eleName = SPECULAR_MAP_ELEMENTS;
		}
		else if (subNo == ID_SI)
		{
			eleName = GLOW_MAP_ELEMENTS;
		}
		else if (subNo == ID_BU)
		{
			eleName = NORMAL_MAP_ELEMENTS;
		}
		else if (subNo == ID_OP)
		{
			eleName = ALPHA_MAP_ELEMENTS;
		}
		if (!eleName.empty() && (m_bIncrementExport && m_bElements[eleName] || !m_bIncrementExport))
		{
			pMapEles = cJSON_GetObjectItem(m_pJSONRoot, eleName.c_str());
			if (pMapEles == NULL)
			{
				pMapEles = cJSON_CreateArray();
				cJSON_AddItemToObject(m_pJSONRoot, eleName.c_str(), pMapEles);
			}

			for (int i = cJSON_GetArraySize(pMapEles); i < currSubMeshIdx; ++i)
			{
				cJSON_AddItemToArray(pMapEles, cJSON_CreateNull());
			}
		}

		if (pMapEles != NULL && (m_bIncrementExport && m_bElements[eleName] || !m_bIncrementExport))
		{
			std::string relPath = extractRelPath(FixupName(mapName));
			std::string strTexFileName = extractName(FixupName(mapName), true);

			//复制贴图
			/*std::string strTexFileAbsPath = m_strAbsPath + m_strName + ".obj/" + strTexFileName;
			setlocale(LC_ALL,"");
			TCHAR wc[MAX_FILE_PATH];
			std::size_t l;
			mbstowcs_s(&l, wc, MAX_FILE_PATH, strTexFileAbsPath.c_str(), MAX_FILE_PATH);
			CopyFile(mapName, wc, FALSE);*/

			//写json
			cJSON *pTex = cJSON_CreateObject();
			cJSON_AddItemToArray(pMapEles, pTex);
			cJSON_AddStringToObject(pTex, "elementType", eleName.c_str());

			std::string strTexFileRelPath = relPath + strTexFileName;
			cJSON_AddStringToObject(pTex, "path", (strTexFileRelPath).c_str());

			//寻址模式
			bool uWrap = !!(tex->GetTextureTiling() & U_WRAP);
			bool vWrap = !!(tex->GetTextureTiling() & V_WRAP);

			bool uMirror = !!(tex->GetTextureTiling() & U_MIRROR);
			bool vMirror = !!(tex->GetTextureTiling() & V_MIRROR);

			cJSON_AddNumberToObject(pTex, "addressU", uWrap ? 0 : (uMirror ? 1 : 2));
			cJSON_AddNumberToObject(pTex, "addressV", vWrap ? 0 : (vMirror ? 1 : 2));
			cJSON_AddNumberToObject(pTex, "addressW", vWrap ? 0 : (vMirror ? 1 : 2));

			if (subNo == ID_SI)
			{
				cJSON_AddNumberToObject(pTex, "scale", 1.0f);
			}
			else if (subNo == ID_BU)
			{
				cJSON_AddBoolToObject(pTex, "tangent", true);
			}
			else if (subNo == ID_OP)
			{
				cJSON_AddNumberToObject(pTex, "cullOff", 0.4f);
			}

			//如果要采帧
			//			if( m_nExportedAnimInterval > 0 )
			// 			{
			// 				Matrix3 uvMat;
			// 				Interval animRange = m_ip->GetAnimRange();
			// 				INT32 start = animRange.Start();
			// 				INT32 end = animRange.End();
			// 				cJSON *orbit = cJSON_CreateObject();
			// 				cJSON_AddItemToObject(pTex, "uvOrbits", orbit);
			// 				cJSON *orbitName = cJSON_CreateObject();
			// 				cJSON_AddItemToObject(orbit, "default", orbitName);
			// 				float timePerFrame = (m_nExportedAnimInterval*160.0f) / (end - start);
			// 				cJSON *positions = cJSON_CreateArray();
			// 				cJSON *scales = cJSON_CreateArray();
			// 				cJSON *rotations = cJSON_CreateArray();
			// 				cJSON_AddItemToObject(orbitName, "positions", 0);
			// 				cJSON_AddItemToObject(orbitName, "scales", scales);
			// 				cJSON_AddItemToObject(orbitName, "rotations", rotations);
			//
			// 				//颜色轨迹
			// 				cJSON *colorOrbit = cJSON_CreateObject();
			// 				cJSON_AddItemToObject(pTex, "reviseOrbits", colorOrbit);
			// 				cJSON *colorName = cJSON_CreateObject();
			// 				cJSON_AddItemToObject(colorOrbit, "default", colorName);
			//
			// 				cJSON *colors = cJSON_CreateArray();
			// 				cJSON_AddItemToObject(colorName, "colors", colors);
			// 				INT32 duration = (INT32)( end / (float)GetTicksPerFrame() / (float)GetFrameRate() * 1000.0f + 0.5f );
			//
			// 				cJSON_AddNumberToObject(colorName, "duration", duration / 1000);
			// 				cJSON_AddNumberToObject(orbit, "duration", duration / 1000);
			//
			// 				for(int i = start; i < end; i+=m_nExportedAnimInterval*160)
			// 				{
			//
			// 					Interval range;
			//
			// 					// 处理颜色
			// 					mtl->Update(i, range);;
			//
			// 					cJSON *frame = cJSON_CreateObject();
			// 					cJSON_AddNumberToObject(frame,"time", timePerFrame);
			// 					cJSON_AddNumberToObject(frame,"r", mtl->GetDiffuse().r);
			// 					cJSON_AddNumberToObject(frame,"g" , mtl->GetDiffuse().g);
			// 					cJSON_AddNumberToObject(frame,"b", mtl->GetDiffuse().b);
			// 					cJSON_AddItemToArray(colors, frame);
			//
			// 					//处理UV
			// 					tex->Update(i, range);
			// 					tex->GetUVTransform(uvMat);
			// 					//position
			// 					frame = cJSON_CreateObject();
			// 					cJSON_AddNumberToObject(frame,"time", timePerFrame);
			// 					cJSON_AddNumberToObject(frame,"x", uvMat.GetTrans().x);
			// 					cJSON_AddNumberToObject(frame,"y", uvMat.GetTrans().y);
			// 					cJSON_AddNumberToObject(frame,"z", uvMat.GetTrans().z);
			// 					cJSON_AddItemToArray(positions, frame);
			//
			//
			// 					//scale
			// 					Point3 scale(1.0f,1.0f,1.0f);
			// 					uvMat.PointTransform(scale);
			// 					frame = cJSON_CreateObject();
			// 					cJSON_AddNumberToObject(frame,"time", timePerFrame);
			// 					cJSON_AddNumberToObject(frame,"x", scale.x);
			// 					cJSON_AddNumberToObject(frame,"y", scale.y);
			// 					cJSON_AddNumberToObject(frame,"z", scale.z);
			// 					cJSON_AddItemToArray(scales, frame);
			//
			// 					//rotate
			// 					frame = cJSON_CreateObject();
			// 					Quat rot(uvMat);
			// 					cJSON_AddNumberToObject(frame,"time", timePerFrame);
			// 					cJSON_AddNumberToObject(frame,"x", rot.x);
			// 					cJSON_AddNumberToObject(frame,"y", rot.y);
			// 					cJSON_AddNumberToObject(frame,"z", rot.z);
			// 					cJSON_AddNumberToObject(frame,"w", rot.w);
			// 					cJSON_AddItemToArray(rotations, frame);
			//
			// 				}
			// 			}
		}
	}
}

/****************************************************************************

Misc Utility functions

****************************************************************************/



/****************************************************************************

String manipulation functions

****************************************************************************/

#define CTL_CHARS  31
#define SINGLE_QUOTE 39

// Replace some characters we don't care for.
TCHAR *PIExp::FixupName(const TCHAR *name)
{
	static TCHAR buffer[MAX_FILE_PATH];
	TCHAR *cPtr;

	_tcscpy(buffer, name);
	cPtr = buffer;

	while (*cPtr)
	{
		if (*cPtr == _T('"'))
		{
			*cPtr = SINGLE_QUOTE;
		}
		else if (*cPtr <= CTL_CHARS)
		{
			*cPtr = _T('_');
		}
		cPtr++;
	}

	return buffer;
}

// International settings in Windows could cause a number to be written
// with a "," instead of a ".".
// To compensate for this we need to convert all , to . in order to make the
// format consistent.
void PIExp::CommaScan(TCHAR *buf)
{
	for (; *buf; buf++) if (*buf == _T(','))
	{
		*buf = _T('.');
	}
}

TSTR PIExp::Format(int value)
{
	TCHAR buf[50];

	_stprintf(buf, _T("%d"), value);
	return buf;
}

TSTR PIExp::Format(float value)
{
	TCHAR buf[40];

	_stprintf(buf, m_szFmtStr, value);
	CommaScan(buf);
	return TSTR(buf);
}

TSTR PIExp::Format(Point3 value)
{
	TCHAR buf[120];
	TCHAR fmt[120];

	_stprintf(fmt, _T("%s\t%s\t%s"), m_szFmtStr, m_szFmtStr, m_szFmtStr);
	_stprintf(buf, fmt, value.x, value.y, value.z);

	CommaScan(buf);
	return buf;
}

TSTR PIExp::Format(Color value)
{
	TCHAR buf[120];
	TCHAR fmt[120];

	_stprintf(fmt, _T("%s\t%s\t%s"), m_szFmtStr, m_szFmtStr, m_szFmtStr);
	_stprintf(buf, fmt, value.r, value.g, value.b);

	CommaScan(buf);
	return buf;
}

TSTR PIExp::Format(AngAxis value)
{
	TCHAR buf[160];
	TCHAR fmt[160];

	_stprintf(fmt, _T("%s\t%s\t%s\t%s"), m_szFmtStr, m_szFmtStr, m_szFmtStr, m_szFmtStr);
	_stprintf(buf, fmt, value.axis.x, value.axis.y, value.axis.z, value.angle);

	CommaScan(buf);
	return buf;
}

TSTR PIExp::Format(Quat value)
{
	// A Quat is converted to an AngAxis before output.

	Point3 axis;
	float angle;
	AngAxisFromQ(value, &angle, axis);

	return Format(AngAxis(axis, angle));
}

TSTR PIExp::Format(ScaleValue value)
{
	TCHAR buf[280];

	_stprintf(buf, _T("%s %s"), Format(value.s), Format(value.q));
	CommaScan(buf);
	return buf;
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject *PIExp::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
	{
		TriObject *tri = (TriObject *)obj->ConvertToType(t,
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri)
		{
			deleteIt = TRUE;
		}
		return tri;
	}
	else
	{
		return NULL;
	}
}