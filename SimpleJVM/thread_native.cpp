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
	namespace jthread
	{
		void __stdcall getCurrentThread(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			f2->pushObject(th->getJavaThread());
		}

		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerEmptyNative("java/lang/Thread", "registerNatives");
				NativeMethodHandler::registerNative("java/lang/Thread", "currentThread", getCurrentThread);

				NativeMethodHandler::registerEmptyNative("java/lang/Thread", "setPriority0");
				NativeMethodHandler::registerReturnEmptyNative("java/lang/Thread", "isAlive");
				NativeMethodHandler::registerEmptyNative("java/lang/Thread", "start0");
			}
		}_1;
	}
}
