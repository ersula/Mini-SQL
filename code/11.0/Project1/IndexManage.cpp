#include "h/IndexManage.h"

#include <fstream>
#include <queue>
#include <algorithm>
#include <cstring>
#include <set>
#include <iostream>
using namespace std;

//将offset与结果相对应
struct OffsetMap
{
	tag offset;
	element record;
};



bool MapCompare(OffsetMap a, OffsetMap b) {
	return a.record < b.record;
}


//在Buffer的基础上创建一棵B+树 ok
BPlusTree IndexManager::CreateTree(const string& IndexName, const string& Table, const string& Attribute, table& T) {

	// <建立索引链+根节点初始化>
	BPlusTree TESTINGTREE;
	TESTINGTREE.IndexName = IndexName;
	TESTINGTREE.Table = Table;
	TESTINGTREE.Attribute = Attribute;
	// 这个数值是需要商榷的

	/*vector<long> Of_Set = RM.GetAllOffsets(T);*/
	vector<tag> TagList = RM.FindAllTag(T);

	vector<OffsetMap> Maplist;
	cout << "Tagsize " << TagList.size() << endl;
	int ProcessingSUM = TagList.size();

	queue<BPTBlock> TreeConstructionQueue;

	// 读取数据：将文件偏移量作为节点
	
	/*Data WholeData = RM.select(T);*/
	int AttriNumber = -1;
	for (int i = 0; i < T.attr.size(); i++)
	{
		if (T.attr[i].name == TESTINGTREE.Attribute)
		{
			std::cout << "Find: " << i << endl;
			AttriNumber = i;
			break;
		}
	}
	if (AttriNumber == -1) {
		std::cout << "Can not find data according to corresponding attribute." << endl;
		return TESTINGTREE;
	}

	std::cout << "Arrtibute: " << AttriNumber << " " << TESTINGTREE.Attribute << endl;

	//构造一个由offset到record的映射集合
	for (int i = 0; i < TagList.size(); i++)
	{
		OffsetMap A ;
		A.offset = TagList[i];
		A.record = RM.select(T, TagList[i])[AttriNumber];
		Maplist.push_back(A);
	}

	sort(Maplist.begin(), Maplist.end(), MapCompare);
	cout << "Maplist " << Maplist.size() << endl;
	//填充建树队列queue

	for (int i = 0; i < Maplist.size(); i++)
	{

		if (i % BranchNum == 0)
		{
			if (ProcessingSUM - i < BranchNum / 2 + 1)
			{
				int LeftNodeNum = (BranchNum + ProcessingSUM - i) / 2;
				BPTBlock NewBlockNode;
				NewBlockNode.Layer = 0;
				TreeConstructionQueue.back().Number = LeftNodeNum;
				for (int k = LeftNodeNum + 1; k <= BranchNum; k++)
				{
					NewBlockNode.BPTList[k - LeftNodeNum] = TreeConstructionQueue.back().BPTList[k];
					NewBlockNode.Number++;
					TreeConstructionQueue.back().BPTList[k].Empty();
				}
			}
			else {
				BPTBlock NewBlockNode;
				NewBlockNode.Layer = 0;
				TreeConstructionQueue.push(NewBlockNode);
			}
		}

		BPTNode Newnode;
		Newnode.IndexNumber = Maplist[i].offset;
		Newnode.Key = Maplist[i].record;

		if (TreeConstructionQueue.back().Number < BranchNum)
		{
			TreeConstructionQueue.back().Number++;
			TreeConstructionQueue.back().BPTList[TreeConstructionQueue.back().Number] = Newnode;
		}
		else {
			if(TreeConstructionQueue.back().Number > BranchNum)
			std::cout << "ERROR 1" << TreeConstructionQueue.back().Number << endl;
			break;
		}

	}

	// </建立索引链+根节点初始化>

	int ProcessingLayer = 0;
	//借助队列自底向上建立B+树

	while (TreeConstructionQueue.size() > 1)
	{
		BPTBlock SubTreeHead;
		SubTreeHead.Layer = ProcessingLayer + 1;
		SubTreeHead.Number = 0;

		//cout << "TEST 1" << endl;

		//每BranchNum个等高节点构建一棵子树
		for (int i = 0; i < BranchNum; i++)
		{
			if (!TreeConstructionQueue.size()) break;
			if (TreeConstructionQueue.front().Layer == ProcessingLayer)
			{
				BPTBlock Catch = TreeConstructionQueue.front();
				SubTreeHead.Number++;
				TESTINGTREE.BPTStack.push_back(Catch);
				SubTreeHead.BPTList[i].ChildPtr = TESTINGTREE.BPTStack.size() - 1;
				if (i != 0)SubTreeHead.BPTList[i].Key = Catch.BPTList[1].Key;
				//SubTreeHead.BPTList[i].Key = Catch.BPTList[0].Key;////////////////////////////////
				TreeConstructionQueue.pop();
			}
			else
			{
				ProcessingLayer++;
				break;
			}
		}

		//cout << "TEST 2" << endl;

		//施加判断条件：非根节点的孩子节点数大于[M/2]
		if (SubTreeHead.Number > 0 && TreeConstructionQueue.size() > 0 && SubTreeHead.Number * 2 < BranchNum && SubTreeHead.Layer == TreeConstructionQueue.back().Layer)
		{
			//cout << "TEST 2.1" << endl;
			int LeftNodeNum = (TreeConstructionQueue.back().Number + SubTreeHead.Number) / 2;
			int AddNodeNum = TreeConstructionQueue.back().Number - LeftNodeNum;
			TreeConstructionQueue.back().Number = LeftNodeNum;
			SubTreeHead.Number += AddNodeNum;
			/* 赋值 */

			//cout << "TEST 2.2" << endl;
			for (int i = SubTreeHead.Number; i >= AddNodeNum; i--)
			{
				//cout << "1" << endl;
				SubTreeHead.BPTList[i] = SubTreeHead.BPTList[i - AddNodeNum];
				//cout << "2: " << SubTreeHead.BPTList[i].ChildPtr << endl;
				if(SubTreeHead.BPTList[i].ChildPtr>=0)
				SubTreeHead.BPTList[i].Key = TESTINGTREE.BPTStack[SubTreeHead.BPTList[i].ChildPtr].BPTList[1].Key;
				//cout << "3" << endl;
				if (i >= SubTreeHead.Number) SubTreeHead.BPTList[i].ClearKey();
			}

			//cout << "TEST 2.3" << endl;
			for (int i = 0; i < AddNodeNum; i++)
			{
				SubTreeHead.BPTList[i] = TreeConstructionQueue.back().BPTList[i + LeftNodeNum];
				if (i != 0)SubTreeHead.BPTList[i].Key = TESTINGTREE.BPTStack[SubTreeHead.BPTList[i].ChildPtr].BPTList[1].Key;
				else {
					SubTreeHead.BPTList[i].ClearKey();
				}
				TreeConstructionQueue.back().BPTList[i + LeftNodeNum].Empty();
			}
			//cout << "TEST 2.4" << endl;
		}

		//cout << "TEST 3" << endl;
		if (SubTreeHead.Number > 0) TreeConstructionQueue.push(SubTreeHead);
	}
	//cout << "ERROR PASS" << endl;
	TESTINGTREE.BPTStack.push_back(TreeConstructionQueue.front());
	//统一维护父节点
	for (int i = TESTINGTREE.BPTStack.size() - 1; i >= 0; i--)
	{
		for (int j = 0; j < BranchNum; j++)
		{
			if (TESTINGTREE.BPTStack[i].BPTList[j].ChildPtr >= 0)
			{
				TESTINGTREE.BPTStack[TESTINGTREE.BPTStack[i].BPTList[j].ChildPtr].Parent = i;
			}
		}
	}

	TESTINGTREE.Head = TESTINGTREE.BPTStack.size() - 1;
	//PrintStack(TESTINGTREE);
	BPlusTreeLibrary.push_back(TESTINGTREE);
	return TESTINGTREE;
}

