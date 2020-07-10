//************************************************************************** 
//* Asciiexp.cpp	- PI File Exporter 
//***************************************************************************

#include "piexp.h"
#include "stdint.h"
#include "3dsmaxport.h"
#include "SlimXml.h"
#include "util.h"
#include "fstream"

HINSTANCE hInstance;

static BOOL showPrompts;
static BOOL exportSelected;

// Class ID. These must be unique and randomly generated!!
// If you use this as a sample project, this is the first thing
// you should change!
#define ASCIIEXP_CLASS_ID	Class_ID(0x72fa08e2, 0x2e333654)

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
   if( fdwReason == DLL_PROCESS_ATTACH )
   {
      MaxSDK::Util::UseLanguagePackLocale();
      hInstance = hinstDLL;
      DisableThreadLibraryCalls(hInstance);
   }

	return (TRUE);
}


__declspec( dllexport ) const TCHAR* LibDescription() 
{
	return GetString(IDS_LIBDESCRIPTION);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS 
__declspec( dllexport ) int LibNumberClasses() 
{
	return 1;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
	case 0: return GetAsciiExpDesc();
	default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion() 
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

class AsciiExpClassDesc:public ClassDesc {
public:
	int				IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new PIExp; } 
	const TCHAR*	ClassName() { return GetString(IDS_ASCIIEXP); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; } 
	Class_ID		ClassID() { return ASCIIEXP_CLASS_ID; }
	const TCHAR*	Category() { return GetString(IDS_CATEGORY); }
};

static AsciiExpClassDesc AsciiExpDesc;

ClassDesc* GetAsciiExpDesc()
{
	return &AsciiExpDesc;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, _countof(buf)) ? buf : NULL;

	return NULL;
}

PIExp::PIExp()
{
	// These are the default values that will be active when 
	// the exporter is ran the first time.
	// After the first session these options are sticky.
	m_nExportedAnimInterval = 0;

	m_bUseLowShading = FALSE;

	m_nCurrentBoneIndex = 0;
	m_eExportType = EXPORT_OBJ;
	m_bUseLowShading = 0;

	m_pSkeletonStream = NULL;

}

PIExp::~PIExp()
{
}

int PIExp::ExtCount()
{
	return 1;
}

const TCHAR * PIExp::Ext(int n)
{
	switch(n) {
	case 0:
		// This cause a static string buffer overwrite
		// return GetString(IDS_EXTENSION1);
		return _T("model");
	}
	return _T("");
}

const TCHAR * PIExp::LongDesc()
{
	return GetString(IDS_LONGDESC);
}

const TCHAR * PIExp::ShortDesc()
{
	return GetString(IDS_SHORTDESC);
}

const TCHAR * PIExp::AuthorName() 
{
	return _T("Christer Janson");
}

const TCHAR * PIExp::CopyrightMessage() 
{
	return GetString(IDS_COPYRIGHT);
}

const TCHAR * PIExp::OtherMessage1() 
{
	return _T("");
}

const TCHAR * PIExp::OtherMessage2() 
{
	return _T("");
}

unsigned int PIExp::Version()
{
	return 100;
}

static INT_PTR CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg, 
	WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		CenterWindow(hWnd, GetParent(hWnd)); 
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hWnd, 1);
			break;
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}       

void PIExp::ShowAbout(HWND hWnd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}


