#pragma once
#include "SlateCore.h"
namespace Air
{

	namespace ESlateShaderResource
	{
		enum Type
		{
			NativeTexture,
			TextureObject,
			Material,
			PostProcess,
			Invalid
		};
	}


	

	class SLATE_CORE_API SlateShaderResource
	{
	public:
		virtual uint32 getWidth() const = 0;

		virtual uint32 getHeight() const = 0;

		virtual ESlateShaderResource::Type getType() const = 0;
	public:
		virtual ~SlateShaderResource() {}
	};

	class IViewportRenderTargetProvider
	{
	public:
		virtual SlateShaderResource* getViewportRenderTargetTexture() = 0;
	};

	template<typename ResourceType>
	class TSlateTexture : public SlateShaderResource
	{
	public:
		TSlateTexture() {}

		TSlateTexture(ResourceType& inShaderResource)
			: mShaderResource(inShaderResource)
		{}

		virtual ~TSlateTexture() {}

	public:
		virtual ESlateShaderResource::Type getType() const override
		{
			return ESlateShaderResource::NativeTexture;
		}

	protected:
		ResourceType	mShaderResource;
	};
}