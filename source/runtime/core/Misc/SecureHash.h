#pragma once
#include "CoreType.h"
#include "Containers/Map.h"
#include "Containers/String.h"
#include "HAL/AirMemory.h"
#include "Async/AsyncWork.h"
#include "Serialization/BufferReader.h"
#include "Serialization/Archive.h"
#include "Containers/StringConv.h"
namespace Air
{
	typedef union
	{
		uint8 c[64];
		uint32 l[16];
	}SHA1_WORKSPACE_BLOCK;

#define HASHES_SHA_DIVIDER "+++"


	class CORE_API SHA1
	{
	public:
		enum { DigestSize = 20 };
		SHA1();
		~SHA1();

		uint32 mState[5];
		uint32 mCount[2];
		uint32 __reserved1[1];
		uint8 mBuffer[64];
		uint8 mDigest[20];
		uint32 __reserved2[3];

		void reset();

		void update(const uint8* data, uint32 len);

		void updateWithString(const TCHAR* data, uint32 len);

		void finalize();

		void getHash(uint8* puDest);

		static void hashBuffer(const void* data, uint32 dataSize, uint8 * outHash);

		static void HMACBuffer(const void* key, uint32 keySize, const void* data, uint32 dataSize, uint8* outHash);

		static void initializeFileHashesFromBuffer(uint8* buffer, int32 bufferSize, bool bDuplicateKeyMemory = false);

		static bool getFileSHAHash(const TCHAR* pathName, uint8 hash[20], bool bIsFullPackageHash = false);

	private:
		void transform(uint32 * state, const uint8 * buffer);

		uint8 mWorkspace[64];
		SHA1_WORKSPACE_BLOCK *mBlock;

		static TMap<wstring, uint8*> mFullFileSHAHashMap;
		static TMap<wstring, uint8*> mScriptSHAHashMap;
	};

	class CORE_API SHAHash
	{
	public:
		uint8 mHash[20];
		SHAHash()
		{
			Memory::Memset(mHash, 0, sizeof(mHash));
		}

		inline wstring toString() const
		{
			return bytesToHex((const uint8*)mHash, sizeof(mHash));
		}

		friend bool operator == (const SHAHash & x, const SHAHash & Y)
		{
			return Memory::memcmp(&x.mHash, &Y.mHash, sizeof(x.mHash)) == 0;
		}
		friend bool operator !=(const SHAHash& x, const SHAHash & y)
		{
			return Memory::memcmp(&x.mHash, &y.mHash, sizeof(x.mHash)) != 0;
		}

		friend CORE_API Archive& operator << (Archive& ar, SHAHash& hash)
		{
			ar.serialize(&hash.mHash, sizeof(hash.mHash));
			return ar;
		}

	};

	class CORE_API AsyncSHAVerify
	{
	protected:
		void* mBuffer;
		int32 mBufferSize;
		uint8 mHash[20];
		wstring mPathname;

		bool bIsUnfoundHashAnError;
		bool bShoudDeleteBuffer;
	public:
		AsyncSHAVerify(void* inBuffer, int32 inmBufferSize, bool bInShouldDeleteBuffer, const TCHAR* inPathname, bool bInIsUnfoundHashAnError)
			:mBuffer(inBuffer)
			,mBufferSize(inmBufferSize)
			,mPathname(inPathname)
			,bIsUnfoundHashAnError(bInIsUnfoundHashAnError)
			,bShoudDeleteBuffer(bInShouldDeleteBuffer)
		{}

		void doWork() {}

		bool canAbandon()
		{
			return true;
		}

		void abandon()
		{
			if (bShoudDeleteBuffer)
			{
				Memory::free(mBuffer);
				mBuffer = nullptr;
			}
		}
	};




	class BufferReaderWithSHA : public BufferReaderBase
	{
	public:
		BufferReaderWithSHA(void* data, int32 size, bool bInFreeOnClose, const TCHAR* SHASourcePathname, bool bIsPersistent = false, bool bInIsUnfoundHashAnError = false)
			:BufferReaderBase(data, size, bInFreeOnClose, bIsPersistent)
			,mSourcePathname(SHASourcePathname)
			,bIsUnfoundHashAnError(bInIsUnfoundHashAnError)
		{}

		~BufferReaderWithSHA()
		{
			close();
		}

