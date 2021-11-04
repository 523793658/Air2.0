// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "Misc/TVariant.h"
#include "Templates/UnrealTemplate.h"

template <typename ArgType>
struct TValueOrError_ValueProxy
{
	TValueOrError_ValueProxy(ArgType&& InArg) : Arg(Forward<ArgType>(InArg)) {}
	ArgType&& Arg;
};

template <typename ArgType>
struct TValueOrError_ErrorProxy
{
	TValueOrError_ErrorProxy(ArgType&& InArg) : Arg(Forward<ArgType>(InArg)) {}
	ArgType&& Arg;
};

template <typename ArgType>
FORCEINLINE TValueOrError_ValueProxy<ArgType> MakeValue(ArgType&& Arg)
{
	return TValueOrError_ValueProxy<ArgType>(Forward<ArgType>(Arg));
}

template <typename ArgType>
FORCEINLINE TValueOrError_ErrorProxy<ArgType> MakeError(ArgType&& Arg)
{
	return TValueOrError_ErrorProxy<ArgType>(Forward<ArgType>(Arg));
}

/**
 * Type used to return either a value or an error.
 *
 * These must have a value or an error when newly constructed, but it is possible to have neither
 * because of the functions to steal the value or error. This is critical for callers to consider
 * since it means that HasValue() and HasError() must be checked independently; a return value of
 * false from one does not imply that the other will return true.
 *
 * The MakeValue and MakeError functions may be used to construct these conveniently.
 */
template <typename ValueType, typename ErrorType>
class TValueOrError
{
	/** Wrap the error type to allow ValueType and ErrorType to be the same type. */
	struct FWrapErrorType
	{
		template <typename... ArgTypes>
		FWrapErrorType(ArgTypes&&... Args) : Error(Forward<ArgTypes>(Args)...) {}
		ErrorType Error;
	};

	/** A unique empty type used in the unset state after stealing the value or error. */
	struct FEmptyType
	{
	};

public:
	/** Construct the value from a proxy from MakeValue. */
	template <typename ArgType>
	inline TValueOrError(TValueOrError_ValueProxy<ArgType>&& Proxy)
		: Variant(TInPlaceType<ValueType>(), Forward<ArgType>(Proxy.Arg))
	{
	}

	/** Construct the error from a proxy from MakeError. */
	template <typename ArgType>
	inline TValueOrError(TValueOrError_ErrorProxy<ArgType>&& Proxy)
		: Variant(TInPlaceType<FWrapErrorType>(), Forward<ArgType>(Proxy.Arg))
	{
	}

	TValueOrError(TValueOrError&&) = default;
	TValueOrError& operator=(TValueOrError&&) = default;

	TValueOrError(const TValueOrError&) = delete;
	TValueOrError& operator=(const TValueOrError&) = delete;

	/** Check whether a value is set. Prefer HasValue and HasError to this. !IsValid() does *not* imply HasError(). */
	inline bool IsValid() const { return Variant.template IsType<ValueType>(); }

	/** Whether the error is set. An error does imply no value. No error does *not* imply that a value is set. */
	inline bool HasError() const { return Variant.template IsType<FWrapErrorType>(); }

	/** Access the error. Asserts if this does not have an error. */
	inline       ErrorType& GetError()       &  { return Variant.template Get<FWrapErrorType>().Error; }
	inline const ErrorType& GetError() const &  { return Variant.template Get<FWrapErrorType>().Error; }
	inline       ErrorType  GetError()       && { return MoveTemp(Variant.template Get<FWrapErrorType>().Error); }

	/** Access the error if it is set. */
	inline       ErrorType* TryGetError()
	{
		if (FWrapErrorType* Wrap = Variant.template TryGet<FWrapErrorType>())
		{
			return &Wrap->Error;
		}
		return nullptr;
	}
	inline const ErrorType* TryGetError() const
	{
		if (const FWrapErrorType* Wrap = Variant.template TryGet<FWrapErrorType>())
		{
			return &Wrap->Error;
		}
		return nullptr;
	}

	/** Steal the error. Asserts if this does not have an error. This causes the error to be unset. */
	inline ErrorType StealError()
	{
		ErrorType Temp = MoveTemp(GetError());
		Variant.template Emplace<FEmptyType>();
		return Temp;
	}

	/** Whether the value is set. A value does imply no error. No value does *not* imply that an error is set. */
	inline bool HasValue() const { return Variant.template IsType<ValueType>(); }

	/** Access the value. Asserts if this does not have a value. */
	inline       ValueType& GetValue()       &  { return Variant.template Get<ValueType>(); }
	inline const ValueType& GetValue() const &  { return Variant.template Get<ValueType>(); }
	inline       ValueType  GetValue()       && { return MoveTemp(Variant.template Get<ValueType>()); }

	/** Access the value if it is set. */
	inline       ValueType* TryGetValue()       { return Variant.template TryGet<ValueType>(); }
	inline const ValueType* TryGetValue() const { return Variant.template TryGet<ValueType>(); }

	/** Steal the value. Asserts if this does not have a value. This causes the value to be unset. */
	inline ValueType StealValue()
	{
		ValueType Temp = MoveTemp(GetValue());
		Variant.template Emplace<FEmptyType>();
		return Temp;
	}

private:
	TVariant<ValueType, FWrapErrorType, FEmptyType> Variant;
};
