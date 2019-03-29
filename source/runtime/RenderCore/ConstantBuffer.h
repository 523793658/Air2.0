#pragma once
#include "CoreType.h"
#include "RenderCore.h"
#include "RHIDefinitions.h"
#include "RHIResource.h"
#include "Containers/LinkList.h"
#include "Template/AlignmentTemplates.h"
#include "Containers/Map.h"
#include "Template/AirTemplate.h"
#include "Containers/StaticArray.h"
#include "RHICommandList.h"
#include "RenderResource.h"
#include "Misc/CoreMisc.h"
#include <array>
namespace Air
{
	template<typename TConstantStruct> class TShaderConstantBufferParameter;

	namespace EShaderPrecisionModifier
	{
		enum Type
		{
			Float,
			Half,
			Fixed
		};
	}

	template<typename TBufferStruct>
	class TConstantBuffer : public RenderResource
	{
	public:
		TConstantBuffer()
			:mBufferUsage(ConstantBuffer_MultiFrame)
			,mContents(nullptr)
		{}
		~TConstantBuffer()
		{
			if (mContents)
			{
				Memory::free(mContents);
			}
		}
		void setContents(const TBufferStruct& newContents)
		{
			setContentsNoUpdate(newContents);
			updateRHI();
		}

		void setContentsToZero()
		{
			if (!mContents)
			{
				mContents = (uint8*)Memory::malloc(sizeof(TBufferStruct), CONSTANT_BUFFER_STRUCT_ALIGNMENT);
			}
			Memory::memzero(mContents, sizeof(TBufferStruct));
			updateRHI();
		}

		virtual void initDynamicRHI() override
		{
			BOOST_ASSERT(isInRenderingThread());
			mConstantBufferRHI.safeRelease();
			if (mContents)
			{
				mConstantBufferRHI = RHICreateConstantBuffer(mContents, TBufferStruct::mStaticStruct.getLayout(), mBufferUsage);
			}
		}
		virtual void releaseDynamicRHI() override
		{
			mConstantBufferRHI.safeRelease();
		}

		ConstantBufferRHIParamRef getConstantBufferRHI() const
		{
			BOOST_ASSERT(mConstantBufferRHI.getReference());
			return mConstantBufferRHI;
		}


		EConstantBufferUsage mBufferUsage;

	protected:
		void setContentsNoUpdate(const TBufferStruct& newContents)
		{
			BOOST_ASSERT(isInRenderingThread());
			if (!mContents)
			{
				mContents = (uint8*)Memory::malloc(sizeof(TBufferStruct), CONSTANT_BUFFER_STRUCT_ALIGNMENT);
			}
			Memory::memcpy(mContents, &newContents, sizeof(TBufferStruct));
		}
	private:
		ConstantBufferRHIRef mConstantBufferRHI;
		uint8* mContents;
	};

	template<typename TBufferStruct>
	class TConstantBufferRef : public ConstantBufferRHIRef
	{
	public:
		TConstantBufferRef()
		{}

		TConstantBufferRef(const TConstantBuffer<TBufferStruct>& inBuffer)
			:ConstantBufferRHIRef(inBuffer.getConstantBufferRHI())
		{}
		static TConstantBufferRef<TBufferStruct> createConstantBufferImmediate(const TBufferStruct& value, EConstantBufferUsage usage)
		{
			BOOST_ASSERT(isInRenderingThread() || isInRHIThread());
			return TConstantBufferRef<TBufferStruct>(RHICreateConstantBuffer(&value, TBufferStruct::mStaticStruct.getLayout(), usage));
		}

		static LocalConstantBuffer createLocalConstantBuffer(RHICommandList& RHICmdList, const TBufferStruct& value, EConstantBufferUsage usage)
		{
			return RHICmdList.buildLocalConstantBuffer(&value, sizeof(TBufferStruct), TBufferStruct::mStaticStruct.getLayout());
		}
	private:
		TConstantBufferRef(ConstantBufferRHIParamRef inRHIRef)
			:ConstantBufferRHIRef(inRHIRef)
		{}
	};


