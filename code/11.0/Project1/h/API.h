#ifndef _API_H
#define _API_H
/*
API 模块是整个系统的核心，其主要功能为提供执行 SQL 语句的接口，供 Interpreter 层调用。
该接口以 Interpreter 层解释生 成的命令内部表示为输入，根据 Catalog Manager 提供的信息确定执行规则，
并调用 Record Manager、Index Manager 和 Catalog Manager 提供的相应接口进行执行，后返回执行结果给 Interpreter 模块
*/
#include "h/CatalogManager.h"
#include "h/IndexManage.h"
#include "h/RecordManage.h"
#include <vector>
using namespace std;


class API {
public:
	API() {};
	sqlresponse Createtable(string tablename, vector<attribute> attr);
	sqlresponse Createindex(string tablename, string indexname,string attr);
	
	sqlresponse Droptable(string tablename);
	sqlresponse Dropindex(string indexname);

	sqlresponse Select(requirement require,int print);
	sqlresponse Insert(string tablename, vector<element> data);
	sqlresponse Delete(requirement require);

	
	void printout(table mytable, vector<vector<element>> rows);

	RecordManage rm;
	CatalogManager cm;
	IndexManager im;
};

#endif