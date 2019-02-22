#pragma once

#include "jvmBase.h"

namespace jvm
{
	class JVMObject
	{
	public:
		JVMObject(JVMClass *pClass);
		virtual ~JVMObject();

	public:
		JVMClass *getClass()
		{
			return pClass;
		}

	public:
		std::shared_ptr<JavaValue> getField(const std::string& name);
		const std::map<std::string, std::shared_ptr<JavaValue>>& existsField()
		{
			return fields;
		}

	protected:
		JVMClass *pClass;
		std::map<std::string, std::shared_ptr<JavaValue>> fields;
	};

	class JVMArray : public JVMObject
	{
	public:
		JVMArray(JVMClass *pClass, int length, int elementSize):JVMObject(pClass)
		{
			this->length = length;
			this->elementSize = elementSize;
			if (length > 0)
			{
				this->data = (uint8*)malloc(elementSize * length);
				memset(this->data, 0, elementSize * length);
			}
			else
			{
				this->data = nullptr;
			}
		}

		~JVMArray()
		{
			free(data);
		}

	public:
		int getLength()
		{
			return length;
		}

		int getElementSize()
		{
			return elementSize;
		}

		void* getAddress(int i)
		{
			if (i < 0 || i >= length)
			{
				throw new std::exception("ArrayIndexOutOfBoundsException");
			}

			return data + elementSize * i;
		}

	private:
		int length;
		int elementSize;
		uint8 *data;
	};
}


