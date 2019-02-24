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
	namespace svm
	{
		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerEmptyNative("sun/misc/VM", "initialize");
			}
		}_1;
	}

}