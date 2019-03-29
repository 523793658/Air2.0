#pragma once
#include "CoreMinimal.h"
namespace Air
{
	class SWidget;

	class ReplyBase
	{
	public:	 
		bool isEventHandled() const { return bIsHandled; }

		const std::shared_ptr<SWidget> getHandler() const { return mEventHandler; }
	protected:
		ReplyBase(bool isInHandled)
			:bIsHandled(isInHandled)
		{}

		bool bIsHandled;

		std::shared_ptr<SWidget> mEventHandler;
	};

	template<typename ReplyType>
	class TReplyBase : public ReplyBase
	{
	public:
		TReplyBase(bool bIsHandled)
			:ReplyBase(bIsHandled)
		{}
	protected:
		friend class EventRouter;

		ReplyType& setHandler(const std::shared_ptr<SWidget>& inHandler)
		{
			this->mEventHandler = inHandler;
			return me();
		}

		ReplyType& me()
		{
			return static_cast<ReplyType&>(*this);
		}
	};

	class NoReply : public TReplyBase<NoReply>
	{
	public:
		static NoReply unhandled() { return NoReply(); }

		NoReply():TReplyBase<NoReply>(false){}
	};
}