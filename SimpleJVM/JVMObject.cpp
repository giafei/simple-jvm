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
		int k = pClass->getFieldKey(name);
		if (k == 0)
		{
			return std::shared_ptr<JavaValue>();
		}

		return getField(k);
	}

	std::shared_ptr<JavaValue> JVMObject::getField(int key)
	{
		auto it = fieldValue.find(key);
		if (it != fieldValue.end())
		{
			return it->second;
		}

		auto fieldInfo = pClass->getFieldByKey(key);
		if (!fieldInfo)
		{
			return std::shared_ptr<JavaValue>();
		}

		auto fieldData = JavaValue::fromFieldDescriptor(*(fieldInfo->getDescriptor()));
		fieldValue[key] = fieldData;

		return fieldData;
	}

}

