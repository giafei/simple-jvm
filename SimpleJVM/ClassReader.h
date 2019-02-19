#pragma once

class DataReader;
namespace ClassReader
{
	class DirClassReader
	{
	public:
		DirClassReader(std::string dir);
		~DirClassReader();

		std::shared_ptr<DataReader> loadClass(const char* className);

	private:
		std::string dir;
	};
}


