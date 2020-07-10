//************************************************************************** 
//* Piiexp.h	- PI File Exporter
//***************************************************************************

#ifndef __ASCIIEXP__H
#define __ASCIIEXP__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "interpik.h"

#include "asciitok.h"
#include "maxtextfile.h"
#include "maxbinarystream.h"

#include "cJSON.h"

#include "cs\BipedApi.h"
#include "iskin.h"



#include <vector>
#include <map>
#include <hash_map>
#include <string>

extern ClassDesc* GetAsciiExpDesc();
extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define VERSION			200			// Version number * 100
//#define FLOAT_OUTPUT	_T("%4.4f")	// Float precision for output
#define CFGFILENAME		_T("piexp.cfg")	// Configuration file

#define MODEL_VERSION "TENGINE_MESH_V03"
#define SKELETOLN_VERSION "TENGINE_SKEL_V01"
#define MAX_BONE_BIND 4
#define BONE_NAME_MAX_LEN 128
#define ANIM_NAME_MAX_LEN 128

#define MODEL_BASE_ELEMENT "modelBaseElement"
#define SKELETON_ELEMENT "skeletonElement"
#define SHADING_PHONG_ELEMENTS "shadingPhongElements"
#define DIFFUSE_MAP_ELEMENTS "diffuseMapElements"
#define SPECULAR_MAP_ELEMENTS "specularMapElements"
#define ALPHA_MAP_ELEMENTS "alphaMapElements"
#define GLOW_MAP_ELEMENTS "glowMapElements"
#define NORMAL_MAP_ELEMENTS "normalMapElements"

#define SCALE_PRECISION 0.000001



using namespace std;
using namespace stdext;

class MtlKeeper {
public:
	BOOL	AddMtl(Mtl* mtl);
	int		GetMtlID(Mtl* mtl);
	int		Count();
	Mtl*	GetMtl(int id);

	Tab<Mtl*> mtlTab;
};

using namespace MaxSDK;

// This is the main class for the exporter.

enum ExportType{
	EXPORT_OBJ,
	EXPORT_ANIM,
	EXPORT_RAGDOLL
};

enum ExportAnimType{
	ANIM_TYPE_SKELETON,
	ANIM_TYPE_VERTEX,
	ANIM_TYPE_UV
};

class PIExp : public SceneExport {
public:
	PIExp();
	~PIExp();

