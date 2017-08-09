#include "wv_mem.h"

void* wv_malloc(wv_mem_hdr_t* mem_header, size_t size)
{
  if( mem_header ){
    if ( mem_header->mem_point ) {
      free(mem_header->mem_point);
      mem_header->size = 0;
    }
    mem_header->mem_point = malloc(sizeof(size));
    mem_header->size = size;

    return mem_header->mem_point;
  }
  else{
    fprintf(stderr, "[wv_mem] tried wv_malloc(), but wv_mem_header was null.\n");
  } 

  return NULL;
}


void* wv_realloc(wv_mem_hdr_t* mem_header, size_t size)
{
  int pre_size = 0;

  if( mem_header ){
    if( mem_header->mem_point ) {

      mem_header->mem_point =  realloc(mem_header->mem_point, pre_size + size);
      mem_header->size += size;

      return mem_header->mem_point;
    }
    else{
      fprintf(stderr, "[wv_mem] tried wv_realloc(), but the mem_point attribute was null.\n");
      return NULL;
    }
  }
  else {
    fprintf(stderr, "[wv_mem] tried wv_realloc(), but wv_mem_header was null.\n");
  }

  return NULL;
}


void wv_free(wv_mem_hdr_t* mem_header){
  if (mem_header && mem_header->mem_point){
    free(mem_header->mem_point);
    mem_header->size = 0;
  }
}
