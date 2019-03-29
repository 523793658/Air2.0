#if !defined(__Delegate_H__) || !defined(FUNC_INCLUDING_INLINE_IMPL)
#error	"This inline header must only be included by Delegate.h"
#endif


#pragma once
#include "CoreType.h"

class DelegateBase;

template <typename WrappedRetValType, typename... ParamTypes>
class TBaseDelegate : public DelegateBase
{

};