	struct ResourceTableEntry
	{
		wstring mConstantBufferName;
		uint16 mType;
		uint16 mResourceIndex;
	};

	inline Archive& operator << (Archive& ar, ResourceTableEntry& entry)
	{
		ar << entry.mConstantBufferName;
		ar << entry.mType;
		ar << entry.mResourceIndex;
		return ar;
	}


	class RENDER_CORE_API ConstantBufferStruct
	{
	public:
		class Member
		{
		public:
			Member(
				const TCHAR* inName,
				const TCHAR* inShaderType,
				uint32 inOffset,
				EConstantBufferBaseType inBaseType,
				EShaderPrecisionModifier::Type inPrecision,
				uint32 inNumRows,
				uint32 inNumColumns,
				uint32 inNumElements,
				const ConstantBufferStruct* inStruct)
				: mName(inName)
				, mShaderType(inShaderType)
				, mOffset(inOffset)
				, mBaseType(inBaseType)
				, mPrecision(inPrecision)
				, mNumRows(inNumRows)
				, mNumColumns(inNumColumns)
				, mNumElements(inNumElements)
				, mStruct(inStruct)
			{}

			const TCHAR* getName() const { return mName; }
			const TCHAR* getShaderType() const { return mShaderType; }
			uint32 getOffset() const { return mOffset; }
			EConstantBufferBaseType getBaseType() const { return mBaseType; }
			EShaderPrecisionModifier::Type getPrecision() const { return mPrecision; }
			uint32 getNumRows() const { return mNumRows; }
			uint32 getNumColumns() const { return mNumColumns; }
			uint32 getNumElements() const { return mNumElements; }
			const ConstantBufferStruct* getStruct() const {
				return mStruct;
			}

			
		private:
			const TCHAR* mName;
			const TCHAR* mShaderType;
			uint32 mOffset;
			EConstantBufferBaseType mBaseType;
			EShaderPrecisionModifier::Type mPrecision;
			uint32 mNumRows;
			uint32 mNumColumns;
			uint32 mNumElements;
			const ConstantBufferStruct* mStruct;
		};
		typedef class ShaderConstantBufferParameter* (*ConstructConstantBufferParameterType)();

		ConstantBufferStruct(const wstring & inLayoutName, const TCHAR* inStructTypeName, const TCHAR* inShaderVariableName, ConstructConstantBufferParameterType inConstructRef, uint32 inSize, const TArray<Member>& inMembers, bool bRegisterForAutoBinding)
			:mStructTypeName(inStructTypeName)
			,mShaderVariableName(inShaderVariableName)
			,mConstructConstantBufferParameterType(inConstructRef)
			,mSize(inSize)
			,mLayout(inLayoutName)
			,mMembers(inMembers)
			,GlobalListLink(this)
		{
			bool bHasDeclaredResource = false;
			mLayout.mConstantBufferSize = inSize;
			mLayout.mResourceOffset = 0;
			for (int32 i = 0; i < mMembers.size(); ++i)
			{
				bool bIsResource = isConstantBufferResourceType(mMembers[i].getBaseType());
				if (bIsResource)
				{
					if (!bHasDeclaredResource)
					{
						mLayout.mConstantBufferSize = (i == 0) ? 0 : align(mMembers[i].getOffset(), CONSTANT_BUFFER_STRUCT_ALIGNMENT);
						mLayout.mResourceOffset = mMembers[i].getOffset();
					}
					mLayout.mResource.push_back(mMembers[i].getBaseType());
				}
				bHasDeclaredResource |= bIsResource;
			}
			if (bRegisterForAutoBinding)
			{
				GlobalListLink.linkHead(getStructList());
				wstring structTypeName(mStructTypeName);
				getNameStructMap().emplace(structTypeName, this);
			}
		}

