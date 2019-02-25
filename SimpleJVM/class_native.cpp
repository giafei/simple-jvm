#include "stdafx.h"
#include "NativeMethodHandler.h"
#include "JVMClass.h"
#include "JVM.h"
#include "JVMThread.h"
#include "JVMObject.h"
#include "ClassLoader.h"
#include "HeapMemory.h"
#include "ClassFileData.h"
#include <boost/algorithm/string.hpp>

namespace jvm
{
	bool objInstanceOf(JVMObject* objPtr, JVMClass* classPtr);
}

using namespace jvm;

namespace native
{
	namespace jclass
	{
		void desiredAssertionStatus0(StackFrame* v1, StackFrame* v2)
		{
			//不支持断言
			v2->pushInt(0);
		}

		void getPrimitiveClass(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			JVMObject *pStrObj = f1->getLocalObject(0);
			JVMArray *pCharArr = dynamic_cast<JVMArray*>(pStrObj->getField("value")->getObjectValue());

			std::string typeName;
			UTF16StringToUTF8(typeName, pCharArr);

			f2->pushObject(vm->getClassLoader()->loadClass(typeName.c_str())->getJavaClassObject());
		}

		void classIsPrimitive(StackFrame* v1, StackFrame* v2)
		{
			JVMObject *objPtr = v1->getLocalObject(0);
			if (objPtr->getClass()->isPrimitive())
			{
				v2->pushInt(1);
			}
			else
			{
				v2->pushInt(0);
			}
		}

		void getCallerClass(JVM* vm, JVMThread* th)
		{
			int i = 0;
			StackFrame* f1 = th->currentFrame();
			const char* descriptor = f1->getMethod()->getDescriptor()->c_str();
			if (descriptor[1] == ')')
			{
				i = 2;
			}
			else
			{
				i = f1->getLocalInt(0);
			}

			StackFrame* t = th->getFrame(i);
			while (t->getMethod() == nullptr)
			{
				t = th->getFrame(++i);
			}

			th->popFrame();
			th->currentFrame()->pushObject(t->getMethod()->getOwnerClass()->getJavaClassObject());
		}

		void getClassName0(StackFrame* f1, StackFrame* f2)
		{
			auto classObjPtr = f1->getLocalObject(0);
			auto str = classObjPtr->getField("name")->getObjectValue();

			f2->pushObject(str);
		}

		void toLoadableClass(std::string& className, JVMObject* strPtr)
		{
			auto charArray = dynamic_cast<JVMArray*>(strPtr->getField("value")->getObjectValue());

			UTF16StringToUTF8(className, charArray);
			boost::replace_all(className, ".", "/");
		}

		void loadClass0(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			std::string className;
			toLoadableClass(className, f1->getLocalObject(0));
			
			JVMObject* resultPtr = nullptr;
			if (f1->getLocalInt(1) == 0)
			{
				resultPtr = vm->getClassLoader()->loadClass(className.c_str())->getJavaClassObject();
			}
			else
			{
				resultPtr = th->loadAndInit(className.c_str())->getJavaClassObject();
			}

			f2->pushObject(resultPtr);
		}

		void __stdcall classGetComponentType(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			std::string className;
			auto strObj = f1->getLocalObject(0)->getField("name")->getObjectValue();
			toLoadableClass(className, strObj);


			auto classPtr = vm->getClassLoader()->loadClass(className.c_str());
			auto eleClassPtr = classPtr->getArrayEleClass();

			if (eleClassPtr != nullptr)
			{
				f2->pushObject(eleClassPtr->getJavaClassObject());
			}
			else
			{
				f2->pushObject(nullptr);
			}
		}

		void classIsInterface(JVM* vm, JVMThread* th, StackFrame* v1, StackFrame* v2)
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
		}

