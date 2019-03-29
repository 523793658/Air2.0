#include "Classes/Engine/Player.h"
#include "SimpleReflection.h"
namespace Air
{
	Player::Player(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	DECLARE_SIMPLER_REFLECTION(Player);
}