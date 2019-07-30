#pragma once 
#include "CoreMinimal.h"
#include "Classes/Materials/MaterialInterface.h"
#include "MaterialShared.h"
namespace Air
{
	enum EFoldedMathOperation
	{
		FMO_Add,
		FMO_Sub,
		FMO_Mul,
		FMO_Div,
		FMO_Dot
	};

	static float getSafeDivisor(float number)
	{
		if (Math::abs(number) < DELTA)
		{
			if (number < 0.0f)
			{
				return -DELTA;
			}
			else
			{
				return DELTA;
			}
		}
		else
		{
			return number;
		}
	}

	class MaterialConstantExpressionComponentSwizzle : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionComponentSwizzle);
	public:
		MaterialConstantExpressionComponentSwizzle() {}
		MaterialConstantExpressionComponentSwizzle(MaterialConstantExpression* inX, int8 intR, int8 inG, int8 inB, int8 inA)
			:mX(inX)
			,mIndexR(intR)
			,mIndexG(inG)
			,mIndexB(inB)
			,mIndexA(inA)
		{
			mNumElements = 0;
			if (inA >= 0)
			{
				BOOST_ASSERT(inA <= 3);
				++mNumElements;
				BOOST_ASSERT(inB >= 0);
			}
			if (inB >= 0)
			{
				BOOST_ASSERT(inB <= 3);
				++mNumElements;
				BOOST_ASSERT(inG >= 0);
			}
			if (inG >= 0)
			{
				BOOST_ASSERT(inG <= 3);
				++mNumElements;
			}
			BOOST_ASSERT(intR >= 0 && intR <= 3);
			++mNumElements;

			if (mNumElements == 1)
			{
				mIndexG = mIndexR;
				mIndexB = mIndexR;
				mIndexA = mIndexR;
				mNumElements = 4;
			}
		}

		void serialize(Archive& ar) override
		{

		}
		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			LinearColor temp = outValue;
			mX->getNumberValue(context, temp);
			outValue *= 0;
			switch (mNumElements)
			{
			case 4:
				outValue.A = temp.component(mIndexA);
			case 3:
				outValue.B = temp.component(mIndexB);
			case 2:
				outValue.G = temp.component(mIndexG);
			case 1:
				outValue.R = temp.component(mIndexR);
				break;
			default:
				AIR_LOG(LogMaterial, Fatal, TEXT("Invalid number of swizzle elements : %d"), mNumElements);
				break;
			}
		}

		virtual bool isConstant() const override
		{
			return mX->isConstant();
		}

		virtual bool isChangingPerFrame() const override
		{
			return mX->isChangingPerFrame();
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			auto* ohtherSwizzle = (MaterialConstantExpressionComponentSwizzle*)otherExpression;
			return mX->isIdentical(ohtherSwizzle->mX) &&
				mNumElements == ohtherSwizzle->mNumElements &&
				mIndexR == ohtherSwizzle->mIndexR &&
				mIndexG == ohtherSwizzle->mIndexG &&
				mIndexB == ohtherSwizzle->mIndexB &&
				mIndexA == ohtherSwizzle->mIndexA;
		}
	private:
		TRefCountPtr<MaterialConstantExpression> mX;
		int8 mIndexR;
		int8 mIndexG;
		int8 mIndexB;
		int8 mIndexA;
		int8 mNumElements;

	};


	class MaterialConstantExpressionVectorParameter : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionVectorParameter);
	public:
		MaterialConstantExpressionVectorParameter() {}
		MaterialConstantExpressionVectorParameter(wstring inParameterName, const LinearColor& inDefaultValue)
			:mDefaultValue(inDefaultValue)
			,mParameterName(inParameterName)
			,bUseOverriddenDefault(false)
		{}

		void serialize(Archive& ar) override
		{

		}
		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const
		{
			outValue.R = outValue.G = outValue.B = outValue.A = 0;
			if (!context.mMaterialRenderProxy->getVectorValue(mParameterName, &outValue, context))
			{
				getDefualtValue(outValue);
			}
		}

		void getDefualtValue(LinearColor& outValue) const
		{
			outValue = bUseOverriddenDefault ? mOverriddenDefaultValue : mDefaultValue;
		}

		void getGameThreadNumberValue(const MaterialInterface* sourceMaterialToCopyFrom, LinearColor& outValue) const;

		virtual bool isConstant() const override
		{
			return false;
		}

		wstring getParameterName() const
		{
			return mParameterName;
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			MaterialConstantExpressionVectorParameter* otherParemeter = (MaterialConstantExpressionVectorParameter*)otherExpression;
			return mParameterName == otherParemeter->mParameterName && mDefaultValue == otherParemeter->mDefaultValue;
		}

		void setTransientOverrideDefaultValue(const LinearColor& inOverrideDefaultValue, bool bInUseOverriddenDefault)
		{
			bUseOverriddenDefault = bInUseOverriddenDefault;
			mOverriddenDefaultValue = inOverrideDefaultValue;
		}
	private:
		wstring mParameterName;
		LinearColor mDefaultValue;
		bool bUseOverriddenDefault;
		LinearColor mOverriddenDefaultValue;
	};

	class MaterialConstantExpressionScalarParameter : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionScalarParameter);
	public:
		MaterialConstantExpressionScalarParameter()
			:bUseOverriddenDefault(false)
		{
		}
		MaterialConstantExpressionScalarParameter(wstring& inParameterName, float inDefualtValue)
			:mParameterName(inParameterName)
			,mDefaultValue(inDefualtValue)
			,bUseOverriddenDefault(false)
		{}

		virtual void serialize(Archive& ar) override
		{
			ar << mParameterName << mDefaultValue;
		}

		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			if (context.mMaterialRenderProxy->getScalarValue(mParameterName, &outValue.R, context))
			{
				outValue.G = outValue.B = outValue.A = outValue.R;
			}
			else
			{
				getDefaultValue(outValue.A);
				outValue.G = outValue.B = outValue.R = outValue.A;
			}
		}

		void getDefaultValue(float& outValue) const
		{
			outValue = bUseOverriddenDefault ? mOverriddenDefaultValue : mDefaultValue;
		}

		void getGameThreadNumberValue(const MaterialInterface* sourceMaterialToCopyFrom, float& outValue) const;

		virtual bool isConstant() const override
		{
			return false;
		}

		wstring getParameterName() const
		{
			return mParameterName;
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			MaterialConstantExpressionScalarParameter* otherParameter = (MaterialConstantExpressionScalarParameter*)otherExpression;
			return mParameterName == otherParameter->mParameterName && mDefaultValue == otherParameter->mDefaultValue;
		}

		void setTransientOverrideDefaultValue(float inOverrideDefaultValue, bool bInUseOverride)
		{
			bUseOverriddenDefault = bInUseOverride;
			mOverriddenDefaultValue = inOverrideDefaultValue;
		}
	private:
		wstring mParameterName;
		float mDefaultValue;
		bool bUseOverriddenDefault;
		float mOverriddenDefaultValue;
	};

	class MaterialConstantExpressionFoldedMath : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionFoldedMath);
	public:
		MaterialConstantExpressionFoldedMath(): mValueType(MCT_Float) {}
		MaterialConstantExpressionFoldedMath(MaterialConstantExpression* inA, MaterialConstantExpression* inB, uint8 inOp, uint32 inValueType = MCT_Float)
			:mA(inA)
			,mB(inB)
			,mValueType(inValueType)
			,mOp(inOp)
		{

		}

		virtual void serialize(Archive& ar) override
		{
			ar << mA << mB << mOp << mValueType;
		}

		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			LinearColor valueA = LinearColor::Black;
			LinearColor valueB = LinearColor::Black;
			mA->getNumberValue(context, valueA);
			mB->getNumberValue(context, valueB);
			switch (mOp)
			{
			case FMO_Add: outValue = valueA + valueB; break;
			case FMO_Sub: outValue = valueA - valueB; break;
			case FMO_Mul: outValue = valueA * valueB; break;
			case FMO_Div:
				outValue.R = valueA.R / getSafeDivisor(valueB.R);
				outValue.G = valueA.G / getSafeDivisor(valueB.G);
				outValue.B = valueA.B / getSafeDivisor(valueB.B);
				outValue.A = valueA.A / getSafeDivisor(valueB.A);
				break;
			case FMO_Dot:
			{
				BOOST_ASSERT(mValueType & MCT_Float);
				float dotProducct = valueA.R * valueB.R;
				dotProducct += (mValueType >= MCT_Float2) ? valueA.G * valueB.G : 0;
				dotProducct += (mValueType >= MCT_Float3) ? valueA.B * valueB.B : 0; 
				dotProducct += (mValueType >= MCT_Float4) ? valueA.A * valueB.A: 0; 
				outValue.R = outValue.G = outValue.B = outValue.A = dotProducct;
				break;
			}
			default:
				AIR_LOG(logMaterial, Fatal, TEXT("Unknown folded math operation: %08x"), (int32)mOp);
				break;
			}
		}

		virtual bool isConstant() const  override
		{
			return mA->isConstant() && mB->isConstant();
		}

		virtual bool isChangingPerFrame() const override
		{
			return mA->isChangingPerFrame() || mB->isChangingPerFrame();
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			MaterialConstantExpressionFoldedMath* otherParameter = (MaterialConstantExpressionFoldedMath*)otherExpression;
			return mA->isIdentical(otherParameter->mA) && mB->isIdentical(otherParameter->mB) && mValueType == otherParameter->mValueType && mOp == otherParameter->mOp;
		}

	private:
		TRefCountPtr<MaterialConstantExpression> mA;
		TRefCountPtr<MaterialConstantExpression> mB;
		uint32 mValueType;
		uint8 mOp;
	};

	class MaterialConstantExpressionLogarithm2 : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionLogarithm2);
	public:
		MaterialConstantExpressionLogarithm2() {}
		MaterialConstantExpressionLogarithm2(MaterialConstantExpression* inX):
			mX(inX)
		{}
		void serialize(Archive& ar) override
		{
			ar << mX;
		}

		void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			LinearColor valueX = LinearColor::Black;
			mX->getNumberValue(context, valueX);
			outValue.R = Math::log2(valueX.R);
			outValue.G = Math::log2(valueX.G);
			outValue.B = Math::log2(valueX.B);
			outValue.A = Math::log2(valueX.A);

		}

		bool isConstant() const override
		{
			return mX->isConstant();
		}

		bool isChangingPerFrame() const override
		{
			return mX->isChangingPerFrame();
		}

		bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			auto otherLog = (MaterialConstantExpressionLogarithm2*)otherExpression;
			return mX->isIdentical(otherLog->mX);
		}
	private:
		TRefCountPtr<MaterialConstantExpression> mX;
	};

	class  MaterialConstantExpressionSquareRoot : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionSquareRoot);
	public:
		MaterialConstantExpressionSquareRoot() {}
		MaterialConstantExpressionSquareRoot(MaterialConstantExpression* inX) :
			mX(inX)
		{}
		void serialize(Archive& ar) override
		{
			ar << mX;
		}

		void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			LinearColor valueX = LinearColor::Black;
			mX->getNumberValue(context, valueX);
			outValue.R = Math::square(valueX.R);
			outValue.G = Math::square(valueX.G);
			outValue.B = Math::square(valueX.B);
			outValue.A = Math::square(valueX.A);

		}

		bool isConstant() const override
		{
			return mX->isConstant();
		}

		bool isChangingPerFrame() const override
		{
			return mX->isChangingPerFrame();
		}

		bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			auto otherLog = (MaterialConstantExpressionSquareRoot*)otherExpression;
			return mX->isIdentical(otherLog->mX);
		}
	private:
		TRefCountPtr<MaterialConstantExpression> mX;
	};

	class MaterialConstantExpressionLength : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionLength);
	public:
		MaterialConstantExpressionLength():mValueType(MCT_Float){}
		MaterialConstantExpressionLength(MaterialConstantExpression* inX, uint32 inValueType)
			:mX(inX)
			,mValueType(inValueType)
		{}

		virtual void serialize(Archive& ar) override
		{
			ar << mX << mValueType;
		}

		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			LinearColor valueX = LinearColor::Black;
			mX->getNumberValue(context, valueX);
			BOOST_ASSERT(mValueType & MCT_Float);
			float lengthSq = valueX.R * valueX.R;
			lengthSq += (mValueType >= MCT_Float2) ? valueX.G * valueX.G : 0;
			lengthSq += (mValueType >= MCT_Float3) ? valueX.B * valueX.B : 0;
			lengthSq += (mValueType >= MCT_Float4) ? valueX.A * valueX.A : 0;
			outValue.R = outValue.G = outValue.B = outValue.A = Math::sqrt(lengthSq);
		}

		virtual bool isConstant() const override
		{
			return mX->isConstant();
		}

		virtual bool isChangingPerFrame() const override
		{
			return mX->isChangingPerFrame();
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}

			auto* other = (MaterialConstantExpressionLength*)otherExpression;
			return other->mX->isIdentical(mX) && mValueType == other->mValueType;
		}

	private:
		TRefCountPtr<MaterialConstantExpression> mX;
		uint32 mValueType;
	};

	class MaterialConstantExpressionMin : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionMin);
	public:
		MaterialConstantExpressionMin() {}
		MaterialConstantExpressionMin(MaterialConstantExpression* inA, MaterialConstantExpression* inB)
			:mA(inA)
			,mB(inB)
		{}
		virtual void serialize(Archive& ar) override
		{
			ar << mA << mB;
		}
		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			LinearColor valueA = LinearColor::Black;
			LinearColor valueB = LinearColor::Black;
			mA->getNumberValue(context, valueA);
			mB->getNumberValue(context, valueB);
			outValue.R = Math::min(valueA.R, valueB.R);
			outValue.G = Math::min(valueA.G, valueB.G);
			outValue.B = Math::min(valueA.B, valueB.B);
			outValue.A = Math::min(valueA.A, valueB.A);

		}

		virtual bool isConstant() const override
		{
			return mA->isConstant() && mB->isConstant();
		}

		virtual bool isChangingPerFrame() const override
		{
			return mA->isChangingPerFrame() || mB->isChangingPerFrame();
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			MaterialConstantExpressionMin* other = (MaterialConstantExpressionMin*)otherExpression;
			return mA->isIdentical(other->mA) && mB->isIdentical(other->mB);
		}


	private:
		TRefCountPtr<MaterialConstantExpression> mA;
		TRefCountPtr<MaterialConstantExpression> mB;
	};

	class MaterialConstantExpressionMax : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionMax);
	public:
		MaterialConstantExpressionMax() {}
		MaterialConstantExpressionMax(MaterialConstantExpression* inA, MaterialConstantExpression* inB)
			:mA(inA)
			, mB(inB)
		{}
		virtual void serialize(Archive& ar) override
		{
			ar << mA << mB;
		}
		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			LinearColor valueA = LinearColor::Black;
			LinearColor valueB = LinearColor::Black;
			mA->getNumberValue(context, valueA);
			mB->getNumberValue(context, valueB);
			outValue.R = Math::max(valueA.R, valueB.R);
			outValue.G = Math::max(valueA.G, valueB.G);
			outValue.B = Math::max(valueA.B, valueB.B);
			outValue.A = Math::max(valueA.A, valueB.A);

		}

		virtual bool isConstant() const override
		{
			return mA->isConstant() && mB->isConstant();
		}

		virtual bool isChangingPerFrame() const override
		{
			return mA->isChangingPerFrame() || mB->isChangingPerFrame();
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			MaterialConstantExpressionMax* other = (MaterialConstantExpressionMax*)otherExpression;
			return mA->isIdentical(other->mA) && mB->isIdentical(other->mB);
		}


	private:
		TRefCountPtr<MaterialConstantExpression> mA;
		TRefCountPtr<MaterialConstantExpression> mB;
	};

	class MaterialConstantExpressionClamp : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionMax);
	public:
		MaterialConstantExpressionClamp() {}
		MaterialConstantExpressionClamp(MaterialConstantExpression* input,  MaterialConstantExpression* inA, MaterialConstantExpression* inB)
			:mA(inA)
			, mB(inB)
			, mInput(input)
		{}
		virtual void serialize(Archive& ar) override
		{
			ar << mA << mB << mInput;
		}
		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			LinearColor valueA = LinearColor::Black;
			LinearColor valueB = LinearColor::Black;
			LinearColor valueInput = LinearColor::Black;
			mA->getNumberValue(context, valueA);
			mB->getNumberValue(context, valueB);
			mInput->getNumberValue(context, valueInput);
			outValue.R = Math::clamp(valueInput.R, valueA.R, valueB.R);
			outValue.G = Math::clamp(valueInput.G, valueA.G, valueB.G);
			outValue.B = Math::clamp(valueInput.B, valueA.B, valueB.B);
			outValue.A = Math::clamp(valueInput.A, valueA.A, valueB.A);

		}

		virtual bool isConstant() const override
		{
			return mA->isConstant() && mB->isConstant() && mInput->isConstant();
		}

		virtual bool isChangingPerFrame() const override
		{
			return mA->isChangingPerFrame() || mB->isChangingPerFrame() || mInput->isChangingPerFrame();
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			MaterialConstantExpressionClamp* other = (MaterialConstantExpressionClamp*)otherExpression;
			return mA->isIdentical(other->mA) && mB->isIdentical(other->mB) && mInput->isIdentical(other->mInput);
		}




	private:
		TRefCountPtr<MaterialConstantExpression> mA;
		TRefCountPtr<MaterialConstantExpression> mB;
		TRefCountPtr<MaterialConstantExpression> mInput;
	};



	class MaterialConstantExpressionSaturate : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionMax);
	public:
		MaterialConstantExpressionSaturate() {}
		MaterialConstantExpressionSaturate(MaterialConstantExpression* input)
			:mInput(input)
		{}
		virtual void serialize(Archive& ar) override
		{
			ar << mInput;
		}
		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			LinearColor valueInput = LinearColor::Black;
			mInput->getNumberValue(context, valueInput);
			outValue.R = Math::clamp(valueInput.R, 0.0f, 1.0f);
			outValue.G = Math::clamp(valueInput.G, 0.f, 1.f);
			outValue.B = Math::clamp(valueInput.B, 0.f, 1.f);
			outValue.A = Math::clamp(valueInput.A, 0.f, 1.f);

		}

		virtual bool isConstant() const override
		{
			return mInput->isConstant();
		}

		virtual bool isChangingPerFrame() const override
		{
			return mInput->isChangingPerFrame();
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			MaterialConstantExpressionSaturate* other = (MaterialConstantExpressionSaturate*)otherExpression;
			return  mInput->isIdentical(other->mInput);
		}




	private:
		TRefCountPtr<MaterialConstantExpression> mInput;
	};

	class MaterialConstantExpressionAppendVector : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionAppendVector);

	public:

		MaterialConstantExpressionAppendVector() {}
		MaterialConstantExpressionAppendVector(MaterialConstantExpression* inA, MaterialConstantExpression* inB, uint32 inNumComponentsA)
			:mA(inA)
			,mB(inB)
			,mNumComponentsA(inNumComponentsA)
		{
			
		}

		virtual void serialize(Archive& ar) override
		{
			ar << mA << mB << mNumComponentsA;
		}

		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			LinearColor colorA = LinearColor::Black;
			LinearColor colorB = LinearColor::Black;
			mA->getNumberValue(context, colorA);
			mB->getNumberValue(context, colorB);

			outValue.R = mNumComponentsA >= 1 ? colorA.R : (&colorB.R)[0 - mNumComponentsA];
			outValue.G = mNumComponentsA >= 2 ? colorA.G : (&colorB.R)[1 - mNumComponentsA];
			outValue.B = mNumComponentsA >= 3 ? colorA.B : (&colorB.R)[2 - mNumComponentsA];
			outValue.A = mNumComponentsA >= 4 ? colorA.A : (&colorB.R)[3 - mNumComponentsA];
		}

		virtual bool isConstant() const override
		{
			return mA->isConstant() && mB->isConstant();
		}

		virtual bool isChangingPerFrame() const override
		{
			return mA->isChangingPerFrame() || mB->isChangingPerFrame();
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			auto* other = (MaterialConstantExpressionAppendVector*)otherExpression;
			return mA->isIdentical(other->mA) && mB->isIdentical(other->mB) && mNumComponentsA == other->mNumComponentsA;
		}
	private:
		TRefCountPtr<MaterialConstantExpression> mA;
		TRefCountPtr<MaterialConstantExpression> mB;
		uint32 mNumComponentsA;
	};

	class MaterialConstantExpressionConstant : public MaterialConstantExpression
	{
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionConstant);
	public:
		MaterialConstantExpressionConstant() {}
		MaterialConstantExpressionConstant(const LinearColor& inValue, int32 inValueType)
			:mValue(inValue)
			, mValueType(inValueType)
		{}

		virtual void serialize(Archive& ar) override
		{
			ar << mValue << mValueType;
		}

		virtual void getNumberValue(const struct MaterialRenderContext& context, LinearColor& outValue) const override
		{
			outValue = mValue;
		}

		virtual bool isConstant() const override
		{
			return true;
		}

		virtual bool isIdentical(const MaterialConstantExpression* otherExpression) const override
		{
			if (getType() != otherExpression->getType())
			{
				return false;
			}
			MaterialConstantExpressionConstant* other = (MaterialConstantExpressionConstant*)otherExpression;
			return mValue == other->mValue && mValueType == other->mValueType;
		}
	private:
		LinearColor mValue;
		uint32 mValueType;
	};

	static std::shared_ptr<RTexture> getIndexedTexture(const FMaterial& material, int32 textureIndex)
	{
		const TArray<std::shared_ptr<RTexture>>& referencedTexture = material.getReferencedTextures();
		std::shared_ptr<RTexture> indexedTexture;
		if (referencedTexture.isValidIndex(textureIndex))
		{
			indexedTexture = referencedTexture[textureIndex];
		}
		else
		{
			static bool bwarnedOnce = false;
			if (!bwarnedOnce)
			{
				AIR_LOG(logMaterial, Warning, TEXT("MaterialConstantExpressionTexture had invalid textureIndex!"));
				bwarnedOnce = true;
			}
		}
		return indexedTexture;
	}


	class MaterialConstantExpressionTextureParameter : public MaterialConstantExpressionTexture
	{
		typedef MaterialConstantExpressionTexture ParentType;
		DECLARE_MATERIALCONSTANTEXPRESSION_TYPE(MaterialConstantExpressionTextureParameter);
	public:
		MaterialConstantExpressionTextureParameter() {}
		MaterialConstantExpressionTextureParameter(wstring inParameterName, int32 inTextureIndex, ESamplerSourceMode inSourceMode)
			:ParentType(inTextureIndex, inSourceMode)
			,mParameterName(inParameterName)
		{}

		virtual void serialize(Archive& ar)
		{
			ar << mParameterName;
			ParentType::serialize(ar);
		}

		virtual void getTextureValue(const MaterialRenderContext& context, const FMaterial& material, std::shared_ptr<const RTexture>& outValue, ESamplerSourceMode& outSamplerSource) const
		{
			BOOST_ASSERT(isInParallelRenderingThread());
			outSamplerSource = mSamplerSource;
			if (mTransientOverrideValue_RenderThread != nullptr)
			{
				outValue = mTransientOverrideValue_RenderThread;
			}
			else
			{
				outValue = nullptr;
				if (!context.mMaterialRenderProxy->getTextureValue(mParameterName, outValue, context))
				{
					outValue = getIndexedTexture(material, mTextureIndex);
				}
			}
		}



	private:
		wstring mParameterName;
	};
}