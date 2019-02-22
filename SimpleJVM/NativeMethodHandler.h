#pragma once

#include "jvmBase.h"

namespace jvm
{
	class NativeMethodHandler
	{
	public:
		static void executeNative(JVM* vm, JVMThread* thread, Method* method);
	};
}


