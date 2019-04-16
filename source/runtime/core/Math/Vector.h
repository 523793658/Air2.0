#pragma once
#include "CoreType.h"
#include "Math/MathUtility.h"
#include "Math/Math.h"
#include "Misc/CoreMiscDefines.h"
#include "Serialization/Archive.h"
#include "Math/Color.h"
#include "Math/Float16.h"
namespace Air
{
	template<typename T>
	struct Vector3;

	struct Vector4;

	template<typename T>
	struct Vector2;

	template<typename T>
	struct Vector3
	{
	public:
		T x, y, z;

	public:
		FORCEINLINE Vector3()
		{

		}
		FORCEINLINE Vector3(const T& inX, const T& inY, const T& inZ)
			:x(inX), y(inY), z(inZ)
		{

		}

		FORCEINLINE Vector3(const T v)
			: x(v), y(v), z(v)
		{

		}

		FORCEINLINE Vector3(const Vector3& rhs)
			: x(rhs.x), y(rhs.y), z(rhs.z)
		{

		}

		FORCEINLINE Vector3(const Vector4& rhs)
			: x(rhs.x), y(rhs.y), z(rhs.z)
		{

		}

		FORCEINLINE Vector3(const Vector2<T>& v, T inZ = T(0));

		FORCEINLINE Vector3(EForceInit forceInit)
			: x(0), y(0), z(0)
		{

		}

		float& operator[] (int32 index)
		{
			BOOST_ASSERT(index >= 0 && index < 3);
			if (index == 0)
			{
				return x;
			}
			else if (index == 1)
			{
				return y;
			}
			else
			{
				return z;
			}
		}

		float operator[] (int32 index) const
		{
			BOOST_ASSERT(index >= 0 && index < 3);
			if (index == 0)
			{
				return x;
			}
			else if (index == 1)
			{
				return y;
			}
			else
			{
				return z;
			}
		}

		FORCEINLINE bool isNearlyZero(float tolerance = KINDA_SMALL_NUMBER) const
		{
			return Math::abs(x) <= tolerance && Math::abs(y) <= tolerance && Math::abs(z) <= tolerance;
		}

		FORCEINLINE bool isZero() const
		{
			return x == 0.0f && y == 0.0f && z == 0.0f;
		}

		FORCEINLINE float operator | (const Vector3& v) const
		{
			return x * v.x + y * v.x + z * v.z;
		}

		FORCEINLINE Vector3 operator - (const Vector3& rhs) const
		{
			return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
		}

		FORCEINLINE Vector3 operator ^ (const Vector3& rhs) const
		{
			return Vector3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
		}

		FORCEINLINE Vector3 operator -() const
		{
			return Vector3(-x, -y, -z);
		}

		FORCEINLINE bool operator == (const Vector3& rhs) const
		{
			return x == rhs.x && y == rhs.y && z == rhs.z;
		}

		FORCEINLINE bool operator !=(const Vector3& v) const
		{
			return x != v.x || y != v.y || z != v.z;
		}

		bool containsNaN() const;

		FORCEINLINE Vector3 operator * (const float v) const
		{
			return Vector3(x * v, y * v, z * v);
		}

		FORCEINLINE Vector3 operator + (const Vector3& rhs) const
		{
			return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
		}

