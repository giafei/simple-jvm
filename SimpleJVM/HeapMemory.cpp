#include "stdafx.h"
#include "HeapMemory.h"
#include "JVMObject.h"
#include "JVMClass.h"

namespace jvm
{
	HeapMemory* pInstance = nullptr;

	HeapMemory::HeapMemory()
	{
		pInstance = this;
	}


	HeapMemory::~HeapMemory()
	{
		pInstance = this;
	}

	JVMObject * HeapMemory::alloc(JVMClass * pClass)
	{
		return new JVMObject(pClass);
	}

	JVMArray * HeapMemory::allocArray(JVMClass *pClass, int length)
	{
		return new JVMArray(pClass, length, arrayElementSize(pClass));
	}

	JAVAClassJVMObject * HeapMemory::allocClassObject(JVMClass * pClass, JVMClass * typeClass)
	{
		return new JAVAClassJVMObject(pClass, typeClass);
	}

	HeapMemory * HeapMemory::getHeap()
	{
		return pInstance;
	}

	int HeapMemory::arrayElementSize(JVMClass * pClass)
	{
		int eleSize = 4;
		auto name = pClass->getName();
		if (name->compare("byte[]") == 0)
		{
			eleSize = 1;
		}
		else if (name->compare("boolean[]") == 0)
		{
			eleSize = 1;
		}
		else if (name->compare("short[]") == 0)
		{
			eleSize = 2;
		}
		else if (name->compare("char[]") == 0)
		{
			eleSize = 2;
		}
		else if (name->compare("long[]") == 0)
		{
			eleSize = 8;
		}
		else if (name->compare("double[]") == 0)
		{
			eleSize = 8;
		}

		return eleSize;
	}
}
