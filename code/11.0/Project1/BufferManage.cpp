#include "h/BufferManage.h"

BufferManage::BufferManage()
{
	for (int bufferIndex = 0; bufferIndex < BlockNum; bufferIndex++)
	{
		bufferBlock[bufferIndex].initialize();
	}
}

BufferManage::~BufferManage()
{
	for (int bufferIndex = 0; bufferIndex < BlockNum; bufferIndex++)
	{
		BufferFlush(bufferIndex);
	}
}

void BufferManage::BufferFlush(int bufferIndex)
{
	if (!bufferBlock[bufferIndex].Dirty) return;

	const string fileName = bufferBlock[bufferIndex].fileName;
	fstream fout(fileName.c_str(), ios::in | ios::out);
	fout.seekp(OneBlockSize*bufferBlock[bufferIndex].blockIndex, fout.beg);
	fout.write(bufferBlock[bufferIndex].data, OneBlockSize);
	bufferBlock[bufferIndex].initialize();
	fout.close();
}

int BufferManage::checkInBuffer(string fileName, int blockIndex)
{
	for (int bufferIndex = 0; bufferIndex < BlockNum; bufferIndex++)
	{
		if (bufferBlock[bufferIndex].fileName == fileName && bufferBlock[bufferIndex].blockIndex == blockIndex)
		{
			return bufferIndex;
		}
	}
	return -1;
}

void BufferManage::BufferPin(int bufferIndex)
{
	bufferBlock[bufferIndex].Pin = true;
}

int BufferManage::GetEmptyBuffer()
{
	int bufferIndex = 0;
	int maxLRU = -1;

	for (int i = 0; i < BlockNum; i++)
	{
		if (bufferBlock[i].isEmpty)
		{
			bufferBlock[i].initialize();
			bufferBlock[i].isEmpty = false;
			bufferIndex = i;
			return  bufferIndex;
		}
		else if (bufferBlock[i].Pin == false)
		{
			if (maxLRU < bufferBlock[i].LRU)
			{
				maxLRU = bufferBlock[i].LRU;
				bufferIndex = i;
			}
		}
	}
	BufferFlush(bufferIndex);
	bufferBlock[bufferIndex].isEmpty = false;
	RecordBuffer(bufferIndex);
	return bufferIndex;
}

void BufferManage::BufferRead(string fileName, int blockIndex, int bufferIndex)
{
	bufferBlock[bufferIndex].fileName = fileName;
	bufferBlock[bufferIndex].blockIndex = blockIndex;
	bufferBlock[bufferIndex].isEmpty = false;
	bufferBlock[bufferIndex].Dirty = true;

	fstream fin(fileName.c_str(), ios::in);
	fin.seekp(OneBlockSize*blockIndex, fin.beg);
	fin.read(bufferBlock[bufferIndex].data, OneBlockSize);
	fin.close();
	RecordBuffer(bufferIndex);
}

int BufferManage::GetBufferIndex(string fileName, int blockIndex)
{
	int bufferIndex = checkInBuffer(fileName, blockIndex);
	if (bufferIndex == -1)
	{
		bufferIndex = GetEmptyBuffer();
		BufferRead(fileName, blockIndex, bufferIndex);
	}
	return bufferIndex;
}

void BufferManage::RecordBuffer(int bufferIndex)
{
	for (int i = 0; i < BlockNum; i++)
	{
		if (i != bufferIndex)
		{
			bufferBlock[i].LRU++;
		}
		else
		{
			bufferBlock[i].LRU = 0;
		}
	}
}

void BufferManage::deleteFile(string fileName)
{
	for (int i = 0; i < BlockNum; i++)
	{
		if (bufferBlock[i].fileName == fileName)
		{
			bufferBlock[i].isEmpty = true;
			bufferBlock[i].Dirty = false;
		}
	}
}

void BufferManage::writeToFile(int bufferIndex)
{
	if (!bufferBlock[bufferIndex].Dirty) return;
	const string fileName = bufferBlock[bufferIndex].fileName;
	fstream fout(fileName.c_str(), ios::in | ios::out);
	fout.seekp(OneBlockSize*bufferBlock[bufferIndex].blockIndex, fout.beg);
	fout.write(bufferBlock[bufferIndex].data, OneBlockSize);
	fout.close();
}
