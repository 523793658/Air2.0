#pragma once
#include "CoreType.h"
namespace Air
{
	enum EloadFlags
	{
		LOAD_None = 0x00000000,
		LOAD_EditorOnly = 0x00000004,
	};

	enum EStaticConstructor { EC_StaticConstructor };


	enum EObjectFlags
	{
		RF_NoFlags = 0x00000000,
		RF_Public = 0x00000001,
		RF_Standalone = 0x00000002,
		RF_MarkAsNative = 0x00000004,
		RF_Transactional = 0x00000008,
		RF_ClassDefaultObject = 0x00000010,
		RF_ArchetypeObject = 0x00000020,
		RF_Transient = 0x00000040,
		RF_MarkAsRootSet = 0x00000080,
		RF_NeedInitialization = 0x00000200,
		RF_NeedPostLoad = 0x00000400,
		RF_BeginDestroyed = 0x00008000,
		RF_DefaultSubobject = 0x00040000
	};

#define RF_AllFlags		(EObjectFlags)0x07ffffff

	FORCEINLINE EObjectFlags operator|(EObjectFlags arg1, EObjectFlags arg2)
	{
		return EObjectFlags(uint32(arg1) | uint32(arg2));
	}

	FORCEINLINE void operator |=(EObjectFlags& dest, EObjectFlags arg)
	{
		dest = EObjectFlags(dest | arg);
	}

	enum class EInternalObjectFlags : int32
	{
		None = 0,
	};

	enum EInternal { EC_InternalUseOnlyConstructor };

#define DEFINE_DEFAULT_CONSTRUCTOR_CALL(TClass) \
	static void __defaultConstructor(const ObjectInitializer& X){new((EInternal*)X.getObj())TClass();}

#define DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(TClass) \
	static void __defaultConstructor(const ObjectInitializer& X) {new ((EInternal*)X.getObj())TClass(X);}

#define GENERATED_RCLASS_BODY(ClassName, ParentClass) \
	private:\
		static RClass* mClassInstance; \
		ClassName& operator=(const ClassName&);\
		ClassName(const ClassName&);\
	public: \
		typedef ParentClass	ParentType;\
		ClassName(const ObjectInitializer& objectInitializer= ObjectInitializer::get());\
		static RClass* StaticClass() { return mClassInstance; }	\
		DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(ClassName)

#define RF_PropagateToSubObjects	((EObjectFlags)(RF_Public | RF_ArchetypeObject | RF_Transactional | RF_Transient))
}