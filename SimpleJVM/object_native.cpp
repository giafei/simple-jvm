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
	namespace object
	{
		void objetHashCode(StackFrame* v1, StackFrame* v2)
		{
			v2->pushInt(v1->getLocalInt(0));
		}
		
		void cloneObject(JVM* vm, JVMThread* th, StackFrame* v1, StackFrame* v2)
		{
			auto oldObj = v1->getLocalObject(0);
			auto oldArr = dynamic_cast<JVMArray*>(oldObj);
			if (oldArr != nullptr)
			{
				auto newArr = vm->getHeap()->allocArray(oldObj->getClass(), oldArr->getLength());
				if (newArr->getLength() > 0)
				{
					memcpy(newArr->getAddress(0), oldArr->getAddress(0), oldArr->getLength() * oldArr->getElementSize());
				}

				v2->pushObject(newArr);
			}
			else
			{
				auto newObj = vm->getHeap()->alloc(oldObj->getClass());

				auto fields = oldObj->existsField();
				for (auto it = fields.begin(); it != fields.end(); it++)
				{
					newObj->getField(it->first) = it->second;
				}

				v2->pushObject(newObj);
			}
		}

		void __stdcall getObjectClass(JVM* vm, JVMThread* th, StackFrame* v1, StackFrame* v2)
		{
			v2->pushObject(v1->getLocalObject(0)->getClass()->getJavaClassObject());
		}

		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerEmptyNative("java/lang/Object", "registerNatives");
				NativeMethodHandler::registerNative("java/lang/Object", "hashCode", objetHashCode);

				NativeMethodHandler::registerNative("java/lang/Object", "clone", cloneObject);
				NativeMethodHandler::registerNative("java/lang/Object", "getClass", getObjectClass);

				NativeMethodHandler::registerEmptyNative("java/lang/Object", "notifyAll");
			}
		}_1;
	}
	
}