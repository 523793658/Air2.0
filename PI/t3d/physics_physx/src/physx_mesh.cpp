#include "physics_wrap.h"
#include "PxDefaultStreams.h"
#include "PxSimpleFactory.h"
#include "PxHeightField.h"
#include "PxPhysics.h"
#include "physx_engine.h"
using namespace physx;
extern "C"
{
	void PI_API physics_height_field_init(HeightField* heightField, void* data, uint size)
	{
		PxDefaultMemoryInputData input(static_cast<PxU8*>(data), size);
		PxHeightField* aheightField = PhysXEngine::getInstance()->getPhysics().createHeightField(input);
		heightField->impl = aheightField;
	}

	void PI_API physics_height_field_free(HeightField* heightField)
	{
		PxHeightField* h = static_cast<PxHeightField*>(heightField->impl);
		h->release();
	}
}