#include "wv_log.h"

int init_ok = 0;
const char* delimit = "/";

int wv_init_log(const char* dir_path, const char* file_name){

  int ret = 0;

  if (access(dir_path, F_OK) != 0){

    fprintf(stderr, "[ !! ] Initializing log was filed...( dir path : %s)", dir_path);
    init_ok = 0;
    ret = -1; goto wv_init_log; 
  }
  else{

    fprintf(stdout, "[ ** ] Initialized log path : %s%s%s)\n", dir_path, delimit, file_name);
    snprintf(log_dir_path, 8192, "%s", dir_path);
    snprintf(log_file_name, 8192, "%s", file_name);
    init_ok = 1;
  }

 wv_init_log:
  return ret;
}

const char* get_log_level_mark(int level){
  if ( level == LOG_INF ){
    return " ** ";
  }
  else if ( level == LOG_WRN ){
    return " ## ";
  }
  else if ( level == LOG_ERR ){
    return " !! ";
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

  char file_full_path[8192] = {0};
  char log_msg[8192] = {0};
  char log_full_msg[8192] = {0};
  va_list argptr;

  if (init_ok){
    time(&raw_time);
    time_info = localtime(&raw_time);

    /* Make the log file path. */
    snprintf(file_full_path, 8192, "%s%s%s", log_dir_path, delimit, log_file_name);


    /* Open the log file */
    if((pfile = fopen(file_full_path, "a+"))){

      /* Make the log message. */
      va_start(argptr, tmpl_msg);
      vsprintf(log_msg, tmpl_msg, argptr);
      va_end(argptr);

      /* Make the full log message. */
      snprintf(log_full_msg, 8192, "[%s] [%02d/%02d/%02d][%02d:%02d:%02d] %s\n",
	       get_log_level_mark(level),
	       time_info->tm_mon, time_info->tm_mday, time_info->tm_year+1900,
	       time_info->tm_hour, time_info->tm_min, time_info->tm_sec,
	       log_msg);

      /* Write log message. */
      fputs(log_full_msg, pfile);

      if (pfile) {
	fclose(pfile);
      }
    }
    else{
      fprintf(stderr, "[ !! ] Open the log file was filed...( path : %s)\n", file_full_path);
    }
  }
  else{
    fprintf(stderr, "[ !! ] Initializing log was filed...( path : %s)\n", file_full_path);
    ret = -1; goto wv_write_log_ret;
  }



 wv_write_log_ret:
  return ret;
}




