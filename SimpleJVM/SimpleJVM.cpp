// SimpleJVM.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ClassReader.h"
#include "ClassFile.h"

int main()
{
	ClassReader::DirClassReader reader("F:\\WorkDir\\C++\\project\\SimpleJVM\\SimpleJVM\\java");
	auto data = reader.loadClass("test/Test");

	ClassFile::ClassFile file(*data);


	printf("%s\n", "Hello world!");
    return 0;
}