	// SceneExport methods
	int    ExtCount();     // Number of extensions supported 
	const TCHAR * Ext(int n);     // Extension #n (i.e. "ASC")
	const TCHAR * LongDesc();     // Long PI description (i.e. "PI Export") 
	const TCHAR * ShortDesc();    // Short PI description (i.e. "PI")
	const TCHAR * AuthorName();    // PI Author name
	const TCHAR * CopyrightMessage();   // PI Copyright message 
	const TCHAR * OtherMessage1();   // Other message #1
	const TCHAR * OtherMessage2();   // Other message #2
	unsigned int Version();     // Version number * 100 (i.e. v3.01 = 301) 
	void	ShowAbout(HWND hWnd);  // Show DLL's "About..." box
	int		DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0); // Export	file
	BOOL	SupportsOptions(int ext, DWORD options);

	// Other methods

	// Node enumeration
	void	PreProcess(INode* node, int& nodeCount);

	// High level export
	void	ExportMainSkeleton();
	void	ExportAnimation(const char *name, int startFrame, int endFrame);
	void	ExportVertexAnimation(const char *name, int startFrame, int endFrame);
	void	ExportUVAnimation(const char *name, int startFrame, int endFrame);

	// Mid level export
	void	ExportMaterial(INode* node, int currSubMeshIdx); 

	// Low level export
	void	DumpJSONMaterial(Mtl *mtl, int currSubMeshIdx);
	void	DumpTexture(Texmap* tex, Class_ID cid, int subNo, float amt, int	indentLevel );
	void	DumpJSONTexture(Mtl *mtl, Texmap *tex, int subNo, int currSubMeshIdx);

	// Misc methods
	TCHAR*	GetMapID(Class_ID cid, int subNo);
	Point3	GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
	BOOL	CheckForAndExportFaceMap(Mtl* mtl, Mesh* mesh, int indentLevel); 
	TCHAR*	FixupName(const TCHAR* name);
	void	CommaScan(TCHAR* buf);
	BOOL	CheckForAnimation(INode* node, BOOL& pos, BOOL& rot, BOOL& scale);
	TriObject*	GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);
	BOOL	IsKnownController(Control* cont);

	// A collection of overloaded value to string converters.
	TSTR	Format(int value);
	TSTR	Format(float value);
	TSTR	Format(Color value);
	TSTR	Format(Point3 value); 
	TSTR	Format(AngAxis value); 
	TSTR	Format(Quat value);
	TSTR	Format(ScaleValue value);

	// Configuration methods
	TSTR	GetCfgFilename();
	BOOL	ReadConfig();
	void	WriteConfig();

	// Interface to member variables
	inline Interface*	GetInterface()		{ return m_ip; }


	inline void SetExportType(ExportType val)		{ m_eExportType = val; }
	inline ExportType GetExportType()				{ return m_eExportType; }

	inline void SetExportAnimType(ExportAnimType val)	{ m_eExportAnimType = val; }
	inline ExportAnimType GetExportAnimType()						{ return m_eExportAnimType; }

	inline void SetUseLowShading(BOOL val)			{ m_bUseLowShading = val; }
	inline BOOL GetUseLowShading()					{ return m_bUseLowShading; }

	inline void SetIncrementExport(BOOL val)		{ m_bIncrementExport = val;	}
	inline BOOL GetIncrementExport()				{ return m_bIncrementExport;}

	inline void SetExportTangent(BOOL val)			{m_export_tangent = val; }
	inline void SetSkeletonSortType(BOOL val)		{ m_skeleton_sort_type = val; }
	inline BOOL GetExportTangent()					{ return m_export_tangent;}

	inline void SetExportSkeletonScale(BOOL val)	{m_export_skeleton_scale = val;}
	inline BOOL GetExportSkeletonScale()			{return m_export_skeleton_scale;}

	inline void SetIfModelBase(BOOL val)			{ m_bModelBase = val;	}
	inline BOOL GetIfModelBase()					{ return m_bModelBase;	}
	inline void SetIfShadingPhong(BOOL val)			{ m_bShadingPhong = val;	}
	inline BOOL GetIfShadingPhong()					{ return m_bShadingPhong;	}
	inline void SetIfSkeleton(BOOL val)				{ m_bSkeleton = val;	}
	inline BOOL GetIfSkeleton()						{ return m_bSkeleton;	}
	inline void SetIfDiffuse(BOOL val)				{ m_bDiffuse = val;	}
	inline BOOL GetIfDiffuse()						{ return m_bDiffuse;	}
	inline void SetIfAlpha(BOOL val)				{ m_bAlpha = val;	}
	inline BOOL GetIfAlpha()						{ return m_bAlpha;	}
	inline void SetIfSpecular(BOOL val)				{ m_bSpecular = val;	}
	inline BOOL GetIfSpecular()						{ return m_bSpecular;	}
	inline void SetIfGlow(BOOL val)					{ m_bGlow = val;	}
	inline BOOL GetIfGlow()							{ return m_bGlow;	}
	inline void SetIfNormal(BOOL val)				{ m_bNormal = val;	}
	inline BOOL GetIfNormal()						{ return m_bNormal;	}
	inline BOOL GetIfExportElement(std::string name)				{ return m_bElements[name];	}
	inline void SetIfExportElement(std::string name, BOOL val)		{ m_bElements[name] = val;	}

	inline void SetExportedAnimName( TCHAR *val )	{ m_strExportedAnimName = val; }
	inline void SetAnimBeginFrame(int val)			{ m_nExportedAnimBeginFrame = val; }
	inline void SetAnimEndFrame(int val)			{ m_nExportedAnimEndFrame = val; }
	inline void SetAnimInterval(int val)			{ m_nExportedAnimInterval = val; }


