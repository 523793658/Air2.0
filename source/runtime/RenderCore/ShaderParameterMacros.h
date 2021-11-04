#pragma once
#include "CoreMinimal.h"
#include "RHI.h"
#include "ShaderParameterMetadata.h"
namespace Air
{
	template <typename T, uint32 Alignment>
	class TAlignedTypedef;

#define IMPLEMENT_ALIGNED_TYPE(Alignment)	\
	template<typename T>	\
	class TAlignedTypedef<T, Alignment>	\
	{\
	public:	\
		typedef MS_ALIGN(Alignment) T Type GCC_ALIGN(Alignment);\
	};

	IMPLEMENT_ALIGNED_TYPE(1);
	IMPLEMENT_ALIGNED_TYPE(2);
	IMPLEMENT_ALIGNED_TYPE(4);
	IMPLEMENT_ALIGNED_TYPE(8);
	IMPLEMENT_ALIGNED_TYPE(16);
#undef IMPLEMENT_ALIGNED_TYPE


	template<typename PtrType>
	using TAlignedShaderParameterPtr = typename TAlignedTypedef<PtrType, SHADER_PARAMETER_STRUCT_ALIGNMENT>::Type;

	template<typename TElement, uint32 NumElements>
	class alignas(SHADER_PARAMETER_POINTER_ALIGNMENT) TShaderResourceParameterArray : public TStaticArray<TElement, NumElements, SHADER_PARAMETER_POINTER_ALIGNMENT>
	{
	public:
		FORCEINLINE TShaderResourceParameterArray()
		{
			for (uint32 i = 0; i < NumElements; i++)
			{
				(*this)[i] = nullptr;
			}
		}
	};

	struct alignas(SHADER_PARAMETER_STRUCT_ALIGNMENT) RenderTargetBinding
	{
	private:
		//TAlignedShaderParameterPtr<
	};

	struct alignas(SHADER_PARAMETER_STRUCT_ALIGNMENT) DepthStencilBinding
	{

	};

	struct alignas(SHADER_PARAMETER_STRUCT_ALIGNMENT) RenderTargetBindingSlots
	{
		TStaticArray < RenderTargetBinding, MaxSimultaneousRenderTargets> mOutputs;
		DepthStencilBinding mDepthStencil;

		RenderTargetBinding& operator[](uint32 index)
		{
			return mOutputs[index];
		}
		const RenderTargetBinding& operator[](uint32 index) const
		{
			return mOutputs[index];
		}

		struct TypeInfo
		{
			static constexpr int32 NumRows = 1;
			static constexpr int32 NumColumns = 1;
			static constexpr int32 NumElements = 0;
			static constexpr int32 Alignment = SHADER_PARAMETER_STRUCT_ALIGNMENT;
			static constexpr bool bIsStoredInConstantBuffer = false;
			using TAlignedType = RenderTargetBindingSlots;

			static inline const ShaderParametersMetadata* getStructMetadata() { return nullptr; }
		};
	};

	static_assert(sizeof(RenderTargetBindingSlots) == 144, "RenderTargetBindingSlots needs to be same size on all platform.");


	template<typename TBufferStruct> 
	class TConstantBufferRef : public ConstantBufferRHIRef
	{
	public:
		TConstantBufferRef() {}

		static TConstantBufferRef<TBufferStruct> createConstantBufferImmediate(const TBufferStruct& value, EConstantBufferUsage usage, EConstantBufferValidation validation = EConstantBufferValidation::ValidateResources)
		{
			BOOST_ASSERT(isInRenderingThread() || isInRHIThread());
			return TConstantBufferRef<TBufferStruct>(RHICreateConstantBuffer(&value, TBufferStruct::TypeInfo::getStructMetadata()->getLayout(), usage, validation));
		}

		static LocalConstantBuffer createLocalConstantBuffer(RHICommandList& RHICmdList, const TBufferStruct& value, EConstantBufferUsage usage)
		{
			return RHICmdList.buildLocalConstantBuffer(&value, sizeof(TBufferStruct), TBufferStruct::TypeInfo::getStructMetadata()->getLayout());
		}

		void updateConstantBufferImmediate(const TBufferStruct& value)
		{
			RHIUpdateConstantBuffer(getReference(), &value);
		}

	private:
		TConstantBufferRef(RHIConstantBuffer* inRHIRef)
			:ConstantBufferRHIRef(inRHIRef)
		{}

