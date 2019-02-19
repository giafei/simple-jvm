#pragma once
class DataBlock
{
public:
	DataBlock(const uint8 *pData, int length, bool owner)
	{
		this->pData = pData;
		this->length = length;
		this->owner = owner;
	}

	~DataBlock()
	{
		if (owner)
		{
			if (pData != nullptr)
			{
				delete[] pData;
			}
		}
	}

public:
	const uint8* getData()
	{
		return pData;
	}

	const int getLength()
	{
		return length;
	}

private:
	const uint8 *pData;
	int length;
	bool owner;
};

