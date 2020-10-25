#include "h/Interpreter.h"
#include "h/Global.h"
#include "h/Exception.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>

/*
query format and specification:
1. create table 表名 
(  
	列名 类型() ,  
	列名 类型() ,    
	列名 类型() ,  
	primary key ( 列名 )
	); 
	type definition: int float char
	special: unique,primary key

2.  drop table 表名；

3. create index 索引名 on 表名 ( 列名 ); 

4. drop index 索引名；

5. select
	select * from 表名 where 条件；

6. insert
	insert into 表名 values(v1,v2...);

7. delete
	delete from 条件 where 表名；

8. quit

9. execfile 文件名；
*/

using namespace std;


Interpreter::Interpreter() {
	Interpreter::query = "";
}


int checkid(vector<attribute> source, string target) {
	int i = 0;
	for (i = 0; i < source.size(); i++)
		if (source[i].name == target)return i;

	return -1;
}

/* normalize the use of white space for future split */
void Interpreter::normalize() {
	int i = 0;
	//make there exist blank space before and after '(' , ')', ','

	for (i = 0; i < query.length(); i++) {
		if (query[i] == '(' || query[i] == ')' || query[i] == ',') {
			query.insert(i, " ");
			query.insert(i + 2, " ");
			i = i + 2;
		}
	}
}


void Interpreter::getcmd() {
	string cmd;
	/*
	read in the cmd line: we assume that two query can not be merged to show in one line
	*/
	while (1) {
		cout << " >> ";
		getline(cin, cmd);
		Interpreter::query = Interpreter::query + cmd;
		if (cmd.length() >= 1) {
			if (cmd[cmd.length() - 1] == ';') {
				query.erase(query.end() - 1);
				query.erase(query.end());
				break;
			}
		}
	}

	Interpreter::normalize();
	Interpreter::process();

	Interpreter::query = "";
}


/*
1. split it into several query if ';' exists;
2. check syntax error: whether it ends with ';'
	error type:
*/
//change the string into an element
element Interpreter::changeformat(string tablename,string target,int id) {
	if (api.cm.Hastable(tablename)) {

		int isstring = target.find("'");
		if (isstring != -1) {
			string temp = target;
			if (temp[0] == '\'' && temp[temp.length() - 1] == '\'') {
				temp.erase(temp.begin());
				temp.erase(temp.end() - 1);
				element result(temp);
				return result;
			}
			else throw syntax_error(target);
		}


		int isfloat = target.find('.');
		if (isfloat != -1) {
			float num = atoi(target.substr(0, isfloat).c_str())*1.0;//the complete part
			num += atoi(target.substr(isfloat + 1, target.length() - 1).c_str())*1.0 / pow(10, target.substr(isfloat + 1, target.length()).length());
			//the not complete part

			element result(num);
			return result;
		}

		//int
		else {
			table mytable = api.cm.Gettable(tablename);
			int num;
			num = atoi(target.c_str());
				if (mytable.attr[id].type == 0) {
					element result(num); return result;
				}
				else {
					element result((float)(num)); return result;
				}

		}
	}
	else throw table_not_exist(tablename);
}


