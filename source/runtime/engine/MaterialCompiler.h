#pragma once
#include "EngineMininal.h"
#include "HAL/StringView.h"
#include "MaterialShared.h"
namespace Air
{
	enum EMaterialForceCastFlags
	{
		MFCF_ForceCast = 1 << 0,
		MFCF_ExactMatch = 1 << 2,
		MFCF_ReplicateValue = 1 << 3
	};


	class MaterialCompiler
	{
	public:
		

		virtual bool translate() = 0;

		virtual int32 constant(float x) = 0;

		virtual int32 constant2(float x, float y) = 0;
		
		virtual int32 constant3(float x, float y, float z) = 0;

		virtual int32 constant4(float x, float y, float z, float w) = 0;

		virtual int32 textureCoordinate(uint32 coordinateIndex, bool unMirrorU, bool unMirrorV) = 0;

		virtual void setShaderString(wstring_view shaderStr) {};
	
		virtual int32 error(const TCHAR* text) = 0;

		ENGINE_API int32 errorf(const TCHAR* format, ...);

		virtual int32 customExpression(class RMaterialExpressionCustom* custom, TArray<int32>& compiledInputs) = 0;

		virtual void setMaterialProperty(EMaterialProperty inProperty, EShaderFrequency overrideShaderFrequency = SF_NumFrequencies, bool bUsePreviousframeTime = false) = 0;

		virtual EShaderFrequency getCurrentShaderFrequency() const = 0;

		virtual int32 componentMask(int32 vector, bool r, bool g, bool b, bool a) = 0;

		virtual int32 vectorParameter(wstring parameterName, const LinearColor& defaultValue) = 0;

		virtual int32 forceCast(int32 code, EMaterialValueType destType, int32 forceCastFlags = 0) = 0;

		virtual void setBaseMaterialAttribute(const Guid& inAttributeId) = 0;

		virtual int32 add(int32 a, int32 b) = 0;
		virtual int32 sub(int32 a, int32 b) = 0;
		virtual int32 mul(int32 a, int32 b) = 0;
		virtual int32 div(int32 a, int32 b) = 0;
		virtual int32 dot(int32 a, int32 b) = 0;
		virtual int32 cross(int32 a, int32 b) = 0;

		virtual int32 power(int32 base, int32 exponent) = 0;
		virtual int32 logarithm2(int32 x) = 0;
		virtual int32 squareRoot(int32 x) = 0;
		virtual int32 length(int32 x) = 0;

		virtual int32 lerp(int32 x, int32 y, int32 a) = 0;
		virtual int32 min(int32 a, int32 b) = 0;
		virtual int32 max(int32 a, int32 b) = 0;
		virtual int32 clamp(int32 x, int32 a, int32 b) = 0;

		virtual int32 saturate(int32 x) = 0;

		virtual int32 appendVector(int32 a, int32 b) = 0;

		virtual int32 callExpression(MaterialExpressionKey expressionKey, MaterialCompiler* compiler) = 0;

		virtual const Guid getMaterialAttribute() = 0;

		virtual bool isCurrentlyCompilingForPreviousFrame() const { return false; }

		virtual Guid popMaterialAttribute() = 0;

		virtual int32 scalarParameter(wstring parameterName, float defaultValue) = 0;
	};

	class ScopedMaterialCompilerAttribute
	{
	public:
		ScopedMaterialCompilerAttribute(MaterialCompiler* inCompiler, const Guid& inAttributeId)
			:mCompiler(inCompiler)
			,mAttributeID(inAttributeId)
		{}

		~ScopedMaterialCompilerAttribute()
		{
			BOOST_ASSERT(mAttributeID == mCompiler->popMaterialAttribute());
		}
	private:
		MaterialCompiler * mCompiler;
		Guid mAttributeID;
	};
}