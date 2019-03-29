#pragma once
uint2 scrambleTEA(uint2 v, uint iterationCount = 3)
{
	uint k[4] = { 0xA341316Cu, 0xC8012EA4u, 0xAD90777Du, 0x7E95761Eu };
	uint y = v[0];
	uint z = v[1];
	uint sum = 0;
	UNROLL for (uint i = 0; i < iterationCount; ++i)
	{
		sum += 0x9e3779b9;
		y += ((z << 4u) + k[0]) ^ (z + sum) ^ ((z >> 5u) + k[1]);
		z += ((y << 4u) + k[2]) ^ (y + sum) ^ ((y >> 5u) + k[3]);
	}
	return uint2(y, z);
}