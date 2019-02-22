#include "stdafx.h"
#include "JVM.h"
#include "ClassLoader.h"
#include "ClassPath.h"
#include "HeapMemory.h"
#include "JVMThread.h"

namespace jvm
{
	JVM::JVM()
	{
		ClassPath *classPath = new ClassPath();

		size_t v = 0;
		char buf[512] = { 0 };
		getenv_s(&v, buf, "JAVA_HOME");

		//strcat_s(buf, "\\jre\\lib\\rt.jar");
		//classPath->addJarFile(buf);          //Ì«ÂýÁË
		strcat_s(buf, "\\jre\\libFolder\\");
		
		char *p = buf + strlen(buf);

		strcpy_s(p, 10, "rt");
		classPath->addFolder(buf);

		strcpy_s(p, 10, "charsets");
		classPath->addFolder(buf);

		classPath->addFolder(".");

		classLoader = new ClassLoader(classPath);
		heap = new HeapMemory();
		thread = new JVMThread(this);

		auto c = classLoader->loadClass("java/lang/ClassLoader");
		classLoader->setJavaObject(heap->alloc(c));

		thread->loadAndInit("java/lang/Object");
		thread->loadAndInit("java/lang/ClassLoader");
		thread->loadAndInit("java/lang/Class");

		thread->initializeSystem();
	}


	JVM::~JVM()
	{
		delete classLoader;
		delete heap;
		delete thread;
	}

	void JVM::run(int argc, char * argv[])
	{
		std::vector<std::string> args;
		args.reserve(argc - 2);

		for (int i=2; i<argc; i++)
		{
			args.push_back(argv[i]);
		}

		JVMClass *pClass = thread->loadAndInit(argv[1]);
		thread->executeMain(pClass, args);
	}

}