#pragma once
#include "Template/IsClass.h"
#include "Template/ChooseClass.h"
#include "Template/IntegralConstant.h"
#include <type_traits>
namespace Air
{
	template<typename ElementType, bool isClass = std::is_class<ElementType>::value>
	class TElementAlignmentCalculator
	{
	private:
		struct FAlignedElements : ElementType
		{
			uint8 MisalignmentPadding;
			FAlignedElements();
			~FAlignedElements();
		};

		enum { CalculatedAlignment = sizeof(FAlignedElements) - sizeof(ElementType) };
	public:
		enum { Value = TChooseClass<CalculatedAlignment != 0, TIntegralConstant<SIZE_T, CalculatedAlignment>, TElementAlignmentCalculator<ElementType, false>>::Result::Value };
	};





	template<typename ElementType>
	class TElementAlignmentCalculator<ElementType, false>
	{
	private:
		struct AlignedElements 
		{
			uint8 MisalignmentPadding;
			ElementType Element;
			AlignedElements();
			~AlignedElements();
		};
	public:
		enum {Value = sizeof(AlignedElements) - sizeof(ElementType) };
	};



#define ALIGNOF(T) (TElementAlignmentCalculator<T>::Value)



}