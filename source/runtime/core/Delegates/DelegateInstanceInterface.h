#pragma once
namespace Air
{
	template<typename FuncType>
	struct IBaseDelegateInstanceCommon;


	template<typename RetType, typename... ArgTypes>
	struct IBaseDelegateInstanceCommon<RetType(ArgTypes...)> : public IDelegateInstance
	{
		virtual void createCopy(DelegateBase& base) = 0;

		virtual RetType execute(ArgTypes...) const = 0;
	};


	template<typename FuncType>
	struct IBaseDelegateInstance : public IBaseDelegateInstanceCommon<FuncType>
	{};

	template<typename... ArgTypes>
	struct IBaseDelegateInstance<void(ArgTypes...)> : public IBaseDelegateInstanceCommon<void(ArgTypes...)>
	{
		virtual bool executeIfSafe(ArgTypes...) const = 0;
	};
}