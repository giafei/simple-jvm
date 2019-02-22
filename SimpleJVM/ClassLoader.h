#pragma once

#include "jvmBase.h"

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

	public:
		JVMObject* getJavaObject()
		{
			return javaObject;
		}

		void setJavaObject(JVMObject *p)
		{
			javaObject = p;
		}

	protected:
		JVMClass* defineArrayClass(const char* className);
		JVMClass* defineClass(std::shared_ptr<const ClassFile::ClassFileData> classData);

	protected:
		ClassPath *classPath;
		std::map<std::string, JVMClass*> classes;

		JVMObject *javaObject;
	};
}



