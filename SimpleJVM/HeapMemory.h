#pragma once

#include "jvm.h"

namespace jvm
{
	class HeapMemory
	{
	public:
		HeapMemory();
		~HeapMemory();

	public:
		JVMObject* alloc(JVMClass *pClass);

	public:
		JVMObject* getString(const char* str)
		{
			return stringPool[str];
		}

		void addString(const char* str, JVMObject* obj)
		{
			stringPool[str] = obj;
		}

	protected:
		std::map<std::string, JVMObject*> stringPool;
	};
}



