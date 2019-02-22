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
	void UTF16StringToUTF8(std::string& result, const std::wstring& src)
	{
		int n = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.length(), NULL, 0, NULL, NULL);

		result.resize(n + 1);
		WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.length(), (char*)result.c_str(), n, NULL, NULL);
		result.resize(n);
	}

	void UTF16StringToUTF8(std::string& result, const wchar_t* src, int len)
	{
		int n = WideCharToMultiByte(CP_UTF8, 0, src, len, NULL, 0, NULL, NULL);

		result.resize(n + 1);
		char* p = (char*)result.c_str();
		WideCharToMultiByte(CP_UTF8, 0, src, len, p, n, NULL, NULL);

		for(; *p; p++)
		{
			if (*p == '.')
			{
				*p = '/';
			}
		}


		result.resize(n);
	}

	void UTF16StringToUTF8(std::string& result, JVMArray *charArr)
	{
		const wchar_t* src = (wchar_t*)charArr->getAddress(0);
		int len = charArr->getLength();

		UTF16StringToUTF8(result, src, len);
	}

	typedef void(__stdcall *NativeFun)(JVM* vm, JVMThread* thread);

	void __stdcall emptyFun(JVM* vm, JVMThread* thread)
	{
		thread->popFrame();
	}

	void __stdcall returnNull(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([](StackFrame* v1, StackFrame* v2)
		{
			v2->pushObject(nullptr);
		});
	}

	void __stdcall doPrivileged(JVM* vm, JVMThread* thread)
	{
		auto objPtr = thread->currentFrame()->getLocalObject(0);
		auto method = objPtr->getClass()->getMethod("run::()Ljava/lang/Object;");
		auto rtnVal = thread->execute(objPtr, method, std::vector<SoltData>());

		thread->popFrame();
		thread->currentFrame()->pushJavaValue(rtnVal);
	}

	void __stdcall FileDescriptorSet(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([](StackFrame* v1, StackFrame* v2)
		{
			int v = v1->getLocalInt(0);
			if (v == 0) //stdin
			{
				v2->pushLong((int)stdin);
			}
			else if (v == 1) //stdout
			{
				v2->pushLong((int)stdout);
			}
			else if (v == 2) //stdout
			{
				v2->pushLong((int)stderr);
			}
			else
			{
				v2->pushLong(v);
			}
		});
	}

	void __stdcall getCallerClass(JVM* vm, JVMThread* thread)
	{
		thread->popFrame();

		auto frame = thread->currentFrame();
		frame->pushObject(frame->getMethod()->getOwnerClass()->getJavaClass());
	}

	void __stdcall getCurrentThread(JVM* vm, JVMThread* thread)
	{
		thread->popFrame();

		auto frame = thread->currentFrame();
		frame->pushObject(thread->getJavaThread());
	}

	void __stdcall getClassName0(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([&](StackFrame* v1, StackFrame* v2)
		{
			auto classObjPtr = v1->getLocalObject(0);

			auto str = classObjPtr->getField("name")->getObjectValue();

			v2->pushObject(str);
		});
	}

	void __stdcall loadClass0(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([&](StackFrame* v1, StackFrame* v2)
		{
			auto strPtr = v1->getLocalObject(0);
			auto charArray = dynamic_cast<JVMArray*>(strPtr->getField("value")->getObjectValue());

			std::string className;
			UTF16StringToUTF8(className, charArray);

			JVMObject* resultPtr = nullptr;
			if (v1->getLocalInt(1) == 0)
			{
				resultPtr = vm->getClassLoader()->loadClass(className.c_str())->getJavaClass();
			}
			else
			{
				resultPtr = thread->loadAndInit(className.c_str())->getJavaClass();
			}

			v2->pushObject(resultPtr);
		});
	}

	void __stdcall arrayBaseOffset(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([](StackFrame* v1, StackFrame* v2)
		{
			v2->pushInt(8);
		});
	}

	void __stdcall arrayIndexScale(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([](StackFrame* v1, StackFrame* v2)
		{
			v2->pushInt(4);
		});
	}

	void __stdcall objetHashCode(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([](StackFrame* v1, StackFrame* v2)
		{
			v2->pushInt(v1->getLocalInt(0));
		});
	}

	void __stdcall initIDs(JVM* vm, JVMThread* thread)
	{
		thread->popFrame();
	}

	void __stdcall desiredAssertionStatus0(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([](StackFrame* v1, StackFrame* v2)
		{
			v2->pushInt(0);
		});
	}

	void __stdcall availableProcessors(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([](StackFrame* v1, StackFrame* v2)
		{
			v2->pushInt(1);
		});
	}

	void __stdcall classIsInterface(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([&](StackFrame* v1, StackFrame* v2)
		{
			auto strObj = v1->getLocalObject(0)->getField("name")->getObjectValue();
			auto charArr = dynamic_cast<JVMArray*>(strObj->getField("value")->getObjectValue());

			std::string className;
			UTF16StringToUTF8(className, charArr);

			auto classPtr = vm->getClassLoader()->loadClass(className.c_str());
			if (classPtr->isInterface())
			{
				v2->pushInt(1);
			}
			else
			{
				v2->pushInt(0);
			}
		});
	}

	void __stdcall classIsArray(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([&](StackFrame* v1, StackFrame* v2)
		{
			auto strObj = v1->getLocalObject(0)->getField("name")->getObjectValue();
			if (strObj == nullptr)
			{
				v2->pushInt(0);
			}
			else
			{
				auto charArr = dynamic_cast<JVMArray*>(strObj->getField("value")->getObjectValue());

				wchar_t *p = (wchar_t*)charArr->getAddress(charArr->getLength() - 1);
				if (*p == ']')
				{
					v2->pushInt(1);
				}
				else
				{
					v2->pushInt(0);
				}
			}
		});
	}

	void __stdcall classGetComponentType(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([&](StackFrame* v1, StackFrame* v2)
		{
			auto strObj = v1->getLocalObject(0)->getField("name")->getObjectValue();
			auto charArr = dynamic_cast<JVMArray*>(strObj->getField("value")->getObjectValue());

			std::string className;
			UTF16StringToUTF8(className, charArr);

			auto classPtr = vm->getClassLoader()->loadClass(className.c_str());
			auto eleClassPtr = classPtr->getArrayEleClass();

			if (eleClassPtr != nullptr)
			{
				v2->pushObject(eleClassPtr->getJavaClass());
			}
			else
			{
				v2->pushObject(nullptr);
			}

		});
	}

	void __stdcall arraycopy(JVM* vm, JVMThread* thread)
	{
		StackFrame* pFrame = thread->currentFrame();

		int len = pFrame->getLocalInt(4);
		if (len > 0)
		{
			JVMArray *src = dynamic_cast<JVMArray*>(pFrame->getLocalObject(0));
			int srcPos = pFrame->getLocalInt(1);

			JVMArray *dest = dynamic_cast<JVMArray*>(pFrame->getLocalObject(2));
			int destPos = pFrame->getLocalInt(3);

			memcpy(dest->getAddress(destPos), src->getAddress(srcPos), len * src->getElementSize());
		}


		thread->popFrame();
	}

	void __stdcall getPrimitiveClass(JVM* vm, JVMThread* thread)
	{
 		StackFrame* pFrame = thread->currentFrame();
 		JVMObject *pStrObj = pFrame->getLocalObject(0);

		auto classPtr = vm->getClassLoader()->loadClass("java/lang/Class");
		auto objPtr = vm->getHeap()->alloc(classPtr);
		objPtr->getField("name")->setObjectValue(pStrObj);
		objPtr->getField("classLoader")->setObjectValue(vm->getClassLoader()->getJavaObject());

		thread->popFrame();
		thread->currentFrame()->pushObject(objPtr);
	}

	void __stdcall classIsPrimitive(JVM* vm, JVMThread* thread)
	{
		StackFrame* pFrame = thread->currentFrame();
		JVMObject *objPtr = pFrame->getLocalObject(0);
		thread->popFrame();
		
		if (objPtr->getField("classLoader")->getObjectValue() == nullptr)
		{
			thread->currentFrame()->pushInt(0);
		}
		else
		{
			thread->currentFrame()->pushInt(1);
		}
	}

	void __stdcall floatToRawIntBits(JVM* vm, JVMThread* thread)
	{
		StackFrame* pFrame = thread->currentFrame();

		float value = pFrame->getLocalFloat(0);

		thread->popFrame();
		thread->currentFrame()->pushInt(*(int*)&value);
	}

	void __stdcall intBitsToFloat(JVM* vm, JVMThread* thread)
	{
		StackFrame* pFrame = thread->currentFrame();

		int value = pFrame->getLocalInt(0);

		thread->popFrame();
		thread->currentFrame()->pushFloat(*(float*)&value);
	}

	void __stdcall doubleToRawLongBits(JVM* vm, JVMThread* thread)
	{
		StackFrame* pFrame = thread->currentFrame();

		double value = pFrame->getLocalDouble(0);

		thread->popFrame();
		thread->currentFrame()->pushLong(*(int64*)&value);
	}

	void __stdcall longBitsToDouble(JVM* vm, JVMThread* thread)
	{
		StackFrame* pFrame = thread->currentFrame();

		int64 value = pFrame->getLocalLong(0);

		thread->popFrame();
		thread->currentFrame()->pushDouble(*(double*)&value);
	}

	void __stdcall stringIntern(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([&](StackFrame* v1, StackFrame* v2)
		{
			v2->pushObject(v1->getLocalObject(0));
		});
	}

	void __stdcall getObjectClass(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([&](StackFrame* v1, StackFrame* v2)
		{
			v2->pushObject(v1->getLocalObject(0)->getClass()->getJavaClass());
		});
	}
	
	void __stdcall cloneObject(JVM* vm, JVMThread* thread)
	{
		thread->popFrame([&](StackFrame* v1, StackFrame* v2)
		{
			auto oldObj = v1->getLocalObject(0);
			auto oldArr = dynamic_cast<JVMArray*>(oldObj);
			if (oldArr != nullptr)
			{
				auto newArr = vm->getHeap()->allocArray(oldObj->getClass(), oldArr->getLength());
				if (newArr->getAddress(0) != nullptr)
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

		});
	}

	void setProperty(JVMThread* thread, JVMObject *props, const std::wstring& k, const std::wstring& v)
	{
		auto method = props->getClass()->getMethod("setProperty::(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;");

		std::vector<SoltData> arguments;
		arguments.reserve(2);
		arguments.push_back(thread->allocString(k));
		arguments.push_back(thread->allocString(v));

		thread->execute(props, method, arguments);
	}

	void __stdcall initProperties(JVM* vm, JVMThread* thread)
	{
		StackFrame* pFrame = thread->currentFrame();
		JVMObject *props = pFrame->getLocalObject(0);

		size_t v = 0;
		wchar_t buf[512];

		setProperty(thread, props, L"java.version", L"0.0.1");
		setProperty(thread, props, L"java.vendor", L"giafei jvm");
		setProperty(thread, props, L"java.vendor.url", L"http://giafei.net");

		_wgetenv_s(&v, buf, L"JAVA_HOME");
		setProperty(thread, props, L"java.home", buf);

		setProperty(thread, props, L"java.class.version", L"52.0");
		setProperty(thread, props, L"java.class.path", _wgetcwd(buf, 512)); //
		setProperty(thread, props, L"os.name", L"Windows 10");
		setProperty(thread, props, L"os.arch", L"amd64");
		setProperty(thread, props, L"os.version", L"10.0");
		setProperty(thread, props, L"file.separator", L"\\");
		setProperty(thread, props, L"path.separator", L";");
		setProperty(thread, props, L"line.separator", L"\r\n");
		setProperty(thread, props, L"user.name", L"giafei");
		setProperty(thread, props, L"user.home", L"f:\\tmp\\home");
		setProperty(thread, props, L"user.dir", buf);
		setProperty(thread, props, L"sun.stdout.encoding", L"GBK");


		thread->popFrame();
		thread->currentFrame()->pushObject(props);
	}

	class _ //注册native function
	{
	public:
		_()
		{
			_nativeFuns["java/lang/Object::registerNatives"] = emptyFun;
			_nativeFuns["java/lang/Class::registerNatives"] = emptyFun;
			_nativeFuns["java/lang/ClassLoader::registerNatives"] = emptyFun;
			_nativeFuns["java/lang/System::registerNatives"] = emptyFun;
			_nativeFuns["sun/misc/VM::initialize"] = emptyFun;
			_nativeFuns["java/lang/Class::desiredAssertionStatus0"] = desiredAssertionStatus0;
			_nativeFuns["java/lang/System::arraycopy"] = arraycopy;
			_nativeFuns["java/lang/Class::getPrimitiveClass"] = getPrimitiveClass;
			_nativeFuns["java/lang/Float::floatToRawIntBits"] = floatToRawIntBits;
			_nativeFuns["java/lang/Float::intBitsToFloat"] = intBitsToFloat;
			_nativeFuns["java/lang/Double::doubleToRawLongBits"] = doubleToRawLongBits;
			_nativeFuns["java/lang/Double::longBitsToDouble"] = longBitsToDouble;
			_nativeFuns["java/lang/System::initProperties"] = initProperties;
			_nativeFuns["java/io/FileInputStream::initIDs"] = initIDs;
			_nativeFuns["java/io/FileDescriptor::initIDs"] = initIDs;
			_nativeFuns["java/io/FileOutputStream::initIDs"] = initIDs;
			_nativeFuns["sun/misc/Unsafe::registerNatives"] = emptyFun;
			_nativeFuns["java/lang/Object::hashCode"] = objetHashCode;
			_nativeFuns["sun/misc/Unsafe::arrayBaseOffset"] = arrayBaseOffset;
			_nativeFuns["sun/misc/Unsafe::arrayIndexScale"] = arrayIndexScale;
			_nativeFuns["sun/misc/Unsafe::addressSize"] = arrayIndexScale;
			_nativeFuns["sun/reflect/Reflection::getCallerClass"] = getCallerClass;
			_nativeFuns["java/io/FileDescriptor::set"] = FileDescriptorSet;
			_nativeFuns["java/security/AccessController::doPrivileged"] = doPrivileged;
			_nativeFuns["java/lang/Thread::registerNatives"] = emptyFun;
			_nativeFuns["java/lang/Thread::currentThread"] = getCurrentThread;
			_nativeFuns["java/lang/Class::getName0"] = getClassName0;
			_nativeFuns["java/lang/Class::forName0"] = loadClass0;
			_nativeFuns["java/security/AccessController::getStackAccessControlContext"] = returnNull;
			_nativeFuns["java/lang/Thread::setPriority0"] = emptyFun;
			_nativeFuns["java/lang/Thread::isAlive"] = returnNull;
			_nativeFuns["java/lang/Thread::start0"] = emptyFun;
			_nativeFuns["java/lang/Class::getDeclaredFields0"] = returnNull;
			_nativeFuns["java/lang/System::setIn0"] = emptyFun;
			_nativeFuns["java/lang/System::setOut0"] = emptyFun;
			_nativeFuns["java/lang/System::setErr0"] = emptyFun;
			_nativeFuns["java/lang/Class::isInterface"] = classIsInterface;
			_nativeFuns["java/lang/Runtime::availableProcessors"] = availableProcessors;
			_nativeFuns["java/lang/Class::isArray"] = classIsArray;
			_nativeFuns["java/lang/Class::getComponentType"] = classGetComponentType;
			_nativeFuns["java/lang/Class::isPrimitive"] = classIsPrimitive;
			_nativeFuns["java/lang/String::intern"] = stringIntern;
			_nativeFuns["java/lang/Object::clone"] = cloneObject;
			_nativeFuns["java/lang/Object::getClass"] = getObjectClass;
			_nativeFuns["java/lang/Class::getEnclosingMethod0"] = returnNull;
			_nativeFuns["java/lang/Class::getDeclaringClass0"] = returnNull;
			_nativeFuns["java/io/WinNTFileSystem::initIDs"] = emptyFun;
		}

	public:
		void invokeNative(JVM* vm, JVMThread* thread, Method* method)
		{
			auto classPtr = method->getOwnerClass();
			std::string k = (*(classPtr->getName()) + "::" + *(method->getName()));

			auto it = _nativeFuns.find(k);
			if (it == _nativeFuns.end())
			{
				throw new std::exception("不支持的native method");
			}
			else
			{
				NativeFun fun = it->second;
				fun(vm, thread);
			}
		}

	private:
		std::map<std::string, NativeFun> _nativeFuns;
	}_r;

	void NativeMethodHandler::executeNative(JVM* vm, JVMThread* thread, Method* method)
	{
		_r.invokeNative(vm, thread, method);
	}
}

