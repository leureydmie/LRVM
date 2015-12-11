#include "rvm.h"

static offset_d all_offset;

using namespace std;

rvm_t rvm_init(const char *directory)
{
	cout << "Initializing rvm" << endl;

	//Command to create the directory
	char cmd[DIR_LEN + 6];
	strcpy(cmd, "mkdir ");
	strcat(cmd, directory);

	//If the file does not exist, create it.
	struct stat file;
	if (stat(directory, &file) == -1) system(cmd);

	//Create the rvm object
	rvm_t rvm(directory);
	
	FILE* prout2 = NULL;
	prout2 = fopen("qwertyuk","w");
	fclose(prout2);
	
	ofstream log_1;
	log_1.open("rvm_segments/testse2g");
	log_1 << " ---------- ";
	log_1.flush();
	log_1.close();
	system("rm rvm_segments/testse2g");
	
	cout << "End Initializing rvm" << endl;
	
	return rvm;
}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create)
{
	cout << "Mapping the segment " << segname << endl;
	if (rvm.getDir() == NULL) perror("RVM directory was not set.");

	seg new_seg(segname, rvm, size_to_create);
	
	struct stat file;
	if (stat(rvm.getLog(), &file) != -1){
		new_seg.restoreFromLog();			
	}
	
	all_seg.push_back(new_seg);
	cout << "END Mapping the segment " << segname << endl;
	return new_seg.getData();
}

void rvm_unmap(rvm_t rvm, void *segbase)
{
	cout << "Unmapping segment" << endl;
	for (seg_d::iterator it = all_seg.begin(); it < all_seg.end(); it++)
	{
		if (it->getData() == segbase)
		{
			all_seg.erase(it);
			return;
		}
	}
	perror("Segment was not mapped");
	return;
}

void rvm_destroy(rvm_t rvm, const char *segname)
{
	cout << "Destroying segment " << segname << endl;
	seg seg_temp(segname, rvm);
	for (seg_d::iterator it = all_seg.begin(); it < all_seg.end(); it++)
	{
		if(strcmp(it->getName(), segname))
		{
			perror("Segment is currently mapped, cannot destroy");
			return;
		}
	}
	
	cout << "INTERM Destroying segment " << segname << endl;
	
	// Then the segment is currently not mapped, so destroy it.
	char* file_to_del = seg_temp.getPath();
	struct stat file;
	if (stat(file_to_del, &file) != -1)
	{
		char* cmd = "rm ";
		strcat(cmd, file_to_del);
		system(cmd);
	}
	cout << "END Destroying segment " << segname << endl;
}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases)
{
	cout << "Beginning new transaction" << endl;
	cout << numsegs << " segments involved" << endl;

	transaction new_trans(all_trans.size(), rvm);
	all_trans.push_back(new_trans);

	for (int i = 0; i < numsegs; i++)
	{
		seg_d::iterator it = all_seg.begin();
		while (it->getData() != segbases[i] && it <= all_seg.end()) it++;
		if (it == all_seg.end())
		{
			cout << "Segment not mapped" << endl;
			return -1;
		}
		cout << (int) it->getTrans().getId() << endl;
		if (it->getTrans().getId() == -1) it->setTrans(new_trans);
		else
		{
			cout << "Segment already involved in a transaction" << endl;
			return -1;
		}
	}
	cout << "END Beginning new transaction" << endl;
	return new_trans.getId();
}

void rvm_about_to_modify(trans_t id, void *segbase, int offset, int size)
{
	if((int)id < 0 )
	{
		cout << "About to modify return : id < 0" << endl;
		return;
	}
		
	transaction tid = all_trans[id];
	cout << "About to modify " << tid.getId() << " " << segbase << " " << offset << " " << size << endl;

	for (seg_d::iterator it = all_seg.begin(); it < all_seg.end(); it++)
	{
		if (it->getTrans().getId() == id /*&& it->getData() == segbase*/)
		{
			cout << "OFFSET!!" << endl;
			offset_t new_offset(offset, size, *it);
			all_offset.push_back(new_offset);
			cout << all_offset.front().getSize() << endl;
		}
	}
	cout << "END About to modify " << endl;
}

void rvm_commit_trans(trans_t id)
{
	if((int)id < 0 )
	{
		cout << "Commiting transaction return : id < 0" << endl;
		return;
	}
	
	transaction tid = all_trans[id];
	cout << "Commiting transaction " << tid.getId() << endl;
	ofstream logp;
	logp.open(tid.getRvm().getLog(), ios::out | ios::app | ios::binary);
	
	if (!logp.is_open())
	{
		perror("Error opening log");
		return;
	}
	for (seg_d::iterator it = all_seg.begin(); it < all_seg.end(); it++)
	{
		cout << it->getTrans().getId() << endl;
		if (it->getTrans().getId() == id)
		{
			char log_content[255];
			writeToFile(*it, &log_content);
			logp << log_content;
			cout << it->getName() << " has successfully been processed" << endl;
			// Now, make the segment unused.
			transaction temp (-1, it->getRvm());
			it->setTrans(temp);
		}
	}
	logp.flush();
	logp.close();
	cout << "END Commiting transaction " << endl;
}

void rvm_abort_trans(trans_t id)
{
	if((int)id < 0 )
	{
		cout << "Aborting transaction return : id < 0" << endl;
		return;
	}
	
	transaction tid = all_trans[id];
	cout << "Aborting transaction " << tid.getId() << endl;
	for (seg_d::iterator it = all_seg.begin(); it < all_seg.end(); it++)
	{
		if (it->getTrans().equals(tid))
		{
			it->restoreFromLog();
		}
	}
	cout << "END Aborting transaction " << endl;
}

void rvm_truncate_log(rvm_t rvm)
{
	cout << "Truncating log" << endl;
	
	ifstream log;
	log.open(rvm.getLog());
	ofstream seg_log;
	
	if (!log.good())
	{
		perror("Error opening log");
		return;
	}

	string line;
	while (getline(log, line))
	{
		// Find segname in the line
		cout << line.c_str() << endl;
		char segname[1024];
		sscanf(line.c_str(),"%[^:]:", segname);

		char* name = rvm.getDir();
		strcat(name, "/");
		strcat(name, segname);
		
		seg_log.open(name);
		if (!seg_log.good())
		{
			perror("Error opening file");
			return;
		}
		seg_log << line.c_str();
		seg_log.close();		
	}
	log.close();	
	
	ofstream log2;
	log2.open(rvm.getLog());
	log2.flush();
	log2.close();
	
	
	cout << "END Truncating log" << endl;
}

void writeToFile(seg segment, char data_to_write[][255])
{
	
	cout << "Writing " << segment.getName() << endl;
	strcat(*data_to_write, segment.getName());
	for (offset_d::iterator it = all_offset.begin(); it < all_offset.end(); it++)
	{
		char *temp1 = segment.getName();
		char *temp2 = it->getSegment().getName();
		int i = 0;
		int flag = 0;
		while(temp1[i] && temp2[i])
		{
			if(temp1[i] != temp2[i]) flag = !flag;
			i++;
		}
		if(flag == 0)
		{
			if(strcmp((char*) it->getSegment().getData() + it->getValue(), ""))
			{
				strcat(*data_to_write, SEPARATOR);
				stringstream s;
				s << it->getValue() << "-" << it->getSize() << "-" << ( (char*) it->getSegment().getData() + it->getValue() );
				char *temp = (char*) s.str().c_str();
				strcat(*data_to_write, temp);
			}
		}
	}
	strcat(*data_to_write, ":\n");
}
