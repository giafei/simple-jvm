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
	namespace jsecurity
	{
		void doPrivileged(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			auto objPtr = f1->getLocalObject(0);

			auto method = objPtr->getClass()->getMethod("run::()Ljava/lang/Object;");
			auto rtnVal = th->execute(objPtr, method);
			if (!th->isExceptionNow())
			{
				f2->pushJavaValue(rtnVal);
			}
		}

		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerNative("java/security/AccessController", "doPrivileged", doPrivileged);
				NativeMethodHandler::registerReturnEmptyNative("java/security/AccessController", "getStackAccessControlContext");
			}
		}_1;
	}
}
