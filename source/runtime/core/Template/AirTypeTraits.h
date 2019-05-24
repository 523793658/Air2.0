#pragma once
#include "CoreType.h"
#include <type_traits>
namespace std
{
	template<typename T, typename Arg>
	struct is_bitwise_constructible
	{
		static_assert(!std::is_reference<T>::value && !std::is_reference<Arg>::value, "TIsBitwiseConstructible is not designed to accept reference types");
		static_assert(std::is_same<T, typename std::remove_cv<T>::type>::value && std::is_same<Arg, typename std::remove_cv<Arg>::type>::value, "TIsBitwiseConstructible is not designed to accept qualified types");
		enum { value = false};
	};

	template<typename T>
	struct is_bitwise_constructible<T, T>
	{
		enum
		{
			value = std::is_trivially_copy_constructible<T>::value
		};
	};

	template <typename T, typename U>
	struct is_bitwise_constructible<const T, U> : is_bitwise_constructible<T, U>
	{

	};

	template<typename T>
	struct is_bitwise_constructible<const T*, T*>
	{
		enum { value = true };
	};

	template <> struct is_bitwise_constructible<uint8, int8> { enum { Value = true }; };
	template <> struct is_bitwise_constructible< int8, uint8> { enum { Value = true }; };
	template <> struct is_bitwise_constructible<uint16, int16> { enum { Value = true }; };
	template <> struct is_bitwise_constructible< int16, uint16> { enum { Value = true }; };
	template <> struct is_bitwise_constructible<uint32, int32> { enum { Value = true }; };
	template <> struct is_bitwise_constructible< int32, uint32> { enum { Value = true }; };
	template <> struct is_bitwise_constructible<uint64, int64> { enum { Value = true }; };
	template <> struct is_bitwise_constructible< int64, uint64> { enum { Value = true }; };
	

	template <typename... Types>
	struct TAnd;

	template<bool LHSValue, typename... RHS>
	struct and_value
	{
		enum { value = TAnd<RHS...>::value };
	};

	template<typename... RHS>
	struct and_value<false, RHS...>
	{
		enum { value = false };
	};

	template<typename LHS, typename... RHS>
	struct TAnd<LHS, RHS...>: and_value<LHS::value, RHS...>
	{};

	template<>
	struct TAnd<>
	{
		enum { value = true };
	};



	template<typename... Types>
	struct TOr;

	template<bool LHSValue, typename... RHS>
	struct or_value
	{
		enum { value = TOr<RHS...>::value };
	};

	template<typename... RHS>
	struct or_value<true, RHS...>
	{
		enum { value = true };
	};
	template<typename LHS, typename... RHS>
	struct TOr<LHS, RHS...> : or_value<LHS::value, RHS...>
	{

	};


	template<>
	struct TOr<>
	{
		enum { value = false };
	};


	template<typename Type>
	struct tNot
	{
		enum { value = !Type::value };
	};



	template <typename T>
	struct use_bitwise_swap
	{
		enum { value = !std::or_value<__is_enum(T), std::is_pointer<T>, std::is_arithmetic<T>>::value };
	};


	template<typename T, bool TypeIsSmall>
	struct TCallTraitsParamTypeHelper
	{
		typedef const T& param_type;
		typedef const T& const_param_type;
	};

	template<typename T>
	struct TCallTraitsParamTypeHelper<T, true>
	{
		typedef const T param_type;
		typedef const T const_param_type;
	};

	template<typename T>
	struct TCallTraitsParamTypeHelper<T*, true>
	{
		typedef T* param_type;
		typedef const T* const_param_type;
	};


	template<typename T>
	struct TCallTraitsBase
	{
	private:
		enum { PassByValue = TOr<and_value<(sizeof(T) <= sizeof(void*)), std::is_pod<T>>, std::is_arithmetic<T>, std::is_pointer<T>>::value };

	public:
		typedef T value_type;
		typedef T& reference;
		typedef const T& const_reference;
		typedef typename TCallTraitsParamTypeHelper<T, PassByValue>::param_type param_type;
		typedef typename TCallTraitsParamTypeHelper<T, PassByValue>::const_param_type const_pointer_type;
	};

	template<typename T>
	struct call_traits : public TCallTraitsBase<T> {};

	template<typename T>
	struct call_traits<T&>
	{
		typedef T& value_type;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T& param_type;
		typedef T& const_pointer_type;
	};

	template<typename T, size_t N>
	struct call_traits<T[N]>
	{
	private:
		typedef T array_type[N];
	public:
		typedef const T* value_type;
		typedef array_type& reference;
		typedef const array_type& const_reference;
		typedef const T* const param_type;
		typedef const T* const const_pointer_type;
	};

	template<typename T, size_t N>
	struct call_traits<const T[N]>
	{
	private:
		typedef const  T array_type[N];
	public:
		typedef const T* value_type;
		typedef array_type& reference;
		typedef const array_type& const_reference;
		typedef const T* const param_type;
		typedef const T* const const_pointer_type;
	};

	template<typename A, typename B>
	struct are_types_equal;

	template<typename , typename>
	struct are_types_equal
	{
		enum {value = false};
	};

	template<typename A>
	struct are_types_equal<A, A>
	{
		enum {value = true};
	};

	template<typename T>
	struct is_bytewise_comparable
	{
		enum {value = TOr<std::is_enum<T>, std::is_pointer<T>, std::is_arithmetic<T>>::value};
	};

#define ARE_TYPES_EQUAL(A, B) are_types_equal<A, B>::value


	template<typename T> struct TContainerTraitsBase
	{
		enum { MoveWillEmptyContainer = false };
	};

	template<typename T>
	struct TCallTraits : public TCallTraitsBase<T> {};

	template<typename T>
	struct TCallTraits<T&>
	{
		typedef T& value_type;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T& param_type;
		typedef T& const_pointer_type;
	};

	template<typename T, size_t N>
	struct TCallTraits<T [N]>
	{
	private:
		typedef T array_type[N];
	public:
		typedef const T* value_type;
		typedef array_type&	reference;
		typedef const array_type& const_reference;
		typedef const T* const param_type;
		typedef const T* const const_pointer_type;
	};

	template<typename T, size_t N>
	struct TCallTraits<const T[N]>
	{
	private:
		typedef const T array_type[N];
	public:
		typedef const T* value_type;
		typedef const array_type& reference;
		typedef const array_type& const_reference;
		typedef const T* const param_type;
		typedef const T* const const_pointer_type;

	};

	template<typename T> struct TContainerTraits : public TContainerTraitsBase<T> {};

	template<typename T>
	struct t_type_traits_base
	{
		typedef typename TCallTraits<T>::param_type const_init_type;
		typedef typename TCallTraits<T>::const_pointer_type const_pointer_type;

		enum {is_bytewise_comparable = TOr<std::is_enum<T>, std::is_arithmetic<T>, std::is_pointer<T>>::value};
	};

	template<typename T> struct type_traits : public t_type_traits_base<T> {};
}