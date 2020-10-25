#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <time.h>
#include<utility>
#include<set>
#include<string>
#include <fstream>
#include <cassert>

using namespace std;

#define _CHAR_ 2
#define _INT_ 0
#define _FLOAT_ 1


//即无论类型，统一用element返回结果
struct element {
	int i;
	float f;
	string s;
	int length;
	int type; // 0: int, 1: double, 2: string, -1:invalid
	//构造函数
	element() :i(-1), f(-1), s(""), type(-1) {};
	element(int x) :i(x), f(-1), s(""), type(0) {};
	element(float x) :i(-1), f(x), s(""), type(1) {};
	element(string x) :i(-1), f(-1), s(x), type(2) {};
	//重载运算符: < 1
	bool operator < (const element &RHS) const {
		switch (type)
		{
		case 0:
			return this->i < RHS.i;     break;
		case 1:
			return this->f < RHS.f;     break;
		case 2:
			//return this->s < RHS.s;     break;
			return strcmp(this->s.c_str(), RHS.s.c_str()) == -1; break;
		default:
			break;
		}
	}

	//重载运算符: == 2
	bool operator == (const element &RHS) const {
		switch (type)
		{
		case 0:
			return this->i == RHS.i;     break;
		case 1:
			return this->f == RHS.f;     break;
		case 2:
			//return this->s == RHS.s;     break;
			return strcmp(this->s.c_str(), RHS.s.c_str()) == 0; break;
		default:
			break;
		}
	}

	//重载运算符: > 3
	bool operator > (const element &RHS) const {
		switch (type)
		{
		case 0:
			return this->i > RHS.i;     break;
		case 1:
			return this->f > RHS.f;     break;
		case 2:
			//return this->s > RHS.s;     break;
			return strcmp(this->s.c_str(), RHS.s.c_str()) == 1; break;
		default:
			break;
		}
	}

	//重载运算符: >= 4
	bool operator >= (const element &RHS) const {
		switch (type)
		{
		case 0:
			return this->i >= RHS.i;     break;
		case 1:
			return this->f >= RHS.f;     break;
		case 2:
			//return this->s >= RHS.s;     break;
			return strcmp(this->s.c_str(), RHS.s.c_str()) >= 0; break;
		default:
			break;
		}
	}

	//重载运算符: <= 5
	bool operator <= (const element &RHS) const {
		switch (type)
		{
		case 0:
			return this->i <= RHS.i;     break;
		case 1:
			return this->f <= RHS.f;     break;
		case 2:
			//return this->s <= RHS.s;     break;
			return strcmp(this->s.c_str(), RHS.s.c_str()) <= 0; break;
		default:
			break;
		}
	}

	//重载运算符: != 6
	bool operator != (const element &RHS) const {
		switch (type)
		{
		case 0:
			return this->i != RHS.i;     break;
		case 1:
			return this->f != RHS.f;     break;
		case 2:
			//return this->s != RHS.s;     break;
			return strcmp(this->s.c_str(), RHS.s.c_str()) != 0; break;
		default:
			break;
		}
	}
};

/*记录数据*/
class Data {
public:
	vector<vector<element>> rows;
};

struct attribute {
	string name; //属性的名字
	int type; //属性的类型 // 0: int, 1: double, 2: string, -1:invalid
	int charnum; //如varchar(20)的20
	int Isprimary;
	int id;
	bool IsUnique; //属性是否unique
	long StartPosition; //属性在存储时开始的位置
	bool HasIndex; //是否已经有索引了
	string Index; //索引名

	attribute() {
		id = 0;
		Isprimary = 0;
		type = 0;
		charnum = 0;
		StartPosition = 0;
		IsUnique = false;
		HasIndex = false;
		Index = "";
		name = "";
	}
};

//表
class table {
public:
	string tablename; //表的名字
	string primarykeys; //表的主键
	int primaryid;
	int EntrySize; //Record长度
	int AttrCount; //属性数量
	int BlockCount;
	vector<attribute>attr; //属性数组

	table() {};

	table(const table &x) {
		tablename = x.tablename;
		primarykeys = x.primarykeys;
		primaryid = x.primaryid;
		EntrySize = x.EntrySize;
		AttrCount = x.AttrCount;
		BlockCount = x.BlockCount;
		attr = x.attr;
	};

