#include "dialog.h"
#include "windows.h"
#include "commdlg.h"
#include "Shlwapi.h"
#include "Shlobj.h"

static void init_struct(OPENFILENAME *ofn, wchar* title, wchar* initPath, wchar* filter, wchar* filePath)
{
    pi_memset(ofn, 0, sizeof(OPENFILENAME));

    //如果路径是目录的话那么就不需要选择到具体文件，反反之亦反
    if (!PathIsDirectory(initPath))
    {
        pi_memcpy_inline(filePath, initPath, pi_wstrlen(initPath)*sizeof(wchar));
    }

    ofn->lStructSize = sizeof(OPENFILENAME);
    ofn->lpstrFilter = filter;
    ofn->nFilterIndex = 1;
    ofn->lpstrFile = filePath;
    ofn->nMaxFile = sizeof(wchar) * 256;
    ofn->lpstrInitialDir = initPath;
    ofn->lpstrTitle = title;
}

wchar* PI_API app_open_file_dialog(wchar* title, wchar* initPath, wchar* filter)
{
    OPENFILENAME ofn;
    wchar* filePath = pi_new0(wchar, 256);
    init_struct(&ofn, title, initPath, filter, filePath);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if(GetOpenFileName(&ofn))
    {
        return filePath;
    }
    return 0;
}

wchar* PI_API app_open_files_dialog(wchar* title, wchar* initPath, wchar* filter)
{
	OPENFILENAME ofn;
	wchar szPath[256];
	wchar* filePath = pi_new0(wchar, 256);
	wchar* result = pi_new0(wchar, 512);
	wchar* pCheck;

	init_struct(&ofn, title, initPath, filter, filePath);
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_NOCHANGEDIR;

	if (GetOpenFileName(&ofn)) 
	{
		lstrcpyn(szPath, filePath, ofn.nFileOffset);
		szPath[ofn.nFileOffset] = TEXT('\0');
		pCheck = filePath + ofn.nFileOffset;
		while (*pCheck)
		{
			lstrcat(result, szPath); //给文件名加上路径
			lstrcat(result, TEXT("\\"));
			lstrcat(result, pCheck); //加上文件名
			lstrcat(result, TEXT(",")); //,分割
			pCheck += lstrlen(pCheck) + 1;
		}
		return result;
	}
	return 0;
}

wchar* PI_API app_save_file_dialog(wchar* title, wchar* initPath, wchar* filter)
{
    OPENFILENAME ofn;
    wchar* filePath = pi_new0(wchar, 256);
    init_struct(&ofn, title, initPath, filter, filePath);

	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = L"";
    if(GetSaveFileName(&ofn))
    {
        return filePath;
    }
    return 0;
}

uint PI_API app_color_dialog(uint iniColor)
{
    CHOOSECOLOR cc = {0};
    COLORREF rgbResult = iniColor;
    static COLORREF g_rgbCustom[16] = {0};
    cc.lpCustColors = g_rgbCustom;
    cc.rgbResult = rgbResult;
    cc.lStructSize = sizeof(cc);
	cc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ANYCOLOR;
    if(ChooseColor(&cc)){
        rgbResult = cc.rgbResult;
    }
    return rgbResult;
}


wchar* PI_API app_browse_directory_dialog(wchar* title, wchar* initDir)
{
    BROWSEINFO bi; 
    LPITEMIDLIST idl;
    wchar* dir = pi_new0(wchar, MAX_PATH);
    SHILCreateFromPath(initDir,  &idl, NULL );

    ZeroMemory(&bi,sizeof(BROWSEINFO)); 
    bi.hwndOwner = NULL; 
    bi.lpszTitle = title;
    bi.pidlRoot = idl;
	bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE | OFN_NOCHANGEDIR;
    idl = SHBrowseForFolder(&bi); 

    if (!idl) 
    { 
        return 0;  
    } 

    dir = pi_new0(wchar, 256);
    SHGetPathFromIDList(idl,dir); 

    return dir;
}


