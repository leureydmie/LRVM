#ifndef __LIBRVM__
#define __LIBRVM__

#include <sys/stat.h>
#include "rvm_internal.h"

rvm_t rvm_init(const char*);
void *rvm_map(rvm_t, const char*, int);
void rvm_unmap(rvm_t, void*);
void rvm_destroy(rvm_t, const char*);
trans_t rvm_begin_trans(rvm_t, int, void**);
void rvm_about_to_modify(trans_t, void*, int, int);
void rvm_commit_trans(trans_t);
void rvm_abort_trans(trans_t);
void rvm_truncate_log(rvm_t);

#endif
