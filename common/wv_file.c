#include "wv_file.h"
#include "wv_log.h"


char file_mode_ret[8] = {0};

const char*
wv_trans_mode_char (int file_mode)
{
  LOG_ENTRY;

  if(F_RDONLY == file_mode){
    strcpy(file_mode_ret, "r");
  }
  else if(F_WRONLY == file_mode){
    strcpy(file_mode_ret, "w");
  }
  else if(F_APND == file_mode){
    strcpy(file_mode_ret, "a");
  }
  else if(F_RDP == file_mode){
    strcpy(file_mode_ret, "a+");
  }
  else if(F_WRP == file_mode){
    strcpy(file_mode_ret, "w+");
  }
  else if(F_WRP == file_mode){
    strcpy(file_mode_ret, "a+");
  }
  else{
    strcpy(file_mode_ret, "");
    fprintf(stderr, "getting file mode is failed.... (file_mode : %d)", file_mode);
  }

  LOG_EXIT;
  return file_mode_ret;
}


int
wv_file_open (const char* file_path, wv_file_t* file_out, int file_mode)
{
  LOG_ENTRY;

  int ret = 0;

  if (file_out){
    if (NULL == (file_out->pfile = fopen(file_path, wv_trans_mode_char(file_mode)))) {
      fprintf(stderr, "wv_file_open was failed....");
      ret = -1;
      goto wv_file_open_ret;
    }
    else{
      snprintf(file_out->path, FILE_PATH_LENGTH, "%s", file_path);
      file_out->file_mode = file_mode;
    }
  }
 wv_file_open_ret:

  LOG_EXIT;
       
  return ret;
}


int
wv_file_is_exists (const char* file_path)
{
  int ret = 0;

  if(access(file_path, F_OK) != -1){
    ret = 1;
  }
  else{
    fprintf(stderr, "Couldn't find the path (%s)", file_path);
    ret = -1;
  }

  return ret;
}


int
wv_file_get_entries (const char* dir_path, wv_file_t** wv_file_t)
{
  LOG_ENTRY;

  int ret = 0;
  DIR* pdir = NULL;

  struct dirent *pentry;

  if (dir_path) {
    if (-1 == wv_file_is_exists(dir_path)) {
      fprintf(stderr, "Coundln't find the directory path");
      ret = -1;
      goto wv_file_get_entries_ret;
    }
  }

  pdir = opendir(dir_path);

  while ((pentry = readdir(pdir))) {
    printf("%s\n", pentry->d_name);
  }

 wv_file_get_entries_ret:

  LOG_EXIT;

  return ret;
}















