#pragma once

#include "jvm.h"

namespace jvm
{
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
		virtual ~Constant() {}

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
	};

	class ConstantClassInfo : public ConstantUTF8String
	{
	};

	class ConstantNameAndType : public Constant
	{
	public:
		void setName(std::shared_ptr<const std::string> v)
		{
			this->name = v;
		}

		std::shared_ptr<const std::string> getName()
		{
			return name;
		}

		void setDescriptor(std::shared_ptr<const std::string> v)
		{
			this->descriptor = v;
		}

		std::shared_ptr<const std::string> getDescriptor()
		{
			return descriptor;
		}

	protected:
		std::shared_ptr<const std::string> name;
		std::shared_ptr<const std::string> descriptor;
	};

	class ConstantMemberRef : public Constant
	{
	public:
		void setClassName(std::shared_ptr<const std::string> v)
		{
			this->className = v;
		}

		std::shared_ptr<const std::string> getClassName() const
		{
			return className;
		}

		void setNameAndType(std::shared_ptr<const ConstantNameAndType> v)
		{
			this->nameAndType = v;
		}

		std::shared_ptr<const ConstantNameAndType> getNameAndType() const
		{
			return nameAndType;
		}

	protected:
		std::shared_ptr<const std::string> className;
		std::shared_ptr<const ConstantNameAndType> nameAndType;
	};

	class ConstantMethodHandleInfo : public Constant
	{
	public:
		void setRefrenceKind(int kind)
		{
			refrenceKind = kind;
		}

		int getRefrenceKind() const
		{
			return refrenceKind;
		}

		void setRefrenceIndex(int i)
		{
			refrenceIndex = i;
		}

		int getRefrenceIndex() const
		{
			return refrenceIndex;
		}

	protected:
		int refrenceKind = 0;
		int refrenceIndex = 0;
	};

	class ConstantMethodTypeInfo : public Constant
	{
	public:
		void setDescriptor(std::shared_ptr<const std::string> v)
		{
			this->descriptor = v;
		}

		std::shared_ptr<const std::string> getDescriptor() const
		{
			return descriptor;
		}

	protected:
		std::shared_ptr<const std::string> descriptor = 0;
	};

	class ConstantInvokeDynamicInfo : public Constant
	{
	public:
		void setMethodAttrIndex(int i)
		{
			methodAttrIndex = i;
		}

		int getMethodAttrIndex() const
		{
			return methodAttrIndex;
		}

		void setNameAndTypeIndex(int i)
		{
			nameAndTypeIndex = i;
		}

		int getNameAndTypeIndex() const
		{
			return nameAndTypeIndex;
		}

	protected:
		int methodAttrIndex = 0;
		int nameAndTypeIndex = 0;
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
		Field(std::shared_ptr<const ClassFile::Field> t, JVMClass* ownerClass);

		virtual ~Field() {}
	public:
		bool isStatic()
		{
			return (accessFlag & FieldAccess::ACC_STATIC) != 0;
		}

		bool isFinal()
		{
			return (accessFlag & FieldAccess::ACC_FINAL) != 0;
		}

		std::string getKey();

		JVMClass* getOwnerClass()
		{
			return ownerClass;
		}

	public:
		uint32 getAccessFlag() const
		{
			return accessFlag;
		}

		std::shared_ptr<const std::string> getName() const
		{
			return name;
		}

		/*
		1）类型描述符。
		①基本类型byte、short、char、int、long、float和double的描述符，是单个字母，分别对应B、S、C、I、J、F和D。注意，long的描述符是J而不是L。
		②引用类型的描述符是L＋类的完全限定名＋分号。
		③数组类型的描述符是[＋数组元素类型描述符。
		*/
		std::shared_ptr<const std::string> getDescriptor() const
		{
			return descriptor;
		}

		const std::map<const std::string, std::shared_ptr<const ClassFile::Attribute>> getAttributes() const
		{
			return attributes;
		}

		std::shared_ptr<const ClassFile::Attribute> getAttribute(const std::string& k) const
		{
			auto it = attributes.find(k);
			if (it != attributes.end())
				return it->second;

			return std::shared_ptr<const ClassFile::Attribute>();
		}

	private:
		uint32 accessFlag;
		std::shared_ptr<const std::string> name;
		std::shared_ptr<const std::string> descriptor;
		std::map<const std::string, std::shared_ptr<const ClassFile::Attribute>> attributes;

		JVMClass* ownerClass;
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
		Method(std::shared_ptr<const ClassFile::Method> t, JVMClass* ownerClass);
		virtual ~Method() {}

	public:
		bool isStatic()
		{
			return (accessFlag & MethodAccess::ACC_STATIC) != 0;
		}

		std::string getKey()
		{
			return *name + "::" + *descriptor;
		}

		JVMClass* getOwnerClass()
		{
			return ownerClass;
		}

		std::shared_ptr<const ClassFile::CodeAttribute> getCode()
		{
			return std::dynamic_pointer_cast<const ClassFile::CodeAttribute>(attributes["code"]);
		}

	public:
		uint32 getAccessFlag() const
		{
			return accessFlag;
		}

		std::shared_ptr<const std::string> getName() const
		{
			return name;
		}

		/*
		接Field
		方法描述符是（分号分隔的参数类型描述符）+返回值类型描述符，其中void返回值由单个字母V表示。
		*/
		std::shared_ptr<const std::string> getDescriptor() const
		{
			return descriptor;
		}

		const std::map<const std::string, std::shared_ptr<const ClassFile::Attribute>> getAttributes() const
		{
			return attributes;
		}

	private:
		uint32 accessFlag;
		std::shared_ptr<const std::string> name;
		std::shared_ptr<const std::string> descriptor;
		std::map<const std::string, std::shared_ptr<const ClassFile::Attribute>> attributes;

		JVMClass* ownerClass;
	};

	namespace ClassAccess
	{
		enum ClassAccess
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

	class JVMClass
	{
	public:
		JVMClass(const ClassFile::ClassFileData *t);
		~JVMClass();

	public:
		std::shared_ptr<const std::string> getName()
		{
			return name;
		}

		bool isInterface()
		{
			return (accessFlags & ClassAccess::ACC_INTERFACE) != 0;
		}

	public:
		void setField(const std::string& k, Field* field)
		{
			fields[k] = field;
		}

		Field* getField(const std::string& k)
		{
			return fields[k];
		}

		void setMethod(const std::string& k, Method* field)
		{
			methods[k] = field;
		}

		Method* getMethod(const std::string& k)
		{
			return methods[k];
		}

		void setSuperClass(JVMClass *p)
		{
			superClass = p;
		}

		void addInterface(JVMClass *p)
		{
			interfaces.push_back(p);
		}

	public:
		std::shared_ptr<const std::string> getConstString(int i)
		{
			return getConstant<ConstantUTF8String>(i)->getData();
		}

		int32 getConstInt(int i)
		{
			return getConstant<ConstantInt>(i)->getData();
		}

		float getConstFloat(int i)
		{
			return getConstant<ConstantFloat>(i)->getData();
		}

		int64 getConstantLong(int i)
		{
			return getConstant<ConstantLong>(i)->getData();
		}

		double getConstantDouble(int i)
		{
			return getConstant<ConstantDouble>(i)->getData();
		}

	public:
		bool isInited()
		{
			return inited;
		}

		void beginInit()
		{
			initStarted = true;
		}

		void finishInit()
		{
			initStarted = false;
			inited = true;
		}

	protected:
		template <typename T>
		std::shared_ptr<const T> getConstant(int i) const
		{
			if (i == 0)
				return std::make_shared<const T>();

			std::shared_ptr<Constant> ptr = this->constantPool.at(i);
			return std::dynamic_pointer_cast<const T>(ptr);
		}

		void initConstantPool(const ClassFile::ClassFileData *t);

	protected:
		std::shared_ptr<const std::string> name;

		JVMClass* superClass;
		std::vector<JVMClass*> interfaces;

		uint32 accessFlags;
		std::map<std::string, SoltData> staticField;
		std::map<int, std::shared_ptr<Constant>> constantPool;

		std::map<std::string, Field *> fields;
		std::map<std::string, Method *> methods;

	protected:
		bool inited;
		bool initStarted;

		int classFieldsCount;
	};
}



