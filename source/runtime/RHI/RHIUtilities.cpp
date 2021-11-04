#include "RHI.h"
namespace Air
{
	RHI_API ERHIAccess RHIGetDefaultResourceState(EBufferUsageFlags inUsage, bool bInHasInitialData)
	{
		ERHIAccess defaultReadingState = ERHIAccess::Unknown;
		if (inUsage & BUF_IndexBuffer)
		{
			defaultReadingState = ERHIAccess::VertexOrIndexBuffer;
		}

		if (inUsage & BUF_VertexBuffer)
		{
			defaultReadingState = defaultReadingState | ERHIAccess::VertexOrIndexBuffer | ERHIAccess::SRVMask;
		}
		if (inUsage & BUF_StructuredBuffer)
		{
			defaultReadingState = defaultReadingState | ERHIAccess::SRVMask;
		}

		ERHIAccess resourceState = (!enumHasAllFlags(defaultReadingState, ERHIAccess::VertexOrIndexBuffer)) ? ERHIAccess::Unknown : defaultReadingState;
		if (bInHasInitialData)
		{
			resourceState = defaultReadingState;
		}
		else
		{
			if (inUsage & BUF_UnorderedAccess)
			{
				resourceState = ERHIAccess::UAVMask;
			}
			else if (inUsage & BUF_ShaderResource)
			{
				resourceState = defaultReadingState | ERHIAccess::SRVMask;
			}
		}
		BOOST_ASSERT(resourceState != ERHIAccess::Unknown);
		return resourceState;
	}
}