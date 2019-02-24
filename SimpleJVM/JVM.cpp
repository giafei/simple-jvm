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
		classPath = new ClassPath();

		size_t v = 0;
		char buf[512] = { 0 };
		getenv_s(&v, buf, "JAVA_HOME");

		//strcat_s(buf, "\\jre\\lib\\rt.jar");
		//classPath->addJarFile(buf);          //太慢了
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
	}


	JVM::~JVM()
	{
		delete classLoader;
		delete heap;
		delete thread;
	}

	void JVM::run(int argc, char * argv[])
	{
		initJVM();

		std::vector<std::string> args;
		args.reserve(argc - 2);

		int i = 1;
		for (; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				if (strcmp(argv[i], "-classpath") == 0)
				{
					classPath->addFolder(argv[++i]);
				}
				else
				{
					++i;
				}
			}
			else
			{
				break;
			}
		}

		JVMClass *pClass = thread->loadAndInit(argv[i++]);
		for (; i<argc; i++)
		{
			args.push_back(argv[i]);
		}

		thread->executeMain(pClass, args);
	}

	void JVM::initJVM()
	{
		//定义基础类型
		classLoader->loadClass("byte");
		classLoader->loadClass("short");
		classLoader->loadClass("int");
		classLoader->loadClass("long");
		classLoader->loadClass("float");
		classLoader->loadClass("double");
		classLoader->loadClass("char");
		classLoader->loadClass("boolean");
		classLoader->loadClass("void");

		thread->initializeSystem();
	}
}