private:

	struct PITangentData
	{
		Point4 tangent;
		Point3 binormal;
		DWORD smGroup;
		int uvRotate;
	};

	struct SearchVertexData
	{
		int realVertexIndex;
		Point3 normal;
		Point3 color;
		PITangentData tangent_data;
		std::vector<Point2> uvcoords;
	};



	struct BoneAssign
	{
		INT16 boneID;
		float weight;
	};

	struct PIVertexData
	{
		Point3 pos;
		Point3 normal;
		Point4 tangent;
		Point3 color;
		std::vector<Point2> uvcoords;

		std::vector<BoneAssign> bongAssigns;
	};

	int _getBoneIndex(const MCHAR *boneName);

	static int _compare_func(const void *a, const void *b) { return *(( int *)a) - *(( int *)b); }

	void _exportKeyframes(FILE *fp, INode *thisNode, Tab<TimeValue> &keyTimes, Interval &interval);

	bool _isRootNode(INode *node);

	int _doesVertexExist(std::map<int,std::vector<SearchVertexData>> &searchTable, int vertexIndex, Point3 &normal, Point3 &color, std::vector<Point2> &uvcoords, PITangentData &tangent);

	ISkin* _getSkin(INode *node);

	//记录骨骼
	void _markBone(INode *node);
	void _markBone(INode *node, TimeValue entry);
	void PIExp::_markMeshBones(INode* node, TimeValue t);

	//求得骨骼的roots
	void _getHierarchyRoots();
	void _getSubMeshs(INode *node,int keyTime, std::vector<INode*> *outMeshNodes);

	//获取node是否是skin的一部分
	bool _isPartOfModifier( INode *node );

	bool _isPelvis(INode *node);
	bool _isBoneObj(INode *node);
	bool _isBipedObj(INode *node);

	bool _isHelperObj(INode *node);

	int _exportObj();
	int _exportAnimation();
	int _exportRagdoll();

	Matrix3 _flipYZTransform(Matrix3 tm);

	AngAxis _toAngleAxis(const Quat &q);

    INT8 _isScale(Point3 *scale);

	//构建骨骼的json结构体
	cJSON* _buildSKJson( const std::string &absPath );

	//生成导出用的顶点数据
	BOOL _genPiVertexData(Mesh *mesh, INode *node, const Matrix3 &tm, std::vector<PIVertexData> &outVertexDatas, std::vector<int> &outFaces);
	//导出模型到指定文件
	void _exportModelToFile(const std::string &strFileName, INode *node, std::vector<INode*> *outMeshNodes, TimeValue t, bool recursion, bool onlyModel, float* aabb);
	BOOL _exportModelData(INode *node, TimeValue t, bool recursion, FILE *modelFP , std::vector<INode*> *outMeshNodes, bool onlyModel, float aabb[6]);
	//导出顶点动画数据
	void _exportVertexAnimInfo(cJSON *rootJSON, std::vector<INode *> &meshNodes);
	//是否有顶点动画
	bool _hasVertexAnim(std::vector<INode*> &meshNodes);
	//生成face的切线
	PITangentData _genFaceTangent(Mesh *mesh, int faceIdx, const Matrix3 &tm, BOOL* error);

	//检测一个node的材质是否有bump tex
	bool _hasBumpTex(INode *node);

private:
	

	std::map< std::wstring, int > m_boneIndexMap;
	int m_nCurrentBoneIndex;

	std::vector<std::wstring> m_helperNames;

	std::vector<std::wstring> m_rootBoneNames;

	std::vector<std::pair<std::wstring, std::vector<std::wstring>>> m_boneGroups;


	//是否处于导出动作的模式
	//BOOL	m_bExportAnim;
	ExportType m_eExportType;
	ExportAnimType m_eExportAnimType;
	
	std::wstring m_strExportedAnimName;
	int		m_nExportedAnimBeginFrame;
	int		m_nExportedAnimEndFrame;
	int		m_nExportedAnimInterval;

	//导出模型是否使用最低阶着色
	BOOL	m_bUseLowShading;

	BOOL	m_bIncrementExport;

	BOOL	m_export_tangent;
	BOOL	m_skeleton_sort_type;
	BOOL	m_export_skeleton_scale;

	hash_map<string, BOOL> m_bElements;
	BOOL	m_bModelBase;
	BOOL	m_bShadingPhong;
	BOOL	m_bSkeleton;
	BOOL	m_bDiffuse;
	BOOL	m_bAlpha;
	BOOL	m_bSpecular;
	BOOL	m_bGlow;
	BOOL	m_bNormal;

	Interface*	m_ip;

	size_t m_nSkeletonSizeOffset;

	FILE *m_pDebugFile;
	cJSON *m_pJSONRoot;
	FILE *m_pSkeletonStream;

	std::string m_strResPath;
	std::string m_strName;
	std::string m_strAbsPath;

	MaxSDK::Util::TextFile::Writer m_pStream;
	int			m_nTotalNodeCount;
	int			m_nCurNode;
	TCHAR		m_szFmtStr[16];

	MtlKeeper	m_mtlList;
};

#endif // __ASCIIEXP__H

