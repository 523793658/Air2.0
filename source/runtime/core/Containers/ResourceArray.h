#pragma once
#include "CoreType.h"
namespace Air
{
	class ResourceBulkDataInterface
	{
	public:
		virtual const void* getResourceBulkData() const = 0;

		virtual uint32 getResourceBulkDataSize() const = 0;

		virtual void discard() = 0;
	};

	class ResourceArrayInterface
	{
	public:
		virtual const void* getResourceData() const = 0;

		virtual uint32 getResourceDataSize() const = 0;

		virtual void discard() = 0;

		virtual bool isStatic() const = 0;

		virtual bool getAllowCPUAccess() const = 0;

		virtual void setAllowCPUAccess(bool bInNeedCpuAccess) = 0;
	};

	class Texture2DResourceMem : public ResourceBulkDataInterface
	{
	public:
		virtual void * getMipData(int32 mipIndex) = 0;

		virtual int32 getNumMips() = 0; 

		virtual int32 getWidth() = 0;

		virtual int32 getHeight() = 0;

		virtual bool isValid() = 0;

		virtual bool hasAsyncAllocationCompleted() const = 0;

		virtual void finishAsyncAllocation() = 0;

		virtual void cancelAsyncAllocation() = 0;

		virtual ~Texture2DResourceMem() {}
	};
}