//从Library找到所需要的B+树 ok
long IndexManager::FindTree(const string& IndexName) {
	for (int i = 0; i < BPlusTreeLibrary.size(); i++)
	{
		if (BPlusTreeLibrary[i].IndexName == IndexName)
		{
			return i;
		}
	}
	return -1;
}

//删除指定的整个B+树  ok
bool IndexManager::DropTree(const string& IndexName) {
	BPlusTreeLibrary[FindTree(IndexName)].IndexName = "";
	return true;
}

//用于排序的比较函数 ok
bool comp(BPTNode a, BPTNode b) {
	if (a.Key.type == -1) return true;
	if (b.Key.type == -1) return false;
	return a.Key < b.Key;
}

bool Extracomp(BPTNode a, BPTNode b) {
	if (a.Key.type == -1) return false;
	if (b.Key.type == -1) return true;
	return a.Key < b.Key;
}

//插入数据，B+树的架构做相应的变化 ok

bool IndexManager::InsertRecord(const string& IndexName, element& e, tag offset) {
	/* 维护索引链 */
	if (FindTree(IndexName) < 0) return false;
	BPlusTree TESTINGTREE = BPlusTreeLibrary[FindTree(IndexName)];
	BPTBlock Tree = TESTINGTREE.BPTStack[TESTINGTREE.BPTStack.size() - 1];

	BPTNode InsertBPTNode;
	InsertBPTNode.IndexNumber = offset;
	InsertBPTNode.Key = e;

	//寻找插入位置
	long Target = -1;
	int LastPosition = -2;
	for (int i = 0; i < TESTINGTREE.BPTStack.size(); i++)
	{
		if (TESTINGTREE.BPTStack[i].Layer == 0)
		{
			if (TESTINGTREE.BPTStack[i].BPTList[1].Key > e)
			{
				Target = LastPosition;
				break;
			}
			LastPosition = i;
		}
	}
	/* 找到：非负数；比所有的都小：-2；比所有的都大：-1（原先数值） */
	if (Target == -1) Target = LastPosition;
	if (Target == -2) Target = 0;

	//对直接插入叶子结点的单独讨论
	if (TESTINGTREE.BPTStack[Target].Layer == 0 && TESTINGTREE.BPTStack[Target].Number < BranchNum)
	{
		int flag = 0;
		for (int i = 1; i <= BranchNum; i++)
		{
			if (TESTINGTREE.BPTStack[Target].BPTList[i].Key.type == -1) {
				flag = i;
				break;
			}
		}
		TESTINGTREE.BPTStack[Target].BPTList[flag] = InsertBPTNode;

		sort(TESTINGTREE.BPTStack[Target].BPTList, TESTINGTREE.BPTStack[Target].BPTList + flag + 1, comp);
		TESTINGTREE.BPTStack[Target].Number++;
		BPlusTreeLibrary[FindTree(IndexName)] = TESTINGTREE;


		return true;
	}

	//插入叶子结点但是插满了
	if (TESTINGTREE.BPTStack[Target].Layer == 0 && TESTINGTREE.BPTStack[Target].Number == BranchNum)
	{
		// 需要开空间然后一路向上
		BPTBlock NewBlock;
		NewBlock.BPTList[0] = InsertBPTNode;
		for (int i = 1; i <= BranchNum; i++)
		{
			NewBlock.BPTList[i] = TESTINGTREE.BPTStack[Target].BPTList[i];
		}
		sort(NewBlock.BPTList, NewBlock.BPTList + BranchNum + 1, comp);

		//先处理原节点
		TESTINGTREE.BPTStack[Target].Number = (1 + BranchNum) / 2;
		for (int i = 1; i <= BranchNum; i++)
		{
			if (i <= (1 + BranchNum) / 2)
			{
				TESTINGTREE.BPTStack[Target].BPTList[i] = NewBlock.BPTList[i - 1];
			}
			else {
				TESTINGTREE.BPTStack[Target].BPTList[i].Empty();
			}
		}
		//再处理新节点
		NewBlock.BPTList[0].Empty();
		NewBlock.Number = BranchNum + 1 - (1 + BranchNum) / 2;
		for (int i = 1; i <= BranchNum; i++)
		{
			if (i <= BranchNum + 1 - (1 + BranchNum) / 2)
			{
				NewBlock.BPTList[i] = NewBlock.BPTList[i + (1 + BranchNum) / 2 - 1];
			}
			else {
				NewBlock.BPTList[i].Empty();
			}
		}
		NewBlock.Layer = TESTINGTREE.BPTStack[Target].Layer;

		//压入B+树容器并维护这个“更新node”
		TESTINGTREE.BPTStack.push_back(NewBlock);

		InsertBPTNode.ChildPtr = TESTINGTREE.BPTStack.size() - 1;
		InsertBPTNode.Key = TESTINGTREE.BPTStack.back().BPTList[1].Key;

		//Target:维护循环

		Target = TESTINGTREE.BPTStack[Target].Parent;

		if (TESTINGTREE.BPTStack[Target].Parent < 0)
		{
			TESTINGTREE.BPTStack[Target].BPTList[BranchNum] = InsertBPTNode;
			TESTINGTREE.BPTStack[Target].Number++;
		}
	}

	while (TESTINGTREE.BPTStack[Target].Parent > 0)
	{
		// 可以直接插入
		if (TESTINGTREE.BPTStack[Target].Number < BranchNum)
		{
			int flag = 0;
			for (int i = 1; i <= BranchNum; i++)
			{
				/* 出了问题看一眼这里 */
				if (TESTINGTREE.BPTStack[Target].BPTList[i].Key.type == -1) {
					flag = i;
					break;
				}
			}
			TESTINGTREE.BPTStack[Target].BPTList[flag] = InsertBPTNode;

			sort(TESTINGTREE.BPTStack[Target].BPTList, TESTINGTREE.BPTStack[Target].BPTList + flag + 1, comp);
			TESTINGTREE.BPTStack[Target].Number++;
			break;

		}
		else if (TESTINGTREE.BPTStack[Target].Number == BranchNum)
		{
			// 需要开空间然后一路向上
			BPTBlock NewBlock;
			NewBlock.BPTList[0] = InsertBPTNode;
			for (int i = 1; i <= BranchNum; i++)
			{
				NewBlock.BPTList[i] = TESTINGTREE.BPTStack[Target].BPTList[i];
			}
			sort(NewBlock.BPTList, NewBlock.BPTList + BranchNum + 1, comp);
			//先处理原节点
			TESTINGTREE.BPTStack[Target].Number = (1 + BranchNum) / 2;
			for (int i = 1; i <= BranchNum; i++)
			{
				if (i < (1 + BranchNum) / 2)
				{
					TESTINGTREE.BPTStack[Target].BPTList[i] = NewBlock.BPTList[i];
				}
				else {
					TESTINGTREE.BPTStack[Target].BPTList[i].Empty();
				}
			}
			//再处理新节点
			NewBlock.BPTList[0].Empty();
			NewBlock.Number = BranchNum + 1 - (1 + BranchNum) / 2;
			for (int i = 0; i <= BranchNum; i++)
			{
				if (i <= BranchNum - (1 + BranchNum) / 2)
				{
					NewBlock.BPTList[i] = NewBlock.BPTList[i + (1 + BranchNum) / 2];
				}
				else {
					NewBlock.BPTList[i].Empty();
				}
				if (i == 0)
				{
					NewBlock.BPTList[i].ClearKey();
				}
			}
			NewBlock.Layer = TESTINGTREE.BPTStack[Target].Layer;

			//压入B+树容器并维护这个“更新node”
			TESTINGTREE.BPTStack.push_back(NewBlock);

			InsertBPTNode.ChildPtr = TESTINGTREE.BPTStack.size() - 1;
			InsertBPTNode.Key = TESTINGTREE.BPTStack.back().BPTList[1].Key;

			//Target:维护循环

			Target = TESTINGTREE.BPTStack[Target].Parent;

			if (TESTINGTREE.BPTStack[Target].Parent < 0)
			{
				TESTINGTREE.BPTStack[Target].BPTList[BranchNum] = InsertBPTNode;
				TESTINGTREE.BPTStack[Target].Number++;
				break;
			}
		}
	}

	//对涉及根结点的单独处理，将整棵树分成两半再合并成一棵树

	element Temp;
	Temp.type = TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[BranchNum].Key.type;
	if (TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[BranchNum].Key.type != -1 && TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[BranchNum].Key > Temp)
	{
		if (TESTINGTREE.BPTStack[TESTINGTREE.Head].Number > BranchNum)
		{
			BPTBlock NewBlock;
			BPTBlock NewROOT;
			//List
			Target = TESTINGTREE.Head;
			sort(TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList, TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList + TESTINGTREE.BPTStack[TESTINGTREE.Head].Number, comp);

			NewBlock.BPTList[0].Empty();
			NewBlock.Number = BranchNum + 1 - (1 + BranchNum) / 2;
			for (int i = 0; i <= BranchNum; i++)
			{
				if (i < (1 + BranchNum) / 2 )
				{
					NewBlock.BPTList[i] = TESTINGTREE.BPTStack[Target].BPTList[i + (1 + BranchNum) / 2];
				}
				else {
					NewBlock.BPTList[i].Empty();
				}
				NewBlock.BPTList[0].ClearKey();
			}

			for (int i = BranchNum - (1 + BranchNum) / 2 + 1; i <= BranchNum; i++)
			{
				TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[i].Empty();
			}

			NewROOT.BPTList[0].ChildPtr = TESTINGTREE.Head;
			NewROOT.BPTList[0].Key = TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[0].Key;

			//Number

			NewROOT.Number = 2;
			NewBlock.Number = (1 + BranchNum) / 2;
			TESTINGTREE.BPTStack[TESTINGTREE.Head].Number = BranchNum + 1 - (1 + BranchNum) / 2;

			//Layer

			NewBlock.Layer = TESTINGTREE.BPTStack[TESTINGTREE.Head].Layer;
			NewROOT.Layer = TESTINGTREE.BPTStack[TESTINGTREE.Head].Layer + 1;

			//Head更新

			TESTINGTREE.BPTStack.push_back(NewBlock);
			NewROOT.BPTList[1].ChildPtr = TESTINGTREE.BPTStack.size() - 1;
			NewROOT.BPTList[1].Key = TESTINGTREE.BPTStack[TESTINGTREE.BPTStack.size() - 1].BPTList[1].Key;

			TESTINGTREE.BPTStack.push_back(NewROOT);
			TESTINGTREE.Head = TESTINGTREE.BPTStack.size() - 1;

		}
		else {
		
			TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[TESTINGTREE.BPTStack[TESTINGTREE.Head].Number] = TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[BranchNum];

			TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[BranchNum].Empty();

			if(TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[0].ChildPtr >= 0) TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[0].Key = TESTINGTREE.BPTStack[TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[0].ChildPtr].BPTList[1].Key;
			
			sort(TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList, TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList + TESTINGTREE.BPTStack[TESTINGTREE.Head].Number+1,Extracomp);

			
			TESTINGTREE.BPTStack[TESTINGTREE.Head].BPTList[0].ClearKey();
		}

	}

	//统一维护父节点
	for (int i = TESTINGTREE.BPTStack.size() - 1; i >= 0; i--)
	{
		for (int j = 0; j < BranchNum; j++)
		{
			if (TESTINGTREE.BPTStack[i].BPTList[j].ChildPtr >= 0)
			{
				TESTINGTREE.BPTStack[TESTINGTREE.BPTStack[i].BPTList[j].ChildPtr].Parent = i;
				if (TESTINGTREE.BPTStack[i].BPTList[j].Key != TESTINGTREE.BPTStack[TESTINGTREE.BPTStack[i].BPTList[j].ChildPtr].BPTList[1].Key) {
					TESTINGTREE.BPTStack[i].BPTList[j].Key = TESTINGTREE.BPTStack[TESTINGTREE.BPTStack[i].BPTList[j].ChildPtr].BPTList[1].Key;
					if (j == 0)TESTINGTREE.BPTStack[i].BPTList[j].ClearKey();
				}
			}
		}
	}
	// std::cout << "B+树的节点插入";
	BPlusTreeLibrary[FindTree(IndexName)] = TESTINGTREE;

	std::cout << "Successfully insert the node" << endl;
	return true;
}