		template<typename TBufferStruct2>
		friend class TConstantBuffer;
	};

	template<typename TypeParameter>
	struct TShaderParameterTypeInfo
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_INVALID;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 1;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = alignof(TypeParameter);

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TypeParameter;

		static const ShaderParametersMetadata* getStructMetadata() {
			return &TypeParameter::StaticStructMetadata;
		}
	};


	template<>
	struct TShaderParameterTypeInfo<bool>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_BOOL;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 1;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 4;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<bool, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<>
	struct TShaderParameterTypeInfo<uint32>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_UINT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 1;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 4;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<uint32, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<>
	struct TShaderParameterTypeInfo<int32>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_INT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 1;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 4;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<int32, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<>
	struct TShaderParameterTypeInfo<float>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_FLOAT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 1;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 4;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<float, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<>
	struct TShaderParameterTypeInfo<float2>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_FLOAT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 2;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 8;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<float2, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<>
	struct TShaderParameterTypeInfo<float3>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_FLOAT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 3;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 16;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<float3, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};


	template<>
	struct TShaderParameterTypeInfo<float4>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_FLOAT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 4;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 16;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<float4, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<>
	struct TShaderParameterTypeInfo<LinearColor>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_FLOAT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 4;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 16;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<LinearColor, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<>
	struct TShaderParameterTypeInfo<int2>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_INT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 2;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 8;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<int2, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<>
	struct TShaderParameterTypeInfo<int3>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_INT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 3;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 16;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<int3, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<>
	struct TShaderParameterTypeInfo<int4>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_INT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 4;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 16;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<int4, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};


	template<>
	struct TShaderParameterTypeInfo<IntRect>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_INT32;

		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 4;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 16;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<IntRect, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<>
	struct TShaderParameterTypeInfo<Matrix>
	{
		static constexpr EConstantBufferBaseType BaseType = CBMT_FLOAT32;

		static constexpr int32 NumRows = 4;
		static constexpr int32 NumColumns = 4;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = 16;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TAlignedTypedef<Matrix, Alignment>::Type;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<typename T, size_t InNumElements>
	struct TShaderParameterTypeInfo<T[InNumElements]>
	{
		static constexpr EConstantBufferBaseType BaseType = TShaderParameterTypeInfo<T>::BaseType;

		static constexpr int32 NumRows = TShaderParameterTypeInfo<T>::NumRows;
		static constexpr int32 NumColumns = TShaderParameterTypeInfo<T>::NumColumns;

		static constexpr int32 NumElements = InNumElements;

		static constexpr int32 Alignment = SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT;

		static constexpr bool bIsStoredInConstantBuffer = TShaderParameterTypeInfo<T>::bIsStoredInConstantBuffer;

		using TAlignedType = TStaticArray<T, InNumElements, Alignment>;

		static const ShaderParametersMetadata* getStructMetadata() {
			return TShaderParameterTypeInfo<T>::getStructMetadata();
		}
	};

	template<typename T, size_t InNumElements, uint32 IgnoredAlignment>
	struct TShaderParameterTypeInfo<TStaticArray<T, InNumElements, IgnoredAlignment>>
	{
		static constexpr EConstantBufferBaseType BaseType = TShaderParameterTypeInfo<T>::BaseType;

		static constexpr int32 NumRows = TShaderParameterTypeInfo<T>::NumRows;
		static constexpr int32 NumColumns = TShaderParameterTypeInfo<T>::NumColumns;

		static constexpr int32 NumElements = InNumElements;

		static constexpr int32 Alignment = SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT;

		static constexpr bool bIsStoredInConstantBuffer = TShaderParameterTypeInfo<T>::bIsStoredInConstantBuffer;

		using TAlignedType = TStaticArray<T, InNumElements, Alignment>;

		static const ShaderParametersMetadata* getStructMetadata() {
			return TShaderParameterTypeInfo<T>::getStructMetadata();
		}
	};

	template<typename ShaderResourceType>
	struct TShaderResourceParameterTypeInfo
	{
		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 1;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = SHADER_PARAMETER_POINTER_ALIGNMENT;

		static constexpr bool bIsStoredInConstantBuffer = false;

		using TAlignedType = TAlignedShaderParameterPtr<ShaderResourceType>;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}

		static_assert(sizeof(TAlignedType) == SHADER_PARAMETER_POINTER_ALIGNMENT, "Constant buffer layout must not be platform dependent.");

	};

	template<typename ShaderResourceType, size_t InNumElements>
	struct TShaderResourceParameterTypeInfo<ShaderResourceType[InNumElements]>
	{
		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 1;

		static constexpr int32 NumElements = InNumElements;

		static constexpr int32 Alignment = SHADER_PARAMETER_POINTER_ALIGNMENT;

		static constexpr bool bIsStoredInConstantBuffer = false;

		using TAlignedType = TShaderResourceParameterArray<ShaderResourceType, InNumElements>;

		static const ShaderParametersMetadata* getStructMetadata() {
			return nullptr;
		}
	};

	template<typename ConstantBufferStructType>
	struct TShaderParameterTypeInfo<TConstantBufferRef<ConstantBufferStructType>>
	{
		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 1;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = SHADER_PARAMETER_POINTER_ALIGNMENT;

		static constexpr bool bIsStoredInConstantBuffer = false;

		using TAlignedType = TAlignedShaderParameterPtr<TConstantBufferRef<ConstantBufferStructType>>;

		static const ShaderParametersMetadata* getStructMetadata() {
			return &ConstantBufferStructType::StaticStructMetadata;
		}
	};

	template<typename StructType>
	struct TShaderParameterStructTypeInfo
	{
		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 1;

		static constexpr int32 NumElements = 0;

		static constexpr int32 Alignment = SHADER_PARAMETER_STRUCT_ALIGNMENT;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = StructType;

		static const ShaderParametersMetadata* getStructMetadata() {
			return StructType::TypeInfo::getStructMetadata();
		}
	};

	template<typename StructType, size_t InNumElements>
	struct TShaderParameterStructTypeInfo<StructType[InNumElements]>
	{
		static constexpr int32 NumRows = 1;
		static constexpr int32 NumColumns = 1;

		static constexpr int32 NumElements = InNumElements;

		static constexpr int32 Alignment = SHADER_PARAMETER_STRUCT_ALIGNMENT;

		static constexpr bool bIsStoredInConstantBuffer = true;

		using TAlignedType = TStaticArray<StructType, InNumElements>;

		static const ShaderParametersMetadata* getStructMetadata() {
			return StructType::TypeInfo::getStructMetadata();
		}
	};



