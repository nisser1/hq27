#ifndef	 __OPENWRAPPER_H__
#define  __OPENWRAPPER_H__
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#ifdef __cplusplus
extern "C"{
#endif
#include "RecvThread.h"
#include "RecvLinklist.h"
#include "fifo_buffer.h"
#ifdef __cplusplus
}
#endif

//linklist * streamOne[9] = {NULL};
/*
c调c++
#ifdef __cplusplus
extern "C"{
#endif
char eventLoopWatchVariable = 0;
struct objTaskScheduler;
struct objUsageEnvironment;
struct objRTSPClient;

extern struct objTaskScheduler *getobjTaskScheduler();
extern struct objUsageEnvironment *getobjUsageEnvironment();
extern struct objRTSPClient *getobjRTSPClient(struct objUsageEnvironment *oue);
extern void setsendDescribeCommand(struct objRTSPClient *orc);
extern void setdoEventLoop(struct objUsageEnvironment *oue);
extern int openall(void);
#ifdef __cplusplus
}
#endif
*/
extern int openall(MESSAGE *msg);
//extern int openall(int fd);
#endif 
