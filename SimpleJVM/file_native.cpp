#include "stdafx.h"
#include "NativeMethodHandler.h"
#include "JVMClass.h"
#include "JVM.h"
#include "JVMThread.h"
#include "JVMObject.h"
#include "ClassLoader.h"
#include "HeapMemory.h"

using namespace jvm;

namespace native
{
	namespace jfile
	{
		void FileDescriptorSet(StackFrame* v1, StackFrame* v2)
		{
			int v = v1->getLocalInt(0);
			if (v == 0) //stdin
			{
				v2->pushLong((int)stdin);
			}
			else if (v == 1) //stdout
			{
				v2->pushLong((int)stdout);
			}
			else if (v == 2) //stdout
			{
				v2->pushLong((int)stderr);
			}
			else
			{
				v2->pushLong(v);
			}
		}


		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerEmptyNative("java/io/WinNTFileSystem", "initIDs");
				NativeMethodHandler::registerEmptyNative("java/io/FileDescriptor", "initIDs");
				NativeMethodHandler::registerNative("java/io/FileDescriptor", "set", FileDescriptorSet);
			}
		}_1;
	}

}