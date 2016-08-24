#ifndef __LPLAY_H__
#define __LPLAY_H__

ssize_t SNDWAV_P_SaveRead(FILE * fd, void *buf, size_t count);
void SNDWAV_Play(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav, FILE *fd);
void SNDWAV_Record(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav, int fd);

#endif
