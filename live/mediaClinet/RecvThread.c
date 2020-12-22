#include  "RecvLinklist.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include "liveMedia.hh"
//#include "BasicUsageEnvironment.hh"
void *streamOneThreadCallBack(void *arg)
{
	
	linklist * streamLinkList = (linklist *)arg;
	while(1)
	{
		//printf("lsls\n");
		while(streamLinkList->next != NULL)
		{
			printf("delete linklist\n");
			delete_head_linlist(streamLinkList);	
		}
		usleep(1);
	}
}


