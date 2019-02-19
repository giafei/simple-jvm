#include "stdafx.h"
#include "ClassReader.h"
#include "DataReader.h"

namespace ClassReader
{
	DirClassReader::DirClassReader(std::string path):dir(path)
	{
		if (dir.at(dir.length() - 1) != '/')
		{
			dir.append("\\");
		}
	}

	DirClassReader::~DirClassReader()
	{
	}


	void string_replace(std::string &str, const char* old0, const char* new0)
	{
		std::string::size_type nPos = 0;
		std::string::size_type nsrclen = strlen(old0);
		std::string::size_type ndstlen = strlen(new0);
		while (nPos = str.find(old0, nPos))
		{
			if (nPos == std::string::npos) break;
			str.replace(nPos, nsrclen, new0);
			nPos += ndstlen;
		}
	}

	std::shared_ptr<DataReader> DirClassReader::loadClass(const char * className)
	{
		std::string filePath = dir + className + ".class";
		string_replace(filePath, "/", "\\");

		FILE *fp = nullptr;
		fopen_s(&fp, filePath.c_str(), "rb");

		if (fp == nullptr)
		{
			return std::shared_ptr<DataReader>();
		}

		fseek(fp, 0, SEEK_END);
		long len = ftell(fp);
		uint8 *buff = new uint8[len];
		fseek(fp, 0, SEEK_SET);
		fread(buff, 1, len, fp);
		fclose(fp);

		return std::shared_ptr<DataReader>(new DataReader(buff, len));
	}
}