		virtual ~ConstantBufferStruct()
		{
			GlobalListLink.unLink();
			getNameStructMap().erase(mStructTypeName);
		}

		ShaderConstantBufferParameter* constructTypeParameter() const
		{
			return (*mConstructConstantBufferParameterType)();
		}

		void addResourceTableEntries(TMap<wstring, ResourceTableEntry>& resourceTableMap, TMap<wstring, uint32>& resourceTableLayoutHashes) const
		{
			uint16 resourceIndex = 0;
			for (int32 memberIndex = 0; memberIndex < mMembers.size(); ++memberIndex)
			{
				const Member & member = mMembers[memberIndex];
				if (isConstantBufferResourceType(member.getBaseType()))
				{
					//ResourceTableEntry& entry;
					wstring shaderVariableName = mShaderVariableName;
					wstring memberName = member.getName();
					wstring key = shaderVariableName + L"_" + memberName;
					ResourceTableEntry * entry;
					auto it = resourceTableMap.find(key);
					if (it == resourceTableMap.end())
					{
						auto r = resourceTableMap.emplace(key, ResourceTableEntry());

						BOOST_ASSERT(r.second);
						entry = &r.first->second;

					}
					else
					{
						entry = &it->second;
					}
					entry->mConstantBufferName = shaderVariableName;
					entry->mType = member.getBaseType();
					entry->mResourceIndex = resourceIndex++;
				}
			}
			resourceTableLayoutHashes.emplace(mShaderVariableName, getLayout().getHash());
		}

		const TCHAR* getStructTypeName() const { return mStructTypeName; }

		const TCHAR* getShaderVariableName() const { return mShaderVariableName; }
		const uint32 getSize() const { return mSize; }

		const RHIConstantBufferLayout& getLayout() const { return mLayout; }

		const TArray<Member> & getMembers() const { return mMembers; }


		static TLinkedList<ConstantBufferStruct*>*& getStructList();
		static TMap<wstring, ConstantBufferStruct*>& getNameStructMap();
	private:
		const TCHAR* mStructTypeName;
		const TCHAR* mShaderVariableName;
		ConstructConstantBufferParameterType mConstructConstantBufferParameterType;
		uint32 mSize;
		RHIConstantBufferLayout mLayout;
		TArray<Member> mMembers;
		TLinkedList<ConstantBufferStruct*> GlobalListLink;

	};

	template<typename>
	class TConstantBufferTypeInfo
	{
	public:
		enum { BaseType = CBMT_INVALID };
		enum { NumRows = 0 };
		enum { NumColumns = 0 };
		enum { NumElements = 0 };
		enum { Alignment = 1 };
		enum { IsResource = 0 };
		static const ConstantBufferStruct* getStruct() { return nullptr; }
	};

	template <typename T, uint32 Alignment>
	class TAlignedTypedef;

#define IMPLEMENT_ALIGNED_TYPE(Alignment)	\
	template<typename T>	\
	class TAlignedTypedef<T, Alignment>	\
	{\
	public:	\
		typedef MS_ALIGN(Alignment) T TAlignedType GCC_ALIGN(Alignment);\
	};

	IMPLEMENT_ALIGNED_TYPE(1);
	IMPLEMENT_ALIGNED_TYPE(2);
	IMPLEMENT_ALIGNED_TYPE(4);
	IMPLEMENT_ALIGNED_TYPE(8);
	IMPLEMENT_ALIGNED_TYPE(16);

	template<>
	class TConstantBufferTypeInfo<bool>
	{
	public:
		enum { BaseType = CBMT_BOOL };
		enum { NumRows = 1 };
		enum { NumColumns = 1 };
		enum { NumElements = 0 };
		enum { Alignment = 4 };
		enum { IsResource = 0 };
		typedef TAlignedTypedef<bool, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};

