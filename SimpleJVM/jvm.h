#pragma once

#include "jvmBase.h"

namespace jvm
{
	class JVM
	{
	public:
		JVM();
		~JVM();

	public:
		ClassLoader* getClassLoader()
		{
			return classLoader;
		}

		HeapMemory* getHeap()
		{
			return heap;
		}

		JVMThread* getThread()
		{
			return thread;
		}

	public:
		void run(int argc, char *argv[]);

	protected:
		ClassLoader *classLoader;
		HeapMemory *heap;
		JVMThread *thread;
	};
}



