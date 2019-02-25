// SimpleJVM.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "JVM.h"

int main(int argc, char *argv[])
{
	setlocale(LC_CTYPE, "");

	jvm::JVM * pJVM = new jvm::JVM();;
	pJVM->run(argc, argv);

    return 0;
}

