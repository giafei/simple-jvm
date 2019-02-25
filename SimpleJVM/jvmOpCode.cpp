#include "stdafx.h"
#include "JVMThread.h"
#include "JVMClass.h"
#include "ClassFileData.h"
#include "ClassLoader.h"
#include "JVM.h"
#include "HeapMemory.h"
#include "DataBlock.h"
#include "JVMObject.h"

namespace jvm
{
	bool objInstanceOf(JVMObject* objPtr, JVMClass* classPtr)
	{
		auto objClassPtr = objPtr->getClass();

		while (objClassPtr != nullptr)
		{
			if (objClassPtr == classPtr)
				break;

			bool isInterface = false;
			auto interfaces = objClassPtr->getInterfaces();
			for (auto it = interfaces.begin(); it != interfaces.end(); it++)
			{
				if (classPtr == *it)
				{
					isInterface = true;
					break;
				}
			}

			if (isInterface)
			{
				break;
			}

			objClassPtr = objClassPtr->getSuperClass();
		}

		if (objClassPtr == nullptr)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	std::string methodKey(std::shared_ptr<const ConstantNameAndType> t)
	{
		return *(t->getName()) + "::" + *(t->getDescriptor());
	}

	void JVMThread::execute()
	{
		HeapMemory *heap = pJVM->getHeap();

		StackFrame* f = nullptr;
		Method* method = nullptr;
		JVMClass* pClass = nullptr;

		const uint8* codeData = nullptr;
		int* pPC = nullptr;
		bool wide = false;

		auto prepare = [&]()
		{
			f = this->currentFrame();
			method = f->getMethod();

			if (method != nullptr)
			{
				pClass = method->getOwnerClass();
				checkClassInited(pClass);

				if (!method->isNative())
				{
					codeData = method->getCode()->getCode()->getData();
				}

// 				if (method->getName()->compare("<init>") == 0)
// 				{
// 					if (method->getOwnerClass()->getName()->compare("java/nio/charset/CharsetDecoder") == 0)
// 					{
// 						printf("1\n");
// 					}
// 				}
			}

			wide = false;
			pPC = f->getPC();
		};


		prepare();

		auto readInt8 = [&]
		{
			int &pc = *pPC;
			return ((int8*)codeData)[pc++];
		};

		auto readUint8 = [&]
		{
			int &pc = *pPC;
			return ((uint8*)codeData)[pc++];
		};

		auto readInt16 = [&]
		{
			int &pc = *pPC;
			int16 v = codeData[pc++];
			v = ((v << 8) | codeData[pc++]);

			return v;
		};

		auto readUInt16 = [&]
		{
			int &pc = *pPC;
			uint16 v = codeData[pc++];
			v = ((v << 8) | codeData[pc++]);

			return v;
		};

		auto swapInt = [](int v)
		{
			uint8 *p = (uint8*)&v;

			uint8 tmp = p[3];
			p[3] = p[0];
			p[0] = tmp;

			tmp = p[2];
			p[2] = p[1];
			p[1] = tmp;

			return v;
		};

		auto toInt32 = [](uint8 *q)
		{
			int v = 0;
			uint8 *p = (uint8*)&v;
			p[3] = q[0];
			p[2] = q[1];
			p[1] = q[2];
			p[0] = q[3];
			return v;
		};

		auto readInt32 = [&]
		{
			int &pc = *pPC;

			int v = 0;
			uint8 *p = (uint8*)&v;
			p[3] = codeData[pc++];
			p[2] = codeData[pc++];
			p[1] = codeData[pc++];
			p[0] = codeData[pc++];
			return v;
		};

		auto skipPadding = [&]
		{
			int v = *pPC % 4;
			if (v == 0)
				return;
			*pPC += 4 - v;
		};

		auto jumpOffset = [&](int offset)
		{
			*pPC += offset;
		};

		while (method != nullptr)
		{
			if (isThreadEnd())
				return;

			if (method->isNative())
			{
				invokeNative(method);
				prepare();
				continue;
			}

			switch (uint8 op = readUint8())
			{
			case 0x00://nop
				break;

			case 0x01: //aconst_null
				f->pushObject(nullptr);
				break;

			case 0x02: //iconst_m1
			case 0x03: //iconst_0
			case 0x04: //iconst_1
			case 0x05: //iconst_2
			case 0x06: //iconst_3
			case 0x07: //iconst_4
			case 0x08: //iconst_5
				f->pushInt(op - 0x03);
				break;

			case 0x09: //lconst_0
				f->pushLong(0);
				break;

			case 0x0A: //lconst_1
				f->pushLong(1);
				break;

			case 0x0B: //fconst_0
				f->pushFloat(0);
				break;

			case 0x0C: //fconst_1
				f->pushFloat(1.0f);
				break;

			case 0x0D: //fconst_2	
				f->pushFloat(2.0f);
				break;

			case 0x0E: //lconst_0
				f->pushDouble(0);
				break;

			case 0x0F: //lconst_1
				f->pushDouble(1.0);
				break;

			case 0x10: //bipush
			{
				f->pushInt(readInt8());
				break;
			}

			case 0x11: //sipush
			{
				f->pushInt(readInt16());
				break;
			}

			case 0x12: //ldc
			case 0x13: //ldc_w
			{
				int i = (op == 0x12) ? readUint8() : readUInt16();
				std::shared_ptr<Constant> ptr = pClass->getConstantAt(i);
				switch (ptr->getType())
				{
				case CONSTANT_Integer_info:
					f->pushInt(pClass->getConstInt(i));
					break;

				case CONSTANT_Float_info:
					f->pushFloat(pClass->getConstFloat(i));
					break;

				case CONSTANT_String_info:
				{
					auto strPtr = pClass->getConstString(i);
					auto strObj = JVMObjectCreator::allocString(*strPtr);
					f->pushObject(strObj);
					break;
				}

				case CONSTANT_Class_info:
				{
					auto strPtr = pClass->getConstString(i);
					auto p = pClass->getClassLoader()->loadClass(strPtr->c_str());
					checkClassInited(p);
					if (!isThreadEnd())
					{
						f->pushObject(p->getJavaClassObject());
					}
					break;
				}

				default:
					throw new std::exception("不支持的ldc");
					break;
				}
				break;
			}

			case 0x14: //ldc2_w
			{
				int i = readUInt16();

				std::shared_ptr<Constant> ptr = pClass->getConstantAt(i);
				switch (ptr->getType())
				{
				case CONSTANT_Long_info:
					f->pushLong(pClass->getConstantLong(i));
					break;

				case CONSTANT_Double_info:
					f->pushDouble(pClass->getConstantDouble(i));
					break;

				default:
					throw new std::exception("不支持的ldc2_w");
					break;
				}
				break;
			}

			case 0x15: //iload
			{
				f->pushInt(f->getLocalInt(wide? readUInt16(): readUint8()));
				wide = false;
				break;
			}

			case 0x16://lload
			{
				f->pushLong(f->getLocalLong(wide ? readUInt16() : readUint8()));
				wide = false;
				break;
			}

			case 0x17://fload
			{
				f->pushFloat(f->getLocalFloat(wide ? readUInt16() : readUint8()));
				wide = false;
				break;
			}

			case 0x18://dload
			{
				f->pushDouble(f->getLocalDouble(wide ? readUInt16() : readUint8()));
				wide = false;
				break;
			}

			case 0x19://aload
			{
				f->pushObject(f->getLocalObject(wide ? readUInt16() : readUint8()));
				wide = false;
				break;
			}

			case 0x1A: //iload_0
			case 0x1B:
			case 0x1C:
			case 0x1D:
			{
				f->pushInt(f->getLocalInt(op - 0x1A));
				break;
			}

			case 0x1E: //lload_0
			case 0x1F:
			case 0x20:
			case 0x21:
			{
				f->pushLong(f->getLocalInt(op - 0x1E));
				break;
			}

			case 0x22: //fload_0
			case 0x23:
			case 0x24:
			case 0x25:
			{
				f->pushFloat(f->getLocalFloat(op - 0x22));
				break;
			}

			case 0x26: //dload_0
			case 0x27:
			case 0x28:
			case 0x29:
			{
				f->pushDouble(f->getLocalDouble(op - 0x26));
				break;
			}

			case 0x2A: //aload_0
			case 0x2B:
			case 0x2C:
			case 0x2D:
			{
				f->pushObject(f->getLocalObject(op - 0x2A));
				break;
			}

			case 0x2E: //iaload
			{
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				f->pushInt(*(int32*)pArr->getAddress(i));
				break;
			}

			case 0x2F: //laload
			{
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				f->pushLong(*(int64*)pArr->getAddress(i));
				break;
			}

			case 0x30: //faload
			{
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				f->pushFloat(*(float*)pArr->getAddress(i));
				break;
			}

			case 0x31: //daload
			{
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				f->pushDouble(*(double*)pArr->getAddress(i));
				break;
			}

			case 0x32: //aaload
			{
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				f->pushObject(*(JVMObject**)pArr->getAddress(i));
				break;
			}

			case 0x33: //baload
			{
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				f->pushInt(*(uint8*)pArr->getAddress(i));
				break;
			}

			case 0x34: //caload
			{
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				if (i >= pArr->getLength())
				{
					dispathException(loadAndInit("java/lang/ArrayIndexOutOfBoundsException"));
				}
				else
				{
					f->pushInt(*(wchar_t*)pArr->getAddress(i));
				}
				break;
			}

			case 0x35: //saload
			{
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				f->pushInt(*(short*)pArr->getAddress(i));
				break;
			}

			case 0x36: //istore
			{
				f->setLocalInt(wide ? readUInt16() : readUint8(), f->popInt());
				wide = false;
				break;
			}

			case 0x37: //lstore
			{
				f->setLocalLong(wide ? readUInt16() : readUint8(), f->popLong());
				wide = false;
				break;
			}

			case 0x38: //fstore
			{
				f->setLocalFloat(wide ? readUInt16() : readUint8(), f->popFloat());
				wide = false;
				break;
			}

			case 0x39: //dstore
			{
				f->setLocalDouble(wide ? readUInt16() : readUint8(), f->popDouble());
				wide = false;
				break;
			}

			case 0x3A: //astore
			{
				f->setLocalObject(wide ? readUInt16() : readUint8(), f->popObject());
				wide = false;
				break;
			}

			case 0x3B: //istore_0
			case 0x3C:
			case 0x3D:
			case 0x3E:
			{
				int i = op - 0x3B;
				f->setLocalInt(i, f->popInt());
				break;
			}

			case 0x3F: //lstore_0
			case 0x40:
			case 0x41:
			case 0x42:
			{
				int i = op - 0x3F;
				f->setLocalLong(i, f->popLong());
				break;
			}

			case 0x43: //fstore_0
			case 0x44:
			case 0x45:
			case 0x46:
			{
				int i = op - 0x43;
				f->setLocalFloat(i, f->popFloat());
				break;
			}

			case 0x47: //dstore_0
			case 0x48:
			case 0x49:
			case 0x4A:
			{
				int i = op - 0x47;
				f->setLocalDouble(i, f->popDouble());
				break;
			}

			case 0x4B: //astore_0
			case 0x4C:
			case 0x4D:
			case 0x4E:
			{
				int i = op - 0x4B;
				f->setLocalObject(i, f->popObject());
				break;
			}

			case 0x4F: //iastore
			{
				int32 v = f->popInt();
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				*(int32*)pArr->getAddress(i) = v;
				break;
			}

			case 0x50: //lastore
			{
				int64 v = f->popLong();
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				*(int64*)pArr->getAddress(i) = v;
				break;
			}

			case 0x51: //fastore
			{
				float v = f->popFloat();
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				*(float*)pArr->getAddress(i) = v;
				break;
			}

			case 0x52: //dastore
			{
				double v = f->popDouble();
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				*(double*)pArr->getAddress(i) = v;
				break;
			}

			case 0x53: //aastore
			{
				JVMObject* v = f->popObject();
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				*(JVMObject**)pArr->getAddress(i) = v;
				break;
			}

			case 0x54: //bastore
			{
				uint8 v = (uint8)f->popInt();
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				*(uint8*)pArr->getAddress(i) = v;
				break;
			}

			case 0x55: //castore
			{
				wchar_t v = (wchar_t)f->popInt();
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				*(wchar_t*)pArr->getAddress(i) = v;
				break;
			}

			case 0x56: //sastore
			{
				short v = (short)f->popInt();
				int i = f->popInt();
				JVMArray *pArr = dynamic_cast<JVMArray*>(f->popObject());
				*(short*)pArr->getAddress(i) = v;
				break;
			}

			case 0x57: //pop
				f->popInt();
				break;

			case 0x58: //pop2
				f->popInt();
				f->popInt();
				break;

			case 0x59: //dup
			{
				f->pushInt(f->getTopInt());
				break;
			}
			case 0x5A: //dup_x1
			{
				int v1 = f->popInt();
				int v2 = f->popInt();
				f->pushInt(v1);
				f->pushInt(v2);
				f->pushInt(v1);
				break;
			}
			case 0x5B: //dup_x2
			{
				int v1 = f->popInt();
				int v2 = f->popInt();
				int v3 = f->popInt();
				f->pushInt(v1);
				f->pushInt(v3);
				f->pushInt(v2);
				f->pushInt(v1);
				break;
			}

			case 0x5C: //dup2
			case 0x5D: //dup2_+1
			case 0x5E: //dup2_+2
			{
				int n = op - 0x5B;
				for (int i = 0; i < n; i++)
				{
					f->pushLong(f->getTopLong());
				}
				break;
			}

			case 0x5F: //swap
			{
				int v1 = f->popInt();
				int v2 = f->popInt();
				f->pushInt(v1);
				f->pushInt(v2);
				break;
			}

			case 0x60: //iadd
			{
				int v2 = f->popInt();
				int v1 = f->popInt();
				f->pushInt(v1 + v2);
				break;
			}

			case 0x61: //ladd
			{
				int64 v2 = f->popLong();
				int64 v1 = f->popLong();
				f->pushLong(v1 + v2);
				break;
			}

			case 0x62: //fadd
			{
				float v2 = f->popFloat();
				float v1 = f->popFloat();
				f->pushFloat(v1 + v2);
				break;
			}

			case 0x63: //dadd
			{
				double v2 = f->popDouble();
				double v1 = f->popDouble();
				f->pushDouble(v1 + v2);
				break;
			}

			case 0x64: //isub
			{
				int v2 = f->popInt();
				int v1 = f->popInt();
				f->pushInt(v1 - v2);
				break;
			}

			case 0x65: //lsub
			{
				int64 v2 = f->popLong();
				int64 v1 = f->popLong();
				f->pushLong(v1 - v2);
				break;
			}

			case 0x66: //fsub
			{
				float v2 = f->popFloat();
				float v1 = f->popFloat();
				f->pushFloat(v1 - v2);
				break;
			}

			case 0x67: //dsub
			{
				double v2 = f->popDouble();
				double v1 = f->popDouble();
				f->pushDouble(v1 - v2);
				break;
			}

			case 0x68: //imul
			{
				int v2 = f->popInt();
				int v1 = f->popInt();
				f->pushInt(v1 * v2);
				break;
			}

			case 0x69: //lmul
			{
				int64 v2 = f->popLong();
				int64 v1 = f->popLong();
				f->pushLong(v1 * v2);
				break;
			}

			case 0x6A: //fmul
			{
				float v2 = f->popFloat();
				float v1 = f->popFloat();
				f->pushFloat(v1 * v2);
				break;
			}

			case 0x6B: //dmul
			{
				double v2 = f->popDouble();
				double v1 = f->popDouble();
				f->pushDouble(v1 * v2);
				break;
			}

			case 0x6C: //idiv
			{
				int v2 = f->popInt();
				int v1 = f->popInt();
				f->pushInt(v1 / v2);
				break;
			}

			case 0x6D: //ldiv
			{
				int64 v2 = f->popLong();
				int64 v1 = f->popLong();
				f->pushLong(v1 / v2);
				break;
			}

			case 0x6E: //fdiv
			{
				float v2 = f->popFloat();
				float v1 = f->popFloat();
				f->pushFloat(v1 / v2);
				break;
			}

			case 0x6F: //ddiv
			{
				double v2 = f->popDouble();
				double v1 = f->popDouble();
				f->pushDouble(v1 / v2);
				break;
			}

			case 0x70: //irem
			{
				int v2 = f->popInt();
				int v1 = f->popInt();
				f->pushInt(v1 % v2);
				break;
			}

			case 0x71: //lrem
			{
				int64 v2 = f->popLong();
				int64 v1 = f->popLong();
				f->pushLong(v1 % v2);
				break;
			}

			case 0x72: //frem
			{
				float v2 = f->popFloat();
				float v1 = f->popFloat();
				f->pushFloat(fmod(v1, v2));
				break;
			}

			case 0x73: //drem
			{
				double v2 = f->popDouble();
				double v1 = f->popDouble();
				f->pushDouble(fmod(v1, v2));
				break;
			}

			case 0x74: //ineg
			{
				f->pushInt(-f->popInt());
				break;
			}

			case 0x75: //lneg
			{
				f->pushLong(-f->popLong());
				break;
			}

			case 0x76: //fneg
			{
				f->pushFloat(-f->popFloat());
				break;
			}

			case 0x77: //dneg
			{
				f->pushDouble(-f->popDouble());
				break;
			}

			case 0x78: //ishl
			{
				int v2 = f->popInt();
				int v1 = f->popInt();
				f->pushInt(v1 << v2);
				break;
			}

			case 0x79: //lshl
			{
				int v2 = f->popInt();
				int64 v1 = f->popLong();
				f->pushLong(v1 << v2);
				break;
			}

			case 0x7A: //ishr
			{
				int v2 = f->popInt();
				int v1 = f->popInt();
				f->pushInt(v1 >> v2);
				break;
			}

			case 0x7B: //lshr
			{
				int v2 = f->popInt();
				int64 v1 = f->popLong();
				f->pushLong(v1 << v2);
				break;
			}

			case 0x7C: //iushr
			{
				int v2 = f->popInt();
				uint32 v1 = f->popInt();
				f->pushInt(v1 >> v2);
				break;
			}

			case 0x7D: //lushr
			{
				int v2 = f->popInt();
				uint64 v1 = f->popLong();
				f->pushLong(v1 << v2);
				break;
			}

			case 0x7E: //iand
			{
				int32 v2 = f->popInt();
				int32 v1 = f->popInt();
				f->pushInt(v1 & v2);
				break;
			}

			case 0x7F: //land
			{
				int64 v2 = f->popLong();
				int64 v1 = f->popLong();
				f->pushLong(v1 & v2);
				break;
			}

			case 0x80: //ior
			{
				int32 v2 = f->popInt();
				int32 v1 = f->popInt();
				f->pushInt(v1 | v2);
				break;
			}

			case 0x81: //lor
			{
				int64 v2 = f->popLong();
				int64 v1 = f->popLong();
				f->pushLong(v1 | v2);
				break;
			}

			case 0x82: //ixor
			{
				int32 v2 = f->popInt();
				int32 v1 = f->popInt();
				f->pushInt(v1 ^ v2);
				break;
			}

			case 0x83: //lxor
			{
				int64 v2 = f->popLong();
				int64 v1 = f->popLong();
				f->pushLong(v1 ^ v2);
				break;
			}

			case 0x84: //iinc
			{
				int i = wide ? readUInt16() : readUint8();
				int v = readInt8();
				f->setLocalInt(i, f->getLocalInt(i) + v);
				wide = false;
				break;
			}

			case 0x85: //i2l
			{
				f->pushLong(f->popInt());
				break;
			}

			case 0x86: //i2f
			{
				f->pushFloat((float)f->popInt());
				break;
			}

			case 0x87: //i2d
			{
				f->pushDouble(f->popInt());
				break;
			}

			case 0x88: //l2i
			{
				f->pushInt((int32)f->popLong());
				break;
			}

			case 0x89: //l2f
			{
				f->pushFloat((float)f->popLong());
				break;
			}

			case 0x8A: //l2d
			{
				f->pushDouble((double)f->popLong());
				break;
			}

			case 0x8B: //f2i
			{
				f->pushInt((int)f->popFloat());
				break;
			}

			case 0x8C: //f2l
			{
				f->pushLong((int64)f->popFloat());
				break;
			}

			case 0x8D: //f2d
			{
				f->pushDouble(f->popFloat());
				break;
			}

			case 0x8E: //d2i
			{
				f->pushInt((int)f->popDouble());
				break;
			}

			case 0x8F: //d2l
			{
				f->pushLong((int64)f->popDouble());
				break;
			}

			case 0x90: //d2f
			{
				f->pushFloat((float)f->popDouble());
				break;
			}

			case 0x91: //i2b
			{
				f->pushInt((uint8)f->popInt());
				break;
			}

			case 0x92: //i2c
			{
				f->pushInt((wchar_t)f->popInt());
				break;
			}

			case 0x93: //i2s
			{
				f->pushInt((short)f->popInt());
				break;
			}

			case 0x94: //lcmp
			{
				int64 v2 = f->popLong();
				int64 v1 = f->popLong();
				if (v1 > v2)
				{
					f->pushInt(1);
				}
				else if (v1 == v2)
				{
					f->pushInt(0);
				}
				else
				{
					f->pushInt(-1);
				}
				break;
			}

			case 0x95: //fcmpl
			{
				float v2 = f->popFloat();
				float v1 = f->popFloat();
				if (isnan(v1) || isnan(v2))
				{
					f->pushInt(-1);
				}
				else if (v1 > v2)
				{
					f->pushInt(1);
				}
				else if (v1 == v2)
				{
					f->pushInt(0);
				}
				else
				{
					f->pushInt(-1);
				}
				break;
			}

			case 0x96: //fcmpg
			{
				float v2 = f->popFloat();
				float v1 = f->popFloat();
				if (isnan(v1) || isnan(v2))
				{
					f->pushInt(1);
				}
				else if (v1 > v2)
				{
					f->pushInt(1);
				}
				else if (v1 == v2)
				{
					f->pushInt(0);
				}
				else
				{
					f->pushInt(-1);
				}
				break;
			}

			case 0x97: //dcmpl
			{
				double v2 = f->popFloat();
				double v1 = f->popFloat();
				if (isnan(v1) || isnan(v2))
				{
					f->pushInt(-1);
				}
				else if (v1 > v2)
				{
					f->pushInt(1);
				}
				else if (v1 == v2)
				{
					f->pushInt(0);
				}
				else
				{
					f->pushInt(-1);
				}
				break;
			}

			case 0x98: //dcmpg
			{
				double v2 = f->popFloat();
				double v1 = f->popFloat();
				if (isnan(v1) || isnan(v2))
				{
					f->pushInt(1);
				}
				else if (v1 > v2)
				{
					f->pushInt(1);
				}
				else if (v1 == v2)
				{
					f->pushInt(0);
				}
				else
				{
					f->pushInt(-1);
				}
				break;
			}

			case 0x99: //ifeq
			{
				int offset = readInt16();
				int v = f->popInt();
				if (v == 0)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0x9A: //ifne
			{
				int offset = readInt16();
				int v = f->popInt();
				if (v != 0)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0x9B: //iflt
			{
				int offset = readInt16();
				int v = f->popInt();
				if (v < 0)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0x9C: //ifge
			{
				int offset = readInt16();
				int v = f->popInt();
				if (v >= 0)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0x9D: //ifgt
			{
				int offset = readInt16();
				int v = f->popInt();
				if (v > 0)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0x9E: //ifle
			{
				int offset = readInt16();
				int v = f->popInt();
				if (v <= 0)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0x9F: //if_icmpeq
			case 0xA5: //if_acmpeq
			{
				int offset = readInt16();
				int v2 = f->popInt();
				int v1 = f->popInt();
				if (v2 == v1)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0xA0: //if_icmpne
			case 0xA6: //if_acmpne
			{
				int offset = readInt16();
				int v2 = f->popInt();
				int v1 = f->popInt();
				if (v2 != v1)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0xA1: //if_icmplt
			{
				int offset = readInt16();
				int v2 = f->popInt();
				int v1 = f->popInt();
				if (v1 < v2)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0xA2: //if_icmpge
			{
				int offset = readInt16();
				int v2 = f->popInt();
				int v1 = f->popInt();
				if (v1 >= v2)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0xA3: //if_icmpgt
			{
				int offset = readInt16();
				int v2 = f->popInt();
				int v1 = f->popInt();
				if (v1 > v2)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0xA4: //if_icmple
			{
				int offset = readInt16();
				int v2 = f->popInt();
				int v1 = f->popInt();
				if (v1 <= v2)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0xA7: //goto
			{
				jumpOffset(readInt16() - 3);
				break;
			}

			case 0xA8: //jsr
			{
				int offset = readInt16();
				f->pushInt(*pPC);
				jumpOffset(offset - 3);

				break;
			}

			case 0xA9: //ret
			{
				*pPC = f->getLocalInt(wide ? readUInt16() : readUint8());
				wide = false;
				break;
			}

			case 0xAA: //tableswitch
			{
				int pc = *pPC - 1;

				skipPadding();
				int defaultOffset = readInt32();
				int low = readInt32();
				int high = readInt32();

				int* pArr = (int*)(codeData + *pPC);
				
				int offset = 0;
				int v = f->popInt();
				if ((v >= low) && (v <= high))
				{
					offset = toInt32((uint8*)&pArr[v - low]);
				}
				else
				{
					offset = defaultOffset;
				}

				*pPC = pc + offset;
				break;
			}

			case 0xAB: //lookupswitch
			{
				int pc = *pPC - 1;

				skipPadding();
				int defaultOffset = readInt32();
				int n = readInt32() * 2;

				int* pArr = (int*)(codeData + *pPC);

				int offset = 0;
				int v = swapInt(f->popInt()), i = 0;
				for (i = 0; i < n; i += 2)
				{
					if (pArr[i] == v)
					{
						offset = toInt32((uint8*)&pArr[i + 1]);
						break;
					}
				}

				if (i >= n)
				{
					offset = defaultOffset;
				}

				*pPC = pc + offset;
				break;
			}

			case 0xAC: //ireturn
			{
				popFrame([](StackFrame* pRtn, StackFrame* pCurr)
				{
					pCurr->pushInt(pRtn->popInt());
				});

				prepare();
				break;
			}

			case 0xAD: //lreturn
			{
				popFrame([](StackFrame* pRtn, StackFrame* pCurr)
				{
					pCurr->pushLong(pRtn->popLong());
				});

				prepare();
				break;
			}

			case 0xAE: //freturn
			{
				popFrame([](StackFrame* pRtn, StackFrame* pCurr)
				{
					pCurr->pushFloat(pRtn->popFloat());
				});

				prepare();
				break;
			}

			case 0xAF: //dreturn
			{
				popFrame([](StackFrame* pRtn, StackFrame* pCurr)
				{
					pCurr->pushDouble(pRtn->popDouble());
				});

				prepare();
				break;
			}

			case 0xB0: //areturn
			{
				popFrame([](StackFrame* pRtn, StackFrame* pCurr)
				{
					pCurr->pushObject(pRtn->popObject());
				});

				prepare();
				break;
			}

			case 0xB1: //return
			{
				popFrame();
				prepare();
				break;
			}

			case 0xB2: //getstatic
			{
				auto fieldRef = pClass->getConstant<ConstantMemberRef>(readUInt16());
				auto classPtr = loadAndInit(fieldRef->getClassName()->c_str());

				if (!isThreadEnd())
				{
					auto nameType = fieldRef->getNameAndType();
					auto fieldValue = classPtr->getStaticField(*(nameType->getName()));

					f->pushJavaValue(fieldValue);
				}
				break;
			}

			case 0xB3: //putstatic
			{
				auto fieldRef = pClass->getConstant<ConstantMemberRef>(readUInt16());
				auto classPtr = loadAndInit(fieldRef->getClassName()->c_str());

				if (!isThreadEnd())
				{
					auto nameType = fieldRef->getNameAndType();
					auto fieldValue = classPtr->getStaticField(*(nameType->getName()));

					f->popToJavaValue(fieldValue);
				}
				break;
			}

			case 0xB4: //getfield
			{
				auto fieldRef = pClass->getConstant<ConstantMemberRef>(readUInt16());
				auto nameType = fieldRef->getNameAndType();
				auto objPtr = f->popObject();
				auto fieldValue = objPtr->getField(*(nameType->getName()));

				f->pushJavaValue(fieldValue);

				break;
			}
			case 0xB5: //putfield
			{
				int index = readUInt16();
				auto fieldRef = pClass->getConstant<ConstantMemberRef>(index);
				auto nameType = fieldRef->getNameAndType();
				int soltSize = JavaValue::soltSizeFromFieldDescriptor(*(nameType->getDescriptor()));
				if (soltSize == 1)
				{
					int32 v = f->popInt();
					auto objPtr = f->popObject();
					auto fieldValue = objPtr->getField(*(nameType->getName()));
					fieldValue->setIntValue(v);
				}
				else if (soltSize == 2)
				{
					int64 v = f->popLong();
					auto objPtr = f->popObject();
					auto fieldValue = objPtr->getField(*(nameType->getName()));
					fieldValue->setLongValue(v);
				}
				else
				{
					throw new std::exception("不支持的soltSize");
				}

				break;
			}
			case 0xB6: //invokevirtual
			{
				//虚函数调用
				//获取对象的this指针， 从里面找具体调用的函数
				auto methodRef = pClass->getConstant<ConstantMemberRef>(readUInt16());
				auto methodName(methodKey(methodRef->getNameAndType()));

				int argSoltcount = getArgSoltCount(methodRef->getNameAndType()->getDescriptor());
				auto objPtr = f->getObjectAt(argSoltcount);
				if (objPtr == nullptr)
				{
					throw new std::exception("NullPointerException");
				}

				auto method = objPtr->getClass()->getMethod(methodName);
				if (method == nullptr)
				{
					throw new std::exception("Method NullPointerException");
				}

				//hack println 看效果
// 				if (methodName == "println::(Ljava/lang/String;)V")
// 				{
// 					auto objPtr = f->getObjectAt(0);
// 					if (objPtr != nullptr)
// 					{
// 						auto charArr = dynamic_cast<JVMArray*>(objPtr->getField("value")->getObjectValue());
// 
// 						std::wstring str((wchar_t*)(charArr->getAddress(0)), charArr->getLength());
// 						wprintf(L"%s\n", str.c_str());
// 					}
// 					else
// 					{
// 						wprintf(L"null\n");
// 					}
// 				}

				auto nextFrame = pushFrame(method);

				prepare();
				break;
			}

			case 0xB7: //invokespecial
			case 0xB8: //invokestatic
			{
				//具体函数 指定类的函数 或 静态函数
				auto methodRef = pClass->getConstant<ConstantMemberRef>(readUInt16());
				auto methodName(methodKey(methodRef->getNameAndType()));
				auto classPtr = loadAndInit(methodRef->getClassName()->c_str());

				if (!isThreadEnd())
				{
					auto method = classPtr->getMethod(methodName);
					if (method == nullptr)
					{
						throw new std::exception("Method NullPointerException");
					}

					pushFrame(method);

					prepare();
				}
				break;
			}

			case 0xB9: //invokeinterface
			{
				//虚函数调用
				//获取对象的this指针， 从里面找具体调用的函数
				auto methodRef = pClass->getConstant<ConstantMemberRef>(readUInt16());
				auto methodName(methodKey(methodRef->getNameAndType()));

				int argSoltcount = readUint8();
				int zero = readUint8(); //固定为0
				auto objPtr = f->getObjectAt(argSoltcount - 1);
				if (objPtr == nullptr)
				{
					throw new std::exception("NullPointerException");
				}

				auto method = objPtr->getClass()->getMethod(methodName);
				if (method == nullptr)
				{
					throw new std::exception("Method NullPointerException");
				}

				pushFrame(method);
				prepare();
				break;
			}

			case 0xBA: //invokedynamic
			{
				throw new std::exception("不支持的指令");
				break;
			}

			case 0xBB: //new
			{
				auto classRef = pClass->getConstant<ConstantClassInfo>(readUInt16());
				auto classPtr = loadAndInit(classRef->getData()->c_str());

				if (!isThreadEnd())
				{
					f->pushObject(heap->alloc(classPtr));
				}
				break;
			}

			case 0xBC: //newarray 创建一个指定原始类型（如int, float, char…）的数组
			{
				JVMClass* clasPtr = nullptr;
				switch (readUint8())
				{
				case 4: //BOOLEAN
					clasPtr = pJVM->getClassLoader()->loadClass("[Z");
					break;

				case 5: //CHAR
					clasPtr = pJVM->getClassLoader()->loadClass("[C");
					break;

				case 6: //FLOAT
					clasPtr = pJVM->getClassLoader()->loadClass("[F");
					break;

				case 7: //DOUBLE
					clasPtr = pJVM->getClassLoader()->loadClass("[D");
					break;

				case 8: //BYTE
					clasPtr = pJVM->getClassLoader()->loadClass("[B");
					break;

				case 9: //SHORT
					clasPtr = pJVM->getClassLoader()->loadClass("[S");
					break;

				case 10: //INT
					clasPtr = pJVM->getClassLoader()->loadClass("[I");
					break;

				case 11: //LONG
					clasPtr = pJVM->getClassLoader()->loadClass("[J");
					break;

				default:
					throw new std::exception("不支持的数组类型");
					break;
				}
			
				f->pushObject(heap->allocArray(clasPtr, f->popInt()));
				break;
			}

			case 0xBD: //anewarray 创建一个对象数组
			{
				auto classRef = pClass->getConstant<ConstantClassInfo>(readUInt16());
				auto classPtr = loadAndInit(classRef->getData()->c_str());
				if (isThreadEnd())
				{
					break;
				}

				std::string arrClassName("[");
				arrClassName.append(*(classPtr->getName()));

				auto arrClassPtr = loadAndInit(arrClassName.c_str());
				if (isThreadEnd())
				{
					break;
				}

				f->pushObject(heap->allocArray(arrClassPtr, f->popInt()));
				break;
			}

			case 0xBE: //arraylength
			{
				auto objPtr = f->popObject();
				auto arrPtr = dynamic_cast<JVMArray*>(objPtr);
				if (arrPtr == nullptr)
				{
					throw new std::exception("NullPointerException");
				}

				f->pushInt(arrPtr->getLength());
				break;
			}

			case 0xBF: //athrow
			{
				auto exPtr = f->popObject();
				dispathException(exPtr);
				if (!isThreadEnd())
					prepare();
				break;
			}

			case 0xC0: //checkcast
			{
				int i = readUInt16();
				auto objPtr = f->popObject();
				if (objPtr != nullptr)
				{
					auto classRef = pClass->getConstant<ConstantClassInfo>(i);
					auto classPtr = loadAndInit(classRef->getData()->c_str());

					if (isThreadEnd())
					{
						break;
					}

					if (!objInstanceOf(objPtr, classPtr))
					{
						throw new std::exception("类型转换失败");
					}
				}
				f->pushObject(objPtr);
				break;
			}

			case 0xC1: //instanceof
			{
				int i = readUInt16();
				auto objPtr = f->popObject();
				if (objPtr != nullptr)
				{
					auto classRef = pClass->getConstant<ConstantClassInfo>(i);
					auto classPtr = loadAndInit(classRef->getData()->c_str());

					if (isThreadEnd())
					{
						break;
					}

					if (objInstanceOf(objPtr, classPtr))
					{
						f->pushInt(1);
					}
					else
					{
						f->pushInt(0);
					}
				}
				else
				{
					f->pushInt(0);
				}
				break;
			}

			case 0xC2: //monitorenter
				f->popObject();
				break; //单线程
			case 0xC3: //monitorenter
				f->popObject();
				break; //单线程
			case 0xC4: //wide
			{
				wide = true;
				break;
			}

			case 0xC5: //multianewarray
			{
				auto classRef = pClass->getConstant<ConstantClassInfo>(readUInt16());
				auto classPtr = loadAndInit(classRef->getData()->c_str());

				if (isThreadEnd())
				{
					break;
				}

				int w = readUint8();
				std::vector<int> arrLens;
				arrLens.resize(w);

				for (int i=0; i<w; i++)
				{
					arrLens[w-i-1] = (f->popInt());
				}

				f->pushObject(JVMObjectCreator::allocArray(classPtr, arrLens, 0));
				break;
			}

			case 0xC6: //ifnull
			{
				int offset = readInt16();
				if (f->popObject() == nullptr)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0xC7: //ifnonnull
			{
				int offset = readInt16();
				if (f->popObject() != nullptr)
				{
					jumpOffset(offset - 3);
				}
				break;
			}

			case 0xC8: //goto_w
			{
				int offset = readInt32();
				jumpOffset(offset - 5);
				break;
			}

			case 0xC9: //jsr_w
			{
				int offset = readInt32();
				f->pushInt(*pPC);
				jumpOffset(offset - 5);

				break;
			}

			case 0xCA: //breakpoint
			{
				throw new std::exception("不支持的指令");
				break;
			}

			case 0xFE: //impdep1
			{
				throw new std::exception("不支持的指令");
				break;
			}

			case 0xff: //impdep2
			{
				throw new std::exception("不支持的指令");
				break;
			}


			default:
				throw new std::exception("不支持的指令");
				break;
			}
		}
	}

	void JVMThread::dispathException(JVMObject *ex)
	{
		while (true)
		{
			auto frame = currentFrame();

			if (frame == nullptr)
			{
				//到了main函数外
				printf("未捕获的异常\n");
				setUnHandledException(ex);
				return;
			}
			else
			{
				auto method = frame->getMethod();

				if (method != nullptr && !method->isNative())
				{
					int pc = *(frame->getPC())  - 1;
					auto exTable = method->getCode()->getExceptions();

					for (auto it = exTable.begin(); it != exTable.end(); it++)
					{
						ClassFile::CodeExceptionTableData&  data = *it;
						if ((pc < data.startPC) || (pc >= data.endPC))
						{
							continue;
						}

						bool canCatch = false;

						if (data.catchType == 0) //catch all
						{
							canCatch = true;
						}
						else
						{
							auto classRef = method->getOwnerClass()->getConstant<ConstantClassInfo>(data.catchType);
							auto classPtr = justLoad(classRef->getData()->c_str());

							if (objInstanceOf(ex, classPtr))
							{
								canCatch = true;
							}
						}

						if (canCatch)
						{
							frame->clearStack();
							frame->pushObject(ex);
							*frame->getPC() = data.handlerPC;
							return;
						}
					}
				}

				//当前函数没有捕获代码，去下一个函数找找看
				popFrame();
			}
		}
	}

	void JVMThread::dispathException(JVMClass* exClass)
	{
		JVMObject *ex = HeapMemory::getHeap()->alloc(exClass);
		dispathException(ex);
	}

	JVMArray* JVMThread::fillStackTrace(JVMObject *ex, int dummy, bool fromNative)
	{
		int i = stacks.size() - 1;

		while (i >= 0)
		{
			//找到异常触发点
			Method* method = stacks[i]->getMethod();
			if (method->isNative() || (method->getName()->compare("<init>") == 0) || (method->getName()->compare("fillInStackTrace") == 0))
			{
				i -= 1;
			}
			else
			{
				break;
			}
		}

		int j = 0, n = 0;
		for (; j<=i; j++)
		{
			if (stacks[j]->getMethod() != nullptr)
				n++;
		}

		HeapMemory *heap = HeapMemory::getHeap();
		auto classPtr = loadAndInit("java/lang/StackTraceElement");
		JVMArray *arrPtr = heap->allocArray(loadAndInit("[java/lang/StackTraceElement"), n);

		auto fileName = JVMObjectCreator::allocString("unknown");
		
		j = 0;
		for (; i>=0; i--)
		{
			Method* method = stacks[i]->getMethod();
			if (method != nullptr)
			{
				auto objPtr = heap->alloc(classPtr);
				objPtr->getField("declaringClass")->setObjectValue(JVMObjectCreator::allocString(*(method->getOwnerClass()->getName())));
				objPtr->getField("methodName")->setObjectValue(JVMObjectCreator::allocString(*(method->getName())));
				objPtr->getField("fileName")->setObjectValue(fileName);

				if (method->isNative())
				{
					objPtr->getField("lineNumber")->setIntValue(-2);
				}
				else
				{
					objPtr->getField("lineNumber")->setIntValue(1);
				}

				*(arrPtr->geElementAddress<JVMObject*>(j++)) = objPtr;
			}
		}

		return arrPtr;
	}
}