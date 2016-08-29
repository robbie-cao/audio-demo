#include <stdio.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <json-c/json.h>

#include "mqtt_hander.h"
#include "lplay.h"

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

  printf("\r\nGET RESPONSE:%s\r\n",(char *)contents);

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  new_obj = json_tokener_parse(mem->memory);

  json_object_object_del(mem->msg,"filename");

  json_object_object_add(mem->msg,"key",json_object_new_string(json_object_get_string(json_object_object_get(new_obj,"key"))));

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

size_t getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream) {
       int r;
       long len = 0;

       /* _snscanf() is Win32 specific */
       // r = _snscanf(ptr, size * nmemb, "Content-Length: %ld\n", &len);
 r = sscanf(ptr, "Content-Length: %ld\n", &len);
       if (r) /* Microsoft: we don't read the specs */
              *((long *) stream) = len;

       return size * nmemb;
}

/* 保存下载文件 */
size_t wirtefunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
        return fwrite(ptr, size, nmemb, stream);
}

int voice_Download_Service(json_object *msg)
{
  //
//  CURL *curl;
//  CURLcode res = CURLE_GOT_NOTHING;
//  struct stat file_info;
//  double speed_upload, total_time;
//  FILE *fd;
//  long filesize =0;
  char remotepath[256];
  bzero(remotepath,256);
//  curl_off_t local_file_len = -1;
//
//  int use_resume = 0;
//
//  struct MemoryStruct chunk;
//
//  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
//  chunk.size = 0;    /* no data at this point */
//  chunk.msg = msg;
//
//  if(stat("/tmp/msg.wav", &file_info) == 0)
//    {
//       local_file_len =  file_info.st_size;
//       use_resume  = 1;
//    }
//
//  //fd = fopen(json_object_get_string(json_object_object_get(msg,"filename")), "ab+"); /* open file to upload */
//  fd = fopen("/tmp/msg.wav", "ab"); /* open file to upload */
//
//  if(!fd) {
//
//	return 1; /* can't continue */
//  }
//
//  /* to get the file size */
//  if(fstat(fileno(fd), &file_info) != 0) {
//
//	return 1; /* can't continue */
//  }
//
//
 sprintf(remotepath,"wget http://ocejshhhr.bkt.clouddn.com/%s exit-t 0 -O /tmp/msg.wav",json_object_get_string(json_object_object_get(msg,"key")));
//  json_object_put(msg);
//
//  curl = curl_easy_init();
//
//  if(curl) {
//	  curl_easy_setopt(curl, CURLOPT_URL, remotepath);
//
//	  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1);  // 设置连接超时，单位秒
//	//设置http 头部处理函数
//	 curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
//	 curl_easy_setopt(curl, CURLOPT_HEADERDATA, &filesize);
//   // 设置文件续传的位置给libcurl
//	 //curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0);
//
//	 curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
//	 curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wirtefunc);
//
//	//curl_easy_setopt(curlhandle, CURLOPT_READFUNCTION, readfunc);
//	//curl_easy_setopt(curlhandle, CURLOPT_READDATA, f);
//	 curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
//	 curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
//
//	 res = curl_easy_perform(curl);
//
//	 fflush(fd);
//	 fclose(fd);
//
//	 if (res == CURLE_OK)
//	 {
		 //play
	     system(remotepath);
		 start_Play_Thread();
//	 }
// }

  return 0;
}
