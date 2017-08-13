#include <wv_log.h>


#define WRITE_LOG	0

#define LOG_INFO	1
#define LOG_WRN		2
#define LOG_ERR 	3

char log_dir_path[8192] = {0};
char log_file_name[8192] = {0};

int wv_init_log(const char* dir_path, const char* file_name){

  int ret = 0;

  if (access(dir_path, F_OK) == 0){

    fprintf(stderr, "[ !! ] Initializing log was filed...( dir path : %s)", dir_path);
    ret = -1; goto wv_init_log; 
  }
  else{

    snprintf(log_dir_path, 8192, "%s", dir_path);
    snprintf(log_file_name, 8192, "%s", file_name);
  }

 wv_init_log:
  return ret;
}

const char* get_log_level_mark(int level){
  if ( level == LOG_INFO ){
    return " ** ";
  }
  else if ( level == LOG_WRN ){
    return " !! ";
  }
  else if ( level == LOG_ERR ){
    return " ## ";
  }
  else{
    return "    ";
  }
}

int wv_write_log(int level, const char* tmpl_msg, ...){

  int ret = 0;

  FILE* pfile = NULL;
  time_t raw_time;
  struct tm *time_info = NULL;
  const char* delimit = "/";

  char file_full_path[8192] = {0};
  char log_msg[8192] = {0};
  char log_full_msg[8192] = {0};
  va_list argptr;

  time(&raw_time);
  time_info = localtime(&raw_time);

  /* Make the log file path. */
  snprintf(file_full_path, 8192, "%s%s%s", log_dir_path, delimit, log_file_name);


  /* Open the log file */
  if ( (pfile = fopen(file_full_path, "a+")) ){

    /* Make the log message. */
    va_start(argptr, tmpl_msg);
    vsprintf(log_msg, tmpl_msg, argptr);
    va_end(argptr);

    /* Make the full log message. */
    snprintf(log_full_msg, 8192, "[%s] [%d/%d/%d] %s",
	     get_log_level_mark(level),
	     time_info->tm_mon, time_info->tm_mday, time_info->tm_year,
	     log_msg);

    /* Write log message. */
    fputs(log_full_msg, pfile);
  }
  else{
    fprintf(stderr, "[ !! ] Initializing log was filed...( path : %s)", file_full_path);
    ret = -1; goto wv_write_log_ret;
  }

  if (pfile) {
    fclose(pfile);
  }

 wv_write_log_ret:
  return ret;
}




