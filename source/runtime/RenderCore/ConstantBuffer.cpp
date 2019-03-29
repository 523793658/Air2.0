#include "ConstantBuffer.h"
namespace Air
{
	TLinkedList<ConstantBufferStruct*>*& ConstantBufferStruct::getStructList()
	{
		static TLinkedList<ConstantBufferStruct*>* GConstantBufferList = nullptr;
		return GConstantBufferList;
	}

	TMap<wstring, ConstantBufferStruct*>& ConstantBufferStruct::getNameStructMap()
	{
		static TMap<wstring, ConstantBufferStruct*> GlobalNameStructMap;
		return GlobalNameStructMap;
	}
}