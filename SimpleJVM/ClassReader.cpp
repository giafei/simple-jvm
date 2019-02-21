#include "stdafx.h"
#include "ClassReader.h"
#include "DataReader.h"

#include "unzip.h"

namespace ClassReader
{
	DirClassReader::DirClassReader(const std::string& path):dir(path)
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

	ZipClassReader::ZipClassReader(const std::string & path):file(path)
	{
	}

	ZipClassReader::~ZipClassReader()
	{
		
	}

	std::shared_ptr<DataReader> ZipClassReader::loadClass(const char * className)
	{
		unzFile zipFile = unzOpen64(file.c_str());

		if (zipFile == nullptr)
		{
			return std::shared_ptr<DataReader>();
		}

		unz_global_info64 gi;
		if (unzGetGlobalInfo64(zipFile, &gi) != UNZ_OK)
		{
			return std::shared_ptr<DataReader>();
		}

		char targetFileName[256];
		strcpy_s(targetFileName, className);
		strcat_s(targetFileName, ".class");

		for (int i=0; i<gi.number_entry; i++)
		{
			if (i)
			{
				if (unzGoToNextFile(zipFile) != UNZ_OK)
				{
					break;
				}
			}

			char filenameInzip[256];
			unz_file_info64 fileInfo;
			if (unzGetCurrentFileInfo64(zipFile, &fileInfo, filenameInzip, sizeof(filenameInzip), NULL, 0, NULL, 0) != UNZ_OK)
			{
				return std::shared_ptr<DataReader>();
			}

			if (strcmp(targetFileName, filenameInzip) == 0)
			{
				unzOpenCurrentFile(zipFile);

				uint8 *buff = new uint8[(int)fileInfo.uncompressed_size];
				int i = 0;

				while (i < fileInfo.uncompressed_size)
				{
					int c = unzReadCurrentFile(zipFile, buff + i, (int)fileInfo.uncompressed_size - i);
					if (c <= 0)
						break;

					i += c;
				}

				DataReader *result = new DataReader(buff, i);

				unzCloseCurrentFile(zipFile);
				unzClose(zipFile);

				return std::shared_ptr<DataReader>(result);
			}
		}


		unzClose(zipFile);
		return std::shared_ptr<DataReader>();
	}
}

