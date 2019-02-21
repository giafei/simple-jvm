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
		std::string k(className);

		JVMClass* p = classes[k];
		if (p == nullptr)
		{
			auto data = classPath->loadClass(className);
			if (data)
			{
				p = defineClass(data);
			}

			if (p != nullptr)
			{
				classes[k] = p;
			}
		}

		return p;
	}

	JVMClass * ClassLoader::defineClass(std::shared_ptr<const ClassFile::ClassFileData> classData)
	{
		JVMClass *p = new JVMClass(classData.get());

		auto super = classData->getSuperClass();
		if (super)
		{
			p->setSuperClass(loadClass(super->c_str()));
		}
		else
		{
			p->setSuperClass(nullptr);
		}

		auto arr = classData->getInterfaces();
		for (auto it=arr.begin(); it!=arr.end(); it++)
		{
			auto name = classData->getConstantString((*it)->getNameIndex());
			p->addInterface(loadClass(name->c_str()));
		}

		return p;
	}

}