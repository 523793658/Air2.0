//#pragma once
//#include "SlateCore.h"
//#include <type_traits>
//#include "SNullWidget.h"
//namespace Air
//{
//	class IToolTip;
//	class SUserWidget;
//	class SWidget;
//	
//	template<typename WidgetType> struct TSlateBaseNamedArgs;
//
//
//	struct TAlwaysValidWidget
//	{
//		TAlwaysValidWidget()
//			:mWidget(SNullWidget::NullWidget)
//		{
//		}
//		std::shared_ptr<SWidget> mWidget;
//	};
//
//
//
//
//
//
//
//
//	template<class DeclarationType>
//	struct NamedSlotProperty
//	{
//		NamedSlotProperty(DeclarationType& inOwnerDeclaration, TAlwaysValidWidget& contentToSet)
//			: OwnerDeclaration(inOwnerDeclaration)
//			, SlotContent(contentToSet)
//		{}
//
//		DeclarationType& operator [] (const std::shared_ptr<SWidget>& inChild)
//		{
//			SlotContent.mWidget = inChild;
//			return OwnerDeclaration;
//		}
//
//		DeclarationType & OwnerDeclaration;
//		TAlwaysValidWidget& SlotContent;
//	};
//
//
//
//#define SLATE_NAMED_SLOT(DeclarationType, SlotName)	\
//	NamedSlotProperty<DeclarationType> SlotName()	\
//	{	\
//		return NamedSlotProperty<DeclarationType>(*this, _##SlotName);	\
//	}	\
//	TAlwaysValidWidget _##SlotName;	\
//
//#define SLATE_DEFAULT_SLOT(DeclarationType, SlotName)	\
//		SLATE_NAMED_SLOT(DeclarationType, SlotName);	\
//		DeclarationType& operator[](const std::shared_ptr<SWidget> inChild) \
//		{	\
//			_##SlotName.mWidget = inChild;	\
//			return *this;	\
//		}
//
//	namespace RequiredArgs
//	{
//		struct 
//	}
//
//
//
//
//#define SNew(WidgetType, ...)	\
//	makeTDecl<WidgetType>(#WidgetType, __FILE__, __LINE__, RequiredArgs)
//
//	template<typename WidgetType, bool IsDerived>
//	struct TWidgetAllocator
//	{
//		struct std::shared_ptr<WidgetType> privateAllocateWidget()
//		{
//			return MakeSharedPtr<WidgetType>();
//		}
//	};
//
//
//
//	template<class WidgetType, typename RequiredArgsPayLoadType>
//	struct TDecl
//	{
//		TDecl(const ANSICHAR* inType, const ANSICHAR* InFile, int32 online, RequiredArgsPayLoadType&& inRequiredArgs)
//			:mWidget(TWidgetAllocator<WidgetType, std::is_base_of<SUserWidget, WidgetType>::value>::privateAllocateWidget())
//			, _RequeiredArgs(inRequiredArgs)
//		{
//			//mWidget->setde
//		}
//		template<class ExposeAsWidgetType>
//		TDecl& expose(std::shared_ptr<ExposeAsWidgetType>& outVarToInit)
//		{
//			outVarToInit = mWidget;
//			return *this;
//		}
//
//		std::shared_ptr<WidgetType> operator <<=(const typename WidgetType::Arguments& inArgs) const
//		{
//
//		}
//		
//
//		const std::shared_ptr<WidgetType> mWidget;
//		RequiredArgsPayLoadType& _RequeiredArgs;
//	};
//
//	template<typename WidgetType, typename RequiredArgsPayLoadType>
//	TDecl<WidgetType, RequiredArgsPayLoadType> makeTDecl(const ANSICHAR* inType, const ANSICHAR* inFile, int32 online, RequiredArgsPayloadType && inRequiredArgs)
//	{
//		return TDecl<WidgetType, RequiredArgsPayloadType>(inType, inFile, online, std::forward<RequiredArgsPayloadType>(inRequiredArgs));
//	}
//
//
//	template<typename WidgetType>
//	struct TSlateBaseNamedArgs
//	{
//
//	};
//}