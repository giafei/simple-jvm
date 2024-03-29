#include "stdafx.h"
#include "JVMClass.h"
#include "ClassFileData.h"
#include "ClassLoader.h"
#include "JVMObject.h"
#include "JVMThread.h"
#include "HeapMemory.h"

namespace jvm
{
	Field::Field(std::shared_ptr<const ClassFile::Field> t, JVMClass * ownerClass)
		:accessFlag(t->getAccessFlag()), name(t->getName()), descriptor(t->getDescriptor()), attributes(t->getAttributes())
	{
		this->ownerClass = ownerClass;
	}

	Field::Field(JVMClass * ownerClass)
	{
		this->ownerClass = ownerClass;
	}

	Method::Method(std::shared_ptr<const ClassFile::Method> t, JVMClass * ownerClass)
		: accessFlag(t->getAccessFlag()), name(t->getName()), descriptor(t->getDescriptor()), attributes(t->getAttributes())
	{
		this->ownerClass = ownerClass;
	}

	std::shared_ptr<const ClassFile::CodeAttribute> Method::getCode()
	{
		return std::dynamic_pointer_cast<const ClassFile::CodeAttribute>(attributes["Code"]);
	}

	std::shared_ptr<const ClassFile::ExceptionAttribute> Method::getException()
	{
		return std::dynamic_pointer_cast<const ClassFile::ExceptionAttribute>(attributes["Exceptions"]);
	}

	JVMClass::JVMClass(const ClassFile::ClassFileData *t, ClassLoader* classLoader)
	{
		this->classLoader = classLoader;

		inited = false;
		initStarted = false;

		arrayEleClass = nullptr;

		int nameIndex = t->getConstant<ClassFile::ConstantClassInfo>(t->getThisClass())->getNameIndex();
		name = t->getConstantString(nameIndex);
		accessFlags = t->getAccessFlags();

		initConstantPool(t);

		auto super = t->getSuperClass();
		if (super)
		{
			int classIndex = t->getConstant<ClassFile::ConstantClassInfo>(super)->getNameIndex();
			superClass = classLoader->loadClass(t->getConstantString(classIndex)->c_str());
		}
		else
		{
			superClass = nullptr;
		}

		auto fileds = t->getFields();
		for (auto it = fileds.begin(); it != fileds.end(); it++)
		{
			auto field = *it;
			Field *p = new Field(field, this);

			addField(p); //先增加自己的，再增加父类的
		}

		if (superClass != nullptr)
		{
			std::map<int, Field *>& superFields = superClass->fields;
			for (auto it=superFields.begin(); it != superFields.end(); it++)
			{
				addField(it->second);
			}
		}

		auto methods = t->getMethods();
		for (auto it = methods.begin(); it != methods.end(); it++)
		{
			auto m = *it;
			Method *p = new Method(m, this);
			p->setSlot(nextMethodIndex++);
			this->methods[p->getKey()] = p;
		}

		auto arr = t->getInterfaces();
		interfaces.reserve(arr.size());
		for (auto it = arr.begin(); it != arr.end(); it++)
		{
			int nameIndex = t->getConstant<ClassFile::ConstantClassInfo>(*it)->getNameIndex();
			interfaces.push_back(classLoader->loadClass(t->getConstantString(nameIndex)->c_str()));
		}
	}

	int JVMClass::fieldNoPos = 0;

	JVMClass::JVMClass(ClassLoader * classLoader)
	{
		this->classLoader = classLoader;

		inited = false;
		initStarted = false;

		arrayEleClass = nullptr;
	}

	JVMClass::~JVMClass()
	{
		//TODO
	}

	void JVMClass::addField(Field * field)
	{
		const std::string& name = *(field->getName());
		if (fieldIndexs.find(name) != fieldIndexs.end())
			return;

		int i = nextFieldIndex++;
		fieldIndexs[name] = i;
		fields[i] = field;
	}

	Field * JVMClass::getField(const std::string & name)
	{
		auto it = fieldIndexs.find(name);
		if (it == fieldIndexs.end())
			return nullptr;

		int i = it->second;
		return fields[i];
	}

	JVMObject * JVMClass::getJavaClassObject()
	{
		if (javaClass == nullptr)
		{
			auto heap = HeapMemory::getHeap();

			javaClass = heap->allocClassObject(classLoader->loadClass("java/lang/Class"), this);

			std::string className = *name;
			char *p = (char*)className.c_str();

			while (*p)
			{
				if (*p == '/')
				{
					*p = '.';
				}
				p++;
			}

			auto classNameStr = JVMObjectCreator::allocString(className);
			javaClass->getField("name")->setObjectValue(classNameStr);
		}

		return javaClass;
	}

