#ifndef _Vector_H_
#define _Vector_H_
#include "CoreType.h"
#include <array>
#include <limits>
#include "boost/operators.hpp"
#include "Math/MathHelper.hpp"

namespace Air
{
	template<typename T, int N>
	class Vector_T : boost::addable<Vector_T<T, N>,
		boost::subtractable<Vector_T<T, N>,
		boost::multipliable<Vector_T<T, N>,
		boost::dividable<Vector_T<T, N>,
		boost::dividable2<Vector_T<T, N>, T,
		boost::multipliable2<Vector_T<T, N>, T,
		boost::addable2<Vector_T<T, N>, T,
		boost::subtractable2<Vector_T<T, N>, T,
		boost::equality_comparable<Vector_T<T, N>> >> >> >>>>
	{
		template<typename U, int N>
		friend class Vector_T;
		typedef std::array<T, N>	DetailType;
	public:
		typedef typename DetailType::value_type				value_type;
		typedef	value_type*									pointer;
		typedef value_type const *							const_pointer;

		typedef	typename DetailType::reference				reference;
		typedef typename DetailType::const_reference		const_reference;

		typedef typename DetailType::iterator				iterator;
		typedef typename DetailType::const_iterator			const_iterator;

		typedef typename DetailType::size_type				size_type;
		typedef typename DetailType::difference_type		difference_type;

		enum { elem_num = N};

	public:
		Vector_T() AIR_NOEXCEPT
		{

		}
		explicit Vector_T(T const * rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doCopy(&mVec[0], rhs);
		}

		explicit Vector_T(T const & rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doAssign(&mVec[0], rhs);
		}

		Vector_T(Vector_T const & rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doCopy(&mVec[0], &rhs[0]);
		}

		Vector_T(Vector_T && rhs) AIR_NOEXCEPT
			:mVec(std::move(rhs.mVec))
		{
		}

		template <typename U, int M>
		Vector_T(Vector_T<U, M> const & rhs) AIR_NOEXCEPT
		{
			static_assert(M >= N, "Could not convert to a smaller vector.");
			detail::vector_helper<T, N>::doCopy(&mVec[0], &rhs[0]);
		}

		Vector_T(T const & x, T const & y) AIR_NOEXCEPT
		{
			static_assert(2 == elem_num, "Must be 2D vector.");
			mVec[0] = x;
			mVec[1] = y;
		}
		Vector_T(T && x, T && y) AIR_NOEXCEPT
		{
			static_assert(2 == elem_num, "Must be 2D vector.");
			mVec[0] = std::move(x);
			mVec[1] = std::move(y);
		}
		Vector_T(T const & x, T const & y, T const & z) AIR_NOEXCEPT
		{
			static_assert(3 == elem_num, "Must be 3D vector.");
			mVec[0] = x;
			mVec[1] = y;
			mVec[2] = z;
		}
		Vector_T(T && x, T && y, T && z) AIR_NOEXCEPT
		{
			static_assert(3 == elem_num, "Must be 3D vector.");
			mVec[0] = std::move(x);
			mVec[1] = std::move(y);
			mVec[2] = std::move(z);
		}

		Vector_T(T const & x, T const & y, T const & z, T const & w) AIR_NOEXCEPT
		{
			static_assert(4 == elem_num, "Must be 4D vector.");
			mVec[0] = x;
			mVec[1] = y;
			mVec[2] = z;
			mVec[3] = w;
		}
		Vector_T(T && x, T && y, T && z, T && w) AIR_NOEXCEPT
		{
			static_assert(4 == elem_num, "Must be 4D vector.");
			mVec[0] = std::move(x);
			mVec[1] = std::move(y);
			mVec[2] = std::move(z);
			mVec[3] = std::move(w);
		}

		static Vector_T const & getMaxVector() AIR_NOEXCEPT
		{
			static Vector_T<T, N> const maxP(std::numeric_limits<T>::max());
			return maxP;
		}

		static Vector_T const & getMinVector() AIR_NOEXCEPT
		{
			static Vector_T<T, N> const minP(std::numeric_limits<T>::min());
			return minP;
		}

		static size_t size() AIR_NOEXCEPT
		{
			return elem_num;
		}

		static Vector_T const & zero() AIR_NOEXCEPT
		{
			static Vector_T<T, N> const zero(T(0));
			return zero;
		}

		Vector_T & normalize() AIR_NOEXCEPT
		{
			*this = MathLib::normalize(*this);
			return *this;
		}

		iterator begin() AIR_NOEXCEPT
		{
			return mVec.begin();
		}

		const_iterator begin() const AIR_NOEXCEPT
		{
			return mVec.begin();
		}

		iterator end() AIR_NOEXCEPT
		{
			return mVec.end();
		}

		const_iterator end() const AIR_NOEXCEPT
		{
			return mVec.end();
		}

		reference operator[](size_t index) AIR_NOEXCEPT
		{
			return mVec[index];
		}
		const_reference operator[](size_t index) const AIR_NOEXCEPT
		{
			return mVec[index];
		}


		reference x() AIR_NOEXCEPT
		{
			static_assert(elem_num >= 1, "Must be 1D vector");
			return mVec[0];
		}
		const_reference x() const AIR_NOEXCEPT
		{
			static_assert(elem_num >= 1, "Must be 1D vector");
			return mVec[0];
		}

		reference y() AIR_NOEXCEPT
		{
			static_assert ( elem_num >= 2, "Must be 2D vector");
			return mVec[1];
		}

		const_reference y() const AIR_NOEXCEPT
		{
			static_assert ( elem_num >= 2, "Must be 2D vector");
			return mVec[1];
		}

		reference z() AIR_NOEXCEPT
		{
			static_assert (elem_num >= 3, "Must be 3D vector");
			return mVec[2];
		}

		const_reference z() const AIR_NOEXCEPT
		{
			static_assert (elem_num >= 3, "Must be 3D vector");
			return mVec[2];
		}

		reference w() AIR_NOEXCEPT
		{
			static_assert (elem_num >= 4, "Must be 3D vector");
			return mVec[3];
		}

		const_reference w() const AIR_NOEXCEPT
		{
			static_assert (elem_num >= 4, "Must be 3D vector");
			return mVec[3];
		}

		// 赋值操作符
		template <typename U>
		Vector_T const & operator+=(Vector_T<U, N> const & rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doAdd(&mVec[0], &mVec[0], &rhs.mVec[0]);
			return *this;
		}
		template <typename U>
		Vector_T const & operator+=(U const & rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doAdd(&mVec[0], &mVec[0], rhs);
			return *this;
		}
		template <typename U>
		Vector_T const & operator-=(Vector_T<U, N> const & rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doSub(&mVec[0], &mVec[0], &rhs.mVec[0]);
			return *this;
		}
		template <typename U>
		Vector_T const & operator-=(U const & rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doSub(&mVec[0], &mVec[0], rhs);
			return *this;
		}
		template <typename U>
		Vector_T const & operator*=(Vector_T<U, N> const & rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doMul(&mVec[0], &mVec[0], &rhs.mVec[0]);
			return *this;
		}
		template <typename U>
		Vector_T const & operator*=(U const & rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doScale(&mVec[0], &mVec[0], rhs);
			return *this;
		}
		template <typename U>
		Vector_T const & operator/=(Vector_T<U, N> const & rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doDiv(&mVec[0], &mVec[0], &rhs.mVec[0]);
			return *this;
		}
		template <typename U>
		Vector_T const & operator/=(U const & rhs) AIR_NOEXCEPT
		{
			return this->operator*=(1.0f / rhs);
		}

		Vector_T& operator=(Vector_T const & rhs) AIR_NOEXCEPT
		{
			if (this != &rhs)
			{
				mVec = rhs.mVec;
			}
			return *this;
		}
		Vector_T& operator=(Vector_T&& rhs) AIR_NOEXCEPT
		{
			mVec = std::move(rhs.mVec);
			return *this;
		}
		template <typename U, int M>
		Vector_T& operator=(Vector_T<U, M> const & rhs) AIR_NOEXCEPT
		{
			static_assert(M >= N, "Could not assign to a smaller vector.");

			detail::vector_helper<T, N>::doCopy(&mVec[0], &rhs.mVec[0]);
			return *this;
		}

		// 一元操作符
		Vector_T const operator+() const AIR_NOEXCEPT
		{
			return *this;
		}
		Vector_T const operator-() const AIR_NOEXCEPT
		{
			Vector_T temp(*this);
			detail::vector_helper<T, N>::doNegate(&temp.mVec[0], &mVec[0]);
			return temp;
		}

		void swap(Vector_T& rhs) AIR_NOEXCEPT
		{
			detail::vector_helper<T, N>::doSwap(&mVec[0], &rhs.mVec[0]);
		}

		bool operator==(Vector_T const & rhs) const AIR_NOEXCEPT
		{
			return detail::vector_helper<T, N>::doEqual(&mVec[0], &rhs[0]);
		}

		T length() const AIR_NOEXCEPT
		{
			return MathLib::length(*this);
		}


	private:
		DetailType mVec;
	};

	template <typename T, int N>
	inline void swap(Vector_T<T, N>& lhs, Vector_T<T, N> & rhs) AIR_NOEXCEPT
	{
		lhs.swap(rhs);
	}

}



#endif