/*
1. analyse which type the query belongs to
	if not exists throw "command not exists"
2. go into each function to process this query
*/
int Interpreter::process() {
	
	stringstream ss;
	string operation, blank;
	ss << query;
	ss >> operation;
	try {
		if (operation == "quit")sql_exit();
		else if (operation == "execfile") {
			string file_path;
			ss >> file_path;
			sql_execfile(file_path);
		}
		else if (operation == "create") {
			string choice;
			ss >> choice;
			if (choice == "table") {
				create_table(ss);
			}
			else if (choice == "index") {
				create_index(ss);
			}
			else {
				cout << "you should only create type'table' and type 'index'" << endl;
				throw syntax_error(choice);
			}
		}
		else if (operation == "insert") {

			sql_insert(ss);
		}
		else if (operation == "drop") {
			string choice;
			ss >> choice;
			if (choice == "table") {
				drop_table(ss);
			}
			else if (choice == "index") {
				drop_index(ss);
			}
			else {
				cout << "you should only drop type'table' and type 'index'" << endl;
				throw syntax_error(choice);
			}
		}
		else if (operation == "select") {
			sql_select(ss);
		}
		else if (operation == "delete") {
			sql_delete(ss);
		}
		else {
			throw syntax_error(operation);
		}
	}
	catch (EXCEPTION e) {
		switch (e.name) {
		case e_unique_conflict:
			cout << "the attribute " + e.target + " is unique!" << endl;
			break;
		case e_not_unique:
			cout << "the index is created on a not unique attribute " + e.target << "!" << endl;
			break;
		case e_syntax_error:
			cout << "syntax error: error may exist around " + e.target << endl;
			break;
		case e_table_exist:
			cout << "table " + e.target + " has existed!" << endl;
			break;
		case e_table_not_exist:
			cout << "table " + e.target + " has not existed!" << endl;
			break;
		case e_attribute_not_exist:
			cout << "attribute " + e.target + " has not existed!" << endl;
			break;
		case e_index_exist:
			cout << "index " + e.target + " has existed!" << endl;
			break;
		case e_index_not_exist:
			cout << "index " + e.target + " has not existed!" << endl;
			break;
		case e_primary_key_conflict:
			cout << "primarykey does not exists! " << endl;
			break;

		case e_data_type_conflict:
			cout << "datatype  " + e.target + " has conflict!" << endl;
			break;

		case e_format_error:
			cout << "the input may have format error around " + e.target + " definition! please justify it." << endl;
			break;
		case e_invalid_indentifier:
			cout << "there exists an invalid indentifier around " + e.target << endl;
			break;
		}
		return 0;
	}

	return 1;
}



/*
format: insert into 表名 values(v1,v2...);
check: table exists? attribute match?
*/
void Interpreter::sql_insert(stringstream&ss) {
	string tablename, blank;
	ss >> blank;
	if (blank != "into") throw syntax_error(blank);
	ss >> tablename >> blank;
	if (blank != "values" && blank != "value") throw syntax_error(blank);
	ss >> blank;
	vector<element> input;
	string one;
	ss >> one;
	int count = 0;
	while (1) {
		if (one == ")")break;
		else if (one == ",");
		else {
			element temp = changeformat(tablename,one,count);
			count++;
			input.push_back(temp);
		}
		ss >> one;
	}

	api.Insert(tablename, input).output();

}


/*  exit   */
void Interpreter::sql_exit() {
	exit(0);
}

/*
format: delete from 条件 where 表名；
check: table exists? attribute match?
*/
void Interpreter::sql_delete(stringstream&ss) {
	string tablename, blank,attrname,operation, bound;
	ss >> blank >> tablename;
	if (blank != "from")throw syntax_error(blank);


	int switchop;
	requirement require;
	require.renametable(tablename);

	while (ss >> blank) {
			ss >> attrname >> operation >> bound;

				if (operation == "<")switchop = 1;
				else if (operation == "==")switchop = 2;
				else if (operation == ">")switchop = 3;
				else if (operation == ">=")switchop = 4;
				else if (operation == "<=")switchop = 5;
				else if (operation == "!=")switchop = 6;
				else {
					throw syntax_error(operation);
				}

				if (api.cm.Hastable(tablename)) {
					table mytable = api.cm.Gettable(tablename);
					int id = checkid(mytable.attr, attrname);
					if (id == -1)throw attribute_not_exist(attrname);

					limit temp(attrname, switchop, changeformat(tablename, bound, id));
					require.limitation.push_back(temp);
				}
				else throw table_not_exist(tablename);
			}

	api.Delete(require).output();
}


/*  exec the file  */
void Interpreter::sql_execfile(string file_path) {
	
	/*ifstream fs(file_path);
	if(!fs) cout << "file does not exist" << std::endl;
	else {
		std::string buf;
		while (getline(fin, buf)) {
			int len = buf.size();
			while (len && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
				len--;
			}
			if (process(buf.substr(0, len))) return true;
		}
	}*/
	int count = 0; 
	ifstream fs(file_path);
	if (!fs) {
		cout << " file not exist!" << endl;
		return;
	}
	string cmd;
	Interpreter::query = "";

	while (!fs.eof()) {
			getline(fs, cmd);
		
			Interpreter::query = Interpreter::query + cmd;
			if (cmd.length() != 0) {
				if (cmd[cmd.length() - 1] == ';') {
					query.erase(query.end() - 1);
					query.erase(query.end());

					cout << query << endl;

					Interpreter::normalize();
					if(Interpreter::process() == 1)count++;
					Interpreter::query = "";
				}
			}
		
	}
	cout << "execfile " << " succeed. \t" << count << " operations affected. \t("  << "s)\t" << endl;
}


