#ifndef _PI_MESH_PROG_H_
#define _PI_MESH_PROG_H_
#include "pi_lib.h"

typedef enum
{
	MRDT_NORMAL,
	MRDT_WATER,
	MRDT_TERRAIN
}MeshReduceType;

PI_BEGIN_DECLS

PiMesh* PI_API pi_mesh_prog(PiMesh* mesh, MeshReduceType type);

PI_END_DECLS
#endif