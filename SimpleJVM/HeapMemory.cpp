#include "stdafx.h"
#include "HeapMemory.h"
#include "JVMObject.h"
#include "JVMClass.h"

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

	JVMArray * HeapMemory::allocArray(JVMClass *pClass, int length)
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

		return new JVMArray(pClass, length, eleSize);
	}
}
