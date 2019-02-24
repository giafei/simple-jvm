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
	namespace jstring
	{
		void stringIntern(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			f2->pushObject(f1->getLocalObject(0));
		}

		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerNative("java/lang/String", "intern", stringIntern);
			}
		}_1;
	}
}