// Dialog proc
static INT_PTR CALLBACK ExportDlgProc(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	Interval animRange;
	ISpinnerControl  *spin;
	ExportAnimType animType;

	PIExp *exp = DLGetWindowLongPtr<PIExp*>(hWnd); 
	switch (msg) {
	case WM_INITDIALOG:
		exp = (PIExp*)lParam;
		DLSetWindowLongPtr(hWnd, lParam); 
		CenterWindow(hWnd, GetParent(hWnd)); 
		CheckDlgButton(hWnd, IDC_RADIO_OBJ, exp->GetExportType() == EXPORT_OBJ);
		CheckDlgButton(hWnd, IDC_RADIO_SKELETON, TRUE);
		CheckDlgButton(hWnd, IDC_CHECK_USE_LOW_SHADING, exp->GetUseLowShading()); 
		SetDlgItemText(hWnd, IDC_INTERVAL, L"1");
		
		animRange = exp->GetInterface()->GetAnimRange();
		// Setup the spinner controls 
		// We take the frame 0 as the default value
		spin = GetISpinner(GetDlgItem(hWnd, IDC_ANIM_BEGIN_SPIN)); 
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_BEGIN_FRAME), EDITTYPE_INT ); 
		spin->SetLimits(animRange.Start() / GetTicksPerFrame(), animRange.End() / GetTicksPerFrame(), TRUE); 
		spin->SetScale(1.0f);
		spin->SetValue(0 ,FALSE);
		ReleaseISpinner(spin);

		spin = GetISpinner(GetDlgItem(hWnd, IDC_ANIM_END_SPIN)); 
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_END_FRAME), EDITTYPE_INT ); 
		spin->SetLimits(animRange.Start() / GetTicksPerFrame(), animRange.End() / GetTicksPerFrame(), TRUE); 
		spin->SetScale(1.0f);
		spin->SetValue(0, FALSE);
		ReleaseISpinner(spin);

		// Enable / disable options
		EnableWindow(GetDlgItem(hWnd, IDC_ANIM_BEGIN_SPIN), exp->GetExportType() == EXPORT_ANIM);
		EnableWindow(GetDlgItem(hWnd, IDC_BEGIN_FRAME), exp->GetExportType() == EXPORT_ANIM);
		EnableWindow(GetDlgItem(hWnd, IDC_ANIM_END_SPIN), exp->GetExportType() == EXPORT_ANIM);
		EnableWindow(GetDlgItem(hWnd, IDC_END_FRAME), exp->GetExportType() == EXPORT_ANIM);
		EnableWindow(GetDlgItem(hWnd, IDC_EXPORT_ANIM_NAME), exp->GetExportType() == EXPORT_ANIM);
		EnableWindow(GetDlgItem(hWnd, IDC_INTERVAL), exp->GetExportType() == EXPORT_ANIM);
		EnableWindow(GetDlgItem(hWnd, IDC_RADIO_SKELETON), exp->GetExportType() == EXPORT_ANIM);
		EnableWindow(GetDlgItem(hWnd, IDC_RADIO_VERTEX), exp->GetExportType() == EXPORT_ANIM);
		EnableWindow(GetDlgItem(hWnd, IDC_RADIO_UV), exp->GetExportType() == EXPORT_ANIM);


		EnableWindow(GetDlgItem(hWnd, IDC_CHECK_SHADING_PHONG), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
		EnableWindow(GetDlgItem(hWnd, IDC_CHECK_SKELETON), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
		EnableWindow(GetDlgItem(hWnd, IDC_CHECK_DIFFUSE), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
		EnableWindow(GetDlgItem(hWnd, IDC_CHECK_ALPHA), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
		EnableWindow(GetDlgItem(hWnd, IDC_CHECK_SPECULAR), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
		EnableWindow(GetDlgItem(hWnd, IDC_CHECK_NORMAL), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
		EnableWindow(GetDlgItem(hWnd, IDC_CHECK_GLOW), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
		EnableWindow(GetDlgItem(hWnd, IDC_CHECK_MODEL_BASE), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
		break;

	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam; 
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RADIO_ANIMATION:
		case IDC_RADIO_SKELETON:
		case IDC_RADIO_VERTEX:
		case IDC_RADIO_UV:
		case IDC_RADIO_RAGDOLL:
		case IDC_RADIO_OBJ:
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_SCALE), IsDlgButtonChecked(hWnd, IDC_RADIO_SKELETON) && IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION));
			EnableWindow(GetDlgItem(hWnd, IDC_ANIM_BEGIN_SPIN), IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION ));
			EnableWindow(GetDlgItem(hWnd, IDC_BEGIN_FRAME), IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION ));
			EnableWindow(GetDlgItem(hWnd, IDC_ANIM_END_SPIN), IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION ));
			EnableWindow(GetDlgItem(hWnd, IDC_END_FRAME), IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION ));
			EnableWindow(GetDlgItem(hWnd, IDC_EXPORT_ANIM_NAME), IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION ));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_USE_LOW_SHADING), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ ));
			EnableWindow(GetDlgItem(hWnd, IDC_INTERVAL), IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION ));
			EnableWindow(GetDlgItem(hWnd, IDC_RADIO_SKELETON), IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION ));
			EnableWindow(GetDlgItem(hWnd, IDC_RADIO_VERTEX), IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION ));
			EnableWindow(GetDlgItem(hWnd, IDC_RADIO_UV), IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION ));


			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_INCREMENT_EXPORT), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK2_TANGENT), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK2_SORT), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ) || IsDlgButtonChecked(hWnd, IDC_RADIO_RAGDOLL));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_SHADING_PHONG), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ) && IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_SKELETON), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ) && IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_DIFFUSE), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ) && IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_ALPHA), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ) && IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_SPECULAR), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ) && IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_NORMAL), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ) && IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_GLOW), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ) && IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_MODEL_BASE), IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ) && IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			break;
		case IDOK:

			//设置 导出动画 模式
			
			if (IsDlgButtonChecked(hWnd, IDC_RADIO_ANIMATION)){
				exp->SetExportType(EXPORT_ANIM);
			}
			else if (IsDlgButtonChecked(hWnd, IDC_RADIO_OBJ)){
				exp->SetExportType(EXPORT_OBJ);
			}
			else if (IsDlgButtonChecked(hWnd, IDC_RADIO_RAGDOLL))
			{
				exp->SetExportType(EXPORT_RAGDOLL);
			}
			else{
				return FALSE;
			}

			if(IsDlgButtonChecked(hWnd, IDC_RADIO_SKELETON ))
			{
				animType = ANIM_TYPE_SKELETON;
			}
			else
			{
				animType = IsDlgButtonChecked(hWnd, IDC_RADIO_VERTEX )?ANIM_TYPE_VERTEX:ANIM_TYPE_UV;
			}

			exp->SetExportAnimType( animType );
			exp->SetUseLowShading( IsDlgButtonChecked(hWnd, IDC_CHECK_USE_LOW_SHADING));

			//设置增量导出类型
			exp->SetIncrementExport( IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			exp->SetExportTangent( IsDlgButtonChecked(hWnd, IDC_CHECK2_TANGENT));
			exp->SetSkeletonSortType(IsDlgButtonChecked(hWnd, IDC_CHECK2_SORT));
			exp->SetExportSkeletonScale(IsDlgButtonChecked(hWnd, IDC_CHECK_SCALE));
			exp->SetIfExportElement(MODEL_BASE_ELEMENT,		IsDlgButtonChecked(hWnd, IDC_CHECK_MODEL_BASE));
			exp->SetIfExportElement(DIFFUSE_MAP_ELEMENTS,	IsDlgButtonChecked(hWnd, IDC_CHECK_DIFFUSE));
			exp->SetIfExportElement(SKELETON_ELEMENT,		IsDlgButtonChecked(hWnd, IDC_CHECK_SKELETON));
			exp->SetIfExportElement(SHADING_PHONG_ELEMENTS,	IsDlgButtonChecked(hWnd, IDC_CHECK_SHADING_PHONG));
			exp->SetIfExportElement(ALPHA_MAP_ELEMENTS,		IsDlgButtonChecked(hWnd, IDC_CHECK_ALPHA));
			exp->SetIfExportElement(SPECULAR_MAP_ELEMENTS,	IsDlgButtonChecked(hWnd, IDC_CHECK_SPECULAR));
			exp->SetIfExportElement(GLOW_MAP_ELEMENTS,		IsDlgButtonChecked(hWnd, IDC_CHECK_GLOW));
			exp->SetIfExportElement(NORMAL_MAP_ELEMENTS,	IsDlgButtonChecked(hWnd, IDC_CHECK_NORMAL));

			//设置动画名字
			TCHAR name[MAX_PATH];
			GetDlgItemText(hWnd, IDC_EXPORT_ANIM_NAME, name, MAX_PATH);
			exp->SetExportedAnimName( name );

			//动作的起始和终止帧
			spin = GetISpinner(GetDlgItem(hWnd, IDC_ANIM_BEGIN_SPIN)); 
			exp->SetAnimBeginFrame(spin->GetIVal());
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd, IDC_ANIM_END_SPIN)); 
			exp->SetAnimEndFrame(spin->GetIVal());
			ReleaseISpinner(spin);

			//设置采样率
			TCHAR szInterval[MAX_PATH];
			GetDlgItemText(hWnd, IDC_INTERVAL, szInterval, MAX_PATH);
			
			exp->SetAnimInterval(wcstol(szInterval, NULL, 10));
			
			EndDialog(hWnd, 1);
			break;
		case IDC_CHECK_INCREMENT_EXPORT:
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_SHADING_PHONG), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_SKELETON), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_DIFFUSE), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_ALPHA), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_SPECULAR), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_NORMAL), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_GLOW), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_MODEL_BASE), IsDlgButtonChecked(hWnd, IDC_CHECK_INCREMENT_EXPORT));
			break;
		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}       

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}

