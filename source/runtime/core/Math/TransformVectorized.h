#pragma once
#include "CoreType.h"
#include "Math/Vector.h"
#include "Math/Quaternion.h"
#include "Math/ScalarRegister.h"
namespace Air
{
	MS_ALIGN(16) struct Transform
	{
	protected:
		VectorRegister mRotation;
		VectorRegister mTranslation;
		VectorRegister mScale3D;
	public:

		FORCEINLINE Transform()
		{
			mRotation = VectorSet_W1(VectorZero());
			mTranslation = VectorZero();
			mScale3D = VectorSet_W0(VectorOne());
		}

		FORCEINLINE Transform(const Quaternion& inRotation, const float3 & inTranslation, const float3 & inScale = float3(1.0f))
		{
			mRotation = VectorLoadAligned(&inRotation.x);
			mTranslation = MakeVectorRegister(inTranslation.x
				, inTranslation.y, inTranslation.z, 0.0f);
			mScale3D = MakeVectorRegister(inScale.x, inScale.y, inScale.z, 0.0f);
			
		}

		FORCEINLINE explicit Transform(ENoInit)
		{

		}

		inline bool equals(const Transform& other, float tolerance = KINDA_SMALL_NUMBER) const
		{
			const ScalarRegister toleranceRegister(tolerance);
			return private_translationEquals(other.mTranslation, toleranceRegister) && private_scale3DEquals(other.mScale3D, toleranceRegister) && private_rotationEquals(other.mRotation, toleranceRegister);
		}

		FORCEINLINE static float3 subtractTranslations(const Transform& a, const Transform& b)
		{
			float3 result;
			VectorStoreFloat3(VectorSubtract(a.mTranslation, b.mTranslation), &result);
			return result;
		}

		FORCEINLINE float3 getLocation()const
		{
			return getTranslation();
		}

		FORCEINLINE Transform(const Rotator& inRotation, const float3 & inTranslation, const float3& inScale3D = float3(1.0f))
		{
			Quaternion inQuatRotation = inRotation.quaternion();
			mRotation = VectorLoadAligned(&inQuatRotation.x);
			mTranslation = MakeVectorRegister(inTranslation.x, inTranslation.y, inTranslation.z, 0.0f);
			mScale3D = MakeVectorRegister(inScale3D.x, inScale3D.y, inScale3D.z, 0.0f);
		}

		FORCEINLINE void setLocation(const float3& origin)
		{
			mTranslation = VectorLoadFloat3_W0(&origin);
		}

		FORCEINLINE void setRotation(const Quaternion& newRotation)
		{
			mRotation = VectorLoadAligned(&newRotation);
		}

		FORCEINLINE void copyTranslation(const Transform& other)
		{
			mTranslation = other.mTranslation;
		}

		FORCEINLINE void copyRotation(const Transform& other)
		{
			mRotation = other.mRotation;
		}

		FORCEINLINE void copyScale(const Transform& other)
		{
			mScale3D = other.mScale3D;
		}

		FORCEINLINE float3 getTranslation() const
		{
			float3 outTranslation;
			VectorStoreFloat3(mTranslation, &outTranslation);
			return outTranslation;
		}


		FORCEINLINE Quaternion getRotation() const
		{
			Quaternion quaRotation;
			VectorStoreAligned(mRotation, &quaRotation);
			return quaRotation;
		}

		FORCEINLINE float3 getScale3D() const
		{
			float3 outScale3D;
			VectorStoreFloat3(mScale3D, &outScale3D);
			return outScale3D;
		}
		static FORCEINLINE void constructTransformFromMatrixWithDesiredScale(const Matrix& AMatrix, const Matrix& BMatrix, const VectorRegister& desiredScale, Transform& outTransfrom)
		{
			Matrix m = AMatrix * BMatrix;
			m.removeScaling();
			float3 signedScale;
			VectorStoreFloat3(VectorSign(desiredScale), &signedScale);
			m.setAxis(0, signedScale.x * m.getScaleAxis(EAxis::X));
			m.setAxis(1, signedScale.y * m.getScaleAxis(EAxis::Y));
			m.setAxis(2, signedScale.z * m.getScaleAxis(EAxis::Z));

			Quaternion rotation = Quaternion(m);
			rotation.normalize();

			outTransfrom.mScale3D = desiredScale;
			outTransfrom.mRotation = VectorLoadAligned(&rotation);
			float3 translation = m.getOrigin();
			outTransfrom.mTranslation = VectorLoadFloat3_W0(&translation);
		}

		FORCEINLINE Matrix toMatrixWithScale() const
		{
			Matrix outMatrix;
			VectorRegister diagonalsXYZ;
			VectorRegister adds;
			VectorRegister subtracts;

			toMatrixInternal(diagonalsXYZ, adds, subtracts);

			const VectorRegister diagonalsXYZ_W0 = VectorSet_W0(diagonalsXYZ);

			const VectorRegister AddX_DC_DiagX_DC = VectorShuffle(adds, diagonalsXYZ_W0, 0, 0, 0, 0);
			const VectorRegister SubZ_DC_DiagW_DC = VectorShuffle(subtracts, diagonalsXYZ_W0, 2, 0, 3, 0);
			const VectorRegister Row0 = VectorShuffle(AddX_DC_DiagX_DC, SubZ_DC_DiagW_DC, 2, 0, 0, 2);

			const VectorRegister subX_DC_DiagY_DC = VectorShuffle(subtracts, diagonalsXYZ_W0, 0, 0, 1, 0);
			const VectorRegister addY_dc_diagw_dc = VectorShuffle(adds, diagonalsXYZ_W0, 1, 0, 3, 0);
			const VectorRegister row1 = VectorShuffle(subX_DC_DiagY_DC, addY_dc_diagw_dc, 0, 2, 0, 2);

			const VectorRegister AddZ_Dc_SubY_DC = VectorShuffle(adds, subtracts, 2, 0, 1, 0);
			const VectorRegister row2 = VectorShuffle(AddZ_Dc_SubY_DC, diagonalsXYZ_W0, 0, 2, 2, 3);

			VectorStoreAligned(Row0, &(outMatrix.M[0][0]));
			VectorStoreAligned(row1, &(outMatrix.M[1][0]));
			VectorStoreAligned(row2, &(outMatrix.M[2][0]));

			const VectorRegister row3 = VectorSet_W1(mTranslation);
			VectorStoreAligned(row3, &(outMatrix.M[3][0]));
			return outMatrix;
		}

		FORCEINLINE Matrix toMatrixNoScale() const
		{
			Matrix outMatirx;
			VectorRegister diagonalsXYA;
			VectorRegister adds;
			VectorRegister substracts;

			toMatrixInternalNoScale(diagonalsXYA, adds, substracts);
			const VectorRegister diagnoalsXYZ_W0 = VectorSet_W0(diagonalsXYA);

			const VectorRegister addx_dc_diagx_dc = VectorShuffle(adds, diagnoalsXYZ_W0, 0, 0, 0, 0);
			const VectorRegister subZ_dc_diagw_dc = VectorShuffle(substracts, diagnoalsXYZ_W0, 2, 0, 3, 0);
			const VectorRegister row0 = VectorShuffle(addx_dc_diagx_dc, subZ_dc_diagw_dc, 2, 0, 0, 2);

			const VectorRegister subx_dc_diagy_dc = VectorShuffle(substracts, diagnoalsXYZ_W0, 0, 0, 1, 0);
			const VectorRegister addY_dc_DiagW_dc = VectorShuffle(adds, diagnoalsXYZ_W0, 1, 0, 3, 0);
			const VectorRegister row1 = VectorShuffle(subx_dc_diagy_dc, addY_dc_DiagW_dc, 0, 2, 0, 2);

			const VectorRegister addZ_Dc_subY_dc = VectorShuffle(adds, substracts, 2, 0, 1, 0);
			const VectorRegister row2 = VectorShuffle(addZ_Dc_subY_dc, diagnoalsXYZ_W0, 0, 2, 2, 3);

			VectorStoreAligned(row0, &(outMatirx.M[0][0]));
			VectorStoreAligned(row1, &(outMatirx.M[1][0]));
			VectorStoreAligned(row2, &(outMatirx.M[2][0]));
			const VectorRegister row3 = VectorSet_W1(mTranslation);
			VectorStoreAligned(row3, &(outMatirx.M[3][0]));
			return outMatirx;
		}


		FORCEINLINE void toMatrixInternalNoScale(VectorRegister& outDiagonals, VectorRegister& outAdds, VectorRegister& outSubstract) const
		{
			const VectorRegister rotationX2Y2Z2 = VectorAdd(mRotation, mRotation);
			const VectorRegister rotationxx2yy2zz2 = VectorMultiply(rotationX2Y2Z2, mRotation);
			const VectorRegister yy2_xx2_xx2 = VectorSwizzle(rotationxx2yy2zz2, 1, 0, 0, 0);
			const VectorRegister zz2_zz2_yy2 = VectorSwizzle(rotationxx2yy2zz2, 2, 2, 1, 0);
			const VectorRegister diagnonalSum = VectorAdd(yy2_xx2_xx2, zz2_zz2_yy2);
			outDiagonals = VectorSubtract(VectorOne(), diagnonalSum);

			const VectorRegister x_y_z = VectorSwizzle(mRotation, 0, 1, 0, 0);
			const VectorRegister y2_z2_z2 = VectorSwizzle(rotationX2Y2Z2, 1, 2, 2, 0);
			const VectorRegister rotBase = VectorMultiply(x_y_z, y2_z2_z2);
			const VectorRegister w_w_w = VectorReplicate(mRotation, 3);
			const VectorRegister z2_x2_y2 = VectorSwizzle(rotationX2Y2Z2, 2, 0, 1, 0);
			const VectorRegister rotOffset = VectorMultiply(w_w_w, z2_x2_y2);

			outAdds = VectorAdd(rotBase, rotOffset);
			outSubstract = VectorSubtract(rotBase, rotOffset);
		}

		static FORCEINLINE void multiplyUsingMatrixWithScale(Transform* outTransform, const Transform* A, const Transform* B)
		{
			constructTransformFromMatrixWithDesiredScale(A->toMatrixWithScale(), B->toMatrixWithScale(), VectorMultiply(A->mScale3D, B->mScale3D), *outTransform);
		}

		FORCEINLINE bool isRotationNormalized()const
		{
			const VectorRegister testValue = VectorAbs(VectorSubtract(VectorOne(), VectorDot4(mRotation, mRotation)));
			return !VectorAnyGreaterThan(testValue, GlobalVectorConstants::ThreshQuatNormalized);
		}

		FORCEINLINE static void multiply(Transform* outTransfrom, const Transform* A, const Transform* B)
		{
			BOOST_ASSERT(A->isRotationNormalized());
			BOOST_ASSERT(B->isRotationNormalized());

			BOOST_ASSERT(VectorGetComponent(A->mScale3D, 3) == 0.f);
			BOOST_ASSERT(VectorGetComponent(B->mScale3D, 3) == 0.f);

			if (VectorAnyLesserThan(VectorMin(A->mScale3D, B->mScale3D), GlobalVectorConstants::FloatZero))
			{
				multiplyUsingMatrixWithScale(outTransfrom, A, B);
			}
			else
			{
				const VectorRegister quatA = A->mRotation;
				const VectorRegister quatB = B->mRotation;
				const VectorRegister translateA = A->mTranslation;
				const VectorRegister translateB = B->mTranslation;

				const VectorRegister scaleA = A->mScale3D;
				const VectorRegister scaleB = B->mScale3D;

				outTransfrom->mRotation = vectorQuaternionMultiply2(quatB, quatA);

				const VectorRegister scaledTransA = VectorMultiply(translateA, scaleB);
				const VectorRegister rotatedTanslate = vectorQuaternionRotateVector(quatB, scaledTransA);
				outTransfrom->mTranslation = VectorAdd(rotatedTanslate, translateB);
				outTransfrom->mScale3D = VectorMultiply(scaleA, scaleB);
			}
		}

		FORCEINLINE Transform operator*(const Transform& other) const
		{
			Transform output;
			multiply(&output, this, &other);
			return output;
		}

		static void getRelativeTransformUsingMatrixWithScale(Transform* outTransform, const Transform* base, const Transform* relative);

		static FORCEINLINE VectorRegister getSafeScaleReciprocal(const VectorRegister& inScale, const ScalarRegister& tolerance = ScalarRegister(GlobalVectorConstants::SmallNumber))
		{
			VectorRegister safeReciprocalScale;
			const VectorRegister reciprocalScale = VectorReciprocalAccurate(inScale);
			const VectorRegister scaleZeroMask = VectorCompareGE(tolerance.mValue, VectorAbs(inScale));
			safeReciprocalScale = VectorSelect(scaleZeroMask, VectorZero(), reciprocalScale);
			return safeReciprocalScale;
		}

		CORE_API Transform getRelativeTransform(const Transform& other) const
		{
			Transform result;
			if (other.isRotationNormalized() == false)
			{
				return Transform::identity;
			}

			if (VectorAnyLesserThan(VectorMin(this->mScale3D, other.mScale3D), GlobalVectorConstants::FloatZero))
			{
				getRelativeTransformUsingMatrixWithScale(&result, this, &other);
			}
			else
			{
				static ScalarRegister STolerance(SMALL_NUMBER);
				VectorRegister vSaveScale3D = VectorSet_W0(getSafeScaleReciprocal(other.mScale3D, STolerance));
				VectorRegister vscale3D = VectorMultiply(mScale3D, vSaveScale3D);
				VectorRegister vQtranslation = VectorSet_W0(VectorSubtract(mTranslation, other.mTranslation));
				VectorRegister vInverseRot = vectorQuaternionInverse(other.mRotation);

				VectorRegister vr = vectorQuaternionRotateVector(vInverseRot, vQtranslation);

				VectorRegister vTranslation = VectorMultiply(vr, vSaveScale3D);
				VectorRegister vrotation = vectorQuaternionMultiply2(vInverseRot, mRotation);
				result.mScale3D = vscale3D;
				result.mTranslation = vTranslation;
				result.mRotation = vrotation;
			}
			return result;
		}


		FORCEINLINE float3 inverseTransformPosition(const float3& v) const
		{
			const VectorRegister inputVector = VectorLoadFloat3_W0(&v);

			const VectorRegister tanslatedVec = VectorSet_W0(VectorSubtract(inputVector, mTranslation));

			const VectorRegister vr = vectorQuaternionInverseRotateVector(mRotation, tanslatedVec);

			const VectorRegister safeReciprocal = getSafeScaleReciprocal(mScale3D);
			const VectorRegister VResult = VectorMultiply(vr, safeReciprocal);

			float3 result;
			VectorStoreFloat3(VResult, &result);
			return result;
		}

		FORCEINLINE float3 transformVectorNoScale(const float3& v) const
		{
			const VectorRegister inputVectorW0 = VectorLoadFloat3_W0(&v);
			const VectorRegister rotatedVec = vectorQuaternionRotateVector(mRotation, inputVectorW0);
			float3 result;
			VectorStoreFloat3(rotatedVec, &result);
			return result;
		}

		FORCEINLINE float3 getUnitAxis(EAxis::Type inAxis) const
		{
			if (inAxis == EAxis::X)
			{
				return transformVectorNoScale(float3(1, 0, 0));
			}
			else if (inAxis == EAxis::Y)
			{
				return transformVectorNoScale(float3(0, 1, 0));
			}
			return transformVectorNoScale(float3(0, 0, 1));
		}
	private:
		FORCEINLINE void toMatrixInternal(VectorRegister& outDiagonals, VectorRegister& outAdds, VectorRegister& outSubtracts) const
		{
			const VectorRegister rotationX2Y2Z2 = VectorAdd(mRotation, mRotation);
			const VectorRegister rotationXX2YY2ZZ2 = VectorMultiply(rotationX2Y2Z2, mRotation);
			const VectorRegister yy2_xx2_xx2 = VectorSwizzle(rotationXX2YY2ZZ2, 1, 0, 0, 0);
			const VectorRegister zz2_zz2_yy2 = VectorSwizzle(rotationXX2YY2ZZ2, 2, 2, 1, 0);
			const VectorRegister diagonalSum = VectorAdd(yy2_xx2_xx2, zz2_zz2_yy2);
			const VectorRegister diagonals = VectorSubtract(VectorOne(), diagonalSum);
			outDiagonals = VectorMultiply(diagonals, mScale3D);

			const VectorRegister x_y_x = VectorSwizzle(mRotation, 0, 1, 0, 0);
			const VectorRegister y2_z2_z2 = VectorSwizzle(rotationX2Y2Z2, 1, 2, 2, 0);
			const VectorRegister rotBase = VectorMultiply(x_y_x, y2_z2_z2);

			const VectorRegister w_w_w = VectorReplicate(mRotation, 3);
			const VectorRegister z2_x2_y2 = VectorSwizzle(rotationX2Y2Z2, 2, 0, 1, 0);
			const VectorRegister rotOffset = VectorMultiply(w_w_w, z2_x2_y2);

			const VectorRegister adds = VectorAdd(rotBase, rotOffset);
			outAdds = VectorMultiply(adds, mScale3D);
			const VectorRegister scale3DYZXW = VectorSwizzle(mScale3D, 1, 2, 0, 3);
			const VectorRegister subtracts = VectorSubtract(rotBase, rotOffset);
			outSubtracts = VectorMultiply(subtracts, scale3DYZXW);
		}

		inline bool private_translationEquals(const VectorRegister& inTranslation, const ScalarRegister& tolerance = ScalarRegister(GlobalVectorConstants::KindaSmallNumber))const
		{
			const VectorRegister translationDiff = VectorAbs(VectorSubtract(mTranslation, inTranslation));
			return !VectorAnyGreaterThan(translationDiff, tolerance.mValue);
		}

		inline bool private_scale3DEquals(const VectorRegister& inScale3D, const ScalarRegister& tolerance = ScalarRegister(GlobalVectorConstants::KindaSmallNumber)) const
		{
			const VectorRegister scaleDiff = VectorAbs(VectorSubtract(mScale3D, inScale3D));
			return !VectorAnyGreaterThan(scaleDiff, tolerance.mValue);
		}

		inline bool private_rotationEquals(const VectorRegister& inRotation, const ScalarRegister& tolerance = ScalarRegister(GlobalVectorConstants::KindaSmallNumber))const
		{
			const VectorRegister rotationSub = VectorAbs(VectorSubtract(mRotation, inRotation));
			const VectorRegister rotationAdd = VectorAbs(VectorAdd(mRotation, inRotation));
			return !VectorAnyGreaterThan(rotationSub, tolerance.mValue) || !VectorAnyGreaterThan(rotationAdd, tolerance.mValue);
		}

		
	public:
		static CORE_API const Transform identity;
	};
}