#define INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN(StructTypeName, PrefixKeywords, ConstructorSuffix, GetStructMetadataScope, CreateConstantBufferImpl) \
	MS_ALIGN(SHADER_PARAMETER_STRUCT_ALIGNMENT) class PrefixKeywords StructTypeName \
	{ \
	public:\
		StructTypeName() ConstructorSuffix \
		struct TypeInfo {\
			static constexpr int32 NumRows = 1;\
			static constexpr int32 NumColumns = 1;\
			static constexpr int32 NumElements = 0; \
			static constexpr int32 Alignment = SHADER_PARAMETER_STRUCT_ALIGNMENT;\
			static constexpr bool bIsStoredInConstantBuffer = true ;\
			using TAlignedType = StructTypeName; \
			static inline const ShaderParametersMetadata* getStructMetadata() {GetStructMetadataScope}\
		};\
		static ConstantBufferRHIRef createConstantBuffer(const StructTypeName& inContents, EConstantBufferUsage inUsage) \
		{\
			CreateConstantBufferImpl\
		}\
	private:\
		typedef StructTypeName zzTThisStruct;\
		struct zzFirstMemberId{enum {HasDeclaredResource = 0};};\
		static TArray<ShaderParametersMetadata::Member> zzGetMembersBefore(zzFirstMemberId) \
	{\
		return TArray<ShaderParametersMetadata::Member>(); \
	}\
		typedef zzFirstMemberId

#define INTERNAL_SHADER_PARAMETER_EXPLICIT(BaseType, TypeInfo, MemberType, MemberName, ArrayDecl, DefaultValue, Precision, OptionalShaderType, IsMemberStruct) \
		zzMemberId##MemberName;\
	public:\
		TypeInfo::TAlignedType MemberName DefaultValue;\
		static_assert(BaseType != CBMT_INVALID, "Invalid type " #MemberType " of member " #MemberName ".");\
	private:\
		struct zzNextMemberId##MemberName {enum {HasDeclaredResource = zzMemberId##MemberName::HasDeclaredResource || !TypeInfo::bIsStoredInConstantBuffer};};\
		static TArray<ShaderParametersMetadata::Member> zzGetMembersBefore(zzNextMemberId##MemberName)\
		{\
			static_assert(TypeInfo::bIsStoredInConstantBuffer || std::is_array<decltype(OptionalShaderType)>::value || std::is_reference<decltype(OptionalShaderType)>::value, "No shader type for " #MemberName ".");\
			TArray<ShaderParametersMetadata::Member> outMembers = zzGetMembersBefore(zzMemberId##MemberName());\
				outMembers.add(ShaderParametersMetadata::Member(TEXT(#MemberName),\
				OptionalShaderType,\
				STRUCT_OFFSET(zzTThisStruct, MemberName),\
				EConstantBufferBaseType(BaseType),\
				Precision,\
				TypeInfo::NumRows,\
				TypeInfo::NumColumns,\
				TypeInfo::NumElements,\
				TypeInfo::getStructMetadata()\
				));\
			static_assert(\
				(STRUCT_OFFSET(zzTThisStruct, MemberName) & (TypeInfo::Alignment - 1)) == 0, \
				"Misaligned constant buffer struct member " #MemberName ".");\
			return outMembers;\
		}\
		typedef zzNextMemberId##MemberName

#define INTERNAL_BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT \
	static ShaderParametersMetadata StaticStructMetadata; \

#define INTERNAL_GLOBAL_SHADER_PARAMETER_GET_STRUCT_METADATA(StructTypeName) \
	return &StructTypeName::StaticStructMetadata;

#define INTERNAL_GLOBAL_SHADER_PARAMETER_CREATE_CONSTANT_BUFFER return RHICreateConstantBuffer(&inContents, StaticStructMetadata.getLayout(), inUsage);


#define END_SHADER_PARAMETER_STRUCT() \
	zzLastMemberId;\
	static TArray<ShaderParametersMetadata::Member> zzGetMembers(){return zzGetMembersBefore(zzLastMemberId());}\
	} GCC_ALIGN(SHADER_PARAMETER_STRUCT_ALIGNMENT);

#define BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(StructTypeName, PrefixKeywords) \
	INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN(StructTypeName, PrefixKeywords, {} INTERNAL_BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT, INTERNAL_GLOBAL_SHADER_PARAMETER_GET_STRUCT_METADATA(StructTypeName), INTERNAL_GLOBAL_SHADER_PARAMETER_CREATE_CONSTANT_BUFFER)

#define BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT_WITH_CONSTRUCTOR(StructTypeName, PrefixKeywords) \
	INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN(StructTypeName, PrefixKeywords, ; INTERNAL_BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT, INTERNAL_GLOBAL_SHADER_PARAMETER_GET_STRUCT_METADATA(StructTypeName), INTERNAL_GLOBAL_SHADER_PARAMETER_CREATE_CONSTANT_BUFFER)

#define END_GLOBAL_SHADER_PARAMETER_STRUCT() \
	END_SHADER_PARAMETER_STRUCT()


#define SHADER_PARAMETER(MemberType, MemberName) \
	SHADER_PARAMETER_EX(MemberType, MemberName, EShaderPrecisionModifier::Float)

#define SHADER_PARAMETER_EX(MemberType, MemberName, Precision) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(TShaderParameterTypeInfo<MemberType>::BaseType, TShaderParameterTypeInfo<MemberType>, MemberType, MemberName,,,Precision, TEXT(""), false)

#define SHADER_PARAMETER_ARRAY(MemberType, MemberName, ArrayDecl) \
	SHADER_PARAMETER_ARRAY_EX(MemberType, MemberName, ArrayDecl, EShaderPrecisionModifier::Float)

#define SHADER_PARAMETER_ARRAY_EX(MemberType, MemberName, ArrayDecl, Precision) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(TShaderParameterTypeInfo<MemberType ArrayDecl>::BaseType, TShaderParameterTypeInfo<MemberType ArrayDecl>, MemberType, MemberName, ArrayDecl, ,Precision, TEXT(""), false)

#define SHADER_PARAMETER_TEXTURE(ShaderType, MemberName) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(CBMT_TEXTURE, TShaderResourceParameterTypeInfo<RHITexture*>, RHITexture*, MemberName,, = nullptr, EShaderPrecisionModifier::Float, TEXT(#ShaderType), false)

#define SHADER_PARAMETER_TEXTURE_ARRAY(ShaderType, MemberName, ArrayDecl) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(CBMT_TEXTURE, TShaderResourceParameterTypeInfo<RHITexture* ArrayDecl>, RHITexture*, MemberName, ArrayDecl,, EShaderPrecisionModifier::Float, TEXT(#ShaderType), false)

#define SHADER_PARAMETER_SAMPLER(ShaderType, MemberName) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(CBMT_SAMPLER, TShaderResourceParameterTypeInfo<RHISamplerState*>, RHISamplerState*, MemberName,, = nullptr, EShaderPrecisionModifier::Float, TEXT(#ShaderType), false)

#define SHADER_PARAMETER_SAMPLER_ARRAY(ShaderType, MemberName, ArrayDecl) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(CBMT_SAMPLER, TShaderResourceParameterTypeInfo<RHISamplerState* ArrayDecl>, RHISamplerState*, MemberName, ArrayDecl,, EShaderPrecisionModifier::Float, TEXT(#ShaderType), false)

#define IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(StructTypeName, ShaderVariableName) \
	ShaderParametersMetadata StructTypeName::StaticStructMetadata( \
		ShaderParametersMetadata::EUseCase::GlobalShaderParameterStruct, \
		wstring(TEXT(#StructTypeName)), \
		TEXT(#StructTypeName), \
		TEXT(ShaderVariableName), \
		sizeof(StructTypeName), \
		StructTypeName::zzGetMembers())

#define SHADER_PARAMETER_STRUCT_INCLUDE(StructType, MemberName) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(CBMT_INCLUDED_STRUCT, StructType::TypeInfo, StructType, MemberName,,,EShaderPrecisionModifier::Float, TEXT(#StructType), true)

#define SHADER_PARAMETER_SRV(ShaderType, MemberName) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(CBMT_SRV, TShaderResourceParameterTypeInfo<RHIShaderResourceView*>, RHIShaderResourceView*, MemberName,, = nullptr, EShaderPrecisionModifier::Float, TEXT(#ShaderType), false)

#define SHADER_PARAMETER_SRV_ARRAY(ShaderType, MemberName, ArrayDecl) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(CBMT_SRV, TShaderResourceParameterTypeInfo<RHIShaderResourceView* ArrayDecl>, RHIShaderResourceView*, MemberName, ArrayDecl, = nullptr, EShaderPrecisionModifier::Float, TEXT(#ShaderType), false)

#define INTERNAL_LOCAL_SHADER_PARAMETER_GET_STRUCT_METADATA(StructTypeName) \
	static ShaderParametersMetadata StaticStructMetadata(\
		ShaderParametersMetadata::EUseCase::ShaderParameterStruct,\
		wstring(TEXT(#StructTypeName)),\
		TEXT(#StructTypeName),\
		nullptr,\
		sizeof(StructTypeName),\
		StructTypeName::zzGetMembers());\
	return &StaticStructMetadata;

#define INTERNAL_LOCAL_SHADER_PARAMETER_CREATE_CONSTANT_BUFFER return nullptr;

#define BEGIN_SHADER_PARAMETER_STRUCT(StructTypeName, PrefixKeywords) \
	INTERNAL_SHADER_PARAMETER_STRUCT_BEGIN(StructTypeName, PrefixKeywords, {}, INTERNAL_LOCAL_SHADER_PARAMETER_GET_STRUCT_METADATA(StructTypeName), INTERNAL_LOCAL_SHADER_PARAMETER_CREATE_CONSTANT_BUFFER)

extern RENDER_CORE_API ShaderParametersMetadata* findConstantBufferStructByName(wstring structName);
extern RENDER_CORE_API ShaderParametersMetadata* findConstantBufferStructByName(const TCHAR* structName);

#define SHADER_PARAMETER_STRUCT(StructType, MemberName) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(CBMT_NESTED_STRUCT, StructType::TypeInfo, StructType, MemberName,,,EShaderPrecisionModifier::Float, TEXT(#StructType), true)

#define SHADER_PARAMETER_STRUCT_ARRAY(StructType, MemberName, ArrayDecl) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(CBMT_NESTED_STRUCT, TShaderParameterStructTypeInfo<StructType, ArrayDecl>, StructType, MemberName,ArrayDecl,,EShaderPrecisionModifier::Float, TEXT(#StructType), true)

#define SHADER_PARAMETER_UAV(ShaderType, MemberName) \
	INTERNAL_SHADER_PARAMETER_EXPLICIT(CBMT_UAV, TShaderResourceParameterTypeInfo<RHIUnorderedAccessView*>, RHIUnorderedAccessView*, MemberName,, = nullptr, EShaderPrecisionModifier::Float, TEXT(#ShaderType), false)
}