#ifndef _WV_LOG_H
#define _WV_LOG_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#define LOG_ENTRY fprintf( stderr, "%s(line %d): %s entry\n", \
                                   __FILE__, __LINE__, __FUNCTION__ )

#define LOG_EXIT  fprintf( stderr, "%s(line %d): %s exit\n", \
                                   __FILE__, __LINE__, __FUNCTION__ )

#define WRITE_LOG	0

#define LOG_INF		1
#define LOG_WRN		2
#define LOG_ERR 	3

char log_dir_path[8192]; 
char log_file_name[8192];
int init_ok;

int
wv_init_log(const char* dir_path, const char* file_name);

const char*
get_log_level_mark(int level);

int
wv_write_log(int level, const char* tmpl_msg, ...);

#endif
