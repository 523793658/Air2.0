#pragma once
#include "CoreMinimal.h"
#include "EngineMininal.h"
#include "Object.h"
#include "rapidxml/src/rapidxml.hpp" 
#include "misc/Guid.h"
#include "MaterialExpressionIO.h"
#include "Containers/Set.h"
#include "MaterialShared.h"
namespace Air
{
	struct MaterialAttributesInput : public ExpressionInput
	{
		MaterialAttributesInput()
			:mPropertyConnectedBitMask(0)
		{
			static_assert((uint32)(MP_Max)-1 <= (8 * sizeof(mPropertyConnectedBitMask)), "mPropertyConnectedBitMask cannot contain entire EMaterialProperty enumeration");

		}

#if WITH_EDITOR
		ENGINE_API int32 compileWithDefault(class MaterialCompiler* compiler, const Guid& attributeID);
#endif

		ENGINE_API void setConnectedProperty(EMaterialProperty prop, bool bIsConnected)
		{
			mPropertyConnectedBitMask = bIsConnected ? mPropertyConnectedBitMask | (1 << (uint32)prop) : mPropertyConnectedBitMask & ~(1 << (uint32)prop);
		}

		int32 mPropertyConnectedBitMask;
	};

	struct ExpressionOutput
	{
		wstring mOutputName;
		int32 mMask;
		int32 mMaskR;
		int32 mMaskG;
		int32 mMaskB;
		int32 mMaskA;

		ExpressionOutput(int32 inMask = 0, int32 inMaskR = 0, int32 inMaskG = 0, int32 inMaskB = 0, int32 inMaskA = 0)
			:mMask(inMask)
			,mMaskR(inMaskR)
			,mMaskG(inMaskG)
			,mMaskB(inMaskB)
			,mMaskA(inMaskA)
		{}

		ExpressionOutput(wstring inName, int32 inMask = 0, int32 inMaskR = 0, int32 inMaskG = 0, int32 inMaskB = 0, int32 inMaskA = 0)
			: mOutputName(inName)
			, mMask(inMask)
			, mMaskR(inMaskR)
			, mMaskG(inMaskG)
			, mMaskB(inMaskB)
			, mMaskA(inMaskA)
		{}
	};

	class ENGINE_API RMaterialExpression : public Object
	{
		GENERATED_RCLASS_BODY(RMaterialExpression, Object)
	public:
		class RMaterial* mMaterial;
		class RMaterialFunction* mFunction;
		virtual void serialize(Archive& ar);
		virtual void serialize(XMLNode* node);
		virtual void postLoad() override;

		const TArray<ExpressionInput*> getInputs();

		virtual const void innerGetInputs(TArray<ExpressionInput*>& result);

		virtual ExpressionInput* getInput(int32 inputIndex);

		virtual wstring getInputName(uint32 inputInde) const;

		void updateParameterGuid(bool bForceGeneration, bool bAllowMarkingPakageDirty);

		void updateMaterialExpressionGuid(bool bForceGeration, bool allowMarkingPacakagedirty);

		virtual Guid& getParameterExpressionId();

		virtual Guid& getMaterialExpressionId();

		virtual wstring getDescription() const;

		virtual int32 compile(class MaterialCompiler* compiler, int32 outputIndex) { return INDEX_NONE; }

		void validateState();

		bool containsInputLoop(const bool bStopOnFunctionCall = true);

		bool containsInputLoopInternal(TArray < MaterialExpressionKey > &expressionStack, TSet<MaterialExpressionKey>& visitedExpressions, const bool bStopOnFunctionCall);

		virtual bool isResultMaterialAttributes(int32 outputIndex) { return false; }

#if WITH_EDITOR
		virtual uint32 getInputType(int32 inputIndex);
		virtual uint32 getOutputType(int32 outputIndex);
		virtual TArray<ExpressionOutput>& getOutputs();
		virtual void getCaption(TArray<wstring>& outCaptions) const;
#endif

#if WITH_EDITORONLY_DATA
		wstring mLastErrorText;
#endif

		TArray<ExpressionOutput> mOutputs;
		uint32 bIsParameterExpression : 1;
		uint32 mID;
		Guid mMaterialExpressionGuid;
	};
}