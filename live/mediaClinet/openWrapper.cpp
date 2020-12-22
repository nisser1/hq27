/*#include <RecvLinklist.h>
#include <stdio.h>
#include <string.h>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "openWrapper.h"
#ifdef __cplusplus
extern "C"{
#endif

struct objTaskScheduler{
	TaskScheduler* scheduler;
}
struct objUsageEnvironment{
	UsageEnvironment* env;
}
struct objRTSPClient{
	RTSPClient* rtspClient;
}

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);

extern struct objTaskScheduler *getobjTaskScheduler()
{
	struct objTaskScheduler *ots;
	ots->scheduler = BasicTaskScheduler::createNew();
	return ots;
}
extern struct objUsageEnvironment *getobjUsageEnvironment()
{
	struct objUsageEnvironment oue;
	oue->env = BasicUsageEnvironment::createNew(*scheduler);
	return oue;
}
extern struct objRTSPClient *getobjRTSPClient(struct objUsageEnvironment *oue)
{
	struct objRTSPClient *orc;
	orc->rtspClient = ourRTSPClient::createNew(oue->env, "rtsp://192.168.10.165:8554/qw", 1, "testRTSPClinent");
	if (orc->rtspClient == NULL)
    {
        printf("Failed to create a RTSP client for URL!!!\n\n");
        return ;
    }
	return orc;
}
extern void setsendDescribeCommand(struct objRTSPClient *orc)
{
	orc->rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
	return ;
}
extern void setdoEventLoop(struct objUsageEnvironment *oue)
{
	oue->env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
	return ;
}
#ifdef __cplusplus
}
#endif
*/



