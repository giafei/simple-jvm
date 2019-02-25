#include "stdafx.h"
#include "JVMThread.h"
#include "JVMClass.h"
#include "ClassFileData.h"
#include "ClassLoader.h"
#include "JVM.h"
#include "HeapMemory.h"
#include "DataBlock.h"
#include "JVMObject.h"
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
	}

	JVMThread::~JVMThread()
	{
		pThread = nullptr;
		while (!stacks.empty())
		{
			popFrame();
		}
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
		if (isThreadEnd())
		{
			return std::shared_ptr<JavaValue>();
		}

		try
		{
			auto r = JavaValue::fromMethodDescriptor(*(method->getDescriptor()));

			if (r->getSoltSize() == 0)
			{
				r->setSoltSize(1);
				r->setObjectValue(nullptr);
			}
			else
			{
				f->popToJavaValue(r);
			}
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
		
		auto strArrClass = loadAndInit("[Ljava/lang/String;");
		auto strArr = pJVM->getHeap()->allocArray(strArrClass, args.size());

		for (int i=0; i<(int)args.size(); i++)
		{
			*(JVMObject**)strArr->getAddress(i) = JVMObjectCreator::allocString(args[i]);
		}

		arguments.push_back(strArr);
		execute(nullptr, method, arguments);
	}

	void JVMThread::checkClassInited(JVMClass * pClass)
	{
		if (isThreadEnd())
			return;

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

// 		if (pClass->getName()->compare("java/lang/reflect/Constructor") == 0)
// 		{
// 			printf("1\n");
// 		}
		
		auto method = pClass->getDeclaredMethod("<clinit>::()V");
		if (method != nullptr)
		{
			execute(nullptr, method);
		}

		pClass->finishInit();
	}

	void JVMThread::initializeSystem()
	{
		loadAndInit("sun/misc/VM");
		if (isThreadEnd())
			return;

 		JVMClass *classPtr = loadAndInit("java/lang/System");

// 		JVMClass *properties = loadAndInit("java/util/Properties");
// 		JVMObject *pro = pJVM->getHeap()->alloc(properties);
// 		Method *proInit = properties->getMethod("<init>::()V");
// 		execute(pro, proInit);
// 
// 		classPtr->getStaticField("props")->setObjectValue(pro);
// 		
// 		Method *sysInitPro = classPtr->getMethod("initProperties::(Ljava/util/Properties;)Ljava/util/Properties;");
// 		std::vector<SoltData> args;
// 		args.push_back(pro);
// 
// 		execute(nullptr, sysInitPro, args);

 		Method *method = classPtr->getMethod("initializeSystemClass::()V");
 		execute(nullptr, method);
	}

	JVMClass * JVMThread::justLoad(const char * className)
	{
		return pJVM->getClassLoader()->loadClass(className);
	}

	JVMClass * JVMThread::loadAndInit(const char* className)
	{
		auto ptr = pJVM->getClassLoader()->loadClass(className);

		checkClassInited(ptr);

		return ptr;
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
			execute(threadGroup, method);

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

			if (!method->isStatic())
			{
				soltCount += 1;
			}

			if (method->isNative())
			{
				if (soltCount > 0)
				{
					f->allocLocal(soltCount);
				}
			}
			else
			{
				f->allocLocal(method->getCode()->getMaxLocals());
			}
			
			StackFrame *current = currentFrame();
			if (current != nullptr)
			{
				if (!argSoltSizes.empty())
				{
					int p = soltCount - 1;
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

		stacks.push_back(f);

		return f;
	}

	void JVMThread::popFrame()
	{
		auto ptr = currentFrame();
		stacks.pop_back();

		delete ptr;
	}

	void JVMThread::popFrame(std::function<void(StackFrame*, StackFrame*)> callback)
	{
		auto ptr = currentFrame();
		stacks.pop_back();

		callback(ptr, currentFrame());

		delete ptr;
	}

	JVMThread * JVMThread::current()
	{
		return pThread;
	}

	JVMObject* JVMObjectCreator::allocString(const std::string & str)
	{
		HeapMemory *heap = HeapMemory::getHeap();
		auto strObj = heap->getString(str.c_str());
		if (strObj != nullptr)
			return strObj;

		std::wstring wstr;
		if (str.length() > 0)
		{
			UTF8StringToUTF16(wstr, str);
		}

		strObj = allocString(wstr);
		heap->addString(str.c_str(), strObj);

		return strObj;
	}

	JVMObject* JVMObjectCreator::allocString(const std::wstring & str)
	{
		auto arr = HeapMemory::getHeap()->allocArray(JVMThread::current()->loadAndInit("[C"), str.length()); //char[]

		if (str.length() > 0)
		{
			auto ptr = arr->getAddress(0);
			memcpy(ptr, str.c_str(), str.length() * 2);
		}

		return allocString(arr);
	}

	JVMObject * JVMObjectCreator::allocString(JVMArray * charArr)
	{
		JVMThread *thread = JVMThread::current();

		auto stringClass = thread->loadAndInit("java/lang/String");
		auto method = stringClass->getMethod("<init>::([C)V");

		auto strObj = HeapMemory::getHeap()->alloc(stringClass);

		std::vector<SoltData> arguments;
		arguments.push_back(charArr);

		thread->execute(strObj, method, arguments);

		return strObj;
	}

	JVMArray * JVMObjectCreator::allocArray(JVMClass * classPtr, std::vector<int>& arrLens, int lenPos)
	{
		bool multi = (int)arrLens.size() > (lenPos + 1);

		int arrLen = arrLens[lenPos];
		JVMArray *pArr = HeapMemory::getHeap()->allocArray(classPtr, arrLen);
		if (multi)
		{
			for (int i = 0; i < arrLen; i++)
			{
				JVMArray** p = (JVMArray**)pArr->getAddress(i);
				*p = allocArray(classPtr->getArrayEleClass(), arrLens, lenPos + 1);
			}
		}

		return pArr;
	}
}

