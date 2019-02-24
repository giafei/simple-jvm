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
	namespace number
	{
		void floatToRawIntBits(JVM* vm, JVMThread* th)
		{
			th->popFrame([th](StackFrame* f1, StackFrame* f2)
			{
				f2->pushInt(f1->getLocalInt(0));
			});
		}

		void intBitsToFloat(JVM* vm, JVMThread* th)
		{
			th->popFrame([th](StackFrame* f1, StackFrame* f2)
			{
				f2->pushInt(f1->getLocalInt(0));
			});
		}

		void doubleToRawLongBits(JVM* vm, JVMThread* th)
		{
			th->popFrame([th](StackFrame* f1, StackFrame* f2)
			{
				f2->pushLong(f1->getLocalLong(0));
			});
		}

		void longBitsToDouble(JVM* vm, JVMThread* th)
		{
			th->popFrame([th](StackFrame* f1, StackFrame* f2)
			{
				f2->pushLong(f1->getLocalLong(0));
			});
		}

		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerNative("java/lang/Float", "floatToRawIntBits", floatToRawIntBits);
				NativeMethodHandler::registerNative("java/lang/Float", "intBitsToFloat", intBitsToFloat);
				NativeMethodHandler::registerNative("java/lang/Double", "doubleToRawLongBits", doubleToRawLongBits);
				NativeMethodHandler::registerNative("java/lang/Double", "longBitsToDouble", longBitsToDouble);
			}
		}_1;
	}
}
