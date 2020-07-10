#include "DinoUIEngineInclude.h"
#include "DinoUIPIRenderer.h"

DinoUIEngine *gpUIEngine = DinoUIEngine::GetUIEngine();

// ³õÊ¼»¯
extern "C" int DinoUI_Initialize()
{
	DinoUIEngine::DinoUIEngineCreateParameter tCreateParameter;
	tCreateParameter.mstrWorkingDirectory = _DT("./Test");
	tCreateParameter.mnScreenWidth = 1440;
	tCreateParameter.mnScreenHeight = 900;

	tCreateParameter.mpRenderer = new DinoUIPIRenderer();
	gpUIEngine->Initialize(&tCreateParameter);

	return 0;
}

extern "C" void DinoUI_Finalize()
{
	gpUIEngine->Finalize();
}

extern "C" void DinoUI_Update(const float vfElapsedTime)
{
	gpUIEngine->Update(vfElapsedTime);
}

extern "C" void DinoUI_Render()
{
	gpUIEngine->Render();
}