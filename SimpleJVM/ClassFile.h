#pragma once

#include "DataBlock.h"

class DataReader;
namespace ClassFile
{
	class ClassFile;
	enum ConstantType
	{
		CONSTANT_Empty = 0,
		CONSTANT_Utf8_info = 1,
		CONSTANT_Integer_info = 3,
		CONSTANT_Float_info = 4,
		CONSTANT_Long_info = 5,
		CONSTANT_Double_info = 6,
		CONSTANT_Class_info = 7,
		CONSTANT_String_info = 8,
		CONSTANT_Fieldref_info = 9,
		CONSTANT_Methodref_info = 10,
		CONSTANT_InterfaceMethodref_info = 11,
		CONSTANT_NameAndType_info = 12,
		CONSTANT_MethodHandle_info = 15,
		CONSTANT_MethodType_info = 16,
		CONSTANT_InvokeDynamic_info = 18
	};

	class Constant
	{
	public:
		virtual ~Constant(){}

	public:
		void setType(ConstantType type)
		{
			this->type = type;
		}

		ConstantType getType() const
		{
			return this->type;
		}

	protected:
		friend class ClassFile;
		virtual void onPostPoolReady(const ClassFile *pClassFile) {}

	protected:
		ConstantType type;
	};

	class ConstantInt : public Constant
	{
	public:
		uint32 getData() const
		{
			return data;
		}

		void setData(uint32 data)
		{
			this->data = data;
		}

	protected:
		uint32 data;
	};

	class ConstantFloat : public Constant
	{
	public:
		float getData() const
		{
			return data;
		}

		void setData(float data)
		{
			this->data = data;
		}

	protected:
		float data;
	};

	class ConstantLong : public Constant
	{
	public:
		uint64 getData() const
		{
			return data;
		}

		void setData(uint64 data)
		{
			this->data = data;
		}

	protected:
		uint64 data;
	};

	class ConstantDouble : public Constant
	{
	public:
		double getData() const
		{
			return data;
		}

		void setData(double data)
		{
			this->data = data;
		}

	protected:
		double data;
	};

	class ConstantUTF8String : public Constant
	{
	public:
		std::shared_ptr<const std::string> getData() const
		{
			return data;
		}

		void setData(std::shared_ptr  <const std::string> data)
		{
			this->data = data;
		}

	protected:
		std::shared_ptr<const std::string> data;
	};

	class ConstantString : public ConstantUTF8String
	{
	public:
		void setValueIndex(int i)
		{
			valueIndex = i;
		}

	protected:
		virtual void onPostPoolReady(const ClassFile *pClassFile);

	protected:
		int valueIndex;
	};

	class ConstantClassInfo : public ConstantUTF8String
	{
	public:
		void setNameIndex(int i)
		{
			nameIndex = i;
		}

	protected:
		virtual void onPostPoolReady(const ClassFile *pClassFile);

	protected:
		int nameIndex;
	};

	class ConstantNameAndType : public Constant
	{
	public:
		std::shared_ptr<const std::string> getName() const
		{
			return name;
		}

		std::shared_ptr<const std::string> getDescriptor() const
		{
			return descriptor;
		}

		void setNameIndex(int i)
		{
			this->nameIndex = i;
		}

		void setDescriptorIndex(int i)
		{
			this->descriptorIndex = i;
		}

	protected:
		virtual void onPostPoolReady(const ClassFile *pClassFile);

	protected:
		std::shared_ptr<const std::string> name;
		std::shared_ptr<const std::string> descriptor;

		int nameIndex = 0;
		int descriptorIndex = 0;
	};

	class ConstantMemberRef : public Constant
	{
	public:
		std::shared_ptr<const ConstantClassInfo> getClassData() const
		{
			return classData;
		}

		std::shared_ptr<const ConstantNameAndType> getNameAndType() const
		{
			return nameAndType;
		}

		void setClassIndex(int i)
		{
			this->classIndex = i;
		}

		void setNameAndTypeIndex(int i)
		{
			this->nameAndTypeIndex = i;
		}

	protected:
		virtual void onPostPoolReady(const ClassFile *pClassFile);

	protected:
		std::shared_ptr<const ConstantClassInfo> classData;
		std::shared_ptr<const ConstantNameAndType> nameAndType;

		int classIndex = 0;
		int nameAndTypeIndex = 0;
	};

	class ConstantMethodHandleInfo : public Constant
	{
	public:
		void setRefrenceKind(int kind)
		{
			refrenceKind = kind;
		}

		void setRefrenceIndex(int i)
		{
			refrenceIndex = i;
		}

	protected:
		int refrenceKind = 0;
		int refrenceIndex = 0;
	};