		FORCEINLINE Vector3 operator +=(const Vector3& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		FORCEINLINE Vector3 operator *= (T scale)
		{
			x *= scale;
			y *= scale;
			z *= scale;
			return *this;
		}

		FORCEINLINE Vector3 operator *(const Vector3& v) const
		{
			return Vector3(x * v.x, y * v.y, z * v.z);
		}

		FORCEINLINE Vector3 operator / (float scale) const
		{
			const float rScale = 1.f / scale;
			return Vector3(x * rScale, y * rScale, z * rScale);
		}

		FORCEINLINE Vector3 getSafeNormal(float toLerance = SMALL_NUMBER) const
		{
			const float squareSum = x * x + y * y + z * z;
			if (squareSum == 1.f)
			{
				return *this;
			}
			else if (squareSum < toLerance)
			{
				return Vector3::Zero;
			}
			const float scale = Math::InvSqrt(squareSum);
			return Vector3(x * scale, y * scale, z * scale);

		}

		static FORCEINLINE float distSquared(const Vector3& v, const Vector3& v2)
		{
			return Math::square(v2.x - v.x) + Math::square(v2.y - v.y) + Math::square(v2.z - v.z);
		}

		static FORCEINLINE float dist(const Vector3& v1, const Vector3& v2)
		{
			return Math::sqrt(Vector3::distSquared(v1, v2));
		}

		FORCEINLINE static Vector3 crossProduct(const Vector3& a, const Vector3 & b)
		{
			return a ^ b;
		}

		FORCEINLINE float size() const
		{
			return Math::sqrt(x * x + y * y + z * z);
		}

		FORCEINLINE float length() const
		{
			return Math::sqrt(x * x + y * y + z * z);
		}

		FORCEINLINE T sizeSquared() const
		{
			return x * x + y * y + z * z;
		}

		FORCEINLINE bool equals(const Vector3 & v, T tolerance = (T)KINDA_SMALL_NUMBER) const
		{
			return Math::abs(x - v.x) <= tolerance && Math::abs(y - v.y) <= tolerance && Math::abs(z - v.z) <= tolerance;
		}

		FORCEINLINE Vector3 getClampedToMaxSize(float maxSize) const
		{
			if (maxSize < KINDA_SMALL_NUMBER)
			{
				return Zero;
			}

			const float VSq = sizeSquared();
			if (VSq > Math::square(maxSize))
			{
				const float scale = maxSize * Math::InvSqrt(VSq);
				return Vector3(x * scale, y * scale, z * scale);
			}
			else
			{
				return *this;
			}
		}

		friend Archive& operator<<(Archive& ar, Vector3& v)
		{
			return ar << v.x << v.y << v.z;
		}

	public:
		static CORE_API const Vector3 Zero;
		static CORE_API const Vector3 Up;
		static CORE_API const Vector3 Forward;
		static CORE_API const Vector3 Right;

	};


	MS_ALIGN(16) struct Vector4
	{
	public:
		float x, y, z, w;
		FORCEINLINE Vector4(const Vector4 &);
		FORCEINLINE Vector4(float inX, float inY, float inZ, float inW);
		FORCEINLINE Vector4();
		FORCEINLINE Vector4(const Vector4& inV, float inW)
			:x(inV.x),
			y(inV.y),
			z(inV.z),
			w(inW)
		{}
		FORCEINLINE Vector4(float v)
			:x(v), y(v), z(v), w(v)
		{

		}

		FORCEINLINE Vector4(const Vector3<float>& inVector, float inW = 1.0f)
			:x(inVector.x)
			,y(inVector.y)
			,z(inVector.z)
			,w(inW)
		{

		}

		Vector4(const LinearColor& inColor);

		FORCEINLINE Vector4 operator - (const Vector4& v) const
		{
			return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
		}

		FORCEINLINE Vector4 operator + (const Vector4& v) const
		{
			return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
		}

		FORCEINLINE Vector4 operator *(const Vector4& v) const
		{
			return Vector4(x * v.x, y* v.y, z * v.z, w * v.w);
		}

		FORCEINLINE Vector4 getSafeNormal(float tolerance = SMALL_NUMBER) const
		{
			const float squareSum = x * x + y * y + z * z;
			if (squareSum > tolerance)
			{
				const float scale = Math::InvSqrt(squareSum);
				return Vector4(x * scale, y * scale, z * scale, 0.0f);
			}
			return Vector4(0.0f);
		}

		FORCEINLINE float& operator[](int32 componentIndex);
		FORCEINLINE float operator[](int32 componentIndex) const;

	public:
	}GCC_ALIGN(16);


	FORCEINLINE Vector4::Vector4(float inX, float inY, float inZ, float inW)
		:x(inX), y(inY), z(inZ), w(inW)
	{
	}

	FORCEINLINE Vector4::Vector4(const Vector4 &rhs)
		:x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
	{

	}

	FORCEINLINE Vector4::Vector4()
	{}

	FORCEINLINE Vector4::Vector4(const LinearColor& inColor)
		:x(inColor.R)
		,y(inColor.G)
		,z(inColor.B)
		,w(inColor.A)
	{}
	
	FORCEINLINE float& Vector4::operator[](int32 componentIndex)
	{
		return (&x)[componentIndex];
	}

	FORCEINLINE float Vector4::operator[](int32 componentIndex) const
	{
		return (&x)[componentIndex];
	}

	template<typename T>
	struct Vector2
	{
	public:
		T x, y;

	public:
		FORCEINLINE Vector2()
		{

		}
		FORCEINLINE Vector2(EForceInit)
			:x(0), y(0)
		{

		}

