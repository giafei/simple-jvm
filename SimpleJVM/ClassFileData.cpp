#include "stdafx.h"
#include "ClassFileData.h"
#include "DataReader.h"

namespace ClassFile
{
	ClassFileData::ClassFileData(DataReader& reader)
	{
		uint32 magic = reader.readUint32();
		if (magic != 0xCAFEBABE)
			throw new std::exception("给定的数据不是java类数据");

		minorVersion = reader.readUint16();
		majorVersion = reader.readUint16();

		int poolCount = reader.readUint16();
		for (int i=1; i<poolCount; i++)
		{
			ConstantType tag = (ConstantType)reader.readUint8();
			this->constantPool[i] = readConstant(reader, tag);


			if (tag == CONSTANT_Long_info || tag == CONSTANT_Double_info)
			{
				i++;
			}
		}

		this->accessFlags = reader.readUint16();
		this->thisClass = getConstantString(reader.readUint16());
		this->superClass = getConstantString(reader.readUint16());

		int interfaceCount = reader.readUint16();
		this->interfaces.reserve(interfaceCount);
		for (int i=0; i<interfaceCount; i++)
		{
			std::shared_ptr<const Constant> v = constantPool[reader.readUint16()];
			this->interfaces.push_back(std::dynamic_pointer_cast<const ConstantClassInfo>(v));
		}

		int fieldsCount = reader.readUint16();
		this->fields.reserve(fieldsCount);
		for (int i=0; i<fieldsCount; i++)
		{
			this->fields.push_back(readField(reader));
		}

		int methodCount = reader.readUint16();
		this->methods.reserve(methodCount);
		for (int i=0; i<methodCount; i++)
		{
			this->methods.push_back(readMethod(reader));
		}

		int attrbuteCount = reader.readUint16();
		for (int i=0; i<attrbuteCount; i++)
		{
			auto attr = readAttribute(reader);
			attributes[*(attr->getName())] = attr;
		}
	}

	ClassFileData::~ClassFileData()
	{
	}

