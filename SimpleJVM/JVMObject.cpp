#include "stdafx.h"
#include "JVMObject.h"
#include "JVMClass.h"

namespace jvm
{
	JVMObject::JVMObject(JVMClass *pClass)
	{
		this->pClass = pClass;
	}


	JVMObject::~JVMObject()
	{
	}

	std::shared_ptr<JavaValue> JVMObject::getField(const std::string & name)
	{
		auto it = fields.find(name);
		if (it != fields.end())
		{
			return it->second;
		}

		auto fieldInfo = pClass->getField(name);
		if (!fieldInfo)
		{
			return std::shared_ptr<JavaValue>();
		}

		auto fieldData = JavaValue::fromFieldDescriptor(*(fieldInfo->getDescriptor()));
		fields[name] = fieldData;

		return fieldData;
	}
}