int PIExp::_exportAnimation()
{
	char mb[MAX_FILE_PATH];
	std::size_t l;
	wcstombs_s(&l, mb, MAX_FILE_PATH, m_strExportedAnimName.c_str(), MAX_FILE_PATH);

	if (m_eExportAnimType == ANIM_TYPE_SKELETON) {
		ExportAnimation(mb, m_nExportedAnimBeginFrame, m_nExportedAnimEndFrame);

		m_ip->ProgressEnd();
		return 1;
	}
	else if (m_eExportAnimType == ANIM_TYPE_VERTEX)
	{
		//导出顶点动画数据
		ExportVertexAnimation(mb, m_nExportedAnimBeginFrame, m_nExportedAnimEndFrame);
	}
	else if (m_eExportAnimType == ANIM_TYPE_UV)
	{
		ExportUVAnimation(mb, m_nExportedAnimBeginFrame, m_nExportedAnimEndFrame);
	}
	return 1;
}

int PIExp::_exportObj()
{
	if (m_bIncrementExport){
		std::string strJsonName = m_strAbsPath + m_strName + ".obj\\" + m_strName + ".json";
		FILE *jsonFP = fopen(strJsonName.c_str(), "r");
		if (jsonFP == NULL)
			return 1;
		int lSize;
		char* buffer;

		fseek(jsonFP, 0, SEEK_END);
		lSize = ftell(jsonFP);
		rewind(jsonFP);
		buffer = (char*)malloc(sizeof(char)*lSize);
		fread(buffer, 1, lSize, jsonFP);
		fclose(jsonFP);
		m_pJSONRoot = cJSON_Parse(buffer);

		if (m_bElements[MODEL_BASE_ELEMENT])
		{
			cJSON_DeleteItemFromObject(m_pJSONRoot, MODEL_BASE_ELEMENT);
		}
		if (m_bElements[SKELETON_ELEMENT])
		{
			cJSON_DeleteItemFromObject(m_pJSONRoot, SKELETON_ELEMENT);
		}
		if (m_bElements[SHADING_PHONG_ELEMENTS])
		{
			cJSON_DeleteItemFromObject(m_pJSONRoot, SHADING_PHONG_ELEMENTS);
		}
		if (m_bElements[DIFFUSE_MAP_ELEMENTS])
		{
			cJSON_DeleteItemFromObject(m_pJSONRoot, DIFFUSE_MAP_ELEMENTS);
		}
		if (m_bElements[ALPHA_MAP_ELEMENTS])
		{
			cJSON_DeleteItemFromObject(m_pJSONRoot, ALPHA_MAP_ELEMENTS);
		}
		if (m_bElements[SPECULAR_MAP_ELEMENTS])
		{
			cJSON_DeleteItemFromObject(m_pJSONRoot, SPECULAR_MAP_ELEMENTS);
		}
		if (m_bElements[GLOW_MAP_ELEMENTS])
		{
			cJSON_DeleteItemFromObject(m_pJSONRoot, GLOW_MAP_ELEMENTS);
		}
		if (m_bElements[NORMAL_MAP_ELEMENTS])
		{
			cJSON_DeleteItemFromObject(m_pJSONRoot, NORMAL_MAP_ELEMENTS);
		}
	}
	else
	{
		createDir(m_strAbsPath + m_strName + ".obj");
		//写json
		m_pJSONRoot = cJSON_CreateObject();
	}


	if (m_bIncrementExport&&m_bElements[MODEL_BASE_ELEMENT] || !m_bIncrementExport){
		cJSON *pModelBaseElement = cJSON_CreateObject();
		cJSON_AddItemToObject(m_pJSONRoot, MODEL_BASE_ELEMENT, pModelBaseElement);
		cJSON_AddStringToObject(pModelBaseElement, "elementType", MODEL_BASE_ELEMENT);
		cJSON_AddStringToObject(pModelBaseElement, "meshPath", (m_strResPath + m_strName + ".obj/" + m_strName + ".model").c_str());
	}

	// Get a total node count by traversing the scene
	// We don't really need to do this, but it doesn't take long, and
	// it is nice to have an accurate progress bar.
	m_nTotalNodeCount = 0;
	m_nCurNode = 0;
	PreProcess(m_ip->GetRootNode(), m_nTotalNodeCount);

	//先记录骨骼
	//_markMeshBones(m_ip->GetRootNode(), m_ip->GetAnimRange().Start());
	_markBone(m_ip->GetRootNode(), m_ip->GetAnimRange().Start());

	std::vector<INode*> meshNodes;

	std::string strModelName = m_strName + ".obj/" + m_strName + ".model";
	float aabb[6] = { 0 };
	_exportModelToFile(strModelName, m_ip->GetRootNode(), &meshNodes, 0, true, false, aabb);
	cJSON *pModelBaseElement = cJSON_GetObjectItem(m_pJSONRoot, MODEL_BASE_ELEMENT);
	cJSON *pAabb = cJSON_CreateObject();
	cJSON_AddItemToObject(pModelBaseElement, "aabb", pAabb);

	cJSON *pMinPt = cJSON_CreateObject();
	cJSON_AddItemToObject(pAabb, "min", pMinPt);
	cJSON_AddNumberToObject(pMinPt, "x", aabb[0]);
	cJSON_AddNumberToObject(pMinPt, "y", aabb[1]);
	cJSON_AddNumberToObject(pMinPt, "z", aabb[2]);

	cJSON *pMaxPt = cJSON_CreateObject();
	cJSON_AddItemToObject(pAabb, "max", pMaxPt);
	cJSON_AddNumberToObject(pMaxPt, "x", aabb[3]);
	cJSON_AddNumberToObject(pMaxPt, "y", aabb[4]);
	cJSON_AddNumberToObject(pMaxPt, "z", aabb[5]);


	//导出顶点动画数据
	//_exportVertexAnimInfo(m_pJSONRoot, meshNodes);

	//导出骨骼数据
	if (m_bIncrementExport&&m_bElements[SKELETON_ELEMENT] || !m_bIncrementExport)
	{
		ExportMainSkeleton();
	}


	// Close the stream
	//m_pStream.Close();

	std::string strResJSON = cJSON_Print(m_pJSONRoot);
	FILE *fp = fopen((m_strName + ".obj/" + m_strName + ".json").c_str(), "wb");
	fwrite(strResJSON.c_str(), sizeof(char), strResJSON.size(), fp);
	fclose(fp);
	cJSON_Delete(m_pJSONRoot);

	// Write the current options to be used next time around.
	WriteConfig();
	return 1;
}

