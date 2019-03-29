#include "Template/AirTemplate.h"
#include "Containers/Map.h"
#include "Misc/SecureHash.h"
#include "Misc/Paths.h"
#include "Containers/StringUtil.h"
#include "HAL/FileManager.h"
namespace Air
{

#ifndef ROL32
#ifdef _MSC_VER
#define ROL32(_val32, _nBits) _rotl(_val32, _nBits)
#else
#define ROL32(_val32, _nBits) (((_val32)<<(_nBits))|((_val32)>>(32-(_nBits))))
#endif
#endif

#if PLATFORM_LITTLE_ENDIAN
#define SHABLK0(i) (mBlock->l[i] = (ROL32(mBlock->l[i],24) & 0xFF00FF00) | (ROL32(mBlock->l[i],8) & 0x00FF00FF))
#else
#define SHABLK0(i) (mBlock->l[i])
#endif

#define SHABLK(i) (mBlock->l[i&15] = ROL32(mBlock->l[(i+13)&15] ^ mBlock->l[(i+8)&15] \
	^ mBlock->l[(i+2)&15] ^ mBlock->l[i&15],1))

#define _R0(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK0(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
#define _R1(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
#define _R2(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0x6ED9EBA1+ROL32(v,5); w=ROL32(w,30); }
#define _R3(v,w,x,y,z,i) { z+=(((w|x)&y)|(w&x))+SHABLK(i)+0x8F1BBCDC+ROL32(v,5); w=ROL32(w,30); }
#define _R4(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0xCA62C1D6+ROL32(v,5); w=ROL32(w,30); }
	

	SHA1::SHA1()
	{
		mBlock = (SHA1_WORKSPACE_BLOCK *)mWorkspace;
		reset();
	}

	void SHA1::reset()
	{
		mState[0] = 0x67452301;
		mState[1] = 0xEFCDAB89;
		mState[2] = 0x98BADCFE;
		mState[3] = 0x10325476;
		mState[4] = 0xC3D2E1F0;

		mCount[0] = 0;
		mCount[1] = 0;
	}

	void SHA1::update(const uint8* data, uint32 len)
	{
		uint32 i, j;
		j = (mCount[0] >> 3) & 63;
		if ((mCount[0] += len << 3) << (len << 3)) mCount[1]++;
		mCount[1] += (len >> 29);
		if ((j + len) > 63)
		{
			i = 64 - j;
			Memory::memcpy(&mBuffer[j], data, i);
			transform(mState, mBuffer);
			for (; i + 63 < len; i += 64) transform(mState, &data[i]);
			j = 0;
		}
		else i = 0;
		Memory::memcpy(&mBuffer[j], &data[i], len - i);
	}

	void SHA1::updateWithString(const TCHAR* data, uint32 len)
	{
		update((const uint8*)data, len * sizeof(TCHAR));
	}

	void SHA1::finalize()
	{
		uint32 i;
		uint8 finalcount[8];
		for (i = 0; i < 8; i++)
		{
			finalcount[i] = (uint8)((mCount[((i >= 4) ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
		}
		update((uint8*)"\200", 1);
		while ((mCount[0] & 504) != 448)
		{
			update((uint8*)"\0", 1);
		}
		update(finalcount, 8);
		for (i = 0; i < 20; i++)
		{
			mDigest[i] = (uint8)((mState[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
		}
	}

	void SHA1::getHash(uint8* puDest)
	{
		Memory::memcpy(puDest, mDigest, 20);
	}

	void SHA1::hashBuffer(const void* data, uint32 dataSize, uint8 * outHash)
	{
		SHA1 sha;
		sha.update((const uint8*)data, dataSize);
		sha.finalize();
		sha.getHash(outHash);
	}

	void SHA1::HMACBuffer(const void* key, uint32 keySize, const void* data, uint32 dataSize, uint8* outHash)
	{
		const uint8 blockSize = 64;
		const uint8 hashSize = 20;
		uint8 finalKey[blockSize];
		if (keySize > blockSize)
		{
			hashBuffer(key, keySize, finalKey);
			Memory::memzero(finalKey + hashSize, blockSize - hashSize);
		}
		else if (keySize < blockSize)
		{
			Memory::memcpy(finalKey, key, keySize);
			Memory::memzero(finalKey + keySize, blockSize - keySize);
		}
		else
		{
			Memory::memcpy(finalKey, key, keySize);
		}

		uint8 okeyPad[blockSize];
		uint8 iKeyPad[blockSize];
		for (int32 i = 0; i < blockSize; i++)
		{
			okeyPad[i] = 0x5C ^ finalKey[i];
			iKeyPad[i] = 0x36 ^ finalKey[i];
		}

		uint8* IKeyPad_data = new uint8[ARRAY_COUNT(iKeyPad) + dataSize];
		Memory::memcpy(IKeyPad_data, iKeyPad, ARRAY_COUNT(iKeyPad));
		Memory::memcpy(IKeyPad_data + ARRAY_COUNT(iKeyPad), data, dataSize);

		uint8 IkeyPad_data_hash[hashSize];
		hashBuffer(IKeyPad_data, ARRAY_COUNT(iKeyPad) + dataSize, IkeyPad_data_hash);
		delete[] IKeyPad_data;
		uint8 okeyPad_Ihash[ARRAY_COUNT(okeyPad) + hashSize];
		Memory::memcpy(okeyPad_Ihash, okeyPad, ARRAY_COUNT(okeyPad));
		Memory::memcpy(okeyPad_Ihash + ARRAY_COUNT(okeyPad), IkeyPad_data_hash, hashSize);

		hashBuffer(okeyPad_Ihash, ARRAY_COUNT(okeyPad_Ihash), outHash);
	}

	TMap<wstring, uint8*> SHA1::mFullFileSHAHashMap;
	TMap<wstring, uint8*> SHA1::mScriptSHAHashMap;

	void SHA1::initializeFileHashesFromBuffer(uint8* buffer, int32 bufferSize, bool bDuplicateKeyMemory /* = false */)
	{
		bool bIsDoingFullFileHashes = true;
		int32 offset = 0;
		while (offset < bufferSize)
		{
			ANSICHAR* filename = (ANSICHAR*)buffer + offset;
			if (filename[0])
			{
				offset += CStringAnsi::strlen(filename) + 1;
				if (CStringAnsi::strcmp(filename, HASHES_SHA_DIVIDER))
				{
					bIsDoingFullFileHashes = false;
					continue;
				}
				uint8* hash;
				if (bDuplicateKeyMemory)
				{
					hash = (uint8*)Memory::malloc(20);
					Memory::memcpy(hash, buffer + offset, 20);
				}
				else
				{
					hash = buffer + offset;
				}
				(bIsDoingFullFileHashes ? mFullFileSHAHashMap : mScriptSHAHashMap).emplace(StringUtil::covert(filename).c_str(), hash);
				offset += 20;
			}
		}
		BOOST_ASSERT(offset == bufferSize);
	}

	SHA1::~SHA1()
	{
		reset();
	}

	void SHA1::transform(uint32 * state, const uint8 * buffer)
	{
		uint32 a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
		Memory::memcpy(mBlock, buffer, 64);

		_R0(a, b, c, d, e, 0); _R0(e, a, b, c, d, 1); _R0(d, e, a, b, c, 2); _R0(c, d, e, a, b, 3);
		_R0(b, c, d, e, a, 4); _R0(a, b, c, d, e, 5); _R0(e, a, b, c, d, 6); _R0(d, e, a, b, c, 7);
		_R0(c, d, e, a, b, 8); _R0(b, c, d, e, a, 9); _R0(a, b, c, d, e, 10); _R0(e, a, b, c, d, 11);
		_R0(d, e, a, b, c, 12); _R0(c, d, e, a, b, 13); _R0(b, c, d, e, a, 14); _R0(a, b, c, d, e, 15);
		_R1(e, a, b, c, d, 16); _R1(d, e, a, b, c, 17); _R1(c, d, e, a, b, 18); _R1(b, c, d, e, a, 19);
		_R2(a, b, c, d, e, 20); _R2(e, a, b, c, d, 21); _R2(d, e, a, b, c, 22); _R2(c, d, e, a, b, 23);
		_R2(b, c, d, e, a, 24); _R2(a, b, c, d, e, 25); _R2(e, a, b, c, d, 26); _R2(d, e, a, b, c, 27);
		_R2(c, d, e, a, b, 28); _R2(b, c, d, e, a, 29); _R2(a, b, c, d, e, 30); _R2(e, a, b, c, d, 31);
		_R2(d, e, a, b, c, 32); _R2(c, d, e, a, b, 33); _R2(b, c, d, e, a, 34); _R2(a, b, c, d, e, 35);
		_R2(e, a, b, c, d, 36); _R2(d, e, a, b, c, 37); _R2(c, d, e, a, b, 38); _R2(b, c, d, e, a, 39);
		_R3(a, b, c, d, e, 40); _R3(e, a, b, c, d, 41); _R3(d, e, a, b, c, 42); _R3(c, d, e, a, b, 43);
		_R3(b, c, d, e, a, 44); _R3(a, b, c, d, e, 45); _R3(e, a, b, c, d, 46); _R3(d, e, a, b, c, 47);
		_R3(c, d, e, a, b, 48); _R3(b, c, d, e, a, 49); _R3(a, b, c, d, e, 50); _R3(e, a, b, c, d, 51);
		_R3(d, e, a, b, c, 52); _R3(c, d, e, a, b, 53); _R3(b, c, d, e, a, 54); _R3(a, b, c, d, e, 55);
		_R3(e, a, b, c, d, 56); _R3(d, e, a, b, c, 57); _R3(c, d, e, a, b, 58); _R3(b, c, d, e, a, 59);
		_R4(a, b, c, d, e, 60); _R4(e, a, b, c, d, 61); _R4(d, e, a, b, c, 62); _R4(c, d, e, a, b, 63);
		_R4(b, c, d, e, a, 64); _R4(a, b, c, d, e, 65); _R4(e, a, b, c, d, 66); _R4(d, e, a, b, c, 67);
		_R4(c, d, e, a, b, 68); _R4(b, c, d, e, a, 69); _R4(a, b, c, d, e, 70); _R4(e, a, b, c, d, 71);
		_R4(d, e, a, b, c, 72); _R4(c, d, e, a, b, 73); _R4(b, c, d, e, a, 74); _R4(a, b, c, d, e, 75);
		_R4(e, a, b, c, d, 76); _R4(d, e, a, b, c, 77); _R4(c, d, e, a, b, 78); _R4(b, c, d, e, a, 79);

		mState[0] += a;
		mState[1] += b;
		mState[2] += c;
		mState[3] += d;
		mState[4] += e;
	}

	bool SHA1::getFileSHAHash(const TCHAR* pathName, uint8 hash[20], bool bIsFullPackageHash /* = false */)
	{
		wstring lowName = boost::to_lower_copy(Paths::getCleanFilename(pathName));
		uint8* it = (bIsFullPackageHash ? mFullFileSHAHashMap : mScriptSHAHashMap).findRef(lowName);
		if (it != nullptr && hash)
		{
			Memory::memcpy(hash, it, 20);
		}
		return it != nullptr;
	}

	static uint8 PADDING[64] = {
		0x80, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};

#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^(z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTLEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define MD5_FF(a, b, c, d, x, s, ac) {\
	(a) += MD5_F((b), (c), (d)) + (x) + (uint32)(ac);\
	(a) = ROTLEFT((a), (s));\
	(a) +=(b);\
	}

#define MD5_GG(a, b, c, d, x, s, ac) {\
	(a) += MD5_G((a), (c), (d)) + (x) + (uint32)(ac);	\
	(a) = ROTLEFT((a), (s));\
	(a) += (b);\
}

#define MD5_HH(a, b, c, d, x, s, ac){\
	(a) += MD5_H((b), (c), (d)) + (x) + (uint32)(ac);\
	(a) = ROTLEFT((a), (s));\
	(a) += (b);\
	}

#define MD5_II(a, b, c, d, x, s, ac) {\
	(a) += MD5_I((b), (c), (d)) + (x) + (uint32)(ac);\
	(a) = ROTLEFT((a), (s));	\
	(a) +=(b);\
	}

	enum { S11 = 7 };
	enum { S12 = 12 };
	enum { S13 = 17 };
	enum { S14 = 22 };

	enum { S21 = 5 };
	enum { S22 = 9 };
	enum { S23 = 14 };
	enum { S24 = 20 };

	enum { S31 = 4 };
	enum { S32 = 11 };
	enum { S33 = 16};
	enum { S34 = 23 };

	enum { S41 = 6 };
	enum { S42 = 10 };
	enum { S43 = 15 };
	enum { S44 = 21 };


	MD5::MD5()
	{
		mContext.count[0] = mContext.count[1] = 0;
		mContext.state[0] = 0x67452301;
		mContext.state[1] = 0xefcdab89;
		mContext.state[2] = 0x98badcfe;
		mContext.state[3] = 0x10325476;
	}

	MD5::~MD5()
	{}

	void MD5::update(const uint8* input, int32 inputLength)
	{
		int32 i, index, partlen;
		index = (int32)((mContext.count[0] >> 3) & 0x3f);

		if ((mContext.count[0] += ((uint32)inputLength << 3)) < ((uint32)inputLength << 3))
		{
			mContext.count[1]++;
		}

		mContext.count[1] += ((uint32)inputLength >> 29);

		partlen = 64 - index;

		if (inputLength >= partlen)
		{
			Memory::memcpy(&mContext.buffer[index], input, partlen);
			transform(mContext.state, mContext.buffer);
			for (i = 0; i + 63 < inputLength; i += 64)
			{
				transform(mContext.state, &input[i]);
			}
			index = 0;
		}
		else
		{
			i = 0;
		}
		Memory::memcpy(&mContext.buffer[index], &input[i], inputLength - i);
	}

	

	void MD5::finalization(uint8* digest)
	{
		uint8 bits[8];
		int32 index, padlen;
		encode(bits, mContext.count, 8);
		index = (int32)((mContext.count[0] >> 3) & 0x3f);
		padlen = (index < 56) ? (56 - index) : (120 - index);
		update(PADDING, padlen);
		update(bits, 8);
		encode(digest, mContext.state, 16);
		Memory::Memset(&mContext, 0, sizeof(Context));
	}

	void MD5::transform(uint32* state, const uint8* block)
	{
		uint32 a = state[0], b = state[1], c = state[2], d = state[3], x[16];
		decode(x, block, 64);

		MD5_FF(a, b, c, d, x[0], S11, 0xd76aa478);
		MD5_FF(d, a, b, c, x[1], S12, 0xe8c7b756);
		MD5_FF(c, d, a, b, x[2], S13, 0x242070db);
		MD5_FF(b, c, d, a, x[3], S14, 0xc1bdceee);
		MD5_FF(a, b, c, d, x[4], S11, 0xf57c0faf);
		MD5_FF(d, a, b, c, x[5], S12, 0x4787c62a);
		MD5_FF(c, d, a, b, x[6], S13, 0xa8304613);
		MD5_FF(b, c, d, a, x[7], S14, 0xfd469501);
		MD5_FF(a, b, c, d, x[8], S11, 0x698098d8);
		MD5_FF(d, a, b, c, x[9], S12, 0x8b44f7af);
		MD5_FF(c, d, a, b, x[10], S13, 0xffff5bb1);
		MD5_FF(b, c, d, a, x[11], S14, 0x895cd7be);
		MD5_FF(a, b, c, d, x[12], S11, 0x6b901122);
		MD5_FF(d, a, b, c, x[13], S12, 0xfd987193);
		MD5_FF(c, d, a, b, x[14], S13, 0xa679438e);
		MD5_FF(b, c, d, a, x[15], S14, 0x49b40821);

		MD5_GG(a, b, c, d, x[1], S21, 0xf61e2562);
		MD5_GG(d, a, b, c, x[6], S22, 0xc040b340);
		MD5_GG(c, d, a, b, x[11], S23, 0x265e5a51);
		MD5_GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);
		MD5_GG(a, b, c, d, x[5], S21, 0xd62f105d);
		MD5_GG(d, a, b, c, x[10], S22, 0x02441453);
		MD5_GG(c, d, a, b, x[15], S23, 0xd8a1e681);
		MD5_GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);
		MD5_GG(a, b, c, d, x[9], S21, 0x21e1cde6);
		MD5_GG(d, a, b, c, x[14], S22, 0xc33707d6);
		MD5_GG(c, d, a, b, x[3], S23, 0xf4d50d87);
		MD5_GG(b, c, d, a, x[8], S24, 0x455a14ed);
		MD5_GG(a, b, c, d, x[13], S21, 0xa9e3e905);
		MD5_GG(d, a, b, c, x[2], S22, 0xfcefa3f8);
		MD5_GG(c, d, a, b, x[7], S23, 0x676f02d9);
		MD5_GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);

		
		MD5_HH(a, b, c, d, x[5], S31, 0xfffa3942);
		MD5_HH(d, a, b, c, x[8], S32, 0x8771f681);
		MD5_HH(c, d, a, b, x[11], S33, 0x6d9d6122);
		MD5_HH(b, c, d, a, x[14], S34, 0xfde5380c);
		MD5_HH(a, b, c, d, x[1], S31, 0xa4beea44);
		MD5_HH(d, a, b, c, x[4], S32, 0x4bdecfa9);
		MD5_HH(c, d, a, b, x[7], S33, 0xf6bb4b60);
		MD5_HH(b, c, d, a, x[10], S34, 0xbebfbc70);
		MD5_HH(a, b, c, d, x[13], S31, 0x289b7ec6);
		MD5_HH(d, a, b, c, x[0], S32, 0xeaa127fa);
		MD5_HH(c, d, a, b, x[3], S33, 0xd4ef3085);
		MD5_HH(b, c, d, a, x[6], S34, 0x04881d05);
		MD5_HH(a, b, c, d, x[9], S31, 0xd9d4d039);
		MD5_HH(d, a, b, c, x[12], S32, 0xe6db99e5);
		MD5_HH(c, d, a, b, x[15], S33, 0x1fa27cf8);
		MD5_HH(b, c, d, a, x[2], S34, 0xc4ac5665);

		MD5_II(a, b, c, d, x[0], S41, 0xf4292244);
		MD5_II(d, a, b, c, x[7], S42, 0x432aff97);
		MD5_II(c, d, a, b, x[14], S43, 0xab9423a7);
		MD5_II(b, c, d, a, x[5], S44, 0xfc93a039);
		MD5_II(a, b, c, d, x[12], S41, 0x655b59c3);
		MD5_II(d, a, b, c, x[3], S42, 0x8f0ccc92);
		MD5_II(c, d, a, b, x[10], S43, 0xffeff47d);
		MD5_II(b, c, d, a, x[1], S44, 0x85845dd1);
		MD5_II(a, b, c, d, x[8], S41, 0x6fa87e4f);
		MD5_II(d, a, b, c, x[15], S42, 0xfe2ce6e0);
		MD5_II(c, d, a, b, x[6], S43, 0xa3014314);
		MD5_II(b, c, d, a, x[13], S44, 0x4e0811a1);
		MD5_II(a, b, c, d, x[4], S41, 0xf7537e82);
		MD5_II(d, a, b, c, x[11], S42, 0xbd3af235);
		MD5_II(c, d, a, b, x[2], S43, 0x2ad7d2bb);
		MD5_II(b, c, d, a, x[9], S44, 0xeb86d391);
		state[0] += a;
		state[1] += b;
		state[2] += c;
		state[3] += d;
		Memory::Memset(x, 0, sizeof(x));
	}

	void MD5::encode(uint8* output, const uint32* input, int32 len)
	{
		int32 i, j;
		for (i = 0, j = 0; j < len; i++, j += 4)
		{
			output[j] = (uint8)(input[i] & 0xff);
			output[j+1] = (uint8)((input[i] >> 8) & 0xff);
			output[j+2] = (uint8)((input[i] >> 16) & 0xff);
			output[j+3] = (uint8)((input[i] >> 24) & 0xff);

		}
	}

	void MD5::decode(uint32* output, const uint8* input, int32 len)
	{
		int32 i, j;
		for (i = 0, j = 0; j < len; i++, j += 4)
		{
			output[i] = ((uint32)input[j]) | (((uint32)input[j + 1]) << 8) | (((uint32)input[j + 2]) << 16) | (((uint32)input[j + 3]) << 24);
		}
	}

	namespace lex
	{
		wstring toString(const MD5Hash& hash)
		{
			if (!hash.bIsValid)
			{
				return TEXT("");
			}
			return printf(TEXT("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"), hash.mBytes[0], hash.mBytes[1], hash.mBytes[2], hash.mBytes[3], hash.mBytes[4], hash.mBytes[5], hash.mBytes[6], hash.mBytes[7], hash.mBytes[8], hash.mBytes[9], hash.mBytes[10], hash.mBytes[11], hash.mBytes[12], hash.mBytes[13], hash.mBytes[14], hash.mBytes[15]);
		}

		void fromString(MD5Hash& hash, const TCHAR* buffer)
		{
			auto hexCharacterToDecimalValue = [](const TCHAR inHexChar, uint8& outDecValue)->bool
			{
				TCHAR base = 0;
				if (inHexChar >= '0' && inHexChar <= '9')
				{
					outDecValue = inHexChar - '0';
				}
				else if (inHexChar >= 'A' && inHexChar <= 'F')
				{
					outDecValue = (inHexChar - 'A') + 10;
				}
				else if (inHexChar >= 'a' && inHexChar <= 'f')
				{
					outDecValue = (inHexChar - 'a') + 10;
				}
				else
				{
					return false;
				}
				return true;
			};

			uint8 bytes[16];
			for (int32 byteIndex = 0, bufferIndex = 0; byteIndex < 16; ++byteIndex)
			{
				const TCHAR firstChar = buffer[bufferIndex++];
				if (firstChar == '\0')
				{
					return;
				}
				const TCHAR secondChar = buffer[bufferIndex++];
				if (secondChar == '\0')
				{
					return;
				}

				uint8 firstCharVal, secondCharVal;
				if (!hexCharacterToDecimalValue(firstChar, firstCharVal) || !hexCharacterToDecimalValue(secondChar, secondCharVal))
				{
					return;
				}
				bytes[byteIndex] = (firstCharVal << 4) + secondCharVal;
			}
			Memory::memcpy(hash.mBytes, bytes, 16);
			hash.bIsValid = true;
		}
	}
	MD5Hash MD5Hash::hashFile(const TCHAR* inFilename, TArray<uint8>* buffer /* = nullptr */)
	{
		Archive* ar = IFileManager::get().createFileReader(inFilename);
		MD5Hash result = hashFileFromArchive(ar, buffer);
		delete ar;
		return result;
	}

	MD5Hash MD5Hash::hashFileFromArchive(Archive* ar, TArray<uint8>* buffer /* = nullptr */)
	{
		MD5Hash hash;
		if (ar)
		{
			TArray<uint8> localScratch;
			if (!buffer)
			{
				localScratch.setNumUninitialized(1024 * 64);
				buffer = &localScratch;
			}
			MD5 md5;
			const int64 size = ar->totalSize();
			int64 position = 0;
			while (position < size)
			{
				const auto readNum = Math::min(size - position, (int64)buffer->size());
				ar->serialize(buffer->getData(), readNum);
				md5.update(buffer->getData(), readNum);
				position += readNum;
			}
			hash.set(md5);
		}
		return hash;
	}
}