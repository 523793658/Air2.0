#include "Math/Float16.h"
#include "Math/Color.h"
namespace Air
{
	class Float16Color
	{
	public:
		float16 R;
		float16 G;
		float16 B;
		float16 A;

		Float16Color();

		Float16Color(const Float16Color& src);

		Float16Color(const LinearColor& src);

		Float16Color& operator = (const Float16Color& src);

		bool operator == (const Float16Color& src);
	};

	FORCEINLINE Float16Color::Float16Color() {}

	FORCEINLINE Float16Color::Float16Color(const Float16Color& src)
	{
		R = src.R;
		G = src.G;
		B = src.B;
		A = src.A;
	}

	FORCEINLINE Float16Color::Float16Color(const LinearColor& src) :
		R(src.R)
		,G(src.G)
		,B(src.B)
		,A(src.A)
	{

	}

	FORCEINLINE Float16Color& Float16Color::operator =(const Float16Color& src)
	{
		R = src.R;
		G = src.G;
		B = src.B;
		A = src.A;
		return *this;
	}

	FORCEINLINE bool Float16Color::operator ==(const Float16Color& src)
	{
		return (
			(R == src.R) &&
			(G == src.G) &&
			(B == src.B) &&
			(A == src.A));
	}
}