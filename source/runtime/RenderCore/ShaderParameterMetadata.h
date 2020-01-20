#pragma once
#include "RenderCore.h"
#include "RHIDefinitions.h"
#include "Containers/LinkList.h"
#include "RHIResource.h"
#include "RHI.h"
namespace Air
{

	struct ResourceTableEntry;

	namespace EShaderPrecisionModifier
	{
		enum Type
		{
			Float,
			Half,
			Fixed
		};
	}

	class RENDER_CORE_API ShaderParametersMetadata
	{
	public:
		enum class EUseCase
		{
			ShaderParameterStruct,
			GlobalShaderParameterStruct,
			DataDrivenShaderParameterStruct,
		};

		static constexpr const TCHAR* kRootConstantBufferBindingName = TEXT("_RootShaderParameters");

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
				const ShaderParametersMetadata* inStruct
			)
				:mName(inName)
				,mShaderType(inShaderType)
				,mOffset(inOffset)
				,mBaseType(inBaseType)
				,mPrecision(inPrecision)
				,mNumRows(inNumRows)
				,mNumColumns(inNumColumns)
				,mNumElements(inNumElements)
				,mStruct(inStruct)
			{
			}


			const TCHAR* getName() const { return mName; }

			const TCHAR* getShaderType() const { return mShaderType; }

			uint32 getOffset() const { return mOffset; }

			EConstantBufferBaseType getBaseType() const { return mBaseType; }

			EShaderPrecisionModifier::Type getPrecision() const { return mPrecision; }

			uint32 getNumRows() const { return mNumRows; }

			uint32 getNumColumns() const { return mNumColumns; }

			uint32 getNumElements() const { return mNumElements; }

			const ShaderParametersMetadata* getStructMetadata() const { return mStruct; }

			inline uint32 getMemberSize() const 
			{
				BOOST_ASSERT(mBaseType == CBMT_BOOL || mBaseType == CBMT_FLOAT32 || mBaseType == CBMT_INT32 || mBaseType == CBMT_UINT32);
				uint32 elementSize = sizeof(uint32) * mNumRows * mNumColumns;

				if (mNumElements > 0)
				{
					return align(elementSize, SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT) * mNumElements;
				}
				return elementSize;
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
			const ShaderParametersMetadata* mStruct;
		};

		ShaderParametersMetadata(
			EUseCase useCase,
			const wstring& inLayoutName,
			const TCHAR* inStructTypeName,
			const TCHAR* inShaderVariableName,
			uint32 inSize,
			const TArray<Member>& inMembers
		);

		virtual ~ShaderParametersMetadata()
		{
			mGlobalListLink.unLink();

			getNameStructMap().erase(mStructTypeName);
		}

		void getNestedStructs(TArray<const ShaderParametersMetadata*>& outNestedStructs) const;

		uint32 getSize() const { return mSize; }

		static TMap<wstring, ShaderParametersMetadata*>& getNameStructMap();

		static TLinkedList<ShaderParametersMetadata*>*& getStructList();

		static void initializeAllGlobalStructs();

		const TArray<Member>& getMembers() const { return mMembers; }

		const TCHAR* getShaderVariableName() const { return mShaderVariableName; }

		void addResourceTableEntries(TMap<wstring, ResourceTableEntry>& resourceTableMap, TMap<wstring, uint32>& resourceTableLayoutHashes) const;

		const TCHAR* getStructTypeName() const { return mStructTypeName; }

		const RHIConstantBufferLayout& getLayout() const
		{
			BOOST_ASSERT(bLayoutInitialized);
			return mLayout;
		}
	private:
		const TCHAR* const mStructTypeName;
		const TCHAR* const mShaderVariableName;
		const uint32 mSize;
		const EUseCase mUseCase;

		RHIConstantBufferLayout mLayout;

		TArray<Member> mMembers;

		TLinkedList<ShaderParametersMetadata*> mGlobalListLink;

		uint32 bLayoutInitialized : 1;

		void initializeLayout();

		void addResourceTableEntriesRecursive(const TCHAR* constantBufferName, const TCHAR* prefix, uint16& resourceIndex, TMap<wstring, ResourceTableEntry>& resourceTableMap) const;
	};


}