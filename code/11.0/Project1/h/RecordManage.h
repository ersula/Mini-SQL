#pragma once
#include"BufferManage.h"
#include"Global.h"
#include<sstream>
#define EMPTY '\0'

typedef pair<int, int> tag;

class RecordManage {
public:
	BufferManage bm;

	void CreateTable(table &t);
	/*创建一个信息为RecordTable的表文件*/

	void DropTable(table &t);
	/*删除信息为RecordTable的表文件*/

	tag InsertRecord(table &t, vector<element>colomns);
	/*返回记录所在的bufferIndex和dataPosition，
	方便从内存中定位记录*/

	Data select(table &t);
	/*不带条件的查找，对表文件进行遍历，
	并对每条记录，根据选择属性名组，
	返回出对应的属性值。*/

	vector<element>select(table &t, tag T);
	/*根据tag找到记录并返回*/

	void Delete(table &t, tag T);
	/*根据tag找到记录进行删除*/

	void Delete(table &t);
	/*删除整个表的记录*/

	vector<tag> FindAllTag(table &t);

private:
	string transtostring(table &t, vector<element>colomns);
	/*将vector<element>转化为字符串方便放入缓存中*/

	vector <element> transtoelement(table &t, string str);
	/*从缓存中获取字符串然后转化为vector<element>给api*/

	int mallocBlockInTableFile(table &t);
	/*给数据文件开辟缓存*/
};