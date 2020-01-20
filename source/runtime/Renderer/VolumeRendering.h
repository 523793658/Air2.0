#pragma once
#include "GlobalShader.h"
#include "ScreenRendering.h"
namespace Air
{
	struct VolumeBounds
	{
		int32 MinX, MinY, MinZ;
		int32 MaxX, MaxY, MaxZ;

		VolumeBounds():
			MinX(0),
			MinY(0),
			MinZ(0),
			MaxX(0),
			MaxY(0),
			MaxZ(0)
		{}

		VolumeBounds(int32 max)
			:MinX(0),
			MinY(0),
			MinZ(0),
			MaxX(max),
			MaxY(max),
			MaxZ(max)
		{

		}

		bool isValid() const
		{
			return MaxX > MinX&& MaxY > MinY&& MaxZ > MinZ;
		}
	};

	class WriteToSliceVS : public GlobalShader
	{
		DECLARE_EXPORTED_SHADER_TYPE(WriteToSliceVS, Global, ENGINE_API)
	public:
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::SM4);
		}
		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.mCompilerFlags.add(CFLAG_VertexToGeometryShader);
		}

		WriteToSliceVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mUVScaleBias.bind(initializer.mParameterMap, TEXT("UVScaleBias"));
			mMinZ.bind(initializer.mParameterMap, TEXT("MinZ"));
		}

		WriteToSliceVS() {}

		template<typename TRHICommandList>
		void setParameters(TRHICommandList& RHICmdList, const VolumeBounds& volumeBounds, int3 volumeResolution)
		{
			const float invVolumeResolutionX = 1.0f / volumeResolution.x;
			const float invVolumeResolutionY = 1.0f / volumeResolution.y;
			setShaderValue(RHICmdList, getVertexShader(), mUVScaleBias, float4(
				(volumeBounds.MaxX - volumeBounds.MinX) * invVolumeResolutionX,
				(volumeBounds.MaxY - volumeBounds.MinY) * invVolumeResolutionY,
				volumeBounds.MinX * invVolumeResolutionX, volumeBounds.MinY * invVolumeResolutionY
			));
			setShaderValue(RHICmdList, getVertexShader(), mMinZ, volumeBounds.MinZ);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bshaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mUVScaleBias;
			ar << mMinZ;
			return bshaderHasOutdatedParameters;
		}

	private:
		ShaderParameter mUVScaleBias;
		ShaderParameter mMinZ;
	};

	class WriteToSliceGS : public GlobalShader
	{
		DECLARE_EXPORTED_SHADER_TYPE(WriteToSliceGS, Global, ENGINE_API);
	public:
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::SM4) && RHISupportsGeometryShaders(parameters.mPlatform);
		}

		WriteToSliceGS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mMinZ.bind(initializer.mParameterMap, TEXT("MinZ"));
		}

		WriteToSliceGS() {}

		template<typename TRHICommandList>
		void setParameters(TRHICommandList& RHICmdList, int32 minZValue)
		{
			setShaderValue(RHICmdList, getGeometryShader(), mMinZ, minZValue);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool b = GlobalShader::serialize(ar);
			ar << mMinZ;
			return b;
		}
	private:
		ShaderParameter mMinZ;
	};

	extern RENDERER_API void rasterizeToVolumeTexture(RHICommandList& RHICmdList, VolumeBounds volumeBounds);

	class VolumeRasterizeVertexBuffer : public VertexBuffer
	{
	public:
		virtual void initRHI() override
		{
			const uint32 size = 4 * sizeof(ScreenVertex);
			RHIResourceCreateInfo createinfo;
			void* buffer = nullptr;
			mVertexBufferRHI = RHICreateAndLockVertexBuffer(size, BUF_Static, createinfo, buffer);
			ScreenVertex* destVertex = (ScreenVertex*)buffer;

			destVertex[0].mPosition = float2(1, -GProjectionSignY);
			destVertex[0].mUV = float2(1, 1);
			destVertex[1].mPosition = float2(1, GProjectionSignY);
			destVertex[1].mUV = float2(1, 0);
			destVertex[2].mPosition = float2(-1, -GProjectionSignY);
			destVertex[2].mUV = float2(0, 1);
			destVertex[3].mPosition = float2(-1, GProjectionSignY);
			destVertex[3].mUV = float2(1, 0);
		}
	};
}