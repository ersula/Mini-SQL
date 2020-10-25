#include "h/Interpreter.h"
#include <iostream>
#include <string>
using namespace std;

int main() {
	cout << "Hello, minisql." << endl;
	Interpreter ip;
	while (1) {
		ip.getcmd();
	}
	cout << "byebye, minisql." << endl;
	return 0;
}