/*
format: 
select * from 表名 where 条件；
select * from 表名
check: table exists? attribute match?
*/
void Interpreter::sql_select(stringstream&ss) {
	string tablename, blank, attrname, operation, bound;
	ss >> blank;
	if (blank != "*") throw syntax_error(blank);
	ss >> blank >> tablename;
	if (blank != "from") throw syntax_error(blank);
	//		*		from	tablename
	int switchop;
	requirement require;
	require.renametable(tablename);

	while (ss >> blank) {
			ss >> attrname >> operation >> bound;
				if (operation == "<")switchop = 1;
				else if (operation == "==")switchop = 2;
				else if (operation == ">")switchop = 3;
				else if (operation == ">=")switchop = 4;
				else if (operation == "<=")switchop = 5;
				else if (operation == "!=")switchop = 6;
				else {
					throw syntax_error(operation);
				}
				if (api.cm.Hastable(tablename)) {
					table mytable = api.cm.Gettable(tablename);
					int id = checkid(mytable.attr, attrname);
					if (id == -1)throw attribute_not_exist(attrname);
					limit temp(attrname, switchop, changeformat(tablename, bound, id));
					require.limitation.push_back(temp);
				}
				else throw table_not_exist(tablename);
		}
	
	api.Select(require,1).output();
}







/*
create table 表名
(
	列名 类型() ,
	列名 类型() ,
	列名 类型() ,
	primary key ( 列名 )
	);
	*/
void Interpreter::create_table(stringstream& ss) {
	string tablename, blank;

	bool hasprimary = false;
	ss >> tablename>> blank;
	if (blank != "(")throw syntax_error(blank);
	

	vector<attribute>attr;

	while (!ss.eof()) {
		string attrname;
		ss >> attrname;
		if (attrname == "" || attrname == ")")break;
		if (attrname == "primary") {
			//we assume that the primary key exist at the last row so all attr have been read in 
			ss >> blank >> blank;
			//		key		(
			string primarykey;
			ss >> primarykey;

			int id = checkid(attr, primarykey);
			if (id == -1)return;//the primary key doesnot exist
			else
			{
				attr[id].Isprimary = 1;
				hasprimary = true;
				break;
			}//all have been read in
		}
		
		
		//if the attrname is not "primary", then it is the name of a line
		string type;
		int length = 0;
		ss >> type;
		if (type == "char") ss >> blank >> length >> blank;

		string isunique;
		attribute oneattr;
		ss >> isunique;
		if (isunique == "unique") {
			oneattr.IsUnique = 1;
			ss >> blank;
		}
		//this is a ','
		else {
			oneattr.IsUnique = 0;
		}

		oneattr.id = attr.size();
		oneattr.name = attrname;

		if (type == "char") {
			if (length > 255 || length < 1)throw syntax_error(" char definition ");
			oneattr.type = _CHAR_;
			oneattr.charnum = length + 1;
			oneattr.Isprimary = 0;
		}
		else if (type == "int") {
			oneattr.type = _INT_;
			oneattr.charnum = 10;
			oneattr.Isprimary = 0;
		}
		else if (type == "float") {
			oneattr.type = _FLOAT_;
			oneattr.charnum = 4;
			oneattr.Isprimary = 0;
		}
		else {
			throw invalid_indentifier(type);
		}
		attr.push_back(oneattr);
	}


	if (hasprimary) {

		api.Createtable(tablename, attr).output();
	}
	else {
		throw primary_key_conflict();
	}
}


/*
create index 索引名 on 表名 ( 列名 );
check: table exist? attribute exist? have index or not?
*/
void Interpreter::create_index(stringstream& ss) {
	string indexname, tablename, attrname, blank;
	ss >> indexname >> blank;
	if (blank != "on") throw syntax_error(blank); 
	ss>> tablename >> blank >> attrname;
	if (blank != "(")throw syntax_error(blank);

	api.Createindex(tablename, indexname, attrname).output();
}



/*
drop table 表名；
*/
void Interpreter::drop_table(stringstream &ss) {
	string tablename;
	ss >> tablename;

	api.Droptable(tablename).output();
}

/*
drop index 索引 on 表名；
check: attribute exist? table exist? index exist?
*/
void Interpreter::drop_index(stringstream &ss) {
	string indexname;
	ss >> indexname;

	api.Dropindex(indexname).output();
}