//删除数据，B+树的架构做相应的变化 ok
bool IndexManager::DeleteRecord(const string& IndexName, element& e) {
	if (FindTree(IndexName) < 0) return false;

	BPlusTree TESTINGTREE = BPlusTreeLibrary[FindTree(IndexName)];
	//std::cout << (OneBlockSize - sizeof(int) - 2 * sizeof(long)) / sizeof(BPTNode) - 1 << endl;

	int Block_x = -1, Node_y = -1;

	for (int i = 0; i < TESTINGTREE.BPTStack.size(); i++)
	{
		if (TESTINGTREE.BPTStack[i].Layer == 0) {
			for (int j = 0; j <= BranchNum; j++)
			{
				if ( TESTINGTREE.BPTStack[i].BPTList[j].Key.type >= 0 && TESTINGTREE.BPTStack[i].BPTList[j].Key == e && TESTINGTREE.BPTStack[Block_x].BPTList[Node_y].Valid == true)
				{
					TESTINGTREE.BPTStack[Block_x].BPTList[Node_y].Valid = false;
					return true;

					/*Block_x = i;
					Node_y = j;
					break;*/
				}
			}
		}
	}

	/*if (Block_x == -1 || Node_y == -1) {
		std::cout << "Did not find such a record!" << endl;
		return false;
	}

	if (TESTINGTREE.BPTStack[Block_x].BPTList[Node_y].Valid == false)
	{
		std::cout << "The record has been deleted yet!" << endl;
		return false;
	}
	TESTINGTREE.BPTStack[Block_x].BPTList[Node_y].Valid = false;

	//std::cout << "Position: " << Block_x << " " << Node_y << endl;

	BPlusTreeLibrary[FindTree(IndexName)] = TESTINGTREE;*/
	cout << "Can not found" << endl;
	return false;
}

