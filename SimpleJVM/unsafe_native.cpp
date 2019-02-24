#include "stdafx.h"
#include "NativeMethodHandler.h"
#include "JVMClass.h"
#include "JVM.h"
#include "JVMThread.h"
#include "JVMObject.h"
#include "ClassLoader.h"
#include "HeapMemory.h"
#include <psapi.h>

using namespace jvm;

namespace native
{
	namespace unsafe
	{
		void getInt(StackFrame* f1, StackFrame* f2)
		{
			const char* descriptor = f1->getMethod()->getDescriptor()->c_str();

			if (descriptor[1] == 'J')
			{
				auto thisPtr = f1->getLocalObject(0);
				int64 i = f1->getLocalLong(1);

				char buf[128] = { 0 };
				sprintf_s(buf, "v_%d", (int)i);

				f2->pushInt(thisPtr->getNativeData<int>(std::string(buf)));
			}
			else
			{
				auto objPtr = f1->getLocalObject(1);
				int64 i = f1->getLocalLong(2);

				auto v = objPtr->getField((int)i)->getIntValue();
				f2->pushInt(v);
			}
		}

		void putInt(StackFrame* f1, StackFrame* f2)
		{
			const char* descriptor = f1->getMethod()->getDescriptor()->c_str();

			if (descriptor[1] == 'J')
			{
				auto thisPtr = f1->getLocalObject(0);
				int64 i = f1->getLocalLong(1);
				auto v = f1->getLocalInt(3);

				char buf[128] = { 0 };
				sprintf_s(buf, "v_%d", (int)i);
				thisPtr->setNativeData(buf, v);
			}
			else
			{
				auto objPtr = f1->getLocalObject(1);
				int64 i = f1->getLocalLong(2);
				auto v = f1->getLocalInt(4);

				objPtr->getField((int)i)->setIntValue(v);
			}
		}

		void getLong(StackFrame* f1, StackFrame* f2)
		{
			const char* descriptor = f1->getMethod()->getDescriptor()->c_str();

			if (descriptor[1] == 'J')
			{
				auto thisPtr = f1->getLocalObject(0);
				int64 i = f1->getLocalLong(1);

				char buf[128] = { 0 };
				sprintf_s(buf, "v_%d", (int)i);

				f2->pushLong(thisPtr->getNativeData<int64>(std::string(buf)));
			}
			else
			{
				auto objPtr = f1->getLocalObject(1);
				int64 i = f1->getLocalLong(2);

				auto v = objPtr->getField((int)i)->getLongValue();
				f2->pushLong(v);
			}
		}

		void putLong(StackFrame* f1, StackFrame* f2)
		{
			const char* descriptor = f1->getMethod()->getDescriptor()->c_str();

			if (descriptor[1] == 'J')
			{
				auto thisPtr = f1->getLocalObject(0);
				int64 i = f1->getLocalLong(1);
				auto v = f1->getLocalLong(3);

				char buf[128] = { 0 };
				sprintf_s(buf, "v_%d", (int)i);
				thisPtr->setNativeData(buf, v);
			}
			else
			{
				auto objPtr = f1->getLocalObject(1);
				int64 i = f1->getLocalLong(2);
				auto v = f1->getLocalLong(4);

				objPtr->getField((int)i)->setLongValue(v);
			}
		}

		void allocMemory(StackFrame* f1, StackFrame* f2)
		{
			int size = (int)f1->getLocalLong(1);
			f2->pushLong((int64)malloc(size));
		}

		void reallocateMemory(StackFrame* f1, StackFrame* f2)
		{
			void* p = (void*)f1->getLocalLong(1);
			int size = (int)f1->getLocalLong(3);
			f2->pushLong((int64)realloc(p, size));
		}

		void freeMemory(StackFrame* f1, StackFrame* f2)
		{
			void* p = (void*)f1->getLocalLong(1);
			free(p);
		}

		//´ý¶¨
		void setMemory(StackFrame* f1, StackFrame* f2)
		{
			int8 *p = (int8*)f1->getLocalLong(2);
			int offset = (int)f1->getLocalInt(4);
			p[offset] = (int8)f1->getLocalInt(6);
		}

		void arrayBaseOffset(StackFrame* f1, StackFrame* f2)
		{
			f2->pushInt(0);
		}

