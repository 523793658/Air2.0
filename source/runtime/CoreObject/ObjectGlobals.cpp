#include "ObjectGlobals.h"
#include "Class.h"
#include "Object.h"
#include "ObjectThreadContext.h"
#include "ObjectAllocator.h"
#include "LinkerLoad.h"
namespace Air
{
	void staticTick(float deltaTime, bool bUseFullTimeLimit /* = true */, float asyncLoadingTime /* = 0.005f */)
	{

	}



	ObjectInitializer::ObjectInitializer()
	{

	}

	ObjectInitializer::ObjectInitializer(Object* inObj, Object* inObjectArchetType, bool bInCopyTransientsFromClassDefaults, bool bInShouldInitializedProps)
		:mObj(inObj),
		mObjectArchetype(inObjectArchetType)
		, bCopyTransientsFromClassDefaults(bInCopyTransientsFromClassDefaults)
		, bShouldInitializePropsFromArchetype(true)
		, bIsDeferredInitializer(false)
	{
		ObjectThreadContext& threadContext = ObjectThreadContext::get();
		threadContext.mIsInConstructor++;
		mLastConstructedObject = threadContext.mConstructedObject;
		threadContext.mConstructedObject = mObj;
		threadContext.pushInitializer(this);
	}


	ObjectInitializer::~ObjectInitializer()
	{
		if (!bIsDeferredInitializer)
		{
			ObjectThreadContext& threadContext = ObjectThreadContext::get();
			BOOST_ASSERT(threadContext.topInitializer() == this);
			threadContext.popInitializer();
			threadContext.mIsInConstructor--;
			BOOST_ASSERT(threadContext.mIsInConstructor >= 0);
			threadContext.mConstructedObject = mLastConstructedObject;
			BOOST_ASSERT(mObj != nullptr);
		}
		else if (mObj == nullptr)
		{
			return;
		}
		const bool bIsCDO = mObj->hasAnyFlags(RF_ClassDefaultObject);
		RClass* Class = mObj->getClass();
		if (Class != Object::StaticClass())
		{
			/*if(!mObjectArchetype && Class->getClass())
			{
				mObjectArchetype = Class->getDefaultObject();
			}*/
		}
		else if (bIsCDO)
		{
		BOOST_ASSERT(mObjectArchetype == nullptr);
		}

		{
			postConstructInit();
		}
	}

	void ObjectInitializer::postConstructInit()
	{
		if (mObj == nullptr)
		{
			return;
		}

		const bool bIsCDO = mObj->hasAnyFlags(RF_ClassDefaultObject);
		{
			mObj->postInitProperties();
		}

		mObj->clearFlags(RF_NeedInitialization);
		mObj = nullptr;
	}

	std::shared_ptr<Object> ObjectInitializer::createDefaultSubObject(Object* outer, wstring subObjectName, RClass* returnType, RClass* classToCreateByDefault, bool bIsRequired, bool bAbstract, bool bIsTransient) const
	{
		BOOST_ASSERT(ObjectThreadContext::get().mIsInConstructor);
		std::shared_ptr<Object> result;
		RClass* overrideClass = mComponentOverrides.get(subObjectName, returnType, classToCreateByDefault, *this);
		if (!overrideClass && bIsRequired)
		{
			overrideClass = classToCreateByDefault;
		}
		if (overrideClass)
		{
			BOOST_ASSERT(overrideClass->isChildOf(returnType));
			Object* Template = overrideClass->getDefaultObject();
			EObjectFlags subObjectFlags = outer->getMaskedFlags(RF_PropagateToSubObjects);
			result = staticConstructObject_Internal(overrideClass, outer, subObjectName, subObjectFlags);
			if (outer->hasAnyFlags(RF_ClassDefaultObject) && outer->getClass()->getSupperClass())
			{
				outer->getClass()->addDefaultSubobject(result.get(), returnType);
			}
			result->setFlags(RF_DefaultSubobject);
		}

		
		return result;
	}

	

	void Object::postInitProperties()
	{


	}

