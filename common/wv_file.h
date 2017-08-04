#ifndef _FILE_H_
#define _FILE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>


#define FILE_PATH_LENGTH 8192

enum FILE_MODE {
  F_RDONLY,
  F_WRONLY,
  F_APND,
  F_RDP,
  F_WRP,
  F_APNDP
};

typedef struct wv_file{
  FILE* pfile;
  char path[FILE_PATH_LENGTH];
  int file_mode;
}wv_file_t;

const char* wv_trans_mode_char(int file_mode);

int wv_file_open(const char* file_path, wv_file_t* file_out, int file_mode);

int wv_file_is_exists(const char* file_path);

int wv_file_get_entries(const char* dir_path, wv_file_t** wv_file_t);


#endif
