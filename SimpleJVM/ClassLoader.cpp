#include "stdafx.h"
#include "ClassLoader.h"
#include "JVMClass.h"
#include "ClassFileData.h"
#include "ClassPath.h"

namespace jvm
{
	ClassLoader::ClassLoader(ClassPath *classPath)
	{
		this->classPath = classPath;
	}


	ClassLoader::~ClassLoader()
	{
		for (auto it=classes.begin(); it!=classes.end(); it++)
		{
			auto ptr = it->second;
			delete ptr;
		}
	}

	JVMClass * ClassLoader::loadClass(const char * className)
	{
		if (*className == 0x00)
		{
			throw new std::exception("类路径不能为空");
		}

		std::string k(className);
		if ((k[0] == '[') && (k[1] == 'L'))
		{
			int i = k.length();
			if (k[i - 1] == ';')
			{
				k = "[" + k.substr(2, i - 3);
			}
		}

		JVMClass* p = classes[k];
		if (p == nullptr)
		{
			if (className[0] == '[')
			{
				p = defineArrayClass(k.c_str());
			}
			else
			{
				auto data = classPath->loadClass(k.c_str());
				if (data)
				{
					p = defineClass(data);
				}
			}

			if (p != nullptr)
			{
				classes[k] = p;
				classes[*(p->getName())] = p;
			}
			else
			{
				throw new std::exception("类加载失败");
			}
		}

		return p;
	}

	JVMClass * ClassLoader::defineArrayClass(const char * className)
	{
		JVMClass *p = new JVMClass(this);

		const char* q = className + 1;
		if (q[1] == 0x00)
		{
			//基础类型
			switch (*q)
			{
			case 'Z':
				p->setName(std::shared_ptr<const std::string>(new std::string("boolean[]")));
				break;

			case 'B':
				p->setName(std::shared_ptr<const std::string>(new std::string("byte[]")));
				break;

			case 'S':
				p->setName(std::shared_ptr<const std::string>(new std::string("short[]")));
				break;

			case 'C':
				p->setName(std::shared_ptr<const std::string>(new std::string("char[]")));
				break;

			case 'I':
				p->setName(std::shared_ptr<const std::string>(new std::string("int[]")));
				break;

			case 'J':
				p->setName(std::shared_ptr<const std::string>(new std::string("long[]")));
				break;

			case 'F':
				p->setName(std::shared_ptr<const std::string>(new std::string("float[]")));
				break;

			case 'D':
				p->setName(std::shared_ptr<const std::string>(new std::string("double[]")));
				break;

			default:
				break;
			}
		}
		else
		{
			int i = strlen(q);
			if ((q[i - 1] == ';') && q[0] == 'L')
			{
				std::string tmp(q + 1, strlen(q) - 2);

				auto wrapClass = this->loadClass(tmp.c_str());
				p->setArrayClass(wrapClass);

				p->setName(std::shared_ptr<const std::string>(new std::string(tmp + "[]")));
			}
			else
			{
				auto wrapClass = this->loadClass(q);
				p->setArrayClass(wrapClass);

				std::string tmp(*(wrapClass->getName()));
				p->setName(std::shared_ptr<const std::string>(new std::string(tmp + "[]")));
			}
		}

		p->setSuperClass(this->loadClass("java/lang/Object"));
		p->addInterface(this->loadClass("java/lang/Cloneable"));
		p->addInterface(this->loadClass("java/io/Serializable"));
		p->setAccessFlag(ClassAccess::ACC_PUBLIC);

		Field *field = new Field(p);
		field->setAccessFlag(FieldAccess::ACC_PUBLIC);
		field->setName(std::shared_ptr<const std::string>(new std::string("length")));
		field->setDescriptor(std::shared_ptr<const std::string>(new std::string("I")));

		p->setField(*field->getName(), field);
		p->finishInit();

		return p;
	}

	JVMClass * ClassLoader::defineClass(std::shared_ptr<const ClassFile::ClassFileData> classData)
	{
		return new JVMClass(classData.get(), this);
	}

}