		void classIsArray(StackFrame* v1, StackFrame* v2)
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
		}

		void getDeclaredFields0(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			int publicOnly = f1->getLocalInt(1);
			auto objPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(0));
			auto classPtr = objPtr->getGenericTypeClass();

			auto fields = classPtr->getAllDeclaredField();

			int i = 0, n = (int)fields.size();
			if (publicOnly)
			{
				n = 0;
				for (auto it = fields.begin(); it != fields.end(); it++)
				{
					if (it->second->isPublic())
					{
						n += 1;
					}
				}
			}

			auto heap = vm->getHeap();
			auto fieldClass = th->loadAndInit("java/lang/reflect/Field");
			auto arrPtr = heap->allocArray(th->loadAndInit("[java/lang/reflect/Field"), n);

			for (auto it = fields.begin(); it != fields.end(); it++)
			{
				auto fieldData = it->second;
				if (!publicOnly || fieldData->isPublic())
				{
					auto field = heap->alloc(fieldClass);

					field->getField("clazz")->setObjectValue(classPtr->getJavaClassObject());
					field->getField("name")->setObjectValue(JVMObjectCreator::allocString(*(fieldData->getName())));
					field->getField("type")->setObjectValue(th->justLoad(fieldData->getDescriptor()->c_str())->getJavaClassObject());
					field->getField("modifiers")->setIntValue((int32)fieldData->getAccessFlag());
					field->getField("slot")->setIntValue(it->first);

					*arrPtr->geElementAddress<JVMObject*>(i++) = field;
				}
			}

			f2->pushObject(arrPtr);
		}

		void parseMethodDescriptor(std::vector<std::string>& arr, const char* decriptor)
		{
			arr.clear();
			const char* p = decriptor + 1;

			const char* begin = nullptr;
			bool inClass = false, inArray = false;
			for (char c = *p++; c != ')'; c = *p++)
			{
				if (inClass)
				{
					if (c == ';')
					{
						inArray = inClass = false;
						arr.push_back(std::string(begin, p - begin));
					}
				}
				else if (inArray)
				{
					if (c == 'L')
					{
						inClass = true;
					}
					else if (c != '[')
					{
						inArray = inClass = false;
						arr.push_back(std::string(begin, p - begin));
					}
				}
				else
				{
					if (c == 'L')
					{
						//对象
						begin = p - 1;
						inClass = true;
					}
					else if (c == '[')
					{
						//数组
						begin = p - 1;
						inArray = true;
					}
					else
					{
						arr.push_back(std::string(1, c));
					}
				}
			}
		}

		JVMArray* getMethodParameterTypes(JVMThread* th, const char* decriptor)
		{
			std::vector<std::string> arr;
			parseMethodDescriptor(arr, decriptor);
			
			JVMArray* arrPtr = HeapMemory::getHeap()->allocArray(th->loadAndInit("[java/lang/Class"), arr.size());
			for (int i=0; i<(int)arr.size(); i++)
			{
				auto classPtr = th->justLoad(arr[i].c_str());
				*(arrPtr->geElementAddress<JVMObject*>(i)) = classPtr->getJavaClassObject();
			}

			return arrPtr;
		}

		JVMObject* getMethodReturnTypes(JVMThread* th, const char* decriptor)
		{
			while (*decriptor++ != ')');
			auto classPtr = th->justLoad(decriptor);

			return classPtr->getJavaClassObject();
		}

		JVMArray* getMethodExceptionTypes(JVMThread* th, Method *method)
		{
			std::shared_ptr<const ClassFile::ExceptionAttribute> attr = method->getException();
			if (!attr)
			{
				return HeapMemory::getHeap()->allocArray(th->justLoad("[java/lang/Class"), 0);
			}

			auto arr = attr->getData();

			JVMArray* arrPtr = HeapMemory::getHeap()->allocArray(th->justLoad("[java/lang/Class"), arr.size());
			for (int i = 0; i < (int)arr.size(); i++)
			{
				auto classNamePtr = method->getOwnerClass()->getConstString(arr[i]->getNameIndex());
				auto classPtr = th->justLoad(classNamePtr->c_str());
				*(arrPtr->geElementAddress<JVMObject*>(i)) = classPtr->getJavaClassObject();
			}

			return arrPtr;
		}

		void getDeclaredConstructors0(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			int publicOnly = f1->getLocalInt(1);
			auto objPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(0));
			auto classPtr = objPtr->getGenericTypeClass();

			auto methods = classPtr->getAllDeclaredMethod();

			int i = 0, n = 0;
			for (auto it = methods.begin(); it != methods.end(); it++)
			{
				if (it->second->getName()->compare("<init>") == 0)
				{
					if (!publicOnly || it->second->isPublic())
					{
						n++;
					}
				}
			}

			auto heap = vm->getHeap();
			auto arrEleClass = th->loadAndInit("java/lang/reflect/Constructor");
			auto arrPtr = heap->allocArray(th->loadAndInit("[java/lang/reflect/Constructor"), n);

			for (auto it = methods.begin(); it != methods.end(); it++)
			{
				auto methodData = it->second;
				if (methodData->getName()->compare("<init>") == 0)
				{
					if (!publicOnly || methodData->isPublic())
					{
						auto method = heap->alloc(arrEleClass);

						method->getField("clazz")->setObjectValue(classPtr->getJavaClassObject());
						method->getField("parameterTypes")->setObjectValue(getMethodParameterTypes(th, methodData->getDescriptor()->c_str()));
						method->getField("exceptionTypes")->setObjectValue(getMethodExceptionTypes(th, methodData));
						method->getField("modifiers")->setIntValue((int32)methodData->getAccessFlag());
						method->getField("slot")->setIntValue(methodData->getSlot());

						*arrPtr->geElementAddress<JVMObject*>(i++) = method;
					}
				}
			}

			f2->pushObject(arrPtr);
		}

		void getDeclaredMethods0(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			int publicOnly = f1->getLocalInt(1);
			auto objPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(0));
			auto classPtr = objPtr->getGenericTypeClass();

			auto methods = classPtr->getAllDeclaredMethod();

			int i = 0, n = 0;
			for (auto it = methods.begin(); it != methods.end(); it++)
			{
				if (it->second->getName()->at(0) != '<')
				{
					if (!publicOnly || it->second->isPublic())
					{
						n++;
					}
				}
			}

			auto heap = vm->getHeap();
			auto arrEleClass = th->loadAndInit("java/lang/reflect/Method");
			auto arrPtr = heap->allocArray(th->loadAndInit("[java/lang/reflect/Method"), n);

			for (auto it = methods.begin(); it != methods.end(); it++)
			{
				auto methodData = it->second;
				if (methodData->getName()->at(0) != '<')
				{
					if (!publicOnly || methodData->isPublic())
					{
						auto method = heap->alloc(arrEleClass);

						method->getField("clazz")->setObjectValue(classPtr->getJavaClassObject());
						method->getField("name")->setObjectValue(JVMObjectCreator::allocString(*(methodData->getName())));
						method->getField("parameterTypes")->setObjectValue(getMethodParameterTypes(th, methodData->getDescriptor()->c_str()));
						method->getField("returnType")->setObjectValue(getMethodReturnTypes(th, methodData->getDescriptor()->c_str()));
						method->getField("exceptionTypes")->setObjectValue(getMethodExceptionTypes(th, methodData));
						method->getField("modifiers")->setIntValue((int32)methodData->getAccessFlag());
						method->getField("slot")->setIntValue(methodData->getSlot());

						*arrPtr->geElementAddress<JVMObject*>(i++) = method;
					}
				}
			}

			f2->pushObject(arrPtr);
		}

		void getInterfaces0(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			auto objPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(0));

			auto classes = objPtr->getGenericTypeClass()->getInterfaces();
			auto arrPtr = vm->getHeap()->allocArray(th->justLoad("[java/lang/Class"), classes.size());

			for (int i=0; i<(int)classes.size(); i++)
			{
				auto v = classes[i];
				*(arrPtr->geElementAddress<JVMObject*>(i)) = v->getJavaClassObject();
			}

			f2->pushObject(arrPtr);
		}

		void getModifiers(StackFrame* f1, StackFrame* f2)
		{
			auto objPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(0));
			f2->pushInt((int32)objPtr->getGenericTypeClass()->getAccessFlag());
		}

		void getClassAccessFlags(StackFrame* f1, StackFrame* f2)
		{
			auto objPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(0));
			f2->pushInt((int32)objPtr->getGenericTypeClass()->getAccessFlag());
		}

		void getSuperclass(StackFrame* f1, StackFrame* f2)
		{
			auto objPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(0));
			
			auto superClass = objPtr->getGenericTypeClass()->getSuperClass();

			if (superClass == nullptr)
			{
				f2->pushObject(nullptr);
			}
			else
			{
				f2->pushObject(superClass->getJavaClassObject());
			}
		}

		void connvertArgArray(std::vector<SoltData>& arr, JVMArray* objArr)
		{
			arr.clear();

			if (objArr != nullptr)
			{
				int n = objArr->getLength();
				arr.resize(n);
				for (int i = 0; i < n; i++)
				{
					arr[i] = *(objArr->geElementAddress<JVMObject*>(i));
				}
			}
		}

		void newInstance0(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			auto constructor = f1->getLocalObject(0);

			auto classPtr = dynamic_cast<JAVAClassJVMObject*>(constructor->getField("clazz")->getObjectValue())->getGenericTypeClass();
			auto objPtr = HeapMemory::getHeap()->alloc(classPtr);

			int slot = constructor->getField("slot")->getIntValue();
			auto allMethod = classPtr->getAllDeclaredMethod();
			for (auto it = allMethod.begin(); it != allMethod.end(); it++)
			{
				auto method = it->second;
				if (method->getSlot() == slot)
				{
					std::vector<SoltData> args;
					connvertArgArray(args, dynamic_cast<JVMArray*>(f1->getLocalObject(1)));

					th->execute(objPtr, method, args);

					if (!th->isThreadEnd())
					{
						f2->pushObject(objPtr);
					}
					return;
				}
			}

			th->dispathException(th->loadAndInit("java/lang/InstantiationException"));
		}

		void invokeMethod0(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			auto method = f1->getLocalObject(0);
			auto objPtr = f1->getLocalObject(1);

			auto classPtr = dynamic_cast<JAVAClassJVMObject*>(method->getField("clazz")->getObjectValue())->getGenericTypeClass();

			int slot = method->getField("slot")->getIntValue();
			auto allMethod = classPtr->getAllDeclaredMethod();
			for (auto it = allMethod.begin(); it != allMethod.end(); it++)
			{
				auto method = it->second;
				if (method->getSlot() == slot)
				{
					std::vector<SoltData> args;
					connvertArgArray(args, dynamic_cast<JVMArray*>(f1->getLocalObject(2)));

					auto rtnValue = th->execute(objPtr, method, args);

					if (!th->isThreadEnd())
					{
						f2->pushJavaValue(rtnValue);
					}
					return;
				}
			}
		}

		
		void isInstance(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			auto classPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(0))->getGenericTypeClass();
			auto objPtr = f1->getLocalObject(1);

			if (objInstanceOf(objPtr, classPtr))
			{
				f2->pushInt(1);
			}
			else
			{
				f2->pushInt(0);
			}
		}

		void isAssignableFrom(JVM* vm, JVMThread* th, StackFrame* f1, StackFrame* f2)
		{
			auto superPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(0))->getGenericTypeClass();
			auto childPtr = dynamic_cast<JAVAClassJVMObject*>(f1->getLocalObject(1))->getGenericTypeClass();

			while ((childPtr != nullptr) && (childPtr != superPtr))
			{
				auto interfaces = childPtr->getInterfaces();
				for (auto it = interfaces.begin(); it != interfaces.end(); it++)
				{
					if (superPtr == *it)
					{
						childPtr = superPtr;
						break;
					}
				}

				childPtr = childPtr->getSuperClass();
			}

			if (childPtr == superPtr)
			{
				f2->pushInt(1);
			}
			else
			{
				f2->pushInt(0);
			}
		}

		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerEmptyNative("java/lang/Class", "registerNatives");
				NativeMethodHandler::registerEmptyNative("java/lang/ClassLoader", "registerNatives");

				NativeMethodHandler::registerNative("sun/reflect/Reflection", "getCallerClass", getCallerClass);
				NativeMethodHandler::registerNative("java/lang/Class", "desiredAssertionStatus0", desiredAssertionStatus0);

				NativeMethodHandler::registerNative("java/lang/Class", "getName0", getClassName0);
				NativeMethodHandler::registerNative("java/lang/Class", "forName0", loadClass0);

				NativeMethodHandler::registerNative("sun/reflect/NativeConstructorAccessorImpl", "newInstance0", newInstance0);
				NativeMethodHandler::registerNative("sun/reflect/NativeMethodAccessorImpl", "invoke0", invokeMethod0);

				//isAssignableFrom
				//isInstance
				NativeMethodHandler::registerNative("java/lang/Class", "isInterface", classIsInterface);
				NativeMethodHandler::registerNative("java/lang/Class", "isInstance", isInstance);
				NativeMethodHandler::registerNative("java/lang/Class", "isAssignableFrom", isAssignableFrom);
				NativeMethodHandler::registerNative("java/lang/Class", "isArray", classIsArray);
				NativeMethodHandler::registerNative("java/lang/Class", "getPrimitiveClass", getPrimitiveClass);
				NativeMethodHandler::registerNative("java/lang/Class", "isPrimitive", classIsPrimitive);
				NativeMethodHandler::registerNative("java/lang/Class", "getComponentType", classGetComponentType);

				NativeMethodHandler::registerNative("java/lang/Class", "getModifiers", getModifiers);
				NativeMethodHandler::registerNative("sun/reflect/Reflection", "getClassAccessFlags", getClassAccessFlags);
				NativeMethodHandler::registerNative("java/lang/Class", "getSuperclass", getSuperclass);
				NativeMethodHandler::registerNative("java/lang/Class", "getDeclaredFields0", getDeclaredFields0);
				NativeMethodHandler::registerNative("java/lang/Class", "getDeclaredConstructors0", getDeclaredConstructors0);
				NativeMethodHandler::registerNative("java/lang/Class", "getDeclaredMethods0", getDeclaredMethods0);
				NativeMethodHandler::registerNative("java/lang/Class", "getInterfaces0", getInterfaces0);

				//getDeclaredClasses0
				//getSigners
				//getProtectionDomain0
				//getGenericSignature0
				//getRawAnnotations
				//getRawTypeAnnotations
				//getConstantPool
				NativeMethodHandler::registerEmptyNative("java/lang/Class", "");
				NativeMethodHandler::registerEmptyNative("java/lang/Class", "getEnclosingMethod0");
				NativeMethodHandler::registerEmptyNative("java/lang/Class", "getDeclaringClass0");
			}
		}_1;
	}

}