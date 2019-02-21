#include "stdafx.h"
#include "HeapMemory.h"
#include "JVMObject.h"

namespace jvm
{
	HeapMemory::HeapMemory()
	{
	}


	HeapMemory::~HeapMemory()
	{
	}

	JVMObject * HeapMemory::alloc(JVMClass * pClass)
	{
		return new JVMObject(pClass);
	}
}