	class ConstantMethodTypeInfo : public Constant
	{
	public:
		std::shared_ptr<const std::string> getDescriptor() const
		{
			return descriptor;
		}

		void setDescriptorIndex(int i)
		{
			this->descriptorIndex = i;
		}

	protected:
		virtual void onPostPoolReady(const ClassFile *pClassFile);

	protected:
		int descriptorIndex = 0;
		std::shared_ptr<const std::string> descriptor;
	};

	class ConstantInvokeDynamicInfo : public Constant
	{
	public:
		std::shared_ptr<const ConstantNameAndType> getNameAndType() const
		{
			return nameAndType;
		}

		void setMethodAttrIndex(int i)
		{
			methodAttrIndex = i;
		}

		void setNameAndTypeIndex(int i)
		{
			nameAndTypeIndex = i;
		}

	protected:
		virtual void onPostPoolReady(const ClassFile *pClassFile);

	protected:
		std::shared_ptr<const ConstantNameAndType> nameAndType;
		int methodAttrIndex = 0;
		int nameAndTypeIndex = 0;
	};

	class Attribute
	{
	public:
		virtual ~Attribute() {}

	public:
		std::shared_ptr<const std::string> getName() const
		{
			return name;
		}

		void setName(std::shared_ptr<const std::string> name)
		{
			this->name = name;
		}

	protected:
		std::shared_ptr<const std::string> name;
	};

	class StringAttribute: public Attribute
	{
	public:
		std::shared_ptr<const std::string> getData() const
		{
			return data;
		}

		void setData(std::shared_ptr<const std::string> data)
		{
			this->data = data;
		}

	protected:
		std::shared_ptr<const std::string> data;
	};

	class CodeAttribute : public Attribute
	{
	public:
		void addAttribute(const std::string& k, std::shared_ptr<const Attribute> v)
		{
			attributes[k] = v;
		}

		const std::map<const std::string, std::shared_ptr<const Attribute>> getAttributes() const
		{
			return attributes;
		}

		void setMaxStack(int maxStack)
		{
			this->maxStack = maxStack;
		}

		int getMaxStack() const
		{
			return this->maxStack;
		}

		void setMaxLocals(int maxLocals)
		{
			this->maxLocals = maxLocals;
		}

		int getMaxLocals() const
		{
			return this->maxLocals;
		}

		void setCode(std::shared_ptr<const DataBlock> code)
		{
			this->code = code;
		}

		std::shared_ptr<const DataBlock> getCode() const
		{
			return code;
		}

		void addException(const uint64& e)
		{
			exceptionTable.push_back(e);
		}

		const std::vector<uint64>& getExceptions()
		{
			return exceptionTable;
		}

	protected:
		int maxStack;
		int maxLocals;
		std::shared_ptr<const DataBlock> code;
		std::vector<uint64> exceptionTable;
		std::map<const std::string, std::shared_ptr<const Attribute>> attributes;
	};

	class ConstantValueAttribute : public Attribute
	{
	public:
		std::shared_ptr<const Constant> getData() const
		{
			return data;
		}

		void setData(std::shared_ptr<const Constant> data)
		{
			this->data = data;
		}

	protected:
		std::shared_ptr<const Constant> data;
	};

	class ExceptioneAttribute : public Attribute
	{
	public:
		const std::vector<std::shared_ptr<const ConstantString>> getData() const
		{
			return data;
		}

		void addException(std::shared_ptr<const ConstantString> data)
		{
			this->data.push_back(data);
		}

	protected:
		std::vector<std::shared_ptr<const ConstantString>> data;
	};

	class UnsupportAttribute : public Attribute
	{
	public:
		void setData(std::shared_ptr<const DataBlock> data)
		{
			this->data = data;
		}

		std::shared_ptr<const DataBlock> getData() const
		{
			return data;
		}
	protected:
		std::shared_ptr<const DataBlock> data;
	};

	namespace FieldAccess
	{
		enum FieldAccess
		{
			ACC_PUBLIC = 0x0001,
			ACC_PRIVATE = 0x0002,
			ACC_PROTECTED = 0x0004,
			ACC_STATIC = 0x0008,
			ACC_FINAL = 0x0010,
			ACC_VOLATILE = 0x0040,
			ACC_TRANSIENT = 0x0080,
			ACC_SYNTHETIC = 0x1000,
			ACC_ENUM = 0x4000
		};
	}

	class Field
	{
	public:
		void setAccessFlag(uint32 flag)
		{
			accessFlag = flag;
		}

		void setName(std::shared_ptr<const std::string> name)
		{
			this->name = name;
		}

