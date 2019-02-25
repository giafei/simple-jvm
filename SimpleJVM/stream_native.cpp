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
		void fileWriteBytes(StackFrame* f1, StackFrame *f2)
		{
			auto fdPtr = f1->getLocalObject(0)->getField("fd")->getObjectValue();
			FILE *fp = (FILE*)fdPtr->getField("handle")->getLongValue();

			auto data = dynamic_cast<JVMArray*>(f1->getLocalObject(1));
			int offset = f1->getLocalInt(2);
			int len = f1->getLocalInt(3);
			bool append = f1->getLocalInt(4) != 0;

			if (fp == stdout || fp == stderr)
			{
				//UTF8编码格式的数据 直接显示会乱码 需要先转码
				std::wstring str;
				UTF8StringToUTF16(str, (char*)data->getAddress(offset), len);

				fwprintf(fp, L"%s", str.c_str());
			}
			else
			{
				if (!append)
				{
					fseek(fp, 0, SEEK_SET);
				}

				fwrite(data->getAddress(offset), 1, len, fp);
			}
		}

		static class _
		{
		public:
			_()
			{
				NativeMethodHandler::registerEmptyNative("java/io/FileInputStream", "initIDs");
				NativeMethodHandler::registerEmptyNative("java/io/FileOutputStream", "initIDs");
				NativeMethodHandler::registerNative("java/io/FileOutputStream", "writeBytes", fileWriteBytes);
			}
		}_1;
	}

}