#include "Layout/Geometry.h"
namespace Air
{
	Geometry::Geometry()
		:mSize(0.0f, 0.0f)
		, mScale(1.0f)
		, mAbsolutePosition(0.0f, 0.0f)
	{

	}

	Geometry& Geometry::operator=(const Geometry& rhs)
	{
		if (this != &rhs)
		{
			Memory::memcpy(*this, rhs);
		}
		return *this;
	}
}