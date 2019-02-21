#pragma once

#include "jvm.h"

namespace jvm
{
	class JVMObject
	{
	public:
		JVMObject(JVMClass *pClass);
		~JVMObject();

	public:
		JVMClass *getClass()
		{
			return pClass;
		}

	public:
		void setField(const std::string& name, SoltData value);
		SoltData getField(const std::string& name);

	protected:
		JVMClass *pClass;
		std::map<std::string, SoltData> fields;
	};
}


