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
		std::shared_ptr<JavaValue> getField(int key);

		const std::map<int, std::shared_ptr<JavaValue>>& existsField()
		{
			return fieldValue;
		}

		template<typename T>
		T getNativeData(const std::string& name)
		{
			return (T)nativeData[name];
		}

		void setNativeData(const std::string& name, int64 data)
		{
			nativeData[name] = data;
		}

	protected:
		JVMClass *pClass;
		std::map<std::string, int64> nativeData;
		std::map<int, std::shared_ptr<JavaValue>> fieldValue;
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

		template <typename T>
		T* geElementAddress(int i)
		{
			if (i < 0 || i >= length)
			{
				throw new std::exception("ArrayIndexOutOfBoundsException");
			}

			if (elementSize != sizeof(T))
			{
				throw new std::exception("ArrayElementSizeException");
			}

			return (T*)(data + elementSize * i);
		}

	private:
		int length;
		int elementSize;
		uint8 *data;
	};

	class JAVAClassJVMObject : public JVMObject
	{
	public:
		JAVAClassJVMObject(JVMClass* javaClassClass, JVMClass* genericTypeClass)
			:JVMObject(javaClassClass)
		{
			this->genericTypeClass = genericTypeClass;
		}

	public:
		JVMClass* getGenericTypeClass()
		{
			return genericTypeClass;
		}

	protected:
		JVMClass* genericTypeClass;
	};
}


