#include "h/API.h"
#include "h/Exception.h"
#include <algorithm>
/*
recordemanager返回受影响的行数
sqlresponse 根据recorde调整
*/

//!!
//insert and create index

//求tag的交集

typedef pair<int, int> tag;


vector<tag> intersection(vector<tag> v1, vector<tag> v2) {
	vector<tag> v;
	sort(v1.begin(), v1.end());
	sort(v2.begin(), v2.end());
	set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(v));
	return v;
}

sqlresponse API::Createtable(string tablename, vector<attribute> attr) {

	if (cm.Hastable(tablename)) {
		cout << "cm create table:" << endl;
		throw table_exist(tablename);
	}
	else {
		clock_t start = clock();
		table mytable = cm.Createtable(tablename, attr);

		rm.CreateTable(mytable);
		clock_t end = clock();
		double elaspse = (end - start)*1.0 / 1000;
		return sqlresponse("create table",true, 0, elaspse);
	}
}

void API::printout(table mytable, vector<vector<element>> rows) {
	int length = mytable.AttrCount;
	cout << endl << endl;
	for (int i = 0; i < 100 ;i++)cout << "-";
	cout << endl;

	for (int i = 0; i < length; i++) {
		switch (mytable.attr[i].type)
		{
		case 0: //for (int j = 0; j < 10 - mytable.attr[i].name.length();j++)cout << " ";
			cout << mytable.attr[i].name << "\t\t\t";
			break;
		case 1: //for (int j = 0; j < 10 - mytable.attr[i].name.length(); j++)cout << " ";
			cout << mytable.attr[i].name << "\t\t\t";
			break;
		case 2: //for (int j = 0; j < mytable.attr[i].charnum - mytable.attr[i].name.length(); j++)cout << " ";
			cout << mytable.attr[i].name << "\t\t\t";
			break;

		default:
			break;
		}
	}


	cout << endl;
	for (int i = 0; i < rows.size(); i++) {
		for (int j = 0; j < length; j++) {
			switch (rows[i][j].type) {
				// 0: int, 1: double, 2: string, -1:invalid
			case 0: cout << rows[i][j].i << "\t\t"; break;
			case 1: cout << rows[i][j].f << "\t\t"; break;
			case 2: cout << rows[i][j].s << "\t\t"; break;
			}
		}
		cout << endl;
	}

	for (int i = 0; i < 100; i++)cout << "-";
	cout << endl;
	cout << endl << endl;
}


sqlresponse API::Droptable(string tablename) {
	if (cm.Hastable(tablename)) {
		clock_t start = clock();
		table mytable = cm.Gettable(tablename);

		for (int i = 0; i < mytable.attr.size(); i++) {
			if (mytable.attr[i].HasIndex == 1) {
				Dropindex(mytable.attr[i].Index);
			}
		}
		rm.DropTable(mytable);
		cm.Droptable(tablename);
		
		clock_t end = clock();
		double elapse = (end - start)*1.0 / 1000;
		return sqlresponse("drop table", true,0 , elapse);
	}
	else throw table_not_exist(tablename);
}

sqlresponse API::Insert(string tablename, vector<element>data) {

	if (cm.Hastable(tablename)) {
		clock_t start = clock();
		table mytable = cm.Gettable(tablename);

		
		//data conflict
		if (data.size() != mytable.AttrCount)throw format_error(tablename);

		for (int i = 0; i < mytable.AttrCount; i++) {
			if (mytable.attr[i].type != data[i].type)throw format_error(mytable.attr[i].name);
			if (mytable.attr[i].type == 2 && mytable.attr[i].charnum < data[i].s.length())throw format_error(mytable.attr[i].name);
		}


		//unique violation
		for (int i = 0; i < data.size(); i++) {
			if (mytable.attr[i].IsUnique || mytable.attr[i].Isprimary) {
				requirement require;
				require.renametable(mytable.tablename);
				limit temp(mytable.attr[i].name, 2, data[i]);
				require.limitation.push_back(temp);
				sqlresponse conflict = Select(require,0);

				if (conflict.rowaffected != 0)throw unique_conflict(mytable.attr[i].name);
			}
		}

		tag into = rm.InsertRecord(mytable, data);
		for (int i = 0; i < mytable.attr.size(); i++) {
			if (mytable.attr[i].HasIndex) {
				im.InsertRecord(mytable.attr[i].Index, data[i], into);
			}
		}

		mytable.writeout();

		clock_t end = clock();
		double elapse = (end - start)*1.0 / 1000;
		return sqlresponse("insert", true, 1 , elapse);
	}
	else throw table_not_exist(tablename);
}


