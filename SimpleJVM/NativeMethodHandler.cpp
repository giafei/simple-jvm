#include "stdafx.h"
#include "NativeMethodHandler.h"
#include "JVMClass.h"
#include "JVM.h"
#include "JVMThread.h"
#include "JVMObject.h"
#include "ClassLoader.h"
#include "HeapMemory.h"


namespace jvm
{
	static class _
	{
	public:
		void invokeNative(JVM* vm, JVMThread* thread, Method* method)
		{
			std::map<std::string, std::function<void(JVM*, JVMThread*)>>& funs = *_nativeFuns;

			auto classPtr = method->getOwnerClass();
			std::string k = (*(classPtr->getName()) + "::" + *(method->getName()));

			auto it = funs.find(k);
			if (it == funs.end())
			{
				throw new std::exception("不支持的native method");
			}
			else
			{
				std::function<void(JVM*, JVMThread*)> fun = it->second;
				fun(vm, thread);
			}
		}

		void registerNative(const std::string & className, const std::string & method, std::function<void(JVM*, JVMThread*)> fun)
		{
			if (_nativeFuns == nullptr)
			{
				_nativeFuns = new std::map<std::string, std::function<void(JVM *, JVMThread *)>>();
			}

			std::string k = className + "::" + method;

			std::map<std::string, std::function<void(JVM*, JVMThread*)>>& funs = *_nativeFuns;
			funs[k] = fun;
		}

	private:
		std::map<std::string, std::function<void(JVM*, JVMThread*)>> *_nativeFuns;
	}_r;

	void NativeMethodHandler::registerNative(const std::string & className, const std::string & method, std::function<void(JVM*, JVMThread*)> fun)
	{
		_r.registerNative(className, method, fun);
	}

	void NativeMethodHandler::executeNative(JVM* vm, JVMThread* thread, Method* method)
	{
		_r.invokeNative(vm, thread, method);
	}

	void callAsPopFrame1(std::function<void(StackFrame*, StackFrame*)> fun, JVM * vm, JVMThread * th)
	{
		th->popFrame(fun);
	}

	void NativeMethodHandler::registerNative(const std::string & className, const std::string & method, std::function<void(StackFrame*, StackFrame*)> fun)
	{
 		std::function<void(JVM*, JVMThread*)> v = std::bind<void>(callAsPopFrame1, fun, std::placeholders::_1, std::placeholders::_2);
 		registerNative(className, method, v);
	}

	void callAsPopFrame2(std::function<void(JVM*, JVMThread*, StackFrame*, StackFrame*)> fun, JVM * vm, JVMThread * th)
	{
		th->popFrame(std::bind(fun, vm, th, std::placeholders::_1, std::placeholders::_2));
	}

	void NativeMethodHandler::registerNative(const std::string & className, const std::string & method, std::function<void(JVM*, JVMThread*, StackFrame*, StackFrame*)> fun)
	{
		std::function<void(JVM*, JVMThread*)> v = std::bind(callAsPopFrame2, fun, std::placeholders::_1, std::placeholders::_2);
		registerNative(className, method, v);
	}

	void NativeMethodHandler::emptyFun(JVM * vm, JVMThread * th)
	{
		th->popFrame();
	}

	void NativeMethodHandler::returnEmpty(JVM * vm, JVMThread * th)
	{
		th->popFrame();
		th->currentFrame()->pushInt(0);
	}
}

