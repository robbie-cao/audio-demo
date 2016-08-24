#ifndef __LPLAY_H__
#define __LPLAY_H__

#include <stdio.h>
#include "sndwav_common.h"

ssize_t SNDWAV_P_SaveRead(FILE * fd, void *buf, size_t count);
void SNDWAV_Play(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav, FILE *fd);
void SNDWAV_Record(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav, int fd);

void start_Play(void);
void stop_Play(void);

#endif