	template<>
	class TConstantBufferTypeInfo<uint32>
	{
	public:
		enum { BaseType = CBMT_UINT32 };
		enum { NumRows = 1 };
		enum { NumColumns = 1 };
		enum { NumElements = 0 };
		enum { Alignment = 4 };
		enum { IsResource = 0 };
		typedef uint32 TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }

	};

	template<>
	class TConstantBufferTypeInfo<int32>
	{
	public:
		enum { BaseType = CBMT_INT32 };
		enum { NumRows = 1 };
		enum { NumColumns = 1 };
		enum { NumElements = 0 };
		enum { Alignment = 4 };
		enum { IsResource = 0 };
		typedef int32 TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }

	};

	template<>
	class TConstantBufferTypeInfo<float>
	{
	public:
		enum { BaseType = CBMT_FLOAT32 };
		enum { NumRows = 1 };
		enum { NumColumns = 1 };
		enum { NumElements = 0 };
		enum { Alignment = 4 };
		enum { IsResource = 0 };
		typedef float TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }

	};

	template<>
	class TConstantBufferTypeInfo<float2>
	{
	public:
		enum { BaseType = CBMT_FLOAT32 };
		enum { NumRows = 1 };
		enum { NumColumns = 2 };
		enum { NumElements = 0 };
		enum { Alignment = 8 };
		enum { IsResource = 0 };
		typedef TAlignedTypedef<float2, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};

	template<>
	class TConstantBufferTypeInfo<float3>
	{
	public:
		enum { BaseType = CBMT_FLOAT32 };
		enum { NumRows = 1 };
		enum { NumColumns = 3 };
		enum { NumElements = 0 };
		enum { Alignment = 16 };
		enum { IsResource = 0 };
		typedef TAlignedTypedef<float3, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};


	template<>
	class TConstantBufferTypeInfo<float4>
	{
	public:
		enum { BaseType = CBMT_FLOAT32 };
		enum { NumRows = 1 };
		enum { NumColumns = 4 };
		enum { NumElements = 0 };
		enum { Alignment = 16 };
		enum { IsResource = 0 };
		typedef TAlignedTypedef<float4, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};

	template<>
	class TConstantBufferTypeInfo<LinearColor>
	{
	public:
		enum { BaseType = CBMT_FLOAT32 };
		enum { NumRows = 1 };
		enum { NumColumns = 4 };
		enum { NumElements = 0 };
		enum { Alignment = 16 };
		enum { IsResource = 0 };
		typedef TAlignedTypedef<LinearColor, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};

	template<>
	class TConstantBufferTypeInfo<int2>
	{
	public:
		enum { BaseType = CBMT_INT32 };
		enum { NumRows = 1 };
		enum { NumColumns = 2 };
		enum { NumElements = 0 };
		enum { Alignment = 8 };
		enum { IsResource = 0 };
		typedef TAlignedTypedef<int2, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};

	template<>
	class TConstantBufferTypeInfo<int3>
	{
	public:
		enum { BaseType = CBMT_INT32 };
		enum { NumRows = 1 };
		enum { NumColumns = 3 };
		enum { NumElements = 0 };
		enum { Alignment = 16 };
		enum { IsResource = 0 };
		typedef TAlignedTypedef<int3, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};

	template<>
	class TConstantBufferTypeInfo<IntRect>
	{
	public:
		enum { BaseType = CBMT_INT32 };
		enum { NumRows = 1 };
		enum { NumColumns = 4 };
		enum { NumElements = 0 };
		enum { Alignment = 16 };
		enum { IsResource = 0 };
		typedef TAlignedTypedef<IntRect, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};

	template<>
	class TConstantBufferTypeInfo<Matrix>
	{
	public:
		enum { BaseType = CBMT_FLOAT32 };
		enum { NumRows = 4 };
		enum { NumColumns = 4 };
		enum { NumElements = 0 };
		enum { Alignment = 16 };
		enum { IsResource = 0 };
		typedef TAlignedTypedef<Matrix, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};







	template<typename T, size_t InNumElements>
	class TConstantBufferTypeInfo<T[InNumElements]>
	{
	public:
		enum { BaseType = TConstantBufferTypeInfo<T>::BaseType };
		enum { NumRows = TConstantBufferTypeInfo<T>::NumRows };
		enum { NumColumns = TConstantBufferTypeInfo<T>::NumColumns };
		enum { NumElements = InNumElements };
		enum { Alignment = TConstantBufferTypeInfo<T>::Alignment };
		enum { IsResource = TConstantBufferTypeInfo<T>::IsResource };
		typedef TStaticArray<T, InNumElements, 16> TAlignedType;
		static const ConstantBufferStruct* getStruct() {
			return TConstantBufferTypeInfo<T>::getStruct();
		}
	};

	template<typename T, size_t InNumElements, uint32 IgnoredAlignment>
	class TConstantBufferTypeInfo<TStaticArray<T, InNumElements, IgnoredAlignment>>
	{
	public:
		enum { BaseType = TConstantBufferTypeInfo<T>::BaseType };
		enum { NumRows = TConstantBufferTypeInfo<T>::NumRows };
		enum { NumColumns = TConstantBufferTypeInfo<T>::NumColumns };
		enum { NumElement = InNumElements };
		enum { Alignment = TConstantBufferTypeInfo<T>::Alignment };
		enum { IsResource = TConstantBufferTypeInfo<T>::IsResource };
		typedef TStaticArray<T, InNumElements, 16> TAlignedType;
		static const ConstantBufferStruct* getStruct() {
			return TConstantBufferTypeInfo<T>::getStruct();
		}
	};

	template<>
	class TConstantBufferTypeInfo<ShaderResourceViewRHIParamRef>
	{
	public:
		enum { BaseType = CBMT_SRV };
		enum { NumRows = 1 };
		enum { NumColumns = 1 };
		enum { NumElements = 0 };
		enum { Alignment = sizeof(void*) };
		enum { IsResource = 1 };
		typedef TAlignedTypedef<ShaderResourceViewRHIParamRef, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() {
			return nullptr;
		}
	};

	static_assert(sizeof(ShaderResourceViewRHIParamRef) == sizeof(void*), "FShaderResourceViewRHIParamRef should have size of a pointer.");
	static_assert(sizeof(TConstantBufferTypeInfo<ShaderResourceViewRHIParamRef>::TAlignedType) == sizeof(void*), "SRV UniformBufferParam is not aligned to pointer type");


	template<>
	class TConstantBufferTypeInfo<UnorderedAccessViewRHIParamRef>
	{
	public:
		enum { BaseType = CBMT_UAV };
		enum { NumRows = 1 };
		enum { NumColumns = 1 };
		enum { NumElements = 0 };
		enum { Alignment = sizeof(void*) };
		enum { IsResource = 1 };
		typedef TAlignedTypedef<UnorderedAccessViewRHIParamRef, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};
	static_assert(sizeof(UnorderedAccessViewRHIParamRef) == sizeof(void*), "FUnorderedAccessViewRHIParamRef should have size of a pointer.");
	static_assert(sizeof(TConstantBufferTypeInfo<UnorderedAccessViewRHIParamRef>::TAlignedType) == sizeof(void*), "UAV UniformBufferParam is not aligned to pointer type");

	template<>
	class TConstantBufferTypeInfo<SamplerStateRHIParamRef>
	{
	public:
		enum { BaseType = CBMT_SAMPLER };
		enum { NumRows = 1 };
		enum { NumColumns = 1 };
		enum { NumElements = 0 };
		enum { Alignment = sizeof(void*) };
		enum { IsResource = 1 };
		typedef TAlignedTypedef<SamplerStateRHIParamRef, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};
	static_assert(sizeof(SamplerStateRHIParamRef) == sizeof(void*), "FSamplerStateRHIParamRef should have size of a pointer.");
	static_assert(sizeof(TConstantBufferTypeInfo<SamplerStateRHIParamRef>::TAlignedType) == sizeof(void*), "SamplerState UniformBufferParam is not aligned to pointer type");

	template<>
	class TConstantBufferTypeInfo<TextureRHIParamRef>
	{
	public:
		enum { BaseType = CBMT_TEXTURE };
		enum { NumRows = 1 };
		enum { NumColumns = 1 };
		enum { NumElements = 0 };
		enum { Alignment = sizeof(void*) };
		enum { IsResource = 1 };
		typedef TAlignedTypedef<TextureRHIParamRef, Alignment>::TAlignedType TAlignedType;
		static const ConstantBufferStruct* getStruct() { return NULL; }
	};
	static_assert(sizeof(TextureRHIParamRef) == sizeof(void*), "FTextureRHIParamRef should have size of a pointer.");
	static_assert(sizeof(TConstantBufferTypeInfo<TextureRHIParamRef>::TAlignedType) == sizeof(void*), "Texture UniformBufferParam is not aligned to pointer type");



