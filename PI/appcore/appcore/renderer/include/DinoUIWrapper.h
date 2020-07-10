#ifndef _DinoUIWrapper_H_
#define _DinoUIWrapper_H_

#ifdef _cplusplus
extern "C"
{
#endif

int DinoUI_Initialize();

void DinoUI_Finalize();

void DinoUI_Update(const float vfElapsedTime);

void DinoUI_Render();

#ifdef _cplusplus
}
#endif

#endif