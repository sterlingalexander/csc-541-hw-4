// Homework4.cpp : Defines the entry point for the console application.
//

#define WIN_32_LEAN_AND_MEAN
#include <windows.h>
#include "filereader.h"
#include "str.h"
#include <iostream>

using std::cin;
using std::cout;

const int ROOT = 0;
const int NODE_SIZE = 12;

long find(filereader &index, int record);
int add(filereader &index, int to_insert);
void writeKey(filereader &index, int key);
void readNode(filereader &index, int &key, long &lp, long &rp);
long size(filereader &index);
void print(filereader &index);
void split(char lineinput[], string &command, int &key);

// From class materials
struct bst_node {
	int key;
	long lp;
	long rp;
};

struct offsets {
	long parent;
	long actual;
};

void main(int argc, char* argv[])  {

	string fname = argv[1];					// Get passed in args
	filereader index;						// Create filereader object
	char lineinput[32] = {};					// Console input string
	string command = "";					// String object to parse from command line
	int key = 0;							// Key to be added to index
	string token[2] = {};					// array of strings to hold tokens

	index.open(fname, 'w');					// Open file to truncate
	index.close();							// Close opened file
	index.open(fname, 'x');					// Open file in RW mode

/*	while ( lineinput != "test" )  {
		cin.getline(lineinput, 25);
		cout << "\t\t\t~~~~~~~~ " << lineinput << '\n';
		split(lineinput, command, key);
	}
*/
	while ( 1 )  {
		cin.getline(lineinput, 25);
		split(lineinput, command, key);
		if ( command == "add" )  {
//			cout << "ECHO:  " << key << '\n';
			add(index, key);
		}
		else if ( command == "find" )  {
//			cout << "ECHO:  " << key << '\n';
			find(index, key);
		}
		else if ( command == "print" )  {
			print(index);
		}
		else if ( command == "end" )  {
			exit(0);
		}
		else  {
			cout << "An error occurred, THIS SHOULD NEVER HAPPEN WITH WELL FORMED INPUT.\n\nTERMINATING\n\n";
			exit(1);
		}
	}
}

int add(filereader &index, int to_insert)  {
	
	long eof_offset = size(index);

	if ( eof_offset == 0 )  {
		writeKey(index, to_insert);
		return 1;
	}

	int key = 0;
	long lp = 0;
	long rp = 0;
	index.seek(ROOT, BEGIN);
	readNode(index, key, lp, rp);

	while ( 1 ) {
		if ( to_insert < key )  {
			if ( lp < 0 ) {
				index.seek(-8, CUR);
				index.write_raw( (char*) &eof_offset, sizeof(long) );
				writeKey(index, to_insert);
				return 1;
			}
			else {
				index.seek(lp, BEGIN);
				readNode(index, key, lp, rp);
			}
		}
		else if ( to_insert > key )  {
			if ( rp < 0 ) {
				index.seek(-4, CUR);
				index.write_raw( (char*) &eof_offset, sizeof(long) );
				writeKey(index, to_insert);
				return 1;
			}
			else {
				index.seek(rp, BEGIN);
				readNode(index, key, lp, rp);
			}
		}
	}
}

long find(filereader &index, int target)  {

//	cout << "IN FIND METHOD\n";
//	bst_node node;
	long offset = 0;
	int key = 0;
	long lp = 0;
	long rp = 0;

	index.seek(ROOT, BEGIN);

	while ( 1 ) {

//		cout << "\t\t\t________------------>Now looking at offset " << index.offset() << "<------------________"<< '\n';
		readNode(index, key, lp, rp);
		index.seek(-NODE_SIZE, CUR);
		if ( target < key )  {
			if ( lp > 0) {
			index.seek(lp, BEGIN);
			}
			else  {
				cout << "Record " << target << " does not exist.\n";
				return -1;
			}
		}
		else if ( target > key )  {
			if ( rp > 0 ) {
			index.seek(rp, BEGIN);
			}
			else  {
				cout << "Record " << target << " does not exist.\n";
				return -1;
			}
		}
		else  {
			cout << "Record " << key << " exists.\n";
			return index.offset();
		}
	}
}

void readNode(filereader &index, int &key, long &lp, long &rp)  {

	index.read_raw( (char*) &key, sizeof(int) );
	index.read_raw( (char*) &lp, sizeof(long) );
	index.read_raw( (char*) &rp, sizeof(long) );
}

void writeKey(filereader &index, int key)  {

	long minusone = -1;
	index.seek(0, END);
	index.write_raw( (char*) &key, sizeof(int));
	index.write_raw( (char*) &minusone, sizeof(long) );
	index.write_raw( (char*) &minusone, sizeof(long) );
}

long size(filereader &index)  {

	index.seek(0, END);
	return index.offset();
}

void print(filereader &index)  {

	cout << "METHOD STUB\n\n";
}

void split(char lineinput[], string &command, int &key)  {

//	cout << "ECHO:  " << lineinput << '\n';

	int delim = 0;
	string tmp = "";
	command = "";
	int size;

	for (size = 0; size < lineinput[size] != '\0'; size++)  {	// iterate string
//		cout << "Now parsing lineinput[" << size << "], which is valued at ==> " << lineinput[size] << '\n';
		if ( lineinput[size] == ' ')  {				// find delimiters
			delim = size;
//			cout << "\t\t=======> DELIMITER at " << size << '\n';
		}
	}

	if (delim == 0 )  {
//		cout << "\t\t====================>No DELIMITER\n";
		for (int j = 0; lineinput[j] != '\0'; j++)  {
			command += lineinput[j];
		}
		key = 0;
		return;
	}

	for ( int j = 0; j < delim; j++ )  {
		command += lineinput[j];
	}
	for ( int j = delim + 1; j < size; j++ )  {
		tmp += lineinput[j];
	}
	key = atoi(tmp);

//	cout << "Parsed command ==> " << command << '\n';
//	cout << "Parsed key ===> " << key << '\n';
}
