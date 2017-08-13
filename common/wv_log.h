#ifndef _WV_LOG_H
#define _WV_LOG_H

#include <stdio.h>
#include <string.h>
#include <wv_file.h>
#include <stdarg.h>
#include <time.h>

#define LOG_ENTRY fprintf( stderr, "%s(line %d): %s entry\n", \
                                   __FILE__, __LINE__, __FUNCTION__ )

#define LOG_EXIT  fprintf( stderr, "%s(line %d): %s exit\n", \
                                   __FILE__, __LINE__, __FUNCTION__ )

int wv_init_log(const char* dir_path, const char* file_name);
const char* get_log_level_mark(int level);
int wv_write_log(int level, const char* tmpl_msg, ...);

#endif
