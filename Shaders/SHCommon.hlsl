#pragma once
struct ThreeBandSHVector
{
	half4 V0;
	half4 V1;
	half V2;
};

ThreeBandSHVector SHBasisFunction3(half3 inputVector)
{
	ThreeBandSHVector result;
	result.V0.x = 0.282095f;
	result.V0.y = -0.488603f * inputVector.y;
	result.V0.z = 0.488603f * inputVector.z;
	result.V0.w = -0.488603f * inputVector.x;

	half3 vectorSquared = inputVector * inputVector;
	result.V1.x = 1.092548f * inputVector.x * inputVector.y;
	result.V1.y = -1.092548f * inputVector.y * inputVector.z;
	result.V1.z = 0.315392f * (3.0f * vectorSquared.z - 1.0f);
	result.V1.w = -1.092548f * inputVector.x * inputVector.z;
	
	result.V2 = 0.546274f * (vectorSquared.x * vectorSquared.y);
	return result;
}