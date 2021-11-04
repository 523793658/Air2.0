#include "Shader.h"
namespace Air
{
	bool ShaderMapResource::arePlatformsCompatible(EShaderPlatform CurrentPlatform, EShaderPlatform TargetPlatform)
	{
		bool bFeatureLevelCompatible = CurrentPlatform == TargetPlatform;

		if (!bFeatureLevelCompatible && isPCPlatform(CurrentPlatform) && isPCPlatform(TargetPlatform))
		{
			bFeatureLevelCompatible = getMaxSupportedFeatureLevel(CurrentPlatform) >= getMaxSupportedFeatureLevel(TargetPlatform);

			bool const bIsTargetD3D = TargetPlatform == SP_PCD3D_SM5 ||
				TargetPlatform == SP_PCD3D_ES3_1;

			bool const bIsCurrentPlatformD3D = CurrentPlatform == SP_PCD3D_SM5 ||
				TargetPlatform == SP_PCD3D_ES3_1;

			// For Metal in Editor we can switch feature-levels, but not in cooked projects when using Metal shader librariss.
			bool const bIsCurrentMetal = isMetalPlatform(CurrentPlatform);
			bool const bIsTargetMetal = isMetalPlatform(TargetPlatform);
			bool const bIsMetalCompatible = (bIsCurrentMetal == bIsTargetMetal)
#if !WITH_EDITOR	// Static analysis doesn't like (|| WITH_EDITOR)
				&& (!IsMetalPlatform(CurrentPlatform) || (CurrentPlatform == TargetPlatform))
#endif
				;

			bool const bIsCurrentOpenGL = isOpenGLPlatform(CurrentPlatform);
			bool const bIsTargetOpenGL = isOpenGLPlatform(TargetPlatform);

			bFeatureLevelCompatible = bFeatureLevelCompatible && (bIsCurrentPlatformD3D == bIsTargetD3D && bIsMetalCompatible && bIsCurrentOpenGL == bIsTargetOpenGL);
		}

		return bFeatureLevelCompatible;
	}

	void ShaderMapResource::AddRef()
	{
		PlatformAtomics::interlockedIncrement((volatile int32*)&mNumRef);
	}

	void ShaderMapResource::Release()
	{
		BOOST_ASSERT(mNumRef > 0);
		if (PlatformAtomics::interLockedDecrement((volatile int32*)&mNumRef) == 0 && tryRelease())
		{
			// Send a release message to the rendering thread when the shader loses its last reference.
			beginReleaseResource(this);
			beginCleanup(this);
		}
	}

	void ShaderMapResource::releaseRHI()
	{
#if RHI_RAYTRACING
		for (int32 Index : RayTracingMaterialLibraryIndices)
		{
			RemoveFromRayTracingLibrary(Index);
		}
		RayTracingMaterialLibraryIndices.Empty();
#endif // RHI_RAYTRACING

		releaseShaders();
	}
	void ShaderMapResource::beginCreateAllShaders()
	{
		ShaderMapResource* Resource = this;
		ENQUEUE_RENDER_COMMAND(InitCommand)(
			[Resource](RHICommandListImmediate& RHICmdList)
			{
				for (int32 ShaderIndex = 0; ShaderIndex < Resource->getNumShaders(); ++ShaderIndex)
				{
					Resource->getShader(ShaderIndex);
				}
			});
	}

	ShaderMapResource::ShaderMapResource(EShaderPlatform InPlatform, int32 NumShaders)
		: mNumRHIShaders(NumShaders)
		, mPlatform(InPlatform)
		, mNumRef(0)
	{
		mRHIShaders = std::make_unique<std::atomic<RHIShader*>[]>(mNumRHIShaders); // this MakeUnique() zero-initializes the array
#if RHI_RAYTRACING
		if (GRHISupportsRayTracing)
		{
			RayTracingMaterialLibraryIndices.AddUninitialized(NumShaders);
			FMemory::Memset(RayTracingMaterialLibraryIndices.GetData(), 0xff, NumShaders * RayTracingMaterialLibraryIndices.GetTypeSize());
		}
#endif // RHI_RAYTRACING
	}

	ShaderMapResource::~ShaderMapResource()
	{
		releaseShaders();
		BOOST_ASSERT(mNumRef == 0);
	}

	RHIShader* ShaderMapResource::createShader(int32 ShaderIndex)
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		BOOST_ASSERT(!mRHIShaders[ShaderIndex].load(std::memory_order_acquire));

		TRefCountPtr<RHIShader> RHIShader = createRHIShader(ShaderIndex);
#if RHI_RAYTRACING
		if (GRHISupportsRayTracing && RHIShader.IsValid() && RHIShader->GetFrequency() == SF_RayHitGroup)
		{
			RayTracingMaterialLibraryIndices[ShaderIndex] = AddToRayTracingLibrary(static_cast<FRHIRayTracingShader*>(RHIShader.GetReference()));
		}
#endif // RHI_RAYTRACING

