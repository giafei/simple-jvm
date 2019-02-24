// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once
#define _SCL_SECURE_NO_WARNINGS

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


#include <vector>
#include <stack>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <numeric>
#include <iostream>

#include <windows.h>

#include "types.h"

namespace jvm
{
	class JVMArray;
}

void UTF16StringToUTF8(std::string& result, const std::wstring& src);
void UTF16StringToUTF8(std::string& result, const wchar_t* src, int len);
void UTF16StringToUTF8(std::string& result, jvm::JVMArray *charArr);

void UTF8StringToUTF16(std::wstring& result, const std::string & str);
void UTF8StringToUTF16(std::wstring& result, const char* str, int len);
void UTF8StringToUTF16(jvm::JVMArray *charArr, const char* str, int len);
void UTF8StringToUTF16(jvm::JVMArray *charArr, const std::string & str);
