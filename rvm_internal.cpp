#include "rvm_internal.h"

using namespace std;

/*************************/
/** 	Transaction		**/
/*************************/

transaction::transaction(int id, rvm_t vm)
{
	this->id = id;
	this->rvm = vm;
}
trans_t transaction::getId() { return this->id; }
rvm_t transaction::getRvm() { return this->rvm; }
bool transaction::equals(transaction target) { return(this->id == target.getId()); }


/*************************/
/** 		RVM_T		**/
/*************************/
rvm_t::rvm_t(const char* dir)
{
	strcpy(this->dir, dir);
	strcpy(this->log, dir);
	strcat(this->log, "/logfile");
}
char* rvm_t::getDir() { return this->dir; }
char* rvm_t::getLog() { return this->log; }
void rvm_t::setDir(const char* dir) { strcpy(this->dir, dir); }


/*************************/
/** 		seg			**/
/*************************/
seg::seg(const char* name, rvm_t vm)
{
	this->name = (char*) malloc(sizeof(char)*sizeof(name));
	strcpy(this->name, name);
	this->rvm = vm;
}
seg::seg(const char* name, rvm_t vm, int size): seg(name, vm)
{
	this->size = size;
	transaction* temp = new transaction(-1, vm);
	this->trans = *temp;
	char* dir = this->rvm.getDir();
	this->data = (void *)malloc(sizeof(char)*this->size);
	strcpy(this->path, dir);
	strcat(this->path, "/");
	strcat(this->path, this->getName());	
}
char* seg::getName() { return this->name; }
char* seg::getPath() { return this->path; }
rvm_t seg::getRvm() { return this->rvm; }
int seg::getSize() { return this->size; }
void* seg::getData() { return this->data; }
transaction seg::getTrans() { return this->trans; }
void seg::setTrans(transaction trans) { this->trans = trans; }
void seg::restoreFromLog()
{
	string line;
	char *ptr_name, *ptr_data, *segname, *offset, *data;
	
	cout << "Restoring " << this->getName() << endl;

	
	ifstream segLog;
	segLog.open(this->getPath());
	if (!segLog.is_open())
	{
		perror("Error opening file");
	}
	while (getline(segLog, line))
	{
		char * line_c = (char *)line.c_str();
		// Look for the same name as the segment.
		if ((segname = strtok_r(line_c, ":", &line_c)) != NULL)
		{
			if (strcmp(segname, this->getName()))
			{
				// Not equal so skip to next line
				continue;
			}
			
			// Look for the position and the data.
			while(offset = strtok_r(line_c, ":", &line_c))
			{
				ptr_data = NULL;
				offset = strtok_r(offset, "-", &ptr_data);
				if (offset != NULL)
				{
					int offset_num = atoi(offset);
					strtok_r(ptr_data, "-", &ptr_data);
					data = strtok_r(ptr_data, "-", &ptr_data);
					// Copy data to segment.
					if (data != NULL)
					{
						strcpy((char*)this->data + offset_num, data);
					}
					char * dummy = (char*)this->data + offset_num;
				}
			}
		}
	}
	segLog.close();
	
	
	ifstream log;
	log.open(this->getRvm().getLog());
	if (!log.is_open())
	{
		perror("Error opening file");
		return;
	}
	while (getline(log, line))
	{
		char * line_c = (char *)line.c_str();
		// Look for the same name as the segment.
		if ((segname = strtok_r(line_c, ":", &line_c)) != NULL)
		{
			if (strcmp(segname, this->getName()))
			{
				// Not equal so skip to next line
				continue;
			}
			
			
			// Look for the position and the data.
			while(offset = strtok_r(line_c, ":", &line_c))
			{
				ptr_data = NULL;
				offset = strtok_r(offset, "-", &ptr_data);
				if (offset != NULL)
				{
					int offset_num = atoi(offset);
					strtok_r(ptr_data, "-", &ptr_data);
					data = strtok_r(ptr_data, "-", &ptr_data);
					// Copy data to segment.
					if (data != NULL)
					{
						strcpy((char*)this->data + offset_num, data);
					}
					char * dummy = (char*)this->data + offset_num;
				}
			}
		}
	}
	log.close();
}

bool seg::equals(seg target) { return(strcmp(this->getName(),target.getName())); }


/*************************/
/** 	offset_t		**/
/*************************/
offset_t::offset_t(int value, int size, seg segment)
{
	this->value = value;
	this->size = size;
	this->segment = segment;
}
int offset_t::getValue() { return this->value; }
int offset_t::getSize() { return this->size; }
seg offset_t::getSegment() { return this->segment; }
