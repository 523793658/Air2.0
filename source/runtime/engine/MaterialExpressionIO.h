#pragma once
#include "CoreMinimal.h"
#include "Class.h"
#include "SceneTypes.h"
#include "rapidxml/src/rapidxml.hpp"
namespace Air
{
	using namespace rapidxml;
	typedef xml_node<TCHAR> XMLNode;
	struct ExpressionInput
	{
#if WITH_EDITORONLY_DATA
		class RMaterialExpression* mExpression;
		uint32 mExpressionId;

#endif

		unsigned int mOutputIndex;

		wstring mInputName;

		int32 mMask;

		int32 mMaskR;

		int32 mMaskG;

		int32 mMaskB;

		int32 mMaskA;

		wstring mExpressionName;

#if WITH_EDITOR
		ENGINE_API int32 compile(class MaterialCompiler* compiler);
#endif

		bool isConnected() const
		{
#if WITH_EDITORONLY_DATA
			return (nullptr != mExpression);
#else 
			return mExpressionName != Name_None;
#endif
		}

#if WITH_EDITOR
		ENGINE_API void connect(int32 inOutputIndex, class RMaterialExpression* inExpression);

		ENGINE_API void connect(class RMaterialExpression* inExpression);
#endif

		ENGINE_API bool serialize(Archive& ar);

		ENGINE_API bool serialize(XMLNode* node);

		ENGINE_API ExpressionInput getTracedInput() const;
	};

	template<class InputType>
	struct MaterialInput : ExpressionInput
	{
		uint32 bUseConstant : 1;
		InputType mConstant;
	};

	struct ColorMaterialInput : MaterialInput<Color>
	{
#if WITH_EDITOR
		ENGINE_API int32 compileWithDefault(class MaterialCompiler* comiler, EMaterialProperty pro);
#endif
		ENGINE_API bool serialize(XMLNode* node);
	};

	struct ScalarMaterialInput : public MaterialInput<float>
	{
#if WITH_EDITOR
		ENGINE_API int32 compileWithDefault(class MaterialCompiler* compiler, EMaterialProperty pro);
#endif
		ENGINE_API bool serialize(Archive & ar);
	};

	struct VectorMaterialInput : MaterialInput<float3>
	{
#if WITH_EDITOR
		ENGINE_API int32 compileWithDefault(class MaterialCompiler* compiler, EMaterialProperty prop);
#endif
	};

	struct Vector2MaterialInput : MaterialInput<float2>
	{
#if WITH_EDITOR 
		int32 compileWithDefault(class MaterialCompiler* compiler, EMaterialProperty prop);
#endif

		ENGINE_API bool serialize(XMLNode* node);
	};
}