#pragma once
#include "CoreType.h"
#include "Containers/String.h"
#include "Serialization/CustomVersion.h"
namespace Air
{

#define DEVIRTUALIZE_FLinkerLoad_Serialize (!WITH_EDITORONLY_DATA)
	class ITargetPlatform;
	class Property;

	struct UntypedBulkData;

	class CORE_API Archive
	{
	public:
		Archive();
		Archive(const Archive&);

		Archive& operator = (const Archive& archiveToCopy);

		virtual ~Archive();

		FORCEINLINE bool isLoading() const
		{
			return mArIsLoading;
		}

		FORCEINLINE bool isSaving() const
		{
			return mArIsSaving;
		}

		virtual void serialize(void* v, int64 length) { BOOST_ASSERT(false); }


		virtual void serializeBits(void* v, int64 lengthBits)
		{
			serialize(v, (lengthBits + 7) / 8);
		}

		virtual void serializeInt(uint32& value, uint32 max)
		{
			byteOrderSerialize(&value, sizeof(value));
		}

		void byteSwap(void*, int32 length);

		virtual bool close()
		{
			return !mArIsError;
		}

		virtual void seek(int64 inPos) {}

		virtual int64 tell()
		{
			return INDEX_NONE;
		}
		virtual int64 totalSize()
		{
			return INDEX_NONE;
		}

		virtual wstring getArchiveName() const;

		FORCEINLINE Archive& byteOrderSerialize(void* v, int32 lenght)
		{
			serialize(v, lenght);
			if (isByteSwapping())
			{
				byteSwap(v, lenght);
			}
			return *this;
		}


		FORCEINLINE int64 getMaxSerializeSize() const
		{
			return mArMaxSerializeSize;
		}

		FORCEINLINE bool isError() const

		{
			return mArIsError;
		}

		FORCEINLINE bool isForcingUnicode() const
		{
			return mArForceUnicode;
		}

		FORCEINLINE bool isByteSwapping()
		{
#if PLATFORM_LITTLE_ENDIAN
			bool swapBytes = mArForceByteSwapping;
#else
			bool swapBytes = mArIsPersistent;
#endif
			return swapBytes;
		}

		virtual void countBytes(SIZE_T inNum, SIZE_T inMax){}

	

		template<typename Type>
		FORCEINLINE friend Archive& operator << (Archive& ar, Type& value)
		{
			ar.byteOrderSerialize(&value, sizeof(Type));
			return ar;
		}

		template<typename Type>
		FORCEINLINE friend Archive& operator << (Archive& ar, Type&& value)
		{
			BOOST_ASSERT(ar.isSaving());
			ar.byteOrderSerialize(&value, sizeof(Type));
			return ar;
		}

		FORCEINLINE bool isCountingMemory() const
		{
			return mArIsCountingMemory;
		}


		friend CORE_API Archive& operator << (Archive& ar, wstring& value);

		friend CORE_API Archive& operator << (Archive& ar, TCHAR* value);

		void reset();

		void copyTrivialArchiveStatusMembers(const Archive& archiveToCopy);

		virtual void detachBulkData(UntypedBulkData* bulkData, bool bEnsureBulkDataIsLoaded);
	public:

		const struct CustomPropertyListNode* mArCustomPropertyList;


		uint8 mArIsLoading : 1;
		uint8 mArIsSaving : 1;
		uint8 mArIsTransacting : 1;
		uint8 mArWantBinaryPropertySerialization : 1;
		uint8 mArForceUnicode : 1;
		uint8 mArIsPersistent : 1;
		uint8 mArIsError : 1;
		uint8 mArIsCriticalError : 1;
		uint8 mArContainsCode : 1;
		uint8 mArContainsMap : 1;
		uint8 mArRequiresLocalizationGather : 1;
		uint8 mArForceByteSwapping : 1;
		uint8 mArIgnoreArchetypeRef : 1;
		uint8 mArNoDelta : 1;
		uint8 mArIgnoreOuterRef : 1;
		uint8 mArIgnoreClassGeneratedByRef : 1;
		uint8 mArIgnoreClassRef : 1;
		uint8 mArAllowLazyLoading : 1;
		uint8 mArIsObjectReferenceCollector : 1;
		uint8 mArIsModifyingWeakAndStrongReferences : 1;
		uint8 mArIsCountingMemory : 1;
		uint8 mArShouldSkipBulkData : 1;
		uint8 mArIsFilterEditorOnly : 1;
		uint8 mArIsSaveGame : 1;
		uint8 mArUseCustomPrepertyList : 1;
		int32 mArSerializingDefaults;
		uint32 mArPortFlags;
		int64 mArMaxSerializeSize;
		
	private:
		CustomVersionContainer* mCustomVersionContainer;
		mutable bool bCustomVersionsAreReset;

		const ITargetPlatform* mCookingTargetPlatform;

		class Property* mSerializedProperty;
	};
}