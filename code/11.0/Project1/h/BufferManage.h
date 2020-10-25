#ifndef _BufferManage_H
#define _BufferManage_H

#define OneBlockSize 4096 //the size of one Block is 4KB
#define BlockNum 300//the max number of the blocks
#define EMPTY '\0'

#include<iostream>
#include<string>
#include<fstream>
using namespace std;

/*
	块中的基本信息：内存中
	存放信息的字符型数组、块号、脏数据位、锁
*/
class Block
{
public:
	string fileName;/*表示fileName文件的内存块*/
	bool Dirty;/*是否被修改*/
	bool isEmpty;/*是否为空*/
	bool Pin;/*是否被锁*/
	int  blockIndex;/*表示对应的fileName文件的块号*/
	int  LRU;/*表示未使用次数*/
	char data[OneBlockSize + 1];/*内存中存储的信息*/

	Block()/*重载函数*/
	{
		initialize();
	}

	void initialize()/*初始化*/
	{
		fileName = "";
		Dirty = false;
		isEmpty = true;
		Pin = false;
		blockIndex = 0;
		LRU = 0;
		for (int i = 0; i < OneBlockSize; i++) data[i] = EMPTY;
		data[OneBlockSize] = '\0';
	}

	string readBlockSeg(int start, int end)/*得到内存信息start-end位置的字符串*/
	{
		string content = "";
		for (int i = start; i < end; i++)
		{
			content += data[i];
		}
		return content;
	}
};

class BufferManage
{
public:
	Block bufferBlock[BlockNum];

	BufferManage();
	~BufferManage();

	void BufferFlush(int bufferIndex);
	/*将buffercache里的这一块写到文件中*/

	int GetBufferIndex(string fileName, int blockIndex);
	/*根据文件的名字和块号得到内存中的下标，返回缓存索引bufferIndex*/

	void BufferRead(string fileName, int blockIndex, int bufferIndex);
	/*从磁盘中读取信息存到内存中*/

	void RecordBuffer(int bufferIndex);
	/*记录buffer调用次数，用于LRU算法*/

	int GetEmptyBuffer();
	/*找到空闲的缓存索引*/

	int checkInBuffer(string fileName, int blockIndex);
	/*查找磁盘中该块是否已经写入了内存*/

	void BufferPin(int bufferIndex);
	/*将缓冲区锁住，使其不可被替换*/

	void deleteFile(string fileName);
	/*删除磁盘文件*/

	void writeToFile(int bufferIndex);
	/*将内存中的块写到文件中并不清空内存*/
};

#endif 
