#ifndef _Physics_Offline_Tools_H_
#define _Physics_Offline_Tools_H_
#include "pi_lib.h"
#include "physics_scene.h"
#include "physics_mesh.h"





PI_BEGIN_DECLS

HeightFieldData* PI_API pi_physics_tools_cook_height_field(int nx, int nz, float* heightBuffer);


PI_END_DECLS

#endif