bool eququalified(element source, element target, int operid) {
	switch (operid) {
	case 1://<
		return source < target;
	case 2://==
		return source == target;
	case 3://>
		return source > target;
	case 4://>=
		return source >= target;
	case 5://<=
		return source <= target;
	case 6://!=
		return source != target;
	default:
		cout << "unequalified!" << endl;
		throw syntax_error("exists unqualified operator!");
	}
}

int getattribute(table mytable, string attrname) {
	int found = 0;
	int j;
	for (j = 0; j < mytable.attr.size(); j++) {
		if (mytable.attr[j].name == attrname) return j;
	}
	return -1;
}


sqlresponse API::Select(requirement require,int print) {
	if (!cm.Hastable(require.tablename)) {
		throw table_not_exist(require.tablename);
	}

	clock_t start = clock();
	clock_t end;
	double duration = 0;

	Data rows;

	table mytable = cm.Gettable(require.tablename);

	if (require.limitation.size() == 0)
	{
		rows = rm.select(mytable);
		if(print == 1)printout(mytable, rows.rows);
		end = clock();
		duration = (end - start) *1.0 / 1000;
		return sqlresponse("select",true,rows.rows.size(),duration);
	}


	for (int i = 0; i < require.limitation.size(); i++) {
		element val = require.limitation[i].data;
		int id = getattribute(mytable, require.limitation[i].attrname);
		if(id == -1)throw syntax_error("no such attr exists!");
		else {
			if (mytable.attr[id].type == val.type);
			else throw data_type_conflict(mytable.attr[id].name);
		}
	}

	vector<tag>all = rm.FindAllTag(mytable);
	vector<tag>result = all;

	vector<tag>indexresult;
	vector<tag>first;
	vector<tag>second;

	vector<element>onerow;


	vector<int> mark;
	for (int i = 0; i < require.limitation.size(); i++) {
		mark.push_back(0);
	}

	for (int i = 0; i < require.limitation.size(); i++) {
		if (mark[i] == 0) {
			element val = require.limitation[i].data;
			int id = getattribute(mytable, require.limitation[i].attrname);
			if (mytable.attr[id].HasIndex) {
				string indexfile = mytable.attr[id].Index;
				switch (require.limitation[i].operid) {
				case 1://<
					indexresult = im.Less(mytable.attr[id].Index, val);
					break;
				case 2://==
					indexresult = im.FindRecord(mytable.attr[id].Index, val);
					break;
				case 3://>
					indexresult = im.Greater(mytable.attr[id].Index, val);
					break;
				case 4://>=
					first = im.Greater(mytable.attr[id].Index, val);
					second = im.FindRecord(mytable.attr[id].Index, val);
					indexresult.insert(indexresult.end(), first.begin(), first.end());
					indexresult.insert(indexresult.end(), second.begin(), second.end());
					break;
				case 5://<=
					first = im.Less(mytable.attr[id].Index, val);
					second = im.FindRecord(mytable.attr[id].Index, val);
					indexresult.insert(indexresult.end(), first.begin(), first.end());
					indexresult.insert(indexresult.end(), second.begin(), second.end());
					break;
				case 6://!=
					first = im.FindRecord(mytable.attr[id].Index, val);
					set_difference(all.begin(), all.end(), first.begin(), first.end(), back_inserter(indexresult));
					break;
				default:
					throw syntax_error(require.limitation[i].attrname + "'s operation not exists!");
				}
				result = intersection(indexresult, result);
				mark[i] = 1;
			}
		}
	}

	for (int i = 0; i < require.limitation.size(); i++) {
		if (mark[i] == 0) {
			element val = require.limitation[i].data;
			int id = getattribute(mytable, require.limitation[i].attrname);

			vector<tag>::iterator it;
			for (it = result.begin(); it != result.end();) {
				onerow = rm.select(mytable, *it);
				if (eququalified(onerow[id], val, require.limitation[i].operid))++it;
				else {
					it = result.erase(it);
				}
			}

			mark[i] = 1;
		}
	}

	for (int i = 0; i < result.size(); i++) {
		onerow = rm.select(mytable, result[i]);
		rows.rows.push_back(onerow);
	}

	if(print == 1)printout(mytable,rows.rows);
	end = clock();
	duration = (end - start) *1.0 / 1000;
	return sqlresponse("select",true, rows.rows.size(), duration);

}

