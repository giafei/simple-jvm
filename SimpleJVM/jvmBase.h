#pragma once

namespace ClassFile
{
	class Field;
	class Method;
	class Attribute;
	class StringAttribute;
	class CodeAttribute;
	class ConstantValueAttribute;
	class ExceptionAttribute;
	class UnsupportAttribute;
	class ClassFileData;
}

namespace jvm
{
	class JVMObject;
	class JVMArray;
	class JVMClass;
	class HeapMemory;
	class Field;
	class Method;
	class JVMThread;
	class JVM;
	class ClassLoader;
	class StackFrame;
	class JAVAClassJVMObject;
	class ClassPath;

	struct SoltData
	{
		union
		{
			int32 intValue;
			uint32 uintValue;
			float floatValue;
			JVMObject* object;
		} value;

	public:
		SoltData()
		{
			value.intValue = 0;
		}

		SoltData(int32 v)
		{
			value.intValue = v;
		}

		SoltData(float v)
		{
			value.floatValue = v;
		}

		SoltData(JVMObject* v)
		{
			value.object = v;
		}

	public:
		JVMObject* getObject() const
		{
			return value.object;
		}

		int32 getIntValue() const
		{
			return value.intValue;
		}

		uint32 getUintValue() const
		{
			return value.uintValue;
		}

		float getFloat() const
		{
			return value.floatValue;
		}

	public:
		void setObject(JVMObject* v)
		{
			value.object = v;
		}

		void setIntValue(int32 v)
		{
			value.intValue = v;
		}

		void setUintValue(uint32 v)
		{
			value.uintValue = v;
		}

		void setFlatValue(float v)
		{
			value.floatValue = v;
		}
	};

	class JavaValue
	{
	public:
		JavaValue(int s)
		{
			soltSize = s;
		}

		JavaValue(SoltData v)
		{
			soltSize = 1;
			data[0] = v;
		}

	public:
		static int soltSizeFromFieldDescriptor(const std::string & d)
		{
			if (d.empty())
			{
				return 0;
			}
			else if (d.length() == 1)
			{
				char c = d.at(0);
				if ((c == 'D') || (c == 'J'))
				{
					return 2;
				}
				else if (c == 'V')
				{
					return 0;
				}
			}

			return 1;
		}

		static std::shared_ptr<JavaValue> fromFieldDescriptor(const std::string & d)
		{
			return std::shared_ptr<JavaValue>(new JavaValue(soltSizeFromFieldDescriptor(d)));
		}

		static int soltSizeFromMethodDescriptor(const std::string & d)
		{
			auto pos = d.find(')');
			return soltSizeFromFieldDescriptor(d.substr(pos + 1));
		}

		static std::shared_ptr<JavaValue> fromMethodDescriptor(const std::string & d)
		{
			auto pos = d.find(')');
			return fromFieldDescriptor(d.substr(pos + 1));
		}

	public:
		int getSoltSize()
		{
			return soltSize;
		}

		void setSoltSize(int i)
		{
			soltSize = i;
		}

		SoltData getSolt(int i)
		{
			return data[i];
		}

		void setSolt(int i, SoltData d)
		{
			data[i] = d;
		}

	public:
		int32 getIntValue()
		{
			return data[0].getIntValue();
		}

		void setIntValue(int32 v)
		{
			data[0].setIntValue(v);
			data[1].setIntValue(0);
		}

		float getFloatValue()
		{
			return data[0].getFloat();
		}

		void setFloatValue(float v)
		{
			data[0].setFlatValue(v);
			data[1].setIntValue(0);
		}

		JVMObject* getObjectValue()
		{
			return data[0].getObject();
		}

		void setObjectValue(JVMObject *v)
		{
			data[0].setObject(v);
			data[1].setIntValue(0);
		}

		int64 getLongValue()
		{
			if (soltSize == 1)
			{
				return data[0].getIntValue();
			}
			else
			{
				int64 v = 0;
				int32 *p = (int32*)&v;
				p[0] = data[0].getIntValue();
				p[1] = data[1].getIntValue();

				return v;
			}
		}

		void setLongValue(int64 v)
		{
			if (soltSize == 1)
			{
				data[0].setIntValue((int32)v);
				data[1].setIntValue(0);
			}
			else
			{
				int32 *p = (int32*)&v;
				data[0].setIntValue(p[0]);
				data[1].setIntValue(p[1]);
			}
		}

		double getDoubleValue()
		{
			int64 v = getLongValue();
			return *(double*)&v;
		}

		void setDoubleValue(double v)
		{
			setLongValue(*(int64*)&v);
		}

	private:
		int soltSize;
		SoltData data[2];
	};
}
