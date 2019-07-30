#pragma once
#include "CoreObject.h"
#include "ObjectMacros.h"
namespace Air
{
	class RClass;
	class Object;
	class COREOBJECT_API ObjectInitializer
	{
	public:
		ObjectInitializer();

		ObjectInitializer(Object* inObj, Object* inObjectArchetType, bool bInCopyTransientsFromClassDefaults, bool bInShouldInitializedProps);

		~ObjectInitializer();

		std::shared_ptr<Object> getSharedPtr()
		{
			return mObjectSharedPtr;
		}

		void postConstructInit();

		FORCEINLINE Object* getObj() const
		{
			return mObj;
		}

		std::shared_ptr<Object> createDefaultSubObject(Object* outer, wstring subObjectName, RClass* returnType, RClass* classToCreateByDefault, bool bIsRequired, bool bAbstract, bool bIsTransient) const;

		static ObjectInitializer& get();

		void assertIfSubobjectSetupIsNotAllowed(const TCHAR* subobjectName) const;

		template<typename T>
		FORCEINLINE ObjectInitializer const& setDefaultSubobjectClass(TCHAR const * subObjectname) const
		{
			assertIfSubobjectSetupIsNotAllowed(subObjectname);
			mComponentOverrides.add(subObjectname, T::StaticClass(), *this);
			return *this;
		}

		bool islegalOverride(wstring inComponentName, class RClass* derivedComponentClass, class RClass* baseComponentClass) const;
	private:
		struct Overrides
		{
			void add(wstring inComponentName, RClass* inComponentClass, ObjectInitializer const & objectInitializer)
			{
				int32 index = find(inComponentName);
				if (index == INDEX_NONE)
				{
					new (mOverrides) Override(inComponentName, inComponentClass);
				}
				else if(inComponentClass && mOverrides[index].mComponentClass)
				{
					objectInitializer.islegalOverride(inComponentName, mOverrides[index].mComponentClass, inComponentClass);
				}
			}

			RClass* get(wstring inComponentName, RClass* returnType, RClass* classToConstructByDefault, ObjectInitializer const & objectInitialzier)
			{
				int32 index = find(inComponentName);
				RClass* baseComponentClass = classToConstructByDefault;
				if (index == INDEX_NONE)
				{
					return baseComponentClass;
				}
				else if (mOverrides[index].mComponentClass)
				{
					if (objectInitialzier.islegalOverride(inComponentName, mOverrides[index].mComponentClass, returnType))
					{
						return mOverrides[index].mComponentClass;
					}
				}
				return nullptr;
			}

		private:
			int32 find(wstring inComponentName)
			{
				for (int32 index = 0; index < mOverrides.size(); index++)
				{
					if (mOverrides[index].mComponentName == inComponentName)
					{
						return index;
					}
				}
				return INDEX_NONE;
			}

			struct Override
			{
				wstring mComponentName;
				RClass* mComponentClass;
				Override(wstring inName, RClass* inComponentClass)
					:mComponentName(inName)
					,mComponentClass(inComponentClass)
				{}
			};
			TArray<Override, TInlineAllocator<8>> mOverrides;
		};
		void initSharedPtr();
		friend class ObjectBase;
	public:
		std::shared_ptr<class Object> mObjectSharedPtr;

	private:
		Object* mObj;
		Object* mObjectArchetype;
		bool bCopyTransientsFromClassDefaults;
		bool bShouldInitializePropsFromArchetype;
		bool bSubobjectClassInitializationAllowed;
		mutable TArray<wstring, TInlineAllocator<8>> mConstructedSubobjects;
		Object* mLastConstructedObject;
		bool bIsDeferredInitializer : 1;

		mutable Overrides mComponentOverrides;
	};

	COREOBJECT_API void freeObject(Object* obj);


	COREOBJECT_API Object* staticAllocateObject(RClass* Class, Object* inOuter, wstring name, EObjectFlags setFlags, EInternalObjectFlags = EInternalObjectFlags::None, bool bCanReuseSubobjects = false, bool * bOutReusedSubObject = nullptr);

	COREOBJECT_API std::shared_ptr<Object> staticConstructObject_Internal(RClass* inClass, Object* inOuter = nullptr, wstring name = TEXT(""), EObjectFlags setFlags = RF_NoFlags, EInternalObjectFlags internalSetFlags = EInternalObjectFlags::None, Object* inTemplate = nullptr, bool bCopyTransientsFromClassDefaults = false);
	
	template<class T>
	FUNCTION_NON_NULL_RETURN_START
		std::shared_ptr<T> newObject(Object* outer, RClass* inClass, wstring name = TEXT(""), EObjectFlags flags = RF_NoFlags, Object* inTemplate = nullptr, bool bCopyTransientsFromClassDefaults = false)
	FUNCTION_NON_NULL_RETURN_END
	{
		return std::static_pointer_cast<T>(staticConstructObject_Internal(inClass, outer, name, flags, EInternalObjectFlags::None, inTemplate, bCopyTransientsFromClassDefaults));
	}

	template<class T>
	FUNCTION_NON_NULL_RETURN_START
		std::shared_ptr<T> newObject(Object* outer = nullptr)
	FUNCTION_NON_NULL_RETURN_END
	{
		return std::static_pointer_cast<T>(staticConstructObject_Internal(T::StaticClass(), outer, TEXT(""), RF_NoFlags, EInternalObjectFlags::None, nullptr, false));
	}


	template<class T>
	FUNCTION_NON_NULL_RETURN_START
		std::shared_ptr<T> newObject(Object* outer, wstring name, EObjectFlags flags = RF_NoFlags, Object* inTemplate = nullptr, bool bCopyTransientsFromClassDefaults = false)
	FUNCTION_NON_NULL_RETURN_END
	{
		return std::static_pointer_cast<T>(staticConstructObject_Internal(T::StaticClass(), outer, name, flags, EInternalObjectFlags::None, inTemplate, bCopyTransientsFromClassDefaults));
	}

	template<class T>
	FUNCTION_NON_NULL_RETURN_START
		std::shared_ptr<T> newObject(Object* outer, wstring className, wstring name, EObjectFlags flags = RF_NoFlags, Object* inTemplate = nullptr, bool bCopyTransientsFromClassDefaults = false)
		FUNCTION_NON_NULL_RETURN_END
	{
		return newObject<T>(outer, SimpleReflectionManager::getClass(className), name, flags, inTemplate, bCopyTransientsFromClassDefaults);
	}


	COREOBJECT_API std::shared_ptr<Object> staticLoadObject(RClass* inClass, std::shared_ptr<Object> inOuter, const TCHAR* name, const TCHAR* filename = nullptr, uint32 loadFlags = LOAD_None);


	bool resolveName(Object*& inPackage, wstring& inOutName, bool create, bool bThrow, uint32 loadFlags = LOAD_None);


	/*template<class T>
	inline T* loadObject(const TCHAR* filename = nullptr, uint32 loadFlags = LOAD_None)
	{
		return T::loadFromFile(filename);
	}*/

	template<class T, typename... Args>
	inline std::shared_ptr<T> loadObjectAsync(Args&&... args)
	{
		return ResLoader::instance().asyncQueryT<T>(T::createLoadingDesc(std::forward<Args>(args)...));
	}

	template<class T, typename... Args>
	inline std::shared_ptr<T> loadObjectSync(Args&&... args)
	{
		return ResLoader::instance().syncQueryT<T>(T::createLoadingDesc(std::forward<Args>(args)...));
	}

	template<class T>
	inline const T* getDefault()
	{
		return dynamic_cast<T*>( T::StaticClass()->getDefaultObject());
	}
}