	void JVMClass::finishInit()
	{
		initStarted = false;
		inited = true;
	}

	void JVMClass::initConstantPool(const ClassFile::ClassFileData * t)
	{
		auto pool = t->getConstantPool();
		for (auto it=pool.begin(); it!=pool.end(); it++)
		{
			int k = it->first;
			if (this->constantPool.find(k) != this->constantPool.end())
				continue;

			auto ptr = it->second;


			switch (ptr->getType())
			{
			case ClassFile::CONSTANT_Utf8_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantUTF8String>(ptr);
				auto p = new ConstantUTF8String();
				p->setType(CONSTANT_Utf8_info);
				p->setData(o->getData());

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_Integer_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantInt>(ptr);
				auto p = new ConstantInt();
				p->setType(CONSTANT_Integer_info);
				p->setData(o->getData());

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_Float_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantFloat>(ptr);
				auto p = new ConstantFloat();
				p->setType(CONSTANT_Float_info);
				p->setData(o->getData());

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_Long_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantLong>(ptr);
				auto p = new ConstantLong();
				p->setType(CONSTANT_Long_info);
				p->setData(o->getData());

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_Double_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantDouble>(ptr);
				auto p = new ConstantDouble();
				p->setType(CONSTANT_Double_info);
				p->setData(o->getData());

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_Class_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantClassInfo>(ptr);
				auto p = new ConstantClassInfo();
				p->setType(CONSTANT_Class_info);
				p->setData(t->getConstantString(o->getNameIndex()));

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_String_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantString>(ptr);
				auto p = new ConstantString();
				p->setType(CONSTANT_String_info);
				p->setData(t->getConstantString(o->getValueIndex()));

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_Fieldref_info:
			case ClassFile::CONSTANT_Methodref_info:
			case ClassFile::CONSTANT_InterfaceMethodref_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantMemberRef>(ptr);
				auto p = new ConstantMemberRef();
				p->setType((ConstantType)ptr->getType());
				auto classNameIndex = t->getConstant<const ClassFile::ConstantClassInfo>(o->getClassIndex());

				p->setClassName(t->getConstantString(classNameIndex->getNameIndex()));

				int i = o->getNameAndTypeIndex();
				if (this->constantPool.find(i) == this->constantPool.end())
				{
					auto on = t->getConstant<ClassFile::ConstantNameAndType>(i);
					auto pn = new ConstantNameAndType();
					pn->setType(CONSTANT_NameAndType_info);
					pn->setName(t->getConstantString(on->getNameIndex()));
					pn->setDescriptor(t->getConstantString(on->getDescriptorIndex()));

					this->constantPool[i] = std::shared_ptr<Constant>(pn);
				}

				p->setNameAndType(this->getConstant<ConstantNameAndType>(i));
				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_NameAndType_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantNameAndType>(ptr);
				auto p = new ConstantNameAndType();
				p->setType(CONSTANT_NameAndType_info);
				p->setName(t->getConstantString(o->getNameIndex()));
				p->setDescriptor(t->getConstantString(o->getDescriptorIndex()));

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_MethodHandle_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantMethodHandleInfo>(ptr);
				auto p = new ConstantMethodHandleInfo();
				p->setType(CONSTANT_MethodHandle_info);
				p->setRefrenceIndex(o->getRefrenceIndex());
				p->setRefrenceKind(o->getRefrenceKind());

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_MethodType_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantMethodTypeInfo>(ptr);
				auto p = new ConstantMethodTypeInfo();
				p->setType(CONSTANT_MethodType_info);
				p->setDescriptor(t->getConstantString(o->getDescriptorIndex()));

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			case ClassFile::CONSTANT_InvokeDynamic_info:
			{
				auto o = std::dynamic_pointer_cast<const ClassFile::ConstantInvokeDynamicInfo>(ptr);
				auto p = new ConstantInvokeDynamicInfo();
				p->setType(CONSTANT_InvokeDynamic_info);
				p->setMethodAttrIndex(o->getMethodAttrIndex());
				p->setNameAndTypeIndex(o->getNameAndTypeIndex());

				this->constantPool[k] = std::shared_ptr<Constant>(p);
			}
			break;

			default:
				break;
			}
		}
	}
}