vector<tag> IndexManager::EQVisit(BPTBlock Tree, vector<tag> List, element& e, BPlusTree TESTINGTREE) {
	
	if (List.size() > 0) return List;
	if (Tree.Layer == 0) {
		for (int j = 0; j <= BranchNum; j++)
		{
			if (Tree.BPTList[j].Key.type >= 0 && Tree.BPTList[j].Key == e && Tree.BPTList[j].Valid == true) {
				List.push_back(Tree.BPTList[j].IndexNumber);
				return List;
			}
		}
	}
	int NUM = Tree.Number;
	for (int i = 0; i < NUM; i++)
	{
		if (!(i != 0 && Tree.BPTList[i].Key.type < 0)) {
			int flag = 1;
			if ( i > 1 && Tree.BPTList[i].Key > e && Tree.BPTList[i-1].Key > e ) flag = 0;
			if ( i < NUM - 1 && Tree.BPTList[i+1].Key < e && Tree.BPTList[i].Key < e ) flag = 0;
			if (flag && Tree.BPTList[i].ChildPtr >= 0) {
				List = EQVisit(TESTINGTREE.BPTStack[Tree.BPTList[i].ChildPtr], List, e, TESTINGTREE);
			}
		}
	}
	return List;
}


//对单棵树返回其对应元的全部记录的索引 ok
vector<tag> IndexManager::FindRecord(const string& IndexName, element& e) {
	BPlusTree TESTINGTREE = BPlusTreeLibrary[FindTree(IndexName)];
	vector<tag> A;
	return EQVisit(TESTINGTREE.BPTStack[TESTINGTREE.Head], A, e, TESTINGTREE);
}

