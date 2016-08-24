#ifndef __LRECORD_H__
#define __LRECORD_H__

#include "sndwav_common.h"

int SNDWAV_PrepareWAVParams(WAVContainer_t *wav);

void start_Record(void);
void stop_Record(void);

#endif
