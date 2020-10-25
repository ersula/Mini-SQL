#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <exception>
using namespace std;

typedef enum error {
	e_table_exist, e_table_not_exist, e_attribute_not_exist,
	e_index_exist, e_index_not_exist, e_primary_key_conflict,
	e_not_unique,  e_data_type_conflict, e_unique_conflict,
	e_syntax_error,e_format_error, e_invalid_indentifier
}type;

class EXCEPTION:public exception {
public:
	type name;
	string target;
};

class not_unique :public EXCEPTION {
public:
	not_unique(string attrname) {
		name = e_not_unique;
		target = attrname;
	}
};

class table_exist : public EXCEPTION {
public:
	table_exist(string tablename){
		name = e_table_exist;
		target = tablename;
	}
};

class table_not_exist : public EXCEPTION {
public:
	table_not_exist(string tablename){
		name = e_table_not_exist;
		target = tablename;
	}
};

class attribute_not_exist : public EXCEPTION {
public:
	attribute_not_exist(string attrname){
		name = e_attribute_not_exist;
		target = attrname;
	}
};

class index_exist : public EXCEPTION {
public:
	index_exist(string indexname) {
		name = e_index_exist;
		target = indexname;
	}
};

class index_not_exist : public EXCEPTION {
public:
	index_not_exist(string indexname){
		name = e_index_not_exist;
		target = indexname;
	}
};

class primary_key_conflict : public EXCEPTION {
public:
	primary_key_conflict() {
		name = e_primary_key_conflict;
	}
};

class data_type_conflict : public EXCEPTION {
public:
	data_type_conflict(string data) {
		name = e_data_type_conflict;
		target = data;
	}
};



class format_error : public EXCEPTION {
public:
	format_error() {
		name = e_format_error;
		target = "";
	}
	format_error(string input) {
		name = e_format_error;
		target = input;
	}
};

class unique_conflict :public EXCEPTION {
public:
	unique_conflict(string indexname){
		name = e_unique_conflict;
		target = indexname;
	}
};

class syntax_error:public EXCEPTION {
public:
	syntax_error(string input) {
		name = e_syntax_error;
		target = input;
	}
};

class invalid_indentifier :public EXCEPTION {
public:
	invalid_indentifier(string input) {
		name = e_invalid_indentifier;
		target = input;
	}
};


#endif