//将某棵B+树存回文件 ok 
void IndexManager::BPT_To_File(const string& IndexName, BufferManage& BFM) {
	BPlusTree TESTINGTREE = BPlusTreeLibrary[FindTree(IndexName)];
	stringstream ss;
	string str;
	ss << TESTINGTREE.Head;
	ss >> str;
	string FILENAME = "index." + TESTINGTREE.IndexName + "." + str + "." + TESTINGTREE.Table + "." + TESTINGTREE.Attribute;

	vector<int> BufferNumberList;

	for (int i = 0; i < TESTINGTREE.BPTStack.size(); i++)
	{
		int offset = 0;
		int Number = BFM.GetEmptyBuffer();
		Block NewBlock = BFM.bufferBlock[Number];
		NewBlock.fileName = FILENAME;
		memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].Layer, sizeof(TESTINGTREE.BPTStack[i].Layer));
		offset += sizeof(TESTINGTREE.BPTStack[i].Layer);
		memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].Number, sizeof(TESTINGTREE.BPTStack[i].Number));
		offset += sizeof(TESTINGTREE.BPTStack[i].Number);
		memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].Parent, sizeof(TESTINGTREE.BPTStack[i].Parent));
		offset += sizeof(TESTINGTREE.BPTStack[i].Parent);
		
		for (int j = 0; j < BranchNum + 1; j++)
		{
			memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].BPTList[j].ChildPtr, sizeof(TESTINGTREE.BPTStack[i].BPTList[j].ChildPtr));
			offset += sizeof(TESTINGTREE.BPTStack[i].BPTList[j].ChildPtr);
			memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].BPTList[j].Key.f, sizeof(TESTINGTREE.BPTStack[i].BPTList[j].Key.f));
			offset += sizeof(TESTINGTREE.BPTStack[i].BPTList[j].Key.f);
			memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].BPTList[j].Key.i, sizeof(TESTINGTREE.BPTStack[i].BPTList[j].Key.i));
			offset += sizeof(TESTINGTREE.BPTStack[i].BPTList[j].Key.i);
			memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].BPTList[j].Key.s, sizeof(TESTINGTREE.BPTStack[i].BPTList[j].Key.s));
			offset += sizeof(TESTINGTREE.BPTStack[i].BPTList[j].Key.s);
			memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].BPTList[j].Key.type, sizeof(TESTINGTREE.BPTStack[i].BPTList[j].Key.type));
			offset += sizeof(TESTINGTREE.BPTStack[i].BPTList[j].Key.type);
			memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].BPTList[j].Valid, sizeof(TESTINGTREE.BPTStack[i].BPTList[j].Valid));
			offset += sizeof(TESTINGTREE.BPTStack[i].BPTList[j].Valid);
			memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].BPTList[j].IndexNumber.first, sizeof(TESTINGTREE.BPTStack[i].BPTList[j].IndexNumber.first));
			offset += sizeof(TESTINGTREE.BPTStack[i].BPTList[j].IndexNumber.first);
			memcpy(NewBlock.data + offset, &TESTINGTREE.BPTStack[i].BPTList[j].IndexNumber.second, sizeof(TESTINGTREE.BPTStack[i].BPTList[j].IndexNumber.second));
			offset += sizeof(TESTINGTREE.BPTStack[i].BPTList[j].IndexNumber.second);
		}

		BufferNumberList.push_back(Number);
	}

	//读入
	for (int i = 0; i < BufferNumberList.size(); i++)
	{
		BFM.BufferFlush(BufferNumberList[i]);
	}
}