	std::shared_ptr<Constant> ClassFileData::readConstant(DataReader& reader, ConstantType tag)
	{
		switch (tag)
		{
		case CONSTANT_Utf8_info:
		{
			ConstantUTF8String *p = new ConstantUTF8String();
			p->setType(tag);
			
			auto data = reader.readPart(reader.readUint16());
			
			//MUTF-8格式 直接按UTF-8处理
			p->setData(std::make_shared<std::string>(std::string((const char*)data->getData(), data->getLength())));

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_Integer_info:
		{
			ConstantInt *p = new ConstantInt();
			p->setType(tag);
			p->setData(reader.readUint32());

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_Float_info:
		{
			ConstantFloat *p = new ConstantFloat();
			p->setType(tag);

			auto tmp = reader.readUint32();
			p->setData(*(float*)&tmp);

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_Long_info:
		{
			ConstantLong *p = new ConstantLong();
			p->setType(tag);

			uint64 tmp1 = reader.readUint32();
			uint64 tmp2 = reader.readUint32();
			p->setData((tmp1 << 32) | tmp2);

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_Double_info:
		{
			ConstantDouble *p = new ConstantDouble();
			p->setType(tag);

			uint64 tmp1 = reader.readUint32();
			uint64 tmp2 = reader.readUint32();
			uint64 v = (tmp1 << 32) | tmp2;
			p->setData(*(double*)&v);

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_Class_info:
		{
			ConstantClassInfo *p = new ConstantClassInfo();
			p->setType(tag);
			p->setNameIndex(reader.readUint16());

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_String_info:
		{
			ConstantString *p = new ConstantString();
			p->setType(tag);
			p->setValueIndex(reader.readUint16());

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_Fieldref_info:
		case CONSTANT_Methodref_info:
		case CONSTANT_InterfaceMethodref_info:
		{
			ConstantMemberRef *p = new ConstantMemberRef();
			p->setType(tag);

			p->setClassIndex(reader.readUint16());
			p->setNameAndTypeIndex(reader.readUint16());

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_NameAndType_info:
		{
			ConstantNameAndType *p = new ConstantNameAndType();
			p->setType(tag);

			p->setNameIndex(reader.readUint16());
			p->setDescriptorIndex(reader.readUint16());

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_MethodHandle_info:
		{
			ConstantMethodHandleInfo *p = new ConstantMethodHandleInfo();
			p->setType(tag);

			p->setRefrenceIndex(reader.readUint8());
			p->setRefrenceIndex(reader.readUint16());

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_MethodType_info:
		{
			ConstantMethodTypeInfo *p = new ConstantMethodTypeInfo();
			p->setType(tag);

			p->setDescriptorIndex(reader.readUint16());

			return std::shared_ptr<Constant>(p);
		}

		case CONSTANT_InvokeDynamic_info:
		{
			ConstantInvokeDynamicInfo *p = new ConstantInvokeDynamicInfo();
			p->setType(tag);

			p->setMethodAttrIndex(reader.readUint16());
			p->setNameAndTypeIndex(reader.readUint16());

			return std::shared_ptr<Constant>(p);
		}


		default:
			throw new std::exception("不支持的常量类型");
			break;
		}

		return std::make_shared<Constant>();
	}

	std::shared_ptr<const std::string> ClassFileData::getConstantString(int i) const
	{
		auto v = this->getConstant<ConstantUTF8String>(i);
		if (!v)
			return std::make_shared<const std::string>();

		return v->getData();
	}

	std::shared_ptr<const Field> ClassFileData::readField(DataReader& reader)
	{
		Field *field = new Field();
		field->setAccessFlag(reader.readUint16());
		field->setName(getConstantString(reader.readUint16()));
		field->setDescriptor(getConstantString(reader.readUint16()));

		int attrbuteCount = reader.readUint16();
		for (int i = 0; i < attrbuteCount; i++)
		{
			auto attr = readAttribute(reader);
			field->addAttribute(*(attr->getName()), attr);
		}

		return std::shared_ptr<const Field>(field);
	}

	std::shared_ptr<const Method> ClassFileData::readMethod(DataReader& reader)
	{
		Method *method = new Method();
		method->setAccessFlag(reader.readUint16());
		method->setName(getConstantString(reader.readUint16()));
		method->setDescriptor(getConstantString(reader.readUint16()));

		int attrbuteCount = reader.readUint16();
		for (int i = 0; i < attrbuteCount; i++)
		{
			auto attr = readAttribute(reader);
			method->addAttribute(*(attr->getName()), attr);
		}

		return std::shared_ptr<const Method>(method);
	}

	std::shared_ptr<const Attribute> ClassFileData::readAttribute(DataReader& reader)
	{
		int i = reader.readUint16();
		auto name = getConstantString(i);

		if (name->compare("Code") == 0)
		{
			CodeAttribute *p = new CodeAttribute();
			p->setName(name);

			reader.readUint32(); //不重要
			p->setMaxStack(reader.readUint16());
			p->setMaxLocals(reader.readUint16());

			int codeLen = reader.readInt32();
			p->setCode(reader.readPart(codeLen));

			int exCount = reader.readUint16();
			for (int i=0; i<exCount; i++)
			{
				p->addException(reader.readUint64());
			}

			int attrCount = reader.readUint16();
			for (int i=0; i<attrCount; i++)
			{
				auto attr = readAttribute(reader);
				p->addAttribute(*(attr->getName()), attr);
			}

			return std::shared_ptr<const Attribute>(p);
		}
		else if (name->compare("SourceFile") == 0)
		{
			StringAttribute *p = new StringAttribute();
			p->setName(name);

			reader.readUint32(); //固定是2
			p->setData(getConstantString(reader.readUint16()));

			return std::shared_ptr<const Attribute>(p);
		}
		else if (name->compare("ConstantValue") == 0)
		{
			ConstantValueAttribute *p = new ConstantValueAttribute();
			p->setName(name);

			reader.readUint32(); //固定是2
			p->setData(constantPool[reader.readUint16()]);

			return std::shared_ptr<const Attribute>(p);
		}
		else if (name->compare("Exceptions") == 0)
		{
			ExceptioneAttribute *p = new ExceptioneAttribute();
			p->setName(name);

			reader.readUint32(); //不重要
			int exCount = reader.readUint16();
			for (int i=0; i<exCount; i++)
			{
				p->addException(
					std::dynamic_pointer_cast<const ConstantString>(constantPool[reader.readUint16()])
				);
			}

			return std::shared_ptr<const Attribute>(p);
		}
		else
		{
			UnsupportAttribute *p = new UnsupportAttribute();
			p->setName(name);

			uint32 len = reader.readUint32();
			p->setData(reader.readPart((int)len));
			return std::shared_ptr<const Attribute>(p);
		}
	}
}
