#pragma once
#include "CoreMinimal.h"
namespace Air
{
	struct MovieSceneCaptureHandle 
	{
		MovieSceneCaptureHandle() : ID(0) {}
		friend bool operator == (const MovieSceneCaptureHandle& a, const MovieSceneCaptureHandle& b) { return a.ID == b.ID; }

		friend bool operator != (const MovieSceneCaptureHandle & a, const MovieSceneCaptureHandle & b) { return a.ID != b.ID; }
		bool isValid() const { return ID > 0; }
	protected:
		uint32 ID;
	};
}