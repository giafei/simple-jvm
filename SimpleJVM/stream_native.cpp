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
	namespace jstream
	{
		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerEmptyNative("java/io/FileInputStream", "initIDs");
				NativeMethodHandler::registerEmptyNative("java/io/FileOutputStream", "initIDs");
			}
		}_1;
	}

}