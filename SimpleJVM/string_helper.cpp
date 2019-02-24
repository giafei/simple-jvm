#include "stdafx.h"
#include "JVMObject.h"

void UTF16StringToUTF8(std::string& result, const std::wstring& src)
{
	int n = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.length(), NULL, 0, NULL, NULL);

	result.resize(n + 1);
	WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.length(), (char*)result.c_str(), n, NULL, NULL);
	result.resize(n);
}

void UTF16StringToUTF8(std::string& result, const wchar_t* src, int len)
{
	int n = WideCharToMultiByte(CP_UTF8, 0, src, len, NULL, 0, NULL, NULL);

	result.resize(n + 1);
	char* p = (char*)result.c_str();
	WideCharToMultiByte(CP_UTF8, 0, src, len, p, n, NULL, NULL);

	result.resize(n);
}

void UTF16StringToUTF8(std::string& result, jvm::JVMArray *charArr)
{
	const wchar_t* src = charArr->geElementAddress<wchar_t>(0);
	int len = charArr->getLength();

	UTF16StringToUTF8(result, src, len);
}

void UTF8StringToUTF16(std::wstring& result, const std::string & str)
{
	int n = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
	result.resize(n + 1);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), (wchar_t*)result.c_str(), result.length());
	result.resize(n);
}

void UTF8StringToUTF16(std::wstring& result, const char* str, int len)
{
	int n = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
	result.resize(n + 1);
	MultiByteToWideChar(CP_UTF8, 0, str, len, (wchar_t*)result.c_str(), result.length());
	result.resize(n);
}

void UTF8StringToUTF16(jvm::JVMArray *charArr, const char* str, int len)
{
	MultiByteToWideChar(CP_UTF8, 0, str, len, charArr->geElementAddress<wchar_t>(0), charArr->getLength());
}

void UTF8StringToUTF16(jvm::JVMArray *charArr, const std::string & str)
{
	UTF8StringToUTF16(charArr, str.c_str(), str.length());
}