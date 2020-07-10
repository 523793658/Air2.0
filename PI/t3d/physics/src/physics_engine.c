#include "pi_lib.h"
#include "physics_wrap.h"
#include "physics_engine.h"

PhysicsEngine *g_physics_engine = NULL;

void PI_API pi_physics_engine_init(InitialType type)
{
	g_physics_engine = pi_new0(PhysicsEngine, 1);
	physics_engine_init(g_physics_engine, type, NULL);
}

void PI_API pi_physics_engine_init_by_url(InitialType type, char* url)
{

	g_physics_engine = pi_new0(PhysicsEngine, 1);
	physics_engine_init(g_physics_engine, type, url);
}

void PI_API pi_physics_engine_destroy()
{
	physics_engine_release(g_physics_engine);
	pi_free(g_physics_engine);
}


PiBool PI_API pi_physics_engine_simulate(float tpf)
{
	physics_engine_simulate(g_physics_engine, tpf);
	return TRUE;
}