sqlresponse API::Delete(requirement require) {
	if (!cm.Hastable(require.tablename)) {
		throw table_not_exist(require.tablename);
	}

	clock_t start = clock();
	clock_t end;
	double duration = 0;

	Data rows;
	table mytable = cm.Gettable(require.tablename);
	vector<tag>all = rm.FindAllTag(mytable);
	if (require.limitation.size() == 0)
	{
		// add delete all here;
		rm.Delete(mytable);
		end = clock();
		duration = (end - start) *1.0 / 1000;
		for (int i = 0; i < mytable.attr.size(); i++) {
			if (mytable.attr[i].HasIndex) {
				Dropindex(mytable.attr[i].Index);
			}
		}
		return sqlresponse("delete ", true,all.size(), duration);
	}


	for (int i = 0; i < require.limitation.size(); i++) {
		element val = require.limitation[i].data;
		int id = getattribute(mytable, require.limitation[i].attrname);
		if (id == -1)throw syntax_error("no such attr exists!");
		else {
			if (mytable.attr[id].type == val.type);
			else throw data_type_conflict(mytable.attr[id].name);
		}
	}

	
	vector<tag>result = all;
	vector<tag>indexresult;
	vector<tag>first;
	vector<tag>second;

	vector<element>onerow;

	vector<int> mark;
	for (int i = 0; i < require.limitation.size(); i++) {
		mark.push_back(0);
	}

	for (int i = 0; i < require.limitation.size(); i++) {
		if (mark[i] == 0) {
			element val = require.limitation[i].data;
			int id = getattribute(mytable, require.limitation[i].attrname);
			if (mytable.attr[id].HasIndex) {
				string indexfile = mytable.attr[id].Index;
				switch (require.limitation[i].operid) {
				case 1://<
					indexresult = im.Less(mytable.attr[id].Index, val);
					break;
				case 2://==
					indexresult = im.FindRecord(mytable.attr[id].Index, val);
					break;
				case 3://>
					indexresult = im.Greater(mytable.attr[id].Index, val);
					break;
				case 4://>=
					first = im.Greater(mytable.attr[id].Index, val);
					second = im.FindRecord(mytable.attr[id].Index, val);
					indexresult.insert(indexresult.end(), first.begin(), first.end());
					indexresult.insert(indexresult.end(), second.begin(), second.end());
					break;
				case 5://<=
					first = im.Less(mytable.attr[id].Index, val);
					second = im.FindRecord(mytable.attr[id].Index, val);
					indexresult.insert(indexresult.end(), first.begin(), first.end());
					indexresult.insert(indexresult.end(), second.begin(), second.end());
					break;
				case 6://!=
					first = im.FindRecord(mytable.attr[id].Index, val);
					set_difference(all.begin(), all.end(), first.begin(), first.end(), back_inserter(indexresult));
					break;
				default:
					throw syntax_error(require.limitation[i].attrname + "'s operation not exists!");
				}
				result = intersection(indexresult, result);
				mark[i] = 1;
			}
		}
	}

	for (int i = 0; i < require.limitation.size(); i++) {
		if (mark[i] == 0) {
			element val = require.limitation[i].data;
			int id = getattribute(mytable, require.limitation[i].attrname);

			vector<tag>::iterator it;
			for (it = result.begin(); it != result.end();) {
				onerow = rm.select(mytable, *it);
				if (eququalified(onerow[id], val, require.limitation[i].operid))++it;
				else {
					it = result.erase(it);
				}
			}
			mark[i] = 1;
		}
	}

	for (int i = 0; i < result.size(); i++) {
		onerow = rm.select(mytable, result[i]);
		for (int j = 0; j < mytable.attr.size(); j++) {
			if (mytable.attr[j].HasIndex) {
				im.DeleteRecord(mytable.attr[j].Index, onerow[j]);
			}
		}
		rm.Delete(mytable, result[i]);
	}
	mytable.writeout();
	end = clock();
	duration = (end - start) *1.0 / 1000;
	return sqlresponse("delete",true, result.size(), duration);
}

sqlresponse API::Createindex(string tablename, string indexname, string attrname) {
	if (cm.Hasindex(indexname)) throw index_exist(indexname);

	else {
		if (cm.Hastable(tablename)) {
			clock_t start = clock();
			table mytable = cm.Gettable(tablename);

			im.CreateTree(indexname, tablename, attrname, mytable);
			cm.Createindex(tablename, indexname, attrname);

			clock_t end = clock();
			double elaspse = (end - start)*1.0 / 1000;
			return sqlresponse("create index", true, 0, elaspse);
		}
		else throw table_not_exist(tablename);
	}
}

sqlresponse API::Dropindex(string indexname) {
	if (!cm.Hasindex(indexname))throw index_not_exist(indexname);

	clock_t start = clock();

	im.DropTree(indexname);
	cm.Dropindex(indexname);

	clock_t end = clock();
	double elapse = (end -start)*1.0 / 1000;
	return sqlresponse("drop index", true, 0, elapse);
}
