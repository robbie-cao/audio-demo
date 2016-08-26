#include <stdio.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <json-c/json.h>

#include "mqtt_hander.h"

struct MemoryStruct {
  char *memory;
  size_t size;
  json_object *msg;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  json_object *new_obj;
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  printf("GET RESPONSE:%s\r\n",(char *)contents);

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  new_obj = json_tokener_parse(mem->memory);

  json_object_object_del(mem->msg,"filename");
  json_object_object_add(mem->msg,"key",json_object_object_get(new_obj,"key"));

  printf("KEY:%s\r\n",json_object_get_string(json_object_object_get(new_obj,"key")));

  json_object_put(new_obj);

  free(mem->memory);
  MQTT_Message_Send(mem->msg);

  return realsize;
}

int voice_Upload_Service(json_object *msg)
{
  CURL *curl;
  CURLcode res;
  struct stat file_info;
  double speed_upload, total_time;
  FILE *fd;

  struct MemoryStruct chunk;

  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */
  chunk.msg = msg;

  fd = fopen(json_object_get_string(json_object_object_get(msg,"filename")), "rb"); /* open file to upload */
  if(!fd) {

    return 1; /* can't continue */
  }

  /* to get the file size */
  if(fstat(fileno(fd), &file_info) != 0) {

    return 1; /* can't continue */
  }

  curl = curl_easy_init();
  if(curl) {
    /* upload to this place */
    curl_easy_setopt(curl, CURLOPT_URL,"http://test.muabaobao.com/record/upload");

    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    /* tell it to "upload" to the URL */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* set where to read from (on Windows you need to use READFUNCTION too) */
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);

    /* and give the size of the upload (optional) */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)file_info.st_size);

    /* enable verbose for easier tracing */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    }
    else {
      /* now extract transfer info */
      curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
      curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);

      fprintf(stderr, "Speed: %.3f bytes/sec during %.3f seconds\n",
              speed_upload, total_time);
    }
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return 0;
}