		// keep the reference alive (the caller will release)
		if (RHIShader.isValid())
		{
			RHIShader->AddRef();
		}
		return RHIShader.getReference();
	}

	void ShaderMapResource::releaseShaders()
	{
		if (mRHIShaders)
		{
			for (int32 Idx = 0; Idx < mNumRHIShaders; ++Idx)
			{
				if (RHIShader* shader = mRHIShaders[Idx].load(std::memory_order_acquire))
				{
					shader->Release();
				}
			}
			mRHIShaders = nullptr;
			mNumRHIShaders = 0;
		}
	}

	ShaderMapBase::~ShaderMapBase()
	{
		destroyContent();
		if (mPointerTable)
		{
			delete mPointerTable;
		}
	}


	ShaderMapResourceCode* ShaderMapBase::getResourceCode()
	{
		if (!mCode)
		{
			mCode = new ShaderMapResourceCode();
		}
		return mCode;
	}

	void ShaderMapBase::assignContent(ShaderMapContent* InContent)
	{
		BOOST_ASSERT(!mContent);
		BOOST_ASSERT(!mPointerTable);
		mContent = InContent;
		mPointerTable = createPointerTable();
	}

	void ShaderMapBase::finalizeContent()
	{
		if (mContent)
		{
			Content->validate(*this);
		}

		mCode->finalize();
		mResource = new ShaderMapResource_InlineCode(getShaderPlatform(), mCode);
		beginInitResource(mResource);

	}
	


	bool ShaderMapBase::serialize(Archive& Ar, bool bInlineShaderResources, bool bLoadedByCookedMaterial, bool bInlineShaderCode)
	{
		bool bContentValid = true;
		if (Ar.isSaving())
		{
			BOOST_ASSERT(mContent);
			mContent->validate(*this);

			ShaderMapPointerTable* SavePointerTable = createPointerTable();

			FMemoryImage MemoryImage;
			MemoryImage.PrevPointerTable = PointerTable;
			MemoryImage.PointerTable = SavePointerTable;
			MemoryImage.TargetLayoutParameters.InitializeForArchive(Ar);

			FMemoryImageWriter Writer(MemoryImage);

			Writer.WriteObject(Content, ContentTypeLayout);

			FMemoryImageResult MemoryImageResult;
			MemoryImage.Flatten(MemoryImageResult, true);

			void* SaveFrozenContent = MemoryImageResult.Bytes.GetData();
			uint32 SaveFrozenContentSize = MemoryImageResult.Bytes.Num();
			check(SaveFrozenContentSize > 0u);
			Ar << SaveFrozenContentSize;
			Ar.Serialize(SaveFrozenContent, SaveFrozenContentSize);
			MemoryImageResult.SaveToArchive(Ar);
			SavePointerTable->SaveToArchive(Ar, SaveFrozenContent, bInlineShaderResources);
			delete SavePointerTable;

			int32 NumDependencies = MemoryImage.TypeDependencies.Num();
			Ar << NumDependencies;
			for (const FTypeLayoutDesc* DependencyTypeDesc : MemoryImage.TypeDependencies)
			{
				uint64 NameHash = DependencyTypeDesc->NameHash;
				FSHAHash LayoutHash;
				uint32 LayoutSize = Freeze::HashLayout(*DependencyTypeDesc, MemoryImage.TargetLayoutParameters, LayoutHash);
				Ar << NameHash;
				Ar << LayoutSize;
				Ar << LayoutHash;
			}

			bool bShareCode = false;
#if WITH_EDITOR
			bShareCode = !bInlineShaderCode && FShaderCodeLibrary::IsEnabled() && Ar.IsCooking();
#endif // WITH_EDITOR
			Ar << bShareCode;
#if WITH_EDITOR
			if (Ar.IsCooking())
			{
				Code->NotifyShadersCooked(Ar.CookingTarget());
			}

			if (bShareCode)
			{
				FSHAHash ResourceHash = Code->ResourceHash;
				Ar << ResourceHash;
				FShaderCodeLibrary::AddShaderCode(GetShaderPlatform(), Code, GetAssociatedAssets());
			}
			else
#endif // WITH_EDITOR
			{
				Code->Serialize(Ar, bLoadedByCookedMaterial);
			}
		}
		else
		{
			check(!PointerTable);
			PointerTable = CreatePointerTable();

			Ar << FrozenContentSize;
			// ensure frozen content is at least as big as our FShaderMapContent-derived class
			checkf(FrozenContentSize >= ContentTypeLayout.Size, TEXT("Invalid FrozenContentSize for %s, got %d, expected at least %d"), ContentTypeLayout.Name, FrozenContentSize, ContentTypeLayout.Size);

			void* ContentMemory = FMemory::Malloc(FrozenContentSize);
			Ar.Serialize(ContentMemory, FrozenContentSize);
			Content = static_cast<FShaderMapContent*>(ContentMemory);
			FMemoryImageResult::ApplyPatchesFromArchive(Content, Ar);
			PointerTable->LoadFromArchive(Ar, Content, bInlineShaderResources, bLoadedByCookedMaterial);

			int32 NumDependencies = 0;
			Ar << NumDependencies;
			if (NumDependencies > 0)
			{
#if CHECK_SHADERMAP_DEPENDENCIES
				FPlatformTypeLayoutParameters LayoutParams;
				LayoutParams.InitializeForCurrent();
#endif // CHECK_SHADERMAP_DEPENDENCIES

				// Waste a bit of time even in shipping builds skipping over this stuff
				// Could add a cook-time option to exclude dependencies completely
				for (int32 i = 0u; i < NumDependencies; ++i)
				{
					uint64 NameHash = 0u;
					uint32 SavedLayoutSize = 0u;
					FSHAHash SavedLayoutHash;
					Ar << NameHash;
					Ar << SavedLayoutSize;
					Ar << SavedLayoutHash;
#if CHECK_SHADERMAP_DEPENDENCIES
					const FTypeLayoutDesc* DependencyType = FTypeLayoutDesc::Find(NameHash);
					if (DependencyType)
					{
						FSHAHash CheckLayoutHash;
						const uint32 CheckLayoutSize = Freeze::HashLayout(*DependencyType, LayoutParams, CheckLayoutHash);
						if (CheckLayoutSize != SavedLayoutSize)
						{
							UE_LOG(LogShaders, Error, TEXT("Mismatch size for type %s, compiled size is %d, loaded size is %d"), DependencyType->Name, CheckLayoutSize, SavedLayoutSize);
							bContentValid = false;
						}
						else if (CheckLayoutHash != SavedLayoutHash)
						{
							UE_LOG(LogShaders, Error, TEXT("Mismatch hash for type %s"), DependencyType->Name);
							bContentValid = false;
						}
					}
#endif // CHECK_SHADERMAP_DEPENDENCIES
				}
			}

			bool bShareCode = false;
			Ar << bShareCode;
			if (bShareCode)
			{
				FSHAHash ResourceHash;
				Ar << ResourceHash;
				Resource = FShaderCodeLibrary::LoadResource(ResourceHash, &Ar);
				if (!Resource)
				{
					// do not warn when running -nullrhi (the resource cannot be created as the shader library will not be uninitialized),
					// also do not warn for shader platforms other than current (if the game targets more than one RHI)
					if (FApp::CanEverRender() && GetShaderPlatform() == GMaxRHIShaderPlatform)
					{
						UE_LOG(LogShaders, Error, TEXT("Missing shader resource for hash '%s' for shader platform %d in the shader library"), *ResourceHash.ToString(), GetShaderPlatform());
					}
					bContentValid = false;
				}
			}
			else
			{
				Code = new FShaderMapResourceCode();
				Code->Serialize(Ar, bLoadedByCookedMaterial);
				Resource = new FShaderMapResource_InlineCode(GetShaderPlatform(), Code);
			}

			if (bContentValid)
			{
				check(Resource);
				NumFrozenShaders = Content->GetNumShaders();

				BeginInitResource(Resource);

				INC_DWORD_STAT_BY(STAT_Shaders_ShaderResourceMemory, Resource->GetSizeBytes());
				INC_DWORD_STAT_BY(STAT_Shaders_ShaderMemory, FrozenContentSize);
				INC_DWORD_STAT_BY(STAT_Shaders_NumShadersLoaded, NumFrozenShaders);
			}
			else
			{
				Resource.SafeRelease();

				// Don't call destructors here, this is basically unknown/invalid memory at this point
				FMemory::Free(Content);
				Content = nullptr;
			}
		}

		return bContentValid;
	}

	FString FShaderMapBase::ToString() const
	{
		TStringBuilder<32000> String;
		{
			FMemoryToStringContext Context;
			Context.PrevPointerTable = PointerTable;
			Context.String = &String;

			FPlatformTypeLayoutParameters LayoutParams;
			LayoutParams.InitializeForCurrent();

			ContentTypeLayout.ToStringFunc(Content, ContentTypeLayout, LayoutParams, Context);
		}

		if (Code)
		{
			Code->ToString(String);
		}

		return String.ToString();
	}

	void FShaderMapBase::DestroyContent()
	{
		if (Content)
		{
			DEC_DWORD_STAT_BY(STAT_Shaders_ShaderMemory, FrozenContentSize);
			DEC_DWORD_STAT_BY(STAT_Shaders_NumShadersLoaded, NumFrozenShaders);

			InternalDeleteObjectFromLayout(Content, ContentTypeLayout, FrozenContentSize > 0u);
			if (FrozenContentSize > 0u)
			{
				FMemory::Free(Content);
			}

			FrozenContentSize = 0u;
			NumFrozenShaders = 0u;
			Content = nullptr;
		}
	}

}