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

struct qobj {
	int key;
	long lp;
	long rp;
	long offset;
	qobj *prev;
	qobj *next;
};

long find(filereader &index, int record);
int add(filereader &index, int to_insert);
void writeKey(filereader &index, int key);
void readNode(filereader &index, int &key, long &lp, long &rp);
void readNode(filereader &index, qobj *read);
long size(filereader &index);
void print(filereader &index);
void split(char lineinput[], string &command, int &key);
void printQueue(qobj *head, qobj *tail, int count);
void addQueue(qobj *insert, qobj *head, qobj *tail);
void refillQueue(filereader &index, qobj *head, qobj *tail);
void printQueue(qobj *head, qobj* tail);

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
	LARGE_INTEGER freq;						// Var for clock frequency
	QueryPerformanceFrequency(&freq);		// get clock frequency to convert HPT to seconds
	LARGE_INTEGER tstart, tfinish, tdiff;	// Start time, Finish time, difference in time
	double ttotal = 0;						// Total recorded time
	int totalfinds = 0;						// Total number of times the find() routine is called
	double telapsed = 0;
	double resolution = 0;

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
			add(index, key);
		}
		else if ( command == "find" )  {
			totalfinds++;
			QueryPerformanceCounter(&tstart);						// get start time
			find(index, key);										// perform the find
			QueryPerformanceCounter(&tfinish);						// get the end time
			tdiff.QuadPart = tfinish.QuadPart - tstart.QuadPart;	// get the time difference
			telapsed = tdiff.QuadPart / (double) freq.QuadPart;	// convert to actual time
			ttotal += telapsed;
//			resolution = 1.0 / (double) freq.QuadPart;
//			printf("Your performance counter ticks %I64u times per second\n", freq.QuadPart);
//			printf("Resolution is %lf nanoseconds\n", resolution*1e9);
//			printf("Code under test took %lf sec\n", elapsedTime);
		}
		else if ( command == "print" )  {
			print(index);
		}
		else if ( command == "end" )  {
			cout << '\n';
			printf("Sum: %.6f\n", ttotal);
//			cout << "Sum: " << ttotal << '\n';
//			cout << "Avg: " << ttotal / totalfinds << '\n';
			printf("Avg: %.6f\n", ttotal / totalfinds);
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

void readNode(filereader &index, qobj *read)  {
	int offset = index.offset();
	int key = 0;
	long lp = 0;
	long rp = 0;	
	index.read_raw( (char*) &key, sizeof(int) );
	index.read_raw( (char*) &lp, sizeof(long) );
	index.read_raw( (char*) &rp, sizeof(long) );

	read->key = key;  read->lp = lp; read->rp = rp; read->offset = offset;
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

	qobj *head = new qobj;
	qobj *tail = new qobj;
	qobj *curr = new qobj;
	qobj *left = NULL;
	qobj *right = NULL;
	head->prev = NULL;
	head->next = tail;
	tail->next = NULL;
	tail->prev = head;
	index.seek(ROOT, BEGIN);
	int count = 0;

	readNode(index, curr);  // prime the queue
	addQueue(curr, head, tail);
	cout << "\n";

	while ( head->next != tail )  {
		count ++;
		printQueue(head, tail, count);
		refillQueue(index, head, tail);
	}
}

void printQueue(qobj *head, qobj* tail)  {
	
	qobj *curr = head->next;
	while (curr != tail)  {
		cout << curr->key << " " << curr->offset << "\n";
		curr = curr->next;
	}
}

void refillQueue(filereader &index, qobj *head, qobj *tail)  {

	qobj *mark = tail->prev;
	qobj *curr = head->next;
	qobj *left = new qobj;
	qobj *right = new qobj;
	qobj *delptr = NULL;

	do  {		// while our current pointer is not beyond our marker for this level
		if (curr->lp >= 0)  {
			index.seek(curr->lp, BEGIN);	// seek to left object position
			readNode(index, left);			// read information for left object
			addQueue(left,head, tail);
		}
		if (curr->rp >= 0)  {
			index.seek(curr->rp, BEGIN);	// seek to right object position
			readNode(index, right);			// read information for right object
			addQueue(right, head, tail);	// add right object to queue
		}
//		printQueue(head, tail);
		delptr = curr;					// get a pointer to object to delete
		head->next = curr->next;		// reset head to new next node
		curr = curr->next;				// move current node pointer
		if (delptr != mark)
			delete delptr;				// delete removed object
		left = new qobj;				// craete new objects to insert
		right = new qobj;				// create new objects to insert
	} while ( curr != mark->next );
}

void printQueue(qobj *head, qobj *tail, int count)  {

	qobj *curr = head->next;

	cout << count << ": ";
	while ( curr != tail )  {
		cout << curr->key << "/" << curr->offset << " ";
		curr = curr->next;
	}
	cout << '\n';
}

void addQueue(qobj *insert, qobj *head, qobj *tail)  {

	qobj *prev = tail->prev;
	insert->next = tail;
	insert->prev = tail->prev;
	tail->prev = insert;
	if (head->next == tail)  
		head->next = insert;
	else
		prev->next = insert;
}

void split(char lineinput[], string &command, int &key)  {

//	cout << "ECHO:  " << lineinput << '\n';

	int delim = 0;
	string tmp = "";
	command = "";
	long size;

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
