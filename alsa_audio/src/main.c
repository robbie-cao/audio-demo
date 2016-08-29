#include <stdio.h>
#include <stdlib.h>
#include "lplay.h"
#include "lrecord.h"
#include "mqtt_hander.h"


int main(void)
{
	printf("start mqtt\r\n");

	start_matt();

	printf("start record\r\n");


	start_Record();
//	start_Play();

	while(1)
	{
		sleep(1);
	}

	printf("stop\r\n");

	return 0;
}

