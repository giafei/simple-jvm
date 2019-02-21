#pragma once

class DataReader;
namespace ClassReader
{
	class ClassReader
	{
	public:
		virtual ~ClassReader()
		{
		}

		virtual std::shared_ptr<DataReader> loadClass(const char* className) = 0;
	};

	class DirClassReader : public ClassReader
	{
	public:
		DirClassReader(const std::string& dir);
		~DirClassReader();

		std::shared_ptr<DataReader> loadClass(const char* className);

	private:
		std::string dir;
	};

	class ZipClassReader : public ClassReader
	{
	public:
		ZipClassReader(const std::string& file);
		~ZipClassReader();

		std::shared_ptr<DataReader> loadClass(const char* className);

	private:
		std::string file;
	};
}


