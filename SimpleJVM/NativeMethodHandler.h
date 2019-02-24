#pragma once

#include "jvmBase.h"

namespace jvm
{
	class NativeMethodHandler
	{
	public:
		static void registerNative(const std::string& className, const std::string& method, std::function<void(JVM*, JVMThread*)> fun);
		static void executeNative(JVM* vm, JVMThread* thread, Method* method);

	public:
		static void registerEmptyNative(const std::string& className, const std::string& method)
		{
			registerNative(className, method, emptyFun);
		}

		static void registerReturnEmptyNative(const std::string& className, const std::string& method)
		{
			registerNative(className, method, returnEmpty);
		}

	public:
		static void registerNative(const std::string& className, const std::string& method, std::function<void(StackFrame*, StackFrame*)> fun);
		static void registerNative(const std::string& className, const std::string& method, std::function<void(JVM*, JVMThread*, StackFrame*, StackFrame*)> fun);

	protected:
		static void emptyFun(JVM* vm, JVMThread* th);
		static void returnEmpty(JVM* vm, JVMThread* th);
	};
}


