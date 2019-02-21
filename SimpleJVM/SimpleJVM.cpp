// SimpleJVM.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ClassReader.h"
#include "ClassFileData.h"

int main(int argc, char *argv[])
{
	ClassReader::ZipClassReader reader("D:\\ProgramFiles\\Java\\jdk1.8.0_121\\jre\\lib\\rt.jar");
	auto data = reader.loadClass("java/lang/Object");

	ClassFile::ClassFileData file(*data);


	printf("%s\n", "Hello world!");
    return 0;
}

