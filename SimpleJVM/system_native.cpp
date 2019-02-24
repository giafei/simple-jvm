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

			setProperty(th, props, "java.version", "0.0.1");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "java.vendor", "giafei jvm");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "java.vendor.url", "http://giafei.net");
			if (th->isExceptionNow())
			{
				return;
			}

			getenv_s(&v, buf, "JAVA_HOME");
			setProperty(th, props, "java.home", buf);
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "java.class.version", "52.0");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "java.class.path", _getcwd(buf, 512)); //
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "os.name", "Windows 10");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "os.arch", "amd64");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "os.version", "10.0");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "file.separator", "\\");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "path.separator", ";");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "line.separator", "\r\n");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "user.name", "giafei");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "user.home", "f:\\tmp\\home");
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "user.dir", buf);
			if (th->isExceptionNow())
			{
				return;
			}

			setProperty(th, props, "sun.stdout.encoding", "GBK");
			if (th->isExceptionNow())
			{
				return;
			}

			th->popFrame();
			th->currentFrame()->pushObject(props);
		}

		void __stdcall availableProcessors(StackFrame* f1, StackFrame* f2)
		{
			f2->pushInt(1);
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
			}
		}_1;
	}

}