		void arrayIndexScale(StackFrame* f1, StackFrame* f2)
		{
			JAVAClassJVMObject *objPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(1));
			if (objPtr == nullptr)
			{
				f2->pushInt(0);
			}
			else
			{
				f2->pushInt(HeapMemory::arrayElementSize(objPtr->getGenericTypeClass()));
			}
		}

		void addressSize(StackFrame* f1, StackFrame* f2)
		{
			f2->pushInt(4);
		}

		void pageSize(StackFrame* f1, StackFrame* f2)
		{
			PERFORMANCE_INFORMATION info = { 0 };
			info.cb = sizeof(info);

			GetPerformanceInfo(&info, sizeof(info));

			f2->pushInt(info.PageSize);
		}

		void objectFieldOffset(StackFrame* f1, StackFrame* f2)
		{
			auto fieldClassObj = f1->getLocalObject(1);

			auto javaClassObj = dynamic_cast<JAVAClassJVMObject*>(fieldClassObj->getField("clazz")->getObjectValue());
			auto fieldNameStrObj = fieldClassObj->getField("name")->getObjectValue();
			JVMArray* charArr = dynamic_cast<JVMArray*>(fieldNameStrObj->getField("value")->getObjectValue());

			std::string str;
			UTF16StringToUTF8(str, charArr);

			int v = javaClassObj->getGenericTypeClass()->getFieldKey(str);
			f2->pushLong(v);
		}

		void compareAndSwapInt(StackFrame* f1, StackFrame* f2)
		{
			auto objPtr = f1->getLocalObject(1);
			int64 i = f1->getLocalLong(2);

			auto v1 = f1->getLocalInt(4);
			auto v2 = f1->getLocalInt(5);

			auto field = objPtr->getField((int)i);
			auto v = field->getIntValue();

			if (v1 == v)
			{
				field->setIntValue(v2);

				f2->pushInt(1);
			}
			else
			{
				f2->pushInt(2);
			}
		}

		void compareAndSwapLong(StackFrame* f1, StackFrame* f2)
		{
			auto objPtr = f1->getLocalObject(1);
			int64 i = f1->getLocalLong(2);

			auto v1 = f1->getLocalLong(4);
			auto v2 = f1->getLocalLong(5);

			auto field = objPtr->getField((int)i);
			auto v = field->getLongValue();

			if (v1 == v)
			{
				field->setLongValue(v2);

				f2->pushInt(1);
			}
			else
			{
				f2->pushInt(2);
			}
		}

		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerEmptyNative("sun/misc/Unsafe", "registerNatives");
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getInt", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putInt", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getObject", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putObject", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getBoolean", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putBoolean", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getByte", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putByte", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getShort", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putShort", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getChar", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putChar", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getLong", getLong);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putLong", putLong);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getFloat", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putFloat", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getDouble", getLong);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putDouble", putLong);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getAddress", getLong);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putAddress", putLong);

				NativeMethodHandler::registerNative("sun/misc/Unsafe", "allocateMemory", allocMemory);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "reallocateMemory", reallocateMemory);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "freeMemory", freeMemory);

				NativeMethodHandler::registerNative("sun/misc/Unsafe", "addressSize", addressSize);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "pageSize", pageSize);

				NativeMethodHandler::registerNative("sun/misc/Unsafe", "arrayBaseOffset", arrayBaseOffset);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "arrayIndexScale", arrayIndexScale);

				NativeMethodHandler::registerNative("sun/misc/Unsafe", "objectFieldOffset", objectFieldOffset);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "compareAndSwapObject", compareAndSwapInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "compareAndSwapInt", compareAndSwapInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "compareAndSwapLong", compareAndSwapLong);


				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getIntVolatile", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putIntVolatile", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putOrderedInt", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getObjectVolatile", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putObjectVolatile", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putOrderedObject", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getBooleanVolatile", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putBooleanVolatile", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getByteVolatile", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putByteVolatile", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getShortVolatile", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putShortVolatile", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getCharVolatile", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putCharVolatile", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getLongVolatile", getLong);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putLongVolatile", putLong);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putOrderedLong", putLong);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getFloatVolatile", getInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putFloatVolatile", putInt);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "getDoubleVolatile", getLong);
				NativeMethodHandler::registerNative("sun/misc/Unsafe", "putDoubleVolatile", putLong);

			}
		}_1;
	}

}