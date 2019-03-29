#include "Template/AirTemplate.h"
#include "Serialization/Archive.h"
#include "Misc/CString.h"
#include "boost/assert.hpp"
namespace Air
{
	Archive::Archive()
	{
#if DEVIRTUALIZE_FLinkerLoad_Serialize
	
#endif
		mCustomVersionContainer = new CustomVersionContainer;
		reset();
	}


	Archive::Archive(const Archive& archiveToCopy)
	{
		copyTrivialArchiveStatusMembers(archiveToCopy);

		mArIsFilterEditorOnly = false;

		bCustomVersionsAreReset = archiveToCopy.bCustomVersionsAreReset;

		mCustomVersionContainer = new CustomVersionContainer(*archiveToCopy.mCustomVersionContainer);
	}
	Archive& Archive::operator=(const Archive & archiveToCopy)
	{
		copyTrivialArchiveStatusMembers(archiveToCopy);
		mArIsFilterEditorOnly = false;
		bCustomVersionsAreReset = archiveToCopy.bCustomVersionsAreReset;
		*mCustomVersionContainer = *archiveToCopy.mCustomVersionContainer;
		return *this;
	}

	void Archive::byteSwap(void* v, int32 length)
	{
		uint8* ptr = (uint8*)v;
		int32 top = length - 1;
		int32 bottom = 0;
		while (bottom < top)
		{
			Swap(ptr[top--], ptr[bottom++]);
		}
	}

	Archive::~Archive()
	{
		delete mCustomVersionContainer;
	}

	void Archive::reset()
	{
#if DEVIRTUALIZE_FLinkerLoad_Serialize
		
#endif
		mArIsLoading = false;
		mArIsSaving = false;
		mArIsTransacting = false;
		mArWantBinaryPropertySerialization = false;
		mArForceUnicode = false;
		mArIsPersistent = false;
		mArIsError = false;
		mArIsCriticalError = false;
		mArContainsCode = false;
		mArContainsMap = false;
		mArRequiresLocalizationGather = false;
		mArForceByteSwapping = false;
		mArSerializingDefaults = false;
		mArIgnoreArchetypeRef = false;
		mArNoDelta = false;
		mArIgnoreOuterRef = false;
		mArIgnoreClassGeneratedByRef = false;
		mArIgnoreClassRef = false;
		mArAllowLazyLoading = false;
		mArIsObjectReferenceCollector = false;
		mArIsModifyingWeakAndStrongReferences = false;
		mArIsCountingMemory = false;
		mArPortFlags = 0;
		mArShouldSkipBulkData = false;
		mArMaxSerializeSize = 0;
		mArIsFilterEditorOnly = false;
		mArIsSaveGame = false;
		mArCustomPropertyList = nullptr;
		mArUseCustomPrepertyList = false;
		mCookingTargetPlatform = nullptr;
		mSerializedProperty = nullptr;
		
	}

	void Archive::copyTrivialArchiveStatusMembers(const Archive& archiveToCopy)
	{
		mArIsLoading = archiveToCopy.mArIsLoading;
		mArIsSaving = archiveToCopy.mArIsSaving;
		mArIsTransacting = archiveToCopy.mArIsTransacting;
		mArWantBinaryPropertySerialization = archiveToCopy.mArWantBinaryPropertySerialization;
		mArForceUnicode = archiveToCopy.mArForceUnicode;
		mArIsPersistent = archiveToCopy.mArIsPersistent;
		mArIsError = archiveToCopy.mArIsError;
		mArIsCriticalError = archiveToCopy.mArIsCriticalError;
		mArContainsCode = archiveToCopy.mArContainsCode;
		mArContainsMap = archiveToCopy.mArContainsMap;
		mArRequiresLocalizationGather = archiveToCopy.mArRequiresLocalizationGather;
		mArForceByteSwapping = archiveToCopy.mArForceByteSwapping;
		mArSerializingDefaults = archiveToCopy.mArSerializingDefaults;
		mArIgnoreArchetypeRef = archiveToCopy.mArIgnoreArchetypeRef;
		mArNoDelta = archiveToCopy.mArNoDelta;
		mArIgnoreOuterRef = archiveToCopy.mArIgnoreOuterRef;
		mArIgnoreClassGeneratedByRef = archiveToCopy.mArIgnoreClassGeneratedByRef;
		mArIgnoreClassRef = archiveToCopy.mArIgnoreClassRef;
		mArAllowLazyLoading = archiveToCopy.mArAllowLazyLoading;
		mArIsObjectReferenceCollector = archiveToCopy.mArIsObjectReferenceCollector;
		mArIsModifyingWeakAndStrongReferences = archiveToCopy.mArIsModifyingWeakAndStrongReferences;
		mArIsCountingMemory = archiveToCopy.mArIsCountingMemory;
		mArPortFlags = archiveToCopy.mArPortFlags;
		mArShouldSkipBulkData = archiveToCopy.mArShouldSkipBulkData;
		mArMaxSerializeSize = archiveToCopy.mArMaxSerializeSize;
		mArIsFilterEditorOnly = archiveToCopy.mArIsFilterEditorOnly;
		mArIsSaveGame = archiveToCopy.mArIsSaveGame;
		mArCustomPropertyList = archiveToCopy.mArCustomPropertyList;
		mArUseCustomPrepertyList = archiveToCopy.mArUseCustomPrepertyList;
		mCookingTargetPlatform = archiveToCopy.mCookingTargetPlatform;
		mSerializedProperty = archiveToCopy.mSerializedProperty;
	}

	wstring Archive::getArchiveName() const
	{
		return TEXT("Archive");
	}


}