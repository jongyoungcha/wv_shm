#ifndef _WV_MEM_H_
#define _WV_MEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct wv_mem_hdr{
  void* mem_point;
  size_t size;
} wv_mem_hdr_t;

void* wv_malloc(wv_mem_hdr_t* mem_header, size_t size);

void* wv_realloc(wv_mem_hdr_t* mem_header, size_t size);

void wv_free(wv_mem_hdr_t* mem_header);


#endif
