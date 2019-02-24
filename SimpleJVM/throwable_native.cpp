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
	namespace throwable
	{
		void fillInStackTrace(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			auto exPtr = f1->getLocalObject(0);
			int dummy = f1->getLocalInt(1);

			auto arrPtr = th->fillStackTrace(exPtr, dummy, true);
			exPtr->setNativeData("stackTrace", (int64)arrPtr);

			f2->pushObject(exPtr);
		}

		void getStackTraceDepth(StackFrame* f1, StackFrame* f2)
		{
			auto exPtr = f1->getLocalObject(0);
			JVMArray *arrPtr = dynamic_cast<JVMArray*>(exPtr->getNativeData<JVMObject*>("stackTrace"));

			f2->pushInt(arrPtr->getLength());
		}

		void getStackTraceElement(StackFrame* f1, StackFrame* f2)
		{
			auto exPtr = f1->getLocalObject(0);
			JVMArray *arrPtr = dynamic_cast<JVMArray*>(exPtr->getNativeData<JVMObject*>("stackTrace"));

			int i = f1->getLocalInt(1);
			f2->pushObject(*(arrPtr->geElementAddress<JVMObject*>(i)));
		}

		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerNative("java/lang/Throwable", "fillInStackTrace", fillInStackTrace);
				NativeMethodHandler::registerNative("java/lang/Throwable", "getStackTraceDepth", getStackTraceDepth);
				NativeMethodHandler::registerNative("java/lang/Throwable", "getStackTraceElement", getStackTraceElement);
			}
		}_1;
	}
}