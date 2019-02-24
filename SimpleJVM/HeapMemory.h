#pragma once

#include "jvmBase.h"

namespace jvm
{
	class HeapMemory
	{
	public:
		HeapMemory();
		~HeapMemory();

	public:
		JVMObject* alloc(JVMClass *pClass);
		JVMArray* allocArray(JVMClass* pClass, int length);
		JAVAClassJVMObject* allocClassObject(JVMClass *pClass, JVMClass* typeClass);

	public:
		JVMObject* getString(const char* str)
		{
			return stringPool[str];
		}

		void addString(const char* str, JVMObject* obj)
		{
			stringPool[str] = obj;
		}

	public:
		static HeapMemory* getHeap();
		static int arrayElementSize(JVMClass* pClass);

	protected:
		std::map<std::string, JVMObject*> stringPool;
	};
}