		bool close()
		{
			if (mReaderData)
			{
				(new AutoDeleteAsyncTask<AsyncSHAVerify>(mReaderData, mReaderSize, bFreeOnClose, mSourcePathname.c_str(), bIsUnfoundHashAnError))->startBackgroundTask();
				mReaderData = nullptr;
			}

			return !mArIsError;
		}

		virtual wstring getArchiveName() const { return TEXT("BufferReaderWithSHA"); }
	protected:
		wstring mSourcePathname;
		bool bIsUnfoundHashAnError;
	};

	struct MD5Hash;

	namespace lex
	{
		CORE_API wstring toString(const MD5Hash& hash);

		CORE_API void fromString(MD5Hash& hash, const TCHAR* buffer);
	}

	class CORE_API MD5
	{
	public:
		MD5();
		~MD5();

		void update(const uint8* input, int32 inputLength);

		void finalization(uint8* digest);

		static wstring hashAnsicString(const TCHAR* string)
		{
			uint8  digest[16];
			MD5 md5Gen;
			md5Gen.update((unsigned char*)TCHAR_TO_ANSI(string), CString::strlen(string));
			md5Gen.finalization(digest);
			wstring md5;
			for (int32 i = 0; i < 16; i++)
			{
				md5 += printf(TEXT("%02x"), digest[i]);
			}
			return md5;
		}
	private:
		struct Context
		{
			uint32 state[4];
			uint32 count[2];
			uint8 buffer[64];
		};

		void transform(uint32* state, const uint8* block);
		void encode(uint8* output, const uint32* input, int32 len);
		void decode(uint32* output, const uint8* input, int32 len);

		Context mContext;
	};

	struct MD5Hash
	{
		MD5Hash() :bIsValid(false) {}
		bool isValid() const { return bIsValid; }
		void set(MD5& md5)
		{
			md5.finalization(mBytes);
			bIsValid = true;
		}

		friend bool operator == (const MD5Hash& lhs, const MD5Hash& rhs)
		{
			return lhs.bIsValid == rhs.bIsValid && (!lhs.bIsValid || Memory::memcmp(lhs.mBytes, rhs.mBytes, 16) == 0);
		}

		friend bool operator !=(const MD5Hash& lhs, const MD5Hash& rhs)
		{
			return lhs.bIsValid != rhs.bIsValid || (lhs.bIsValid && Memory::memcmp(lhs.mBytes, rhs.mBytes, 16) != 0);
		}

		friend Archive& operator << (Archive& ar, MD5Hash& hash)
		{
			ar << hash.bIsValid;
			if (hash.bIsValid)
			{
				ar.serialize(hash.mBytes, 16);
			}
			return ar;
		}

		CORE_API static MD5Hash hashFile(const TCHAR* inFilename, TArray<uint8>* buffer = nullptr);
		CORE_API static MD5Hash hashFileFromArchive(Archive* ar, TArray<uint8>* scratchPad = nullptr);

		const uint8* getBytes() const { return mBytes; }
		const int32 getSize() const { return sizeof(mBytes); }
	private:
		bool bIsValid;
		uint8 mBytes[16];
		friend wstring lex::toString(const MD5Hash& hash);
		friend void lex::fromString(MD5Hash& hash, const TCHAR* buffer);
	};

#define PRIME_NUM	0x9e3779b9

		size_t constexpr CTHashImpl(char const* str, size_t seed)
		{
			return 0 == *str ? seed : CTHashImpl(str + 1, seed ^ (*str + PRIME_NUM + (seed << 6) + (seed >> 2)));
		}



#if AIR_COMPILER_MSVC
		template<size_t N>
		struct EnsureConst
		{
			static size_t constexpr value = N;
		};

#define CT_HASH(x) (EnsureConst<CTHashImpl(x, 0)>::value)
#else
#define CT_HASH(x) (CTHashImpl(x, 0))
#endif

		template<typename SizeT>
		inline void hashCombineImpl(SizeT& seed, SizeT value)
		{
			seed ^= value + PRIME_NUM + (seed << 6) + (seed >> 2);
		}

		inline size_t RT_HASH(char const* str)
		{
			size_t seed = 0;
			while (*str != 0)
			{
				hashCombineImpl(seed, static_cast<size_t>(*str));
				++str;
			}
			return seed;
		}
#undef PRIME_NUM
}