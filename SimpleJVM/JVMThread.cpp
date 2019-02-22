#include "stdafx.h"
#include "JVMThread.h"
#include "JVMClass.h"
#include "ClassFileData.h"
#include "ClassLoader.h"
#include "JVM.h"
#include "HeapMemory.h"
#include "DataBlock.h"
#include "JVMObject.h"
#include <boost/algorithm/string.hpp>
#include "NativeMethodHandler.h"

namespace jvm
{
	JVMThread *pThread = nullptr;

	StackFrame::StackFrame(Method * method)
	{
		this->method = method;
		pc = 0;
	}

	JVMThread::JVMThread(JVM *pJVM)
	{
		this->pJVM = pJVM;
		javaThread = nullptr;

		pThread = this;
		unsafeHackClass["java/lang/Class$Atomic"] = 1;
		unsafeHackClass["java/util/concurrent/atomic/AtomicInteger"] = 1;
		unsafeHackClass["java/util/concurrent/ConcurrentHashMap"] = 1;
		unsafeHackClass["java/io/File"] = 1;
	}

	JVMThread::~JVMThread()
	{
		pThread = nullptr;
		while (!stacks.empty())
		{
			popFrame();
		}
	}

	void UTF8StringToUTF16(const std::string & str, std::wstring& result)
	{
		int n = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
		result.resize(n + 1);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), (wchar_t*)result.c_str(), result.length() * 2);
		result.resize(n);
	}

	std::shared_ptr<JavaValue> JVMThread::execute(JVMObject *pThis, Method * method, const std::vector<SoltData>& arguments)
	{
		//占位置 接收返回值 避免破坏当前正在运行的栈帧
		StackFrame *f = pushFrame(nullptr);

		if (pThis != nullptr)
		{
			f->pushObject(pThis);
		}
	
		for (int i=0; i<(int)arguments.size(); i++)
		{
			f->pushInt(arguments[i].getIntValue());
		}

		pushFrame(method);

		execute();

		try
		{
			auto r = JavaValue::fromMethodDescriptor(*(method->getDescriptor()));

			f->popToJavaValue(r);
			popFrame();

			return r;
		}
		catch (const std::exception& e)
		{
			printf(e.what());
			throw e;
		}
	}

	void JVMThread::executeMain(JVMClass * pClass, const std::vector<std::string>& args)
	{
		auto method = pClass->getMethod("main::([Ljava/lang/String;)V");

		std::vector<SoltData> arguments;
		
		auto strArrClass = loadAndInit("[java/lang/String");
		auto strArr = pJVM->getHeap()->allocArray(strArrClass, args.size());

		for (int i=0; i<args.size(); i++)
		{
			*(JVMObject**)strArr->getAddress(i) = allocString(args[i]);
		}

		arguments.push_back(strArr);
		execute(nullptr, method, arguments);
	}

	JVMObject* JVMThread::allocString(const std::string & str)
	{
		std::wstring v;
		UTF8StringToUTF16(str, v);

		return allocString(v);
	}

	JVMObject* JVMThread::allocString(const std::wstring & str)
	{
		auto classLoader = pJVM->getClassLoader();

		auto arr = pJVM->getHeap()->allocArray(classLoader->loadClass("[C"), str.length()); //char[]
		if (str.length() > 0)
		{
			auto ptr = arr->getAddress(0);
			memcpy(ptr, str.c_str(), str.length() * 2);
		}

		auto stringClass = loadAndInit("java/lang/String");
		auto method = stringClass->getMethod("<init>::([C)V"); //调用 String类的构造函数

		auto strObj = pJVM->getHeap()->alloc(stringClass);

		std::vector<SoltData> arguments;
		arguments.push_back(arr);

		execute(strObj, method, arguments);

		return strObj;
	}

	void JVMThread::checkClassInited(JVMClass * pClass)
	{
		JVMClass *pSuper = pClass->getSuperClass();
		if (pSuper != nullptr)
		{
			checkClassInited(pSuper);
		}

		JVMClass *pArrEle = pClass->getArrayEleClass();
		if (pArrEle != nullptr)
		{
			checkClassInited(pArrEle);
		}

		if (pClass->isInited() || pClass->isStartedInit())
			return;

		pClass->beginInit();

		if (unsafeHackClass.find(*pClass->getName()) != unsafeHackClass.end())
		{
			pClass->finishInit();
		}
		else
		{
			auto method = pClass->getMethod("<clinit>::()V");
			if (method != nullptr)
			{
				execute(nullptr, method, std::vector<SoltData>());
			}

			pClass->finishInit();
		}
	}

	void JVMThread::initializeSystem()
	{
		loadAndInit("sun/misc/VM");

		JVMClass *classPtr = loadAndInit("java/lang/System");

		JVMClass *properties = loadAndInit("java/util/Properties");
		JVMObject *pro = pJVM->getHeap()->alloc(properties);
		Method *proInit = properties->getMethod("<init>::()V");
		execute(pro, proInit, std::vector<SoltData>());

		classPtr->getStaticField("props")->setObjectValue(pro);
		
		Method *sysInitPro = classPtr->getMethod("initProperties::(Ljava/util/Properties;)Ljava/util/Properties;");
		std::vector<SoltData> args;
		args.push_back(pro);

		execute(nullptr, sysInitPro, args);


// 		Method *method =classPtr->getMethod("initializeSystemClass::()V");
// 		execute(nullptr, method, std::vector<SoltData>());
	}

	JVMClass * JVMThread::loadAndInit(const char* className)
	{
		auto ptr = pJVM->getClassLoader()->loadClass(className);

		checkClassInited(ptr);

		return ptr;
	}

	JVMArray * JVMThread::allocArray(JVMClass * classPtr, std::vector<int>& arrLens, int lenPos)
	{
		bool multi = (int)arrLens.size() > (lenPos + 1);

		int arrLen = arrLens[lenPos];
		JVMArray *pArr = pJVM->getHeap()->allocArray(classPtr, arrLen);
		if (multi)
		{
			for (int i=0; i<arrLen; i++)
			{
				JVMArray** p = (JVMArray**)pArr->getAddress(i);
				*p = allocArray(classPtr->getArrayEleClass(), arrLens, lenPos + 1);
			}
		}

		return pArr;
	}

	JVMObject * JVMThread::getJavaThread()
	{
		if (javaThread == nullptr)
		{
			auto threadClass = loadAndInit("java/lang/Thread");
			javaThread = pJVM->getHeap()->alloc(threadClass);

			auto threadGroupClass = loadAndInit("java/lang/ThreadGroup");
			auto threadGroup = pJVM->getHeap()->alloc(threadGroupClass);

			auto method = threadGroupClass->getMethod("<init>::()V");
			execute(threadGroup, method, std::vector<SoltData>());

			javaThread->getField("group")->setObjectValue(threadGroup);
			javaThread->getField("priority")->setIntValue(9);
		}

		return javaThread;
	}

	void parseMethodDescriptor(std::vector<int>& arr, const char* decriptor)
	{
		arr.clear();
		const char* p = decriptor + 1;

		auto skipClass = [&p]()
		{
			while (*p++ != ';');
		};

		std::function<void()> skipArrayObj = [&]()
		{
			char c = *p++;
			if (c == 'L')
			{
				skipClass();
			}
			else if (c == '[')
			{
				skipArrayObj();
			}
		};

		for (char c = *p++; c != ')'; c = *p++)
		{
			if (c == 'L')
			{
				//对象
				arr.push_back(1);
				skipClass();
			}
			else if (c == '[')
			{
				//数组
				arr.push_back(1);
				skipArrayObj();
			}
			else if (c == 'J' || c == 'D')
			{
				arr.push_back(2);
			}
			else
			{
				arr.push_back(1);
			}
		}
	}

	int JVMThread::getArgSoltCount(std::shared_ptr<const std::string> methodDescriptor)
	{
		std::vector<int> arr;
		parseMethodDescriptor(arr, methodDescriptor->c_str());

		return std::accumulate(arr.begin(), arr.end(), 0);
	}

	void JVMThread::invokeNative(Method * method)
	{
		NativeMethodHandler::executeNative(pJVM, this, method);
	}

	StackFrame * JVMThread::pushFrame(Method * method)
	{
		StackFrame *f = new StackFrame(method);
		if (method != nullptr)
		{
			//参数
			auto descPtr = method->getDescriptor();

			std::vector<int> argSoltSizes;
			parseMethodDescriptor(argSoltSizes, descPtr->c_str());
			int soltCount = std::accumulate(argSoltSizes.begin(), argSoltSizes.end(), 0);

			if (method->isNative())
			{
				if (!method->isStatic())
				{
					soltCount += 1;
				}

				if (soltCount > 0)
				{
					f->allocLocal(soltCount);
				}
			}
			else
			{
				f->allocLocal(method->getCode()->getMaxLocals());
			}
			
			StackFrame *current = stacks.top();
			if (current != nullptr)
			{
				if (!argSoltSizes.empty())
				{
					int p = soltCount - 1;

					if (!method->isStatic())
					{
						p += 1;
					}

					for (int i = argSoltSizes.size() - 1; i >= 0; i--)
					{
						int l = argSoltSizes[i];
						if (l == 1)
						{
							f->setLocalInt(p--, current->popInt());
						}
						else
						{
							p--;
							f->setLocalLong(p--, current->popLong());
						}
					}
				}

				if (!method->isStatic())
				{
					f->setLocalObject(0, current->popObject());
				}
			}
		}

		stacks.push(f);

		return f;
	}

	void JVMThread::popFrame()
	{
		auto ptr = stacks.top();
		stacks.pop();

		delete ptr;
	}

	void JVMThread::popFrame(std::function<void(StackFrame*, StackFrame*)> callback)
	{
		auto ptr = stacks.top();
		stacks.pop();

		callback(ptr, stacks.top());

		delete ptr;
	}

	JVMThread * JVMThread::current()
	{
		return pThread;
	}
}

