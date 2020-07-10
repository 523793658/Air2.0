#include "fade_in_out.h"
#include "particles.h"
static const char* u_FadeProcess = "u_FadeProcess";
typedef struct
{
	float time;
	float currentTime;
	float process;
	FadeType type;
	PiBool isFirst;
	PiBool isKeep;
	PiBool keeping;
	float keepProcess;
}FadeInOut;

static PiBool _update(PiController* c, float tpf)
{
	FadeInOut* impl = (FadeInOut*)(c->impl);
	if (impl->time < 0.000001f)
	{
		return TRUE;
	}
	if (impl->keeping)
	{
		return FALSE;
	}
	impl->currentTime += tpf;
	if (impl->type == FD_IN)
	{
		impl->process = impl->currentTime / impl->time;
	}
	else
	{
		impl->process = 1.0f - impl->currentTime / impl->time;
		if (impl->isKeep && impl->process <= impl->keepProcess)
		{
			impl->process = impl->keepProcess;
			impl->currentTime = (1.0f - impl->process) * impl->time;
			impl->keeping = TRUE;
		}
		
	}
	return impl->currentTime > impl->time;
}
static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	
	FadeInOut* impl;
	impl = c->impl;
	PI_ASSERT(type == CAT_ENTITY || type == CAT_PARTICLE_CLUSTER, "aplly type error");
	if (type == CAT_PARTICLE_CLUSTER)
	{
		ParticleCluster* cluster = (ParticleCluster*)obj;
		pi_particle_cluster_set_global_alpha(cluster, impl->process);
	}
	if (type == CAT_ENTITY)
	{
		PiEntity* entity;
		PiMaterial* material;
		entity = (PiEntity*)obj;
		material = entity->material;
		pi_material_set_uniform(material, u_FadeProcess, UT_FLOAT, 1, &impl->process, TRUE);
	}
	return TRUE;
}

PiController* PI_API app_fade_in_out_new()
{
	FadeInOut* impl = pi_new0(FadeInOut, 1);
	impl->isFirst = TRUE;
	PiController* c = pi_controller_new((ControllerType)CT_FADEINOUT, _apply, _update, impl);
	return c;
}

void PI_API app_fade_in_out_set_params(PiController* c, float time, FadeType type, PiBool iskeep, float keepProcess)
{
	FadeInOut* impl = c->impl;
	impl->time = time;
	impl->isKeep = iskeep;
	impl->keeping = FALSE;
	impl->keepProcess = keepProcess;
	if (impl->isFirst)
	{
		if (type == FD_IN)
		{
			impl->process = 0.0f;
		}
		else
		{
			impl->process = 1.0f;
		}
		impl->currentTime = 0.0f;
	}
	else
	{
		if (type == FD_IN)
		{
			impl->currentTime = impl->process * impl->time;
		}
		else
		{
			impl->currentTime = (1.0f - impl->process) * impl->time;
		}
	}
	impl->type = type;
	impl->isFirst = FALSE;
}

void PI_API app_fade_in_out_play(PiController*c)
{

}

void PI_API app_fade_in_out_free(PiController* c)
{
	FadeInOut* impl = c->impl;
	pi_free(impl);
	pi_controller_free(c);
}