		/*
		    1）类型描述符。
			①基本类型byte、short、char、int、long、float和double的描述符
			是单个字母，分别对应B、S、C、I、J、F和D。注意，long的描述符是J而不是L。
			②引用类型的描述符是L＋类的完全限定名＋分号。
			③数组类型的描述符是[＋数组元素类型描述符。
		*/
		void setDescriptor(std::shared_ptr<const std::string> descriptor)
		{
			this->descriptor = descriptor;
		}

		void addAttribute(const std::string& k, std::shared_ptr<const Attribute> v)
		{
			attributes[k] = v;
		}

		uint32 getAccessFlag() const
		{
			return accessFlag;
		}

		std::shared_ptr<const std::string> getName() const
		{
			return name;
		}

		std::shared_ptr<const std::string> getDescriptor() const
		{
			return descriptor;
		}

		const std::map<const std::string, std::shared_ptr<const Attribute>> getAttributes() const
		{
			return attributes;
		}

	private:
		uint32 accessFlag;
		std::shared_ptr<const std::string> name;
		std::shared_ptr<const std::string> descriptor;
		std::map<const std::string, std::shared_ptr<const Attribute>> attributes;
	};


	namespace MethodAccess
	{
		enum MethodAccess
		{
			ACC_PUBLIC = 0x0001,
			ACC_PRIVATE = 0x0002,
			ACC_PROTECTED = 0x0004,
			ACC_STATIC = 0x0008,
			ACC_FINAL = 0x0010,
			ACC_SYNCHRONIZED = 0x0020,
			ACC_BRIDGE = 0x0040,
			ACC_VARARGS = 0x0080,
			ACC_NATIVE = 0x0100,
			ACC_ABSTRACT = 0x0400,
			ACC_STRICTFP = 0x0800,
			ACC_SYNTHETIC = 0x1000
		};
	}

	class Method
	{
	public:
		void setAccessFlag(uint32 flag)
		{
			accessFlag = flag;
		}

		void setName(std::shared_ptr<const std::string> name)
		{
			this->name = name;
		}

		/*
		接Field
		方法描述符是（分号分隔的参数类型描述符）+返回值类型描述符，其中void返回值由单个字母V表示。
		*/
		void setDescriptor(std::shared_ptr<const std::string> descriptor)
		{
			this->descriptor = descriptor;
		}

		void addAttribute(const std::string& k, std::shared_ptr<const Attribute> v)
		{
			attributes[k] = v;
		}

		uint32 getAccessFlag() const
		{
			return accessFlag;
		}

		std::shared_ptr<const std::string> getName() const
		{
			return name;
		}

		std::shared_ptr<const std::string> getDescriptor() const
		{
			return descriptor;
		}

		const std::map<const std::string, std::shared_ptr<const Attribute>> getAttributes() const
		{
			return attributes;
		}

	private:
		uint32 accessFlag;
		std::shared_ptr<const std::string> name;
		std::shared_ptr<const std::string> descriptor;
		std::map<const std::string, std::shared_ptr<const Attribute>> attributes;
	};

	namespace ClassFileAccess
	{

		enum ClassFileAccess
		{
			ACC_PUBLIC = 0x0001,
			ACC_FINAL = 0x0010,
			ACC_SUPER = 0x0020,
			ACC_INTERFACE = 0x0200,
			ACC_ABSTRACE = 0x0400,
			ACC_SYNTHETIC = 0x1000,
			ACC_ANNOTATION = 0x2000,
			ACC_ENUM = 0x4000
		};
	}

	class ClassFile
	{
	public:
		ClassFile(DataReader& reader);
		~ClassFile();

	public:
		std::shared_ptr<const std::string> getConstantString(int i) const;

		template <typename T>
		std::shared_ptr<const T> getConstant(int i) const
		{
			std::shared_ptr<Constant> ptr = this->constantPool.at(i);
			return std::dynamic_pointer_cast<const T>(ptr);
		}


	protected:
		std::shared_ptr<Constant> readConstant(DataReader& reader, ConstantType tag);
		std::shared_ptr<const Field> readField(DataReader& reader);
		std::shared_ptr<const Method> readMethod(DataReader& reader);
		std::shared_ptr<const Attribute> readAttribute(DataReader& reader);

	private:
		int minorVersion;
		int majorVersion;
		std::map<int, std::shared_ptr<Constant>> constantPool;
		uint32 accessFlags;

		std::shared_ptr<const std::string> thisClass;
		std::shared_ptr<const std::string> superClass;
		std::vector<std::shared_ptr<const ConstantClassInfo>> interfaces;
		std::vector<std::shared_ptr<const Field>> fields;
		std::vector<std::shared_ptr<const Method>> methods;
		std::map<std::string, std::shared_ptr<const Attribute>> attributes;
	};
}



