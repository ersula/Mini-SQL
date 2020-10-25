#ifndef _CatalogManager_H
#define _CatalogManager_H
#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <fstream>
#include <iostream>
#include <cassert>
#include <algorithm>
#include "h/Global.h"
#include "h/IndexManage.h"
#include "h/BufferManage.h"
/*
Catalog Manager 负责管理数据库的所有模式信息，包括： 
1. 数据库中所有表的定义信息，包括表的名称、表中字段（列）数、主键、定义在该表上的索引。 
2. 表中每个字段的定义信息，包括字段类型、是否唯一等。 
3. 数据库中所有索引的定义，包括所属表、索引建立在那个字段上等。 
	Catalog Manager 还必需提供访问及操作上述信息的接口，
供 Interpreter 和 API 模块使用。 
*/
using namespace std;

class cmIndex {
public:
	string tablename;
	string indexname;
	string attrname;

	cmIndex() {};
	cmIndex(string tablename, string indexname, string attrname) {
		this->tablename = tablename;
		this->indexname = indexname;
		this->attrname = attrname;
	}

	void writeout() {
		ofstream output;
		output.open((indexname + ".cmindex").c_str());
		output << tablename << " " <<indexname << " " << attrname << endl;
		output.close();
	}

	void getin(string indexname) {
		ifstream file;
		file.open((indexname + ".cmindex").c_str());
		file >> tablename >> indexname >> attrname;
		file.close();
	}
	/*friend ostream& operator<<(ostream& output, const cmIndex& obj)
	{
		output << obj.tablename << " " << obj.indexname << " " << obj.attrname << endl;
		return output;
	}

	friend istream& operator>>(istream& input, cmIndex& obj)
	{
		input >> obj.tablename >> obj.indexname >> obj.attrname;
		return input;
	}*/
};


class CatalogManager {
public:
	BufferManage buffer;


	table Createtable(const string &table_name, const vector<attribute> &attrs);
	void Droptable(const string &table_name);
	bool Hastable(const string &table_name);
	table Gettable(const string &table_name);
	
	
	bool Createindex(const string &table_name, const string &index_name, const string& attr_index);
	bool Dropindex(const string &index_name);
	bool Hasindex(const string &index_name);
	cmIndex Getindex(const string &index_name);

	bool Hasattribute(string table ,string attrname);
	attribute Getattribute(string table,string attrname);
};

#endif