	Object* staticAllocateObject(RClass* inClass, Object* inOuter, wstring inName, EObjectFlags inFlags, EInternalObjectFlags internalSetFlags, bool bCanRecycleSubobjects, bool* bOutRecycledSubobject)
	{
		Object* obj = nullptr;
		bool bCreatingCDO = (inFlags& RF_ClassDefaultObject) != 0;
		bool bSubObject = false;
		int32 totalSize = inClass->getPropertiesSize();
		BOOST_ASSERT(totalSize);
		if (obj == nullptr)
		{
			int32 alignment = Math::max(4, inClass->getMinAlignment());
			obj = (Object*)GObjectAllocator.allocateObject(totalSize, alignment, false);
		}
		if (!bSubObject)
		{
			Memory::memzero((void*)obj, totalSize);
			new ((void*)obj)ObjectBase(inClass, inFlags | RF_NeedInitialization, internalSetFlags, inOuter, inName);
		}
		else
		{

		}

		if (bOutRecycledSubobject)
		{
			*bOutRecycledSubobject = bSubObject;
		}
		return obj;
	}

	std::shared_ptr<Object> staticConstructObject_Internal(RClass* inClass, Object* inOuter /* = nullptr */, wstring name /* = TEXT("") */, EObjectFlags setFlags /* = RF_NoFlags */, EInternalObjectFlags internalSetFlags /* = EInternalObjectFlags::None */, Object* inTemplate /* = nullptr */, bool bCopyTransientsFromClassDefaults /* = false */)
	{
		Object* result;
		//BOOST_ASSERT(!inTemplate || inTemplate->is)
		bool bRecycledSubobject = false;
		result = staticAllocateObject(inClass, inOuter, name, setFlags, internalSetFlags, bCopyTransientsFromClassDefaults, &bRecycledSubobject);
		ObjectInitializer initializer(result, inTemplate, bCopyTransientsFromClassDefaults, false);
		(*inClass->classConstructor)(initializer);
		auto ptr = initializer.getSharedPtr();
		initializer.mObjectSharedPtr.reset();
		return ptr;
	}

	ObjectInitializer& ObjectInitializer::get()
	{
		ObjectThreadContext& threadContext = ObjectThreadContext::get();
		return threadContext.topInitializerChecked();
	}



	std::shared_ptr<Object> staticLoadObjectInternal(RClass* objectClass, Object* inOuter, const TCHAR* inName, const TCHAR* filename, uint32 loadFlags)
	{
		BOOST_ASSERT(objectClass);
		BOOST_ASSERT(inName);
		wstring strName = inName;
		Object* result = nullptr;
		const bool bContainsObjectName = !!CString::strstr(inName, TEXT("."));
		resolveName(inOuter, strName, true, true, loadFlags);
		if (inOuter)
		{
		}
		return nullptr;
	}

	bool resolveName(Object*& inPackage, wstring& inOutName, bool create, bool bThrow, uint32 loadFlags /* = LOAD_None */)
	{
		return true;
	}

	std::shared_ptr<Object> staticLoadObject(RClass* inClass, Object* inOuter, const TCHAR* name, const TCHAR* filename /* = nullptr */, uint32 loadFlags /* = LOAD_None */)
	{
		BOOST_ASSERT(ObjectThreadContext::get().isRoutingPostLoad && isInAsyncLoadingThread());
		std::shared_ptr<Object> result = staticLoadObjectInternal(inClass, inOuter, name, filename, loadFlags);

		if (!result)
		{
			wstring objectName = name;
			resolveName(inOuter, objectName, true, true, loadFlags & LOAD_EditorOnly);
			if (inOuter == nullptr || LinkerLoad::isKnownMissingPackage(inOuter->getPathName()) == false)
			{
				if (inOuter)
				{
					LinkerLoad::addKnownMissingPackage(inOuter->getPathName());
				}
			}
		}
		return result;
	}

	void ObjectInitializer::assertIfSubobjectSetupIsNotAllowed(const TCHAR* subobjectName) const
	{
		BOOST_ASSERT(bSubobjectClassInitializationAllowed);
	}

	bool ObjectInitializer::islegalOverride(wstring inComponentName, class RClass* derivedComponentClass, class RClass* baseComponentClass) const
	{
		if (derivedComponentClass && baseComponentClass && !derivedComponentClass->isChildOf(baseComponentClass))
		{
			return false;
		}
		return true;
	}

	void ObjectInitializer::initSharedPtr()
	{
		mObjectSharedPtr = std::shared_ptr<Object>(mObj, freeObject);
	}

	COREOBJECT_API void freeObject(Object* obj)
	{
		obj->~Object();
		GObjectAllocator.freeObject(obj);
	}
}
 