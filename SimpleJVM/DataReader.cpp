#include "stdafx.h"
#include "DataReader.h"
#include "DataBlock.h"

DataReader::DataReader(uint8 * data, int size)
{
	this->pData = data;
	this->size = size;
	this->position = 0;
}


DataReader::~DataReader()
{
	delete[] pData;
}

template<typename T>
inline T swab(T value)
{
	uint8 *p = (uint8*)&value;
	int l = sizeof(value), e = sizeof(value) / 2;
	for (int i = 0; i < e; i++)
	{
		uint8 tmp = p[i];
		p[i] = p[l - i - 1];
		p[l - i - 1] = tmp;
	}

	return value;
}

template<typename T>
inline T DataReader::readSimple()
{
	const uint8 *p = pData + position;
	position += sizeof(T);

	if (sizeof(T) == 1)
		return *(T*)p;

	return swab(*(T*)p);
}

int8 DataReader::readInt8()
{
	return readSimple<int8>();
}

uint8 DataReader::readUint8()
{
	return readSimple<uint8>();
}


int16 DataReader::readInt16()
{
	return readSimple<int16>();
}

uint16 DataReader::readUint16()
{
	return readSimple<uint16>();
}

int32 DataReader::readInt32()
{
	return readSimple<int32>();
}

uint32 DataReader::readUint32()
{
	return readSimple<uint32>();
}

int64 DataReader::readInt64()
{
	return readSimple<int64>();
}

uint64 DataReader::readUint64()
{
	return readSimple<uint64>();
}

std::shared_ptr<DataBlock> DataReader::readPart(int len)
{
	if (len > 0)
	{
		uint8 *pBuf = new uint8[len];
		memcpy(pBuf, pData + position, len);
		position += len;

		return std::shared_ptr<DataBlock>(new DataBlock(pBuf, len, true));
	}
	else
	{
		return std::shared_ptr<DataBlock>(new DataBlock(nullptr, 0, false));
	}
}

