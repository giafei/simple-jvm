// SimpleJVM.cpp : �������̨Ӧ�ó������ڵ㡣
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

