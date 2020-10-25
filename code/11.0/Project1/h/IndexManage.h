#ifndef _INDEXMANAGE_H
#define _INDEXMANAGE_H

#include "Global.h"
#include"BufferManage.h"
#include "RecordManage.h"
using namespace std;
#define BranchNum 55

//B+树的节点
struct BPTBlock;

//B+树节点的一个小块
class BPTNode {
public:
	element Key;                    // 关键值
	long ChildPtr;                  // 非叶子节点的指针，指向B+树起孩子节点
	bool Valid;
	tag IndexNumber;
	BPTNode() :ChildPtr(-1), Valid(true) {
		tag InitialIndex;
		IndexNumber = InitialIndex;
	}
	BPTNode& operator=(const BPTNode& RHS) {
		this->ChildPtr = RHS.ChildPtr;
		this->IndexNumber = RHS.IndexNumber;
		this->Valid = RHS.Valid;
		this->Key = RHS.Key;
		return *this;
	}
	void Empty() {
		this->ChildPtr = -1;
		this->Valid = true;
		tag InitialIndex;
		this->IndexNumber = InitialIndex;
		this->Key.f = -1;
		this->Key.i = -1;
		this->Key.s = "";
		this->Key.type = -1;
	}
	void ClearKey() {
		this->Key.f = -1;
		this->Key.i = -1;
		this->Key.s = "";
		this->Key.type = -1;
	}
};

struct BPTBlock
{
	int Layer;                       // 层数
	int Number;                      // 孩子的个数
	long Parent;                     // 祖先节点
	BPTNode BPTList[BranchNum + 1];
	BPTBlock() :Number(0), Parent(-1), Layer(-1) {
		for (int i = 0; i < BranchNum + 1; i++)
		{
			BPTList[i].Empty();
		}
	}
};

//一棵B+树
struct BPlusTree
{
	int Head;                           //根结点的位置
	string IndexName;                   //索引树的名字，用于唯一标识特定的B+树
	string Table;                       //表的名字
	string Attribute;                   //索引数索引的属性
	vector<BPTBlock> BPTStack;         //B+树的上层
};

/*
	索引文件名：index.IndexName.Table.Attribute (每棵B+树是一个文件)
	索引文件的管理目录：IndexMenu
 */

class RecordeManage;
class IndexManager {
public:
	IndexManager() {};

	//RM
	RecordManage RM;

	//打印B+树
	void Visit(BPTBlock Tree, BPlusTree TESTINGTREE, int Layer);

	//在Buffer的基础上创建一棵B+树
	BPlusTree CreateTree(const string& IndexName, const string& Table, const string& Attribute, table& T);

	//删除某个指定的B+树
	bool DropTree(const string& IndexName);

	//插入数据，B+树的架构做相应的变化
	bool InsertRecord(const string& IndexName, element& e, tag offset);

	//删除数据，B+树的架构做相应的变化
	bool DeleteRecord(const string& IndexName, element& e);

	//寻找查找某一条记录所有相关的记录所在的位置（返回的是偏移量）
	vector<tag> FindRecord(const string& IndexName, element& e);

	//返回所有大于某个值的记录的索引(开区间)
	vector<tag> Greater(const std::string& indexName, element& e);

	//返回所有小于某个值的记录的索引(开区间)
	vector<tag> Less(const std::string& indexName, element& e);

	//将某棵B+树存回文件
	void BPT_To_File(const string& IndexName, BufferManage& BFM);

	//查找树：存在返回下标，不存在返回-1
	long FindTree(const string& IndexName);

	IndexManager(RecordManage& NRM) {
		RM = NRM;
	}
private:
	//范围访问
	vector<tag> GreaterVisit(BPTBlock Tree, vector<tag> List, element& e, BPlusTree TESTINGTREE);

	vector<tag> LessVisit(BPTBlock Tree, vector<tag> List, element& e, BPlusTree TESTINGTREE);

	vector<tag> EQVisit(BPTBlock Tree, vector<tag> List, element& e, BPlusTree TESTINGTREE);

	//管理的所有B+树
	vector<BPlusTree> BPlusTreeLibrary;
};

#endif