vector<tag> IndexManager::GreaterVisit(BPTBlock Tree, vector<tag> List, element& e, BPlusTree TESTINGTREE) {

	if (Tree.Layer == 0) {
		for (int j = 0; j <= BranchNum; j++)
		{
			if (Tree.BPTList[j].Key.type >= 0 && Tree.BPTList[j].Key > e && Tree.BPTList[j].Valid == true) {
				List.push_back(Tree.BPTList[j].IndexNumber);
			}
		}
		return List;
	}

	for (int i = 0; i < BranchNum; i++)
	{
		if(!( i != 0 && Tree.BPTList[i].Key.type < 0)) {
			if ((i == BranchNum-1) || (i >= 0 && Tree.BPTList[i].Key > e) || (i < BranchNum-1 && Tree.BPTList[i + 1].Key > e))
			List = GreaterVisit(TESTINGTREE.BPTStack[Tree.BPTList[i].ChildPtr], List, e, TESTINGTREE);
		}
	}
	return List;
}

//返回所有大于某个值的记录的索引(开区间)
vector<tag> IndexManager::Greater(const std::string& indexName, element& e) {
	BPlusTree TESTINGTREE = BPlusTreeLibrary[FindTree(indexName)];
	vector<tag> A;
	return GreaterVisit(TESTINGTREE.BPTStack[TESTINGTREE.Head], A, e, TESTINGTREE);
}