int PIExp::_exportRagdoll()
{
	using namespace slim;
	XmlDocument doc;
	ifstream inputStream;
	inputStream.open((m_strName + ".obj/" + m_strName + ".Repx").c_str(), ios::in | ios::binary);
	if (!inputStream.is_open())
	{
		return 0;
	}
	doc.loadFromStream(inputStream);

	_markBone(m_ip->GetRootNode(), m_ip->GetAnimRange().Start());
	std::string ragdollPath = m_strName + ".obj/" + m_strName + ".ragdoll";
	FILE* outputStream = fopen(ragdollPath.c_str(), "wb");
	NodeIterator it;

	XmlNode* PhysX30Collection = doc.findFirstChild(L"PhysX30Collection", it);
	XmlNode* node = PhysX30Collection->findFirstChild(L"PxRigidDynamic", it);
	uint32_t nodeCount = 0;
	fwrite(&nodeCount, sizeof(uint32_t), 1, outputStream);
	while (node != nullptr)
	{
		XmlNode* idNode = node->findChild(L"Id");
		int64_t actorId = idNode->getInt64();
		XmlNode* nameNode = node->findChild(L"Name");
		wchar_t* name = (wchar_t*)(nameNode->getString());
		int boneIndex = _getBoneIndex(name);
		if (m_nCurrentBoneIndex - 1 == boneIndex)
		{
			return 0;
		}
		uint32_t nameSize = (uint32_t)((1 + wcslen(name))* sizeof(wchar_t));
		fwrite(&nameSize, sizeof(nameSize), 1, outputStream);
		fwrite(name, nameSize, 1, outputStream);
		fwrite(&actorId, sizeof(int64_t), 1, outputStream);
		fwrite(&boneIndex, sizeof(int), 1, outputStream);
		nodeCount++;  
		node = PhysX30Collection->findNextChild(L"PxRigidDynamic", it);
	}


	inputStream.seekg(0, ios::end);
	uint32_t size = inputStream.tellg();
	std::vector<char> buffer(size);
	inputStream.seekg(0, ios::beg);
	inputStream.read(&buffer[0], size);
	inputStream.close();
	fwrite(&size, sizeof(uint32_t), 1, outputStream);
	fwrite(&buffer[0], size, 1, outputStream);
	size = ftell(outputStream);
	fseek(outputStream, 0, SEEK_SET);
	fwrite(&nodeCount, sizeof(uint32_t), 1, outputStream);
	fseek(outputStream, size, SEEK_SET);
	fclose(outputStream);




	std::string strJsonName = m_strAbsPath + m_strName + ".obj\\" + m_strName + ".json";
	FILE *jsonFP = fopen(strJsonName.c_str(), "r");
	if (jsonFP == NULL)
		return 1;
	int lSize;

	fseek(jsonFP, 0, SEEK_END);
	lSize = ftell(jsonFP);
	rewind(jsonFP);
	char* jsonBuffer = (char*)malloc(sizeof(char)*lSize);



	fread(jsonBuffer, 1, lSize, jsonFP);
	fclose(jsonFP);
	cJSON* json = cJSON_Parse(jsonBuffer);
	cJSON* ragdollJSON = cJSON_GetObjectItem(json, "ragdollPath");
	if (ragdollJSON != NULL)
	{
		cJSON_DeleteItemFromObject(json, "ragdollPath");
	}
	ragdollJSON = cJSON_CreateString((m_strResPath + ragdollPath).c_str());
	cJSON_AddItemToObject(json, "ragdollPath", ragdollJSON);
	free(jsonBuffer);
	char* str = cJSON_Print(json);
	std::string ss = str;
	free(str);
	jsonFP = fopen(strJsonName.c_str(), "wb");
	fwrite(ss.c_str(), sizeof(char), ss.size(), jsonFP);

	fclose(jsonFP);
	return 1;
}