	table(string name, vector<attribute> attri)
	{
		EntrySize = 0;
		tablename = name;
		BlockCount = 0;
		for (int i = 0; i < attri.size(); i++) {
			switch (attri[i].type)
			{
			case 0: {
				EntrySize += 10;
				break;
			}
			case 1: {
				EntrySize += 4;
				break;
			}
			case 2: {
				EntrySize += attri[i].charnum;
				break;
			}
			default:
				break;
			}
		}
		attr = attri;

		this->AttrCount = attr.size();
		for (int i = 0; i < attr.size(); i++)
			if (attr[i].Isprimary == 1) {
				this->primarykeys = attr[i].name;
				primaryid = i;
				break;
			}
	}

	
	void writeout() {
		ofstream output((tablename + ".cmtable").c_str());

		output << tablename << endl;
		output << primarykeys << endl;
		output << EntrySize << endl;
		output << AttrCount << endl;
		output << BlockCount << endl;

		for (int i = 0; i < attr.size(); i++) {
			output
				<< attr[i].name << " " <<attr[i].type << " "
				<< attr[i].charnum << " " << attr[i].IsUnique << " "
				<< attr[i].Isprimary << " " << attr[i].id << " "
				<< attr[i].StartPosition << " " << attr[i].HasIndex
				<< " " << attr[i].Index << endl;
		}
		output.close();
	}
	
	void getin(string table_name) {
		ifstream file((table_name + ".cmtable").c_str());
		file >> tablename >> primarykeys >> EntrySize >> AttrCount >> BlockCount;

		for (int i = 0; i < AttrCount; i++) {
			attribute oneattr;
			file >>oneattr.name;

			file >> oneattr.type >>
				oneattr.charnum >> oneattr.IsUnique >>
				oneattr.Isprimary >> oneattr.id >>
				oneattr.StartPosition;
			
			file >> oneattr.HasIndex;
			if(oneattr.HasIndex) file >> oneattr.Index;
			else oneattr.Index = "";

			this->attr.push_back(oneattr);
		}
		file.close();
	}

	/*
	friend ostream& operator<<(ostream& output, const table& obj)
	{
		output << obj.tablename << endl;
		output << obj.primarykeys << endl;
		output << obj.EntrySize << endl;
		output << obj.AttrCount << endl;
		for (int i = 0; i < obj.attr.size(); i++) {
			output
				<< obj.attr[i].name << " " << obj.attr[i].type << " "
				<< obj.attr[i].charnum << " " << obj.attr[i].IsUnique << " "
				<< obj.attr[i].Isprimary << " " << obj.attr[i].id << " "
				<< obj.attr[i].StartPosition << " " << obj.attr[i].HasIndex
				<< " " << obj.attr[i].Index << endl;
		}
		return output;
	}


	friend istream& operator>>(istream& input, table& obj)
	{
		input >> obj.tablename >> obj.primarykeys >> obj.EntrySize >> obj.AttrCount;

		for (int i = 0; i < obj.attr.size(); i++) {
			attribute attr;
			input >>
				attr.name >> attr.type >>
				attr.charnum >> attr.IsUnique >>
				attr.Isprimary >> attr.id >>
				attr.StartPosition >> attr.HasIndex
				>> attr.Index;
			obj.attr.push_back(attr);
		}
		return input;
	}
*/


};

struct limit {
public:
	string attrname;
	int operid;
	element data;
	limit(string attrname, int operid, element data) {
		this->attrname = attrname;
		this->operid = operid;
		this->data = data;
	}
};

class requirement {
public:
	string tablename;
	vector<limit> limitation;

	void renametable(string tablename) {
		this->tablename = tablename;
	}
};

class sqlresponse {
public:
	string op;
	bool success;
	int rowaffected;
	double time;
	sqlresponse() {
		success = false;
		rowaffected =  0;
		time = 0;
	}
	sqlresponse(string ope, bool suss, int row,double duration) {
		op = ope;
		success = suss;
		rowaffected = row;
		time = duration;
	}
	
	void output() {
		cout << op << " succeed. \t" << rowaffected << " rows affected. \t(" << time << "s)\t" << endl;
	}
	/*
	friend ostream& operator <<(ostream& output, const sqlresponse & obj)
	{
		output << obj.op << " succeed. \t" << obj.rowaffected << " rows affected. \t(" << obj.time << "s)\t" << endl;
		return output;
	}
	*/
};

#endif