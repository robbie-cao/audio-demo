//File   : lplay.c
//Author : Loon <sepnic@gmail.com>


#include "lplay.h"

#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include <pthread.h>

#include "wav_parser.h"
#include "sndwav_common.h"

const char *devicename = "plug:dmix";
SNDPCMContainer_t playback;
pthread_t *playthreadID;


ssize_t SNDWAV_P_SaveRead(FILE * fd, void *buf, size_t count)
{
    ssize_t result = 0, res;

    while (count > 0) {
        if ((res = fread( buf, 1, count,fd)) == 0)
            break;
        if (res < 0)
            return result > 0 ? result : res;
        count -= res;
        result += res;
        buf = (char *)buf + res;
    }
    return result;
}

void SNDWAV_Play(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav, FILE *fd)
{
    int load, ret;
    off64_t written = 0;
    off64_t c;
    off64_t count = LE_INT(wav->chunk.length);

    load = 0;
    while (written < count) {
        /* Must read [chunk_bytes] bytes data enough. */
        do {
            c = count - written;
            if (c > sndpcm->chunk_bytes)
                c = sndpcm->chunk_bytes;
            c -= load;

            if (c == 0)
                break;
            ret = SNDWAV_P_SaveRead(fd, sndpcm->data_buf + load, c);
            if (ret < 0) {
                fprintf(stderr, "Error safe_read/n");
                exit(-1);
            }
            if (ret == 0)
                break;
            load += ret;
        } while ((size_t)load < sndpcm->chunk_bytes);

        /* Transfer to size frame */
        load = load * 8 / sndpcm->bits_per_frame;
        ret = SNDWAV_WritePcm(sndpcm, load);
        if (ret != load)
            break;

        ret = ret * sndpcm->bits_per_frame / 8;
        written += ret;
        load = 0;
    }
}

int init_Play_ENV(void)
{
	memset(&playback, 0x0, sizeof(playback));
	if (snd_output_stdio_attach(&playback.log, stderr, 0) < 0) {
		fprintf(stderr, "Error snd_output_stdio_attach/n");
		return -1;
	}

	if (snd_pcm_open(&playback.handle, devicename, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		return -1;
	}

	return 0;
}

void *thread_func(void *arg)
{
	char *filename;
	FILE *fd;
	WAVContainer_t wav;

	SNDPCMContainer_t *snd = (SNDPCMContainer_t *)arg;

	memset(&wav, 0x0, sizeof(WAVContainer_t));

	//get file
	filename = "/tmp/msg.wav";

	fd = fopen(filename,"r");
	fseek(fd, 0, SEEK_SET);

	if (SNDWAV_SetParams(snd, &wav) < 0) {
		fprintf(stderr, "Error set_snd_pcm_params/n");
	}

	snd_pcm_dump(snd->handle, snd->log);
	SNDWAV_Play(snd, &wav, fd);
	snd_pcm_drain(snd->handle);

	fclose(fd);


	return((void *)0);
}

int start_Play_Thread(void)
{
    pthread_create(playthreadID,NULL,thread_func,&playback);

	return 0;
}

int quit_Play_ENV(void)
{
	//close snd
	snd_pcm_drain(playback.handle);
    snd_output_close(playback.log);
    snd_pcm_close(playback.handle);

	return 0;
}

int stop_Play_Thread(void)
{
	pthread_join(playthreadID,NULL);

	return 0;
}


void start_Play(void)
{
	init_Play_ENV();
	start_Play_Thread();
}

void stop_Play(void)
{
	stop_Play_Thread();
	quit_Play_ENV();
}

//int main(int argc, char *argv[])
//{
//    char *filename;
//    char *devicename = "plug:dmix";
//    FILE *fd;
//    WAVContainer_t wav;
//    SNDPCMContainer_t playback;
//
//    if (argc != 2) {
//        fprintf(stderr, "Usage: ./lplay <FILENAME>/n");
//        return -1;
//    }
//
//    memset(&playback, 0x0, sizeof(playback));
//    memset(&wav, 0x0, sizeof(WAVContainer_t));
//
//    filename = argv[1];
//    fd = fopen(filename,"r");
//    fseek(fd, 0, SEEK_SET);
//
//    if (WAV_ReadHeader(fd, &wav) < 0) {
//        fprintf(stderr, "Error WAV_Parse [%s]/n", filename);
//        goto Err;
//    }
//
//    if (snd_output_stdio_attach(&playback.log, stderr, 0) < 0) {
//        fprintf(stderr, "Error snd_output_stdio_attach/n");
//        goto Err;
//    }
//
//
//    if (snd_pcm_open(&playback.handle, devicename, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
//        fprintf(stderr, "Error snd_pcm_open [ %s]/n", devicename);
//        goto Err;
//    }
//
//    if (SNDWAV_SetParams(&playback, &wav) < 0) {
//        fprintf(stderr, "Error set_snd_pcm_params/n");
//        goto Err;
//    }
//    snd_pcm_dump(playback.handle, playback.log);
//
//    SNDWAV_Play(&playback, &wav, fd);
//
//    snd_pcm_drain(playback.handle);
//
//    fclose(fd);
//    free(playback.data_buf);
//    snd_output_close(playback.log);
//    snd_pcm_close(playback.handle);
//    return 0;
//
//Err:
//    fclose(fd);
//    if (playback.data_buf) free(playback.data_buf);
//    if (playback.log) snd_output_close(playback.log);
//    if (playback.handle) snd_pcm_close(playback.handle);
//    return -1;
//}
