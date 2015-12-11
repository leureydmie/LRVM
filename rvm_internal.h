#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <deque>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/stat.h>

//Define the maximum length of the directory string.
#define DIR_LEN 64
#define LINE_LEN 255

// Useful to serialize data
#define SEPARATOR ":"
#define COM "COMMIT"

typedef int trans_t;

class rvm_t
{
private:
	int id;
	char dir[DIR_LEN];
	char log[DIR_LEN + 8];
public:
	rvm_t(void) {};
	rvm_t(const char*);
	char* getDir(void);
	char* getLog(void);
	void setDir(const char*);
};

class transaction
{
private:
	trans_t id;
	rvm_t rvm;
public:
	transaction(void) {};
	transaction(int,rvm_t);
	trans_t getId(void);
	rvm_t getRvm(void);
	bool equals(transaction);
};

class seg
{
private:
	rvm_t rvm;
	char* name;
	char path[DIR_LEN];
	int size;
	transaction trans;
	void* data;
public:
	seg(void) {};
	seg(const char*, rvm_t);
	seg(const char*, rvm_t, int);
	char* getName(void);
	char* getPath(void);
	rvm_t getRvm(void);
	int getSize(void);
	void* getData(void);
	transaction getTrans(void);
	void setTrans(transaction);
	void restoreFromLog(void);
	bool equals(seg);
};

class offset_t
{
private:
	int value;
	int size;
	seg segment;
public:
	offset_t(int, int, seg);
	int getValue(void);
	int getSize(void);
	seg getSegment(void);
};

typedef std::deque<seg> seg_d;
typedef std::deque<offset_t> offset_d;

static seg_d all_seg;
static std::deque<transaction> all_trans;

void writeToFile(seg, char data_to_write[][255]);