		template<typename M, typename N>
		FORCEINLINE Vector2(M inX, N inY)
			:x(inX), y(inY)
		{}

		FORCEINLINE Vector2(T && inX, T && inY)
			:x(std::move(inX)),
			y(std::move(inY))
		{
		}

		FORCEINLINE Vector2(T inX)
			:x(inX)
			,y(inX)
		{}

		FORCEINLINE bool isZero() const
		{
			return x == 0.f && y == 0.f;
		}

		template<typename M>
		FORCEINLINE Vector2(const Vector2<M>& rhs)
			:x(rhs.x), y(rhs.y)
		{}

		FORCEINLINE Vector2(const Vector2& rhs)
			:x(rhs.x), y(rhs.y)
		{}

		FORCEINLINE Vector2(Vector2&& rhs)
			:x(std::move(rhs.x))
			,y(std::move(rhs.y))
		{

		}

		FORCEINLINE Vector2& operator = (const Vector2& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			return *this;
		}

		FORCEINLINE Vector2& operator = (Vector2&& rhs)
		{
			x = std::move(rhs.x);
			y = std::move(rhs.y);
			return *this;
		}

		template<typename M>
		FORCEINLINE bool operator != (const Vector2<M>& rhs) const
		{
			return x != rhs.x || y != rhs.y;
		}

		FORCEINLINE friend auto operator *(T scale, Vector2<T> v)->Vector2<decltype(v.x * scale)>
		{
			return Vector2<decltype(v.x * scale)>(v.x * scale, v.y * scale);
		}

		FORCEINLINE friend auto operator *(Vector2<T> v, T scale)->Vector2<decltype(v.x * scale)>
		{
			return Vector2<decltype(v.x * scale)>(v.x * scale, v.y * scale);
		}

		FORCEINLINE friend auto operator * (const Vector2<T>lhs,  const Vector2<T> rhs) ->Vector2<decltype(lhs.x * rhs.x)> 
		{
			return Vector2<decltype (lhs.x* rhs.x) > (lhs.x * rhs.x, lhs.y * rhs.y);
		}

		FORCEINLINE friend Vector2<T> operator / (const Vector2<T> lhs, const Vector2<T> rhs)
		{
			return Vector2<T>(lhs.x / rhs.x, lhs.y / rhs.y);
		}

		template <typename M>
		FORCEINLINE bool operator == (const Vector2<M>& rhs) const
		{
			return x == rhs.x && y == rhs.y;
		}

		FORCEINLINE Vector2 operator-() const
		{
			return Vector2(-x, -y);
		}

		template<typename M>
		FORCEINLINE auto operator -(const Vector2<M>& v) const ->Vector2<decltype(x - v.x)>
		{
			return Vector2<decltype(x - v.x)>(x - v.x, y - v.y);
		}

		template<typename M>
		FORCEINLINE auto operator +(const Vector2<M>& v) const -> Vector2<decltype(x + v.x)>
		{
			return Vector2<decltype(x + v.x)>(x + v.x, y + v.y);
		}



		FORCEINLINE static const Vector2 & zero()
		{
			static Vector2<T> zero(T(0), T(0));
			return zero;
		}

		FORCEINLINE Vector2 componentMax(const Vector2& other) const
		{
			return Vector2(std::max<T>(x, other.x), std::max<T>(y, other.y));
		}
	public:
		CORE_API static const Vector2 Zero;
		CORE_API static const Vector2 Right;
		CORE_API static const Vector2 Up;
	};

	typedef Vector4 float4;

	typedef Vector3<float> float3;
	typedef Vector3<int> int3;
	typedef Vector3<float16> half2;

	typedef Vector2<float> float2;
	typedef Vector2<uint32> uint2;
	typedef Vector2<int32>	int2;

	template<typename T>
	FORCEINLINE Vector3<T> operator*(float scale, const Vector3<T>& V)
	{
		return V.operator*(scale);
	}

	template<typename T>
	FORCEINLINE bool Vector3<T>::containsNaN() const
	{
		return (!Math::isFinite(x) || !Math::isFinite(y) || !Math::isFinite(x));
	}

	template<typename T>
	FORCEINLINE Vector3<T>::Vector3(const Vector2<T>& v, T inZ)
		:x(v.x)
		,y(v.y)
		,z(inZ)
	{
		
	}

	template<typename T>
	FORCEINLINE size_t getTypeHash(const Vector3<T>& vector)
	{
		return Crc::memCrc_DEPRECATED(&vector, sizeof(vector));
	}
	
}