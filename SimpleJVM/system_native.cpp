#include "stdafx.h"
#include "NativeMethodHandler.h"
#include "JVMClass.h"
#include "JVM.h"
#include "JVMThread.h"
#include "JVMObject.h"
#include "ClassLoader.h"
#include "HeapMemory.h"
#include <direct.h>

using namespace jvm;

namespace native
{
	namespace jsystem
	{
		void arraycopy(JVM* vm, JVMThread* th)
		{
			StackFrame* pFrame = th->currentFrame();

			int len = pFrame->getLocalInt(4);
			if (len > 0)
			{
				JVMArray *src = dynamic_cast<JVMArray*>(pFrame->getLocalObject(0));
				int srcPos = pFrame->getLocalInt(1);

				JVMArray *dest = dynamic_cast<JVMArray*>(pFrame->getLocalObject(2));
				int destPos = pFrame->getLocalInt(3);

				memcpy(dest->getAddress(destPos), src->getAddress(srcPos), len * src->getElementSize());
			}


			th->popFrame();
		}

		void setProperty(JVMThread* th, JVMObject *props, const std::string& k, const std::string& v)
		{
			auto method = props->getClass()->getMethod("setProperty::(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;");

			std::vector<SoltData> arguments;
			arguments.reserve(2);
			arguments.push_back(JVMObjectCreator::allocString(k));
			arguments.push_back(JVMObjectCreator::allocString(v));

			th->execute(props, method, arguments);
		}

		void initProperties(JVM* vm, JVMThread* th)
		{
			StackFrame* pFrame = th->currentFrame();
			JVMObject *props = pFrame->getLocalObject(0);

			size_t v = 0;
			char buf[512];

			setProperty(th, props, "awt.toolkit", "sun.awt.windows.WToolkit");
			setProperty(th, props, "file.encoding", "UTF-8");
			setProperty(th, props, "file.encoding.pkg", "sun.io");
			setProperty(th, props, "file.separator", "\\");

			setProperty(th, props, "java.class.version", "52.0");

			getenv_s(&v, buf, "TEMP");
			setProperty(th, props, "java.io.tmpdir", buf);
			setProperty(th, props, "java.version", "0.0.1");
			setProperty(th, props, "java.vendor", "giafei jvm");
			setProperty(th, props, "java.vendor.url", "http://giafei.net");

			getenv_s(&v, buf, "JAVA_HOME");
			setProperty(th, props, "java.home", buf);
			strcat_s(buf, "\bin");
			setProperty(th, props, "sun.boot.library.path", buf);

			setProperty(th, props, "java.class.path", _getcwd(buf, 512)); //

			setProperty(th, props, "os.name", "Windows 10");
			setProperty(th, props, "os.arch", "amd64");
			setProperty(th, props, "os.version", "10.0");
			setProperty(th, props, "path.separator", ";");
			setProperty(th, props, "line.separator", "\r\n");
			setProperty(th, props, "user.name", "giafei");
			setProperty(th, props, "user.home", "f:\\tmp\\home");
			setProperty(th, props, "user.dir", buf);
			setProperty(th, props, "sun.stdout.encoding", "GBK");
			setProperty(th, props, "sun.jnu.encoding", "GBK");
			setProperty(th, props, "user.language", "zh");
			setProperty(th, props, "user.script", "");
			setProperty(th, props, "user.timezone", "");
			setProperty(th, props, "user.variant", "");

			th->popFrame();
			th->currentFrame()->pushObject(props);
		}

		void availableProcessors(StackFrame* f1, StackFrame* f2)
		{
			f2->pushInt(1);
		}

		void mapLibraryName(StackFrame* f1, StackFrame* f2)
		{
			f2->pushObject(f1->getLocalObject(0));
		}

		void findBuiltinLib(StackFrame* f1, StackFrame* f2)
		{
			mapLibraryName(f1, f2);
		}

		void loadLibrary(StackFrame* f1, StackFrame* f2)
		{
			f1->getLocalObject(0)->getField("loaded")->setIntValue(1);
		}

		void findSignal(StackFrame* f1, StackFrame* f2)
		{
			auto charArr = dynamic_cast<JVMArray*>(f1->getLocalObject(0)->getField("value")->getObjectValue());
			
			std::string signal;
			UTF16StringToUTF8(signal, charArr);

			if (signal == "INT")
			{
				f2->pushInt(2);
			}
			else if (signal == "TERM")
			{
				f2->pushInt(15);
			}
		}

		void handleSignal(StackFrame* f1, StackFrame* f2)
		{
			f2->pushLong(f1->getLocalLong(1));
		}

		void setErrorMode(StackFrame* f1, StackFrame* f2)
		{
			f2->pushLong(f1->getLocalLong(0));
		}

		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerEmptyNative("java/lang/System", "registerNatives");
				NativeMethodHandler::registerNative("java/lang/System", "arraycopy", arraycopy);
				NativeMethodHandler::registerNative("java/lang/System", "initProperties", initProperties);


				NativeMethodHandler::registerNative("java/lang/Runtime", "availableProcessors", availableProcessors);

				NativeMethodHandler::registerEmptyNative("java/lang/System", "setIn0");
				NativeMethodHandler::registerEmptyNative("java/lang/System", "setOut0");
				NativeMethodHandler::registerEmptyNative("java/lang/System", "setErr0");

				NativeMethodHandler::registerNative("java/lang/System", "mapLibraryName", mapLibraryName);
				NativeMethodHandler::registerNative("java/lang/ClassLoader", "findBuiltinLib", findBuiltinLib);
				NativeMethodHandler::registerNative("java/lang/ClassLoader$NativeLibrary", "load", loadLibrary);

				NativeMethodHandler::registerNative("sun/misc/Signal", "findSignal", findSignal);
				NativeMethodHandler::registerNative("sun/misc/Signal", "handle0", handleSignal);

				NativeMethodHandler::registerNative("sun/io/Win32ErrorMode", "setErrorMode", setErrorMode);
			}
		}_1;
	}

}