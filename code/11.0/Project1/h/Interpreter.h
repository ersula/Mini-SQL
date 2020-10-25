#ifndef _Interpreter_H
#define _Interpreter_H

/*
Interpreter 模块直接与用户交互，主要实现以下功能： 
1. 程序流程控制，即“启动并初始化 ->‘接收命令、处理命令、显示命令结果’循环 -> 退出”流程。 
2. 接收并解释用户输入的命令，生成命令的内部数据结构表示，同时检查命令的语法正确性和部分语义正确性，
对正确 的命令调用 API 层提供的函数执行并显示执行结果，对不正确的命令显示错误信息。 
*/

#include <string>
#include "h/API.h"
#include "h/Exception.h"
using namespace std;

class Interpreter {
public:
	Interpreter();
	void getcmd();
	/*
	1. split it into several query if ';' exists;
	2. check syntax error: whether it ends with ';'
		error type: 
	3. analyse each query, delete the white space.
	*/

	void normalize();
	/* 
	in this function , we need to make the input query easy to split
	*/

	int process();
	/*
	1. analyse which type the query belongs to
		if not exists throw "command not exists"
	2. go into each function to process this query
	*/

	void sql_insert(stringstream &ss);
	/*
	format: insert into 表名 values(v1,v2...);
	check: table exists? attribute match? 
	*/

	void sql_delete(stringstream &ss);
	/*
	format: delete from 条件 where 表名；
	check: table exists? attribute match? 
	*/

	void sql_exit();
	/*
	exit
	*/

	void sql_execfile(string file_path);
	/*
	exec the file
	*/

	void sql_select(stringstream &ss);
	/*
	format: select 列名 from 表名 where 条件；
	check: table exists? attribute match?
	*/

	void create_table(stringstream &ss);
	/*
	create table 表名 
(  
	列名 类型() ,  
	列名 类型() ,    
	列名 类型() ,  
	primary key ( 列名 )
	); 
	*/

	void drop_table(stringstream &ss);
	/*
	drop table 表名；
	*/

	void create_index(stringstream &ss);
	/*
	create index 索引名 on 表名 ( 列名 ); 
	check: table exist? attribute exist? have index or not?
	*/

	void drop_index(stringstream &ss);
	/*
	drop index 索引 on 表名；
	check: attribute exist? table exist? index exist?
	*/

	element changeformat(string tablename, string target,int count);

private:
	string query;
	API api;
};

#endif