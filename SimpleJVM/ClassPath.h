#pragma once

namespace ClassFile
{
	class ClassFileData;
}

namespace ClassReader
{
	class ClassReader;
}

namespace jvm
{
	class ClassPath
	{
	public:
		ClassPath();
		~ClassPath();

	public:
		void addFolder(const std::string& path);
		void addJarFile(const std::string& path);

	public:
		std::shared_ptr<const ClassFile::ClassFileData> loadClass(const char* path);

	private:
		std::vector<std::shared_ptr<ClassReader::ClassReader>> readers;
	};
}


