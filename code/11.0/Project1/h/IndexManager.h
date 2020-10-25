#ifndef _INDEXMANAGER_H
#define _INDEXMANAGER_H

#include "Global.h"
#include "RecordeManage.h"
#include"BufferManage.h"
using namespace std;

//用于构建索引链表的结构体
struct IndexList
{
    long Index;                     // 索引值
    IndexList* Next;                // 用于构造链表，指向下一个链表的值
    Block* BlockPointer;            // 指向对应Block的指针
    IndexList():Next(NULL),BlockPointer(NULL){}
};

//B+树的节点
struct BPTNode{
    long Key;                       // 关键值
    bool LeafNode;                  // 是否是叶子结点
    int ChildNum;                   // 孩子节点个数
    int Layer;                      // 节点所在层数
    BPTNode* Parent;                // 祖先节点
    BPTNode* ChildPtr;              // 非叶子节点的指针，指向B+树起孩子节点
    IndexList* IndexPointer;        // 叶子结点使用的指针，指向索引链表 
    BPTNode* SiblingPtr;            // 指向它的兄弟结点
    BPTNode():Key(-1),Layer(-1),LeafNode(false),ChildNum(0),ChildPtr(NULL),Parent(NULL),IndexPointer(NULL),SiblingPtr(NULL){}
};

//B+树的
struct BPTNodeTitle{
    int NodeNum;                //处理的结点总数字
    int Layer;                  //该B+树的层数
    string BPT_IndexName;       //B+树的名称
    string BPT_DBname;          //对应的数据库
    string BPT_Table;           //对应的表
    string BPT_Attribute;       //对应的元组
    BPTNode* BPlusTree;         //整棵B+树
    BPTNodeTitle():NodeNum(0),Layer(0),BPlusTree(NULL){}
};

class IndexManager {
public:
    //在Buffer的基础上创建一棵B+树
    struct BPTNode* CreateTree( const string& IndexName,  const string& DBname,  const string& Table,  const string& Attribute, const BufferManage& BFM  );

    //删除某个指定的B+树
    bool DropTree( const string& IndexName );

    //插入数据，B+树的架构做相应的变化
    bool InsertRecord( const string& IndexName, const Block& e );

    //删除数据，B+树的架构做相应的变化
    bool DeleteRecord( const string& IndexName, const Block& e );

    //寻找单条查找记录所在的位置
    long FindRecord( const string& IndexName, const element& e );

    //查找树：存在返回下标，不存在返回-1
    long FindTree( const string& IndexName );

    //找到对应的叶子结点
    BPTNode* FindLeaf( const string& IndexName , long index );
private:
    vector<BPTNodeTitle> BPTLibrary;        // 生成的B+树(们)
};

void Visit(BPTNode* Tree, int Layer);       //打印整个B+树的结构
int LayerCalculate( int NodeNum );          //输入Block的大小与总结点数，返回所需搭建的B+树的层数

#endif
