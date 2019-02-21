#include "stdafx.h"
#include "JVMObject.h"

namespace jvm
{
	JVMObject::JVMObject(JVMClass *pClass)
	{
		this->pClass = pClass;
	}


	JVMObject::~JVMObject()
	{
	}

	void JVMObject::setField(const std::string & name, SoltData value)
	{

	}

	SoltData JVMObject::getField(const std::string & name)
	{
		return SoltData();
	}
}

