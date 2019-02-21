#include "stdafx.h"
#include "ClassPath.h"
#include "ClassReader.h"
#include "ClassFileData.h"

namespace jvm
{
	ClassPath::ClassPath()
	{
	}


	ClassPath::~ClassPath()
	{
	}

	void ClassPath::addFolder(const std::string & path)
	{
		readers.push_back(std::shared_ptr<ClassReader::ClassReader>(new ClassReader::DirClassReader(path)));
	}

	void ClassPath::addJarFile(const std::string & path)
	{
		readers.push_back(std::shared_ptr<ClassReader::ClassReader>(new ClassReader::ZipClassReader(path)));
	}

	std::shared_ptr<const ClassFile::ClassFileData> ClassPath::loadClass(const char * path)
	{
		auto it = readers.begin(), end = readers.end();

		for (; it != end; it++)
		{
			auto ptr = *it;
			auto data = ptr->loadClass(path);
			if (data)
				return std::shared_ptr<const ClassFile::ClassFileData>(new ClassFile::ClassFileData(*data));
		}

		return std::shared_ptr<const ClassFile::ClassFileData>();
	}
}