#define BEGIN_CONSTANT_BUFFER_STRUCT_EX(StructTypeName, PrefixKeyworlds, ConstructorSuffix) \
	MS_ALIGN(CONSTANT_BUFFER_STRUCT_ALIGNMENT)	class PrefixKeyworlds StructTypeName	  \
	{	\
	public:		\
		StructTypeName() ConstructorSuffix	\
		static ConstantBufferStruct mStaticStruct;	\
		static ShaderConstantBufferParameter* constructConstantBufferParameter() {return new TShaderConstantBufferParameter<StructTypeName>();}\
		static ConstantBufferRHIRef createConstantBuffer(const StructTypeName& inContents, EConstantBufferUsage inUsage)	 \
		{	\
			return RHICreateConstantBuffer(&inContents, mStaticStruct.getLayout(), inUsage);	\
		}	\
	private:	\
		typedef StructTypeName zzTThisStruct;	\
		struct zzFirstMemberId{enum {HasDeclaredResource = 0};};	\
		static TArray<ConstantBufferStruct::Member> zzGetMembersBefore(zzFirstMemberId) \
		{	\
			return TArray<ConstantBufferStruct::Member>();\
		}\
		typedef zzFirstMemberId


#define DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType, MemberName, ArrayDecl, Precision, OptionalShaderType)	 \
		zzMemberId##MemberName;	\
	public:	\
		typedef MemberType zzA##MemberName ArrayDecl;	\
		typedef TConstantBufferTypeInfo<zzA##MemberName>::TAlignedType zzT##MemberName; \
		zzT##MemberName MemberName;	\
		static_assert(TConstantBufferTypeInfo<zzA##MemberName>::BaseType != CBMT_INVALID, "Invalid Type" #MemberType" of member" #MemberName ".");	\
		static_assert(TConstantBufferTypeInfo<zzA##MemberName>::BaseType != CBMT_UAV, "UAV is not yet supported in resource tables for " #MemberName " of type " #MemberType ".");  \
		private:\
			struct zzNextMemberId##MemberName{enum {HasDeclaredResource = zzMemberId##MemberName::HasDeclaredResource || TConstantBufferTypeInfo<zzT##MemberName>::IsResource};};\
			static TArray<ConstantBufferStruct::Member> zzGetMembersBefore(zzNextMemberId##MemberName)   \
			{	\
				static_assert(TConstantBufferTypeInfo<zzT##MemberName>::IsResource == 1 || zzMemberId##MemberName::HasDeclaredResource == 0, "All resources must be declared last for " #MemberName "."); \
				static_assert(TConstantBufferTypeInfo<zzT##MemberName>::IsResource == 0 || IS_TCHAR_ARRAY(OptionalShaderType), "No shader type for " #MemberName "."); \
				TArray<ConstantBufferStruct::Member> mOutMembers = zzGetMembersBefore(zzMemberId##MemberName()); \
			mOutMembers.push_back(ConstantBufferStruct::Member( \
				TEXT(#MemberName), \
				OptionalShaderType, \
				STRUCT_OFFSET(zzTThisStruct,MemberName), \
				(EConstantBufferBaseType)TConstantBufferTypeInfo<zzA##MemberName>::BaseType, \
				Precision, \
				TConstantBufferTypeInfo<zzA##MemberName>::NumRows, \
				TConstantBufferTypeInfo<zzA##MemberName>::NumColumns, \
				TConstantBufferTypeInfo<zzA##MemberName>::NumElements, \
				TConstantBufferTypeInfo<zzA##MemberName>::getStruct() \
				)); \
			static_assert( \
				(STRUCT_OFFSET(zzTThisStruct,MemberName) & (TConstantBufferTypeInfo<zzA##MemberName>::Alignment - 1)) == 0, \
				"Misaligned uniform buffer struct member " #MemberName "."); \
			return mOutMembers; \
		} \
		typedef zzNextMemberId##MemberName


#define DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_SAMPLER(ShaderType, MemberName) DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EXPLICIT(SamplerStateRHIParamRef, MemberName,, EShaderPrecisionModifier::Float, TEXT(#ShaderType))

#define DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_TEXTURE(ShaderType, MemberName) DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EXPLICIT(TextureRHIParamRef, MemberName, ,EShaderPrecisionModifier::Float, TEXT(#ShaderType))


#define DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_ARRAY_EX(MemberType, MemberName, ArrayDecl, Precision) DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType, MemberName, ArrayDecl, Precision, TEXT(""))

#define DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_ARRAY(MemberType, MemberName, ArrayDecl)	 DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType, MemberName, ArrayDecl, EShaderPrecisionModifier::Float, TEXT(""))

#define DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(MemberType, MemberName) DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType, MemberName, ,EShaderPrecisionModifier::Float, TEXT(""))

#define DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EX(MemberType, MemberName, Precision) DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EXPLICIT(MemberType, MemberName, , Precision, TEXT(""))


#define BEGIN_CONSTANT_BUFFER_STRUCT_WITH_CONSTRUCTOR(StructTypeName, PrefixKeywords) BEGIN_CONSTANT_BUFFER_STRUCT_EX(StructTypeName, PrefixKeywords,;)

#define DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_SAMPLER(shaderType, memberName)	DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EXPLICIT(SamplerStateRHIParamRef, memberName, ,EShaderPrecisionModifier::Float, TEXT(#shaderType))


#define END_CONSTANT_BUFFER_STRUCT(Name)	\
		zzLastMemberId;	\
		static TArray<ConstantBufferStruct::Member> zzGetMembers()\
		{\
			return zzGetMembersBefore(zzLastMemberId());\
		}\
	}GCC_ALIGN(CONSTANT_BUFFER_STRUCT_ALIGNMENT);\
	template<> class TConstantBufferTypeInfo<Name>	\
	{	\
	public:	\
		enum {BaseType= CBMT_STRUCT};\
		enum {NumRows = 1};\
		enum {NumColumns = 1};\
		enum {Alignment = CONSTANT_BUFFER_STRUCT_ALIGNMENT};\
		static const ConstantBufferStruct* getStruct(){return &Name::mStaticStruct;}	  \
	};

#define IMPLEMENT_CONSTANT_BUFFER_STRUCT(StructTypeName, ShaderVariableName)\
	ConstantBufferStruct StructTypeName::mStaticStruct(\
		wstring(TEXT(#StructTypeName)),\
		TEXT(#StructTypeName),	\
		ShaderVariableName,	\
		StructTypeName::constructConstantBufferParameter,\
		sizeof(StructTypeName),	\
		StructTypeName::zzGetMembers(),\
		true);

#define BEGIN_CONSTANT_BUFFER_STRUCT(StructType, PrefixKeywords) BEGIN_CONSTANT_BUFFER_STRUCT_EX(StructType, PrefixKeywords, {})

}