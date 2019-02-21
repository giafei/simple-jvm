#pragma once

#include "jvm.h"

namespace jvm
{
	class ClassPath;
	class ClassLoader
	{
	public:
		ClassLoader(ClassPath *classPath);
		~ClassLoader();

	public:
		JVMClass* loadClass(const char* className);

	protected:
		JVMClass* defineClass(std::shared_ptr<const ClassFile::ClassFileData> classData);

	protected:
		ClassPath *classPath;
		std::map<std::string, JVMClass*> classes;
	};
}



