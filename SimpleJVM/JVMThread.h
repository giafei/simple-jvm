#pragma once
#include "jvm.h"

namespace jvm
{
	class OpStack : public std::vector<SoltData>
	{
	public:
		SoltData getFromTop(int pos)
		{
			return this->at(this->size() - pos - 1);
		}

		SoltData top()
		{
			return getFromTop(0);
		}

		SoltData pop()
		{
			SoltData v = top();
			pop_back();
			return v;
		}

		void push(SoltData v)
		{
			push_back(v);
		}
	};

	class StackFrame
	{
	public:
		StackFrame(Method *method);

	public:
		void allocLocal(int n)
		{
			localVars.resize(n + 1);
		}

		int32 getLocalInt(int i)
		{
			return localVars[i].getIntValue();
		}

		void setLocalInt(int i, int32 v)
		{
			localVars[i].setIntValue(v);
		}

		JVMObject* getLocalObject(int i)
		{
			return localVars[i].getObject();
		}

		void setLocalObject(int i, JVMObject *p)
		{
			localVars[i].setObject(p);
		}

		float getLocalFloat(int i)
		{
			return localVars[i].getFloat();
		}

		void setLocalFloat(int i, float v)
		{
			localVars[i].setFlatValue(v);
		}

		int64 getLocalLong(int i)
		{
			int64 v = 0;
			int32 *p = (int32*)&v;
			p[0] = localVars[i].getIntValue();
			p[1] = localVars[i+1].getIntValue();

			return v;
		}

		void setLocalLong(int i, int64 v)
		{
			int32 *p = (int32*)&v;
			localVars[i].setIntValue(p[0]);
			localVars[i+1].setIntValue(p[1]);
		}

		double getLocalDouble(int i)
		{
			int64 v = getLocalLong(i);
			return *(double*)&v;
		}

		void setLocalDouble(int i, double v)
		{
			setLocalLong(i, *(int64*)&v);
		}

	public:
		int stackSize() const
		{
			return opStack.size();
		}

		int32 getTopInt()
		{
			return opStack.top().getIntValue();
		}

		int32 popInt()
		{
			return opStack.pop().getIntValue();
		}

		void pushInt(int32 v)
		{
			opStack.push_back(v);
		}

		JVMObject* getTopObject()
		{
			return opStack.top().getObject();
		}

		JVMObject* getObjectAt(int i)
		{
			return opStack.getFromTop(i).getObject();
		}

		JVMObject* popObject()
		{
			return opStack.pop().getObject();
		}

		void pushObject(JVMObject* v)
		{
			opStack.push(v);
		}

		float getTopFloat()
		{
			return opStack.top().getFloat();
		}

		float popFloat()
		{
			return opStack.pop().getFloat();
		}

		void pushFloat(float v)
		{
			opStack.push(v);
		}

		int64 getTopLong()
		{
			int64 v = 0;
			int32* p = (int32*)&v;
			p[1] = popInt();
			p[0] = getTopInt();

			pushInt(p[1]);

			return v;
		}

		int64 popLong()
		{
			int64 v = 0;
			int32* p = (int32*)&v;
			p[1] = popInt();
			p[0] = popInt();

			return v;
		}

		void pushLong(int64 v)
		{
			int32* p = (int32*)&v;
			pushInt(p[0]);
			pushInt(p[1]);
		}

		double getTopDouble()
		{
			int64 v = getTopLong();
			return *(double*)&v;
		}

		double popDouble()
		{
			int64 v = popLong();
			return *(double*)&v;
		}

		void pushDouble(double v)
		{
			pushLong(*(int64*)&v);
		}

		void pushJavaValue(std::shared_ptr<JavaValue> ptr)
		{
			if (ptr->getSoltSize() == 1)
			{
				pushInt(ptr->getIntValue());
			}
			else if (ptr->getSoltSize() == 2)
			{
				pushLong(ptr->getLongValue());
			}
		}

		void popToJavaValue(std::shared_ptr<JavaValue> ptr)
		{
			if (ptr->getSoltSize() == 1)
			{
				ptr->setIntValue(popInt());
			}
			else if (ptr->getSoltSize() == 2)
			{
				ptr->setLongValue(popLong());
			}
		}
	public:
		Method* getMethod()
		{
			return method;
		}

		int* getPC()
		{
			return &pc;
		}


	protected:
		int pc;
		std::vector<SoltData> localVars;
		OpStack opStack;

		Method* method;
	};


	class JVMThread
	{
	public:
		JVMThread(JVM *pJVM);
		~JVMThread();

	public:
		std::shared_ptr<JavaValue> execute(JVMObject *pThis, Method* method, const std::vector<SoltData>& arguments);
		void executeMain(JVMClass *pClass, const std::vector<std::string>& args);

		JVMObject* allocString(const std::string& str);
		JVMObject* allocString(const std::wstring& str);
		JVMArray* allocArray(JVMClass* classPtr, std::vector<int>& arrLen, int lenPos);
		JVMObject* getJavaThread();

	public:
		void initializeSystem();
		JVMClass* loadAndInit(const char* className);

	protected:
		void execute();
		void checkClassInited(JVMClass *pClass);

	protected:
		int getArgSoltCount(std::shared_ptr<const std::string> methodDescriptor);
		void invokeNative(Method *method);

	public:
		StackFrame* pushFrame(Method* method);
		StackFrame* currentFrame()
		{
			return stacks.top();
		}

		void popFrame();
		void popFrame(std::function<void(StackFrame*, StackFrame*)> callback);

	public:
		static JVMThread* current();

	protected:
		std::stack<StackFrame*> stacks;
		JVM *pJVM;
		JVMObject *javaThread;

		std::map<std::string, int> unsafeHackClass;
	};

}