// Start the exporter!
// This is the real entrypoint to the exporter. After the user has selected
// the filename (and he's prompted for overwrite etc.) this method is called.
int PIExp::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) 
{
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;
	exportSelected = (options & SCENE_EXPORT_SELECTED) ? TRUE : FALSE;

	// Grab the interface pointer.
	m_ip = i;

	// Get the options the user selected the last time
	ReadConfig();

	

	if(showPrompts) {
		// Prompt the user with our dialogbox, and get all the options.
		if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ASCIIEXPORT_DLG),
			m_ip->GetMAXHWnd(), ExportDlgProc, (LPARAM)this)) {
			return 1;
			}
		}
	
	// Open the stream
   	Interface14 *iface = GetCOREInterface14();
	UINT codepage  = iface-> DefaultTextSaveCodePage(true);

	TCHAR szName[MAX_FILE_PATH];
	_stprintf(szName, _T("%s%s"), name, _T(".log"));
	//if(!m_pStream.Open(szName, false, MaxSDK::Util::TextFile::Writer::WRITE_BOM | codepage )) return 0;

	m_strResPath = extractRelPath(name);
	m_strName = extractName(name);
	m_strAbsPath = extractAbsPath(name);

	// Startup the progress bar.
	m_ip->ProgressStart(GetString(IDS_PROGRESS_MSG), TRUE, fn, NULL);

	switch (m_eExportType)
	{
	case EXPORT_OBJ:
		_exportObj();
		break;
	case EXPORT_ANIM:
		_exportAnimation();
		break;
	case EXPORT_RAGDOLL:
		_exportRagdoll();
		break;
	default:
		break;
	}
	// We're done. Finish the progress bar.
	m_ip->ProgressEnd();

	return 1;
}

