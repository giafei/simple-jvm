#pragma once

class DataBlock;
class DataReader
{
public:
	DataReader(uint8 * data, int size);
	~DataReader();

public:
	int8 readInt8();
	uint8 readUint8();
	int16 readInt16();
	uint16 readUint16();
	int32 readInt32();
	uint32 readUint32();
	int64 readInt64();
	uint64 readUint64();
	std::shared_ptr<DataBlock> readPart(int len);

public:
	int remain()
	{
		return (size - position);
	}

private:
	template<typename T>
	T readSimple();

private:
	const uint8 * pData;
	int position;
	int size;
};