vector<tag> IndexManager::LessVisit(BPTBlock Tree, vector<tag> List, element& e, BPlusTree TESTINGTREE) {
	
	if (Tree.Layer == 0) {
		for (int j = 0; j <= BranchNum; j++)
		{
			if (Tree.BPTList[j].Key.type >= 0 && Tree.BPTList[j].Key < e && Tree.BPTList[j].Valid == true) {
				List.push_back(Tree.BPTList[j].IndexNumber);
			}
		}
		return List;
	}

	for (int i = 0; i < BranchNum; i++)
	{
		if (!(i != 0 && Tree.BPTList[i].Key.type < 0)) {
			if( (i == 0) || (i >= 0 && Tree.BPTList[i].Key < e) || (i >= 1 && Tree.BPTList[i-1].Key < e) )
			List = LessVisit(TESTINGTREE.BPTStack[Tree.BPTList[i].ChildPtr], List, e, TESTINGTREE);
		}
	}
	return List;
}

//返回所有大于某个值的记录的索引(开区间)
vector<tag> IndexManager::Less(const std::string& indexName, element& e) {
	BPlusTree TESTINGTREE = BPlusTreeLibrary[FindTree(indexName)];
	vector<tag> A;
	return LessVisit(TESTINGTREE.BPTStack[TESTINGTREE.Head], A, e, TESTINGTREE);
}
