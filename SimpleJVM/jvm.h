#pragma once

namespace ClassFile
{
	class Field;
	class Method;
	class Attribute;
	class StringAttribute;
	class CodeAttribute;
	class ConstantValueAttribute;
	class ExceptioneAttribute;
	class UnsupportAttribute;
	class ClassFileData;
}

namespace jvm
{
	class JVMObject;
	class JVMClass;
	class HeapMemory;

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

	public:
		JVMObject* getObject()
		{
			return value.object;
		}

		int32 getIntValue()
		{
			return value.intValue;
		}

		uint32 getUintValue()
		{
			return value.uintValue;
		}

		float getFloat()
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
}
