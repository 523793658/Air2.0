#pragma once
#include "CoreType.h"
#include "Internaltionalization/ITextData.h"
namespace Air
{
	class CORE_API Text
	{
	public:
#if (!PLATFORM_WINDOWS) || (!defined(__clang__))
		static const Text& getEmpty()
		{
			static const Text staticEmptyText = Text(Text::EInitToEmptyString::value);
			return staticEmptyText;
		}
#else
		static const Text& getEmpty();
#endif

	public:
		Text();

	private:
		enum class EInitToEmptyString : uint8 { value };

		explicit Text(EInitToEmptyString);


		std::shared_ptr<ITextData>	mTextData;
		uint32 mFlags;
		static bool bEnableErrorCheckingResults;
		static bool bSuppressWarning;
	};
}