BOOL PIExp::SupportsOptions(int ext, DWORD options) {
	assert(ext == 0);	// We only support one extension
	return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
	}


void PIExp::PreProcess(INode* node, int& nodeCount)
{
	nodeCount++;
	
	// Add the nodes material to out material list
	// Null entries are ignored when added...
	m_mtlList.AddMtl(node->GetMtl());

	// For each child of this node, we recurse into ourselves 
	// and increment the counter until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		PreProcess(node->GetChildNode(c), nodeCount);
	}
}

/****************************************************************************

 Configuration.
 To make all options "sticky" across sessions, the options are read and
 written to a configuration file every time the exporter is executed.

 ****************************************************************************/

TSTR PIExp::GetCfgFilename()
{
	TSTR filename;
	
	filename += m_ip->GetDir(APP_PLUGCFG_DIR);
	filename += _T("\\");
	filename += CFGFILENAME;

	return filename;
}

// NOTE: Update anytime the CFG file changes
#define CFG_VERSION 0x03

BOOL PIExp::ReadConfig()
{
	TSTR filename = GetCfgFilename();
	FILE* cfgStream;

	cfgStream = _tfopen(filename, _T("rb"));
	if (!cfgStream)
		return FALSE;

	// First item is a file version
	int fileVersion = _getw(cfgStream);

	if (fileVersion > CFG_VERSION) {
		// Unknown version
		fclose(cfgStream);
		return FALSE;
	}


	fclose(cfgStream);

	return TRUE;
}

void PIExp::WriteConfig()
{
	TSTR filename = GetCfgFilename();
	FILE* cfgStream;

	cfgStream = _tfopen(filename, _T("wb"));
	if (!cfgStream)
		return;

	// Write CFG version
	_putw(CFG_VERSION,				cfgStream);

	fclose(cfgStream);
}


BOOL MtlKeeper::AddMtl(Mtl* mtl)
{
	if (!mtl) {
		return FALSE;
	}

	int numMtls = mtlTab.Count();
	for (int i=0; i<numMtls; i++) {
		if (mtlTab[i] == mtl) {
			return FALSE;
		}
	}
	mtlTab.Append(1, &mtl, 25);

	return TRUE;
}

int MtlKeeper::GetMtlID(Mtl* mtl)
{
	int numMtls = mtlTab.Count();
	for (int i=0; i<numMtls; i++) {
		if (mtlTab[i] == mtl) {
			return i;
		}
	}
	return -1;
}

int MtlKeeper::Count()
{
	return mtlTab.Count();
}

Mtl* MtlKeeper::GetMtl(int id)
{
	return mtlTab[id];
}
