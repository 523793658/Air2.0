#include "ShowFlags.h"
namespace Air
{
	void engineShowFlagOrthographicOverride(bool isPerspective, EngineShowFlags& engineShowFlags)
	{
		engineShowFlags.setDirectLighting(true);
	}
}