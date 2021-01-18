/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2020, Live Networks, Inc.  All rights reserved
// A demo application, showing how to create and run a RTSP client (that can potentially receive multiple streams concurrently).
//
// NOTE: This code - although it builds a running application - is intended only to illustrate how to develop your own RTSP
// client application.  For a full-featured RTSP client application - with much more functionality, and many options - see
// "openRTSP": http://www.live555.com/openRTSP/


/****************************************************************NOTE****************************************************************
	1：目前只接受视频信息  不对音频进行处理
	2：
*/

#include "openWrapper.h"
#include <iostream>
#include <pthread.h>
extern "C"{
#include "fifo_buffer.h"
}
using namespace std;
extern void *streamOneThreadCallBack(void *arg);
int firststream[VDEC_DECODE_CHN] = {1};

MESSAGE *fifo = NULL;
unsigned int sendfd[VDEC_DECODE_CHN] = {0};
char rtsp_address[VDEC_DECODE_CHN][64] = {0};
// Forward function definitions:

// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData, char const* reason);
// called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
// called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// The main streaming routine (for each "rtsp://" URL):
void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient)
{
    return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession)
{
    return env << subsession.mediumName() << "/" << subsession.codecName();
}

void usage(UsageEnvironment& env, char const* progName)
{
    env << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
    env << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}
char eventLoopWatchVariable = 0;


int openall(MESSAGE *msg)
{
	int i = 0,ret = 0;
	fifo = msg;
	FILE *fp = NULL;
	char buff_rtsp_address[64];
	for(i = 0;i<VDEC_DECODE_CHN;i++)
	{
		sendfd[i] = msg->fd[i];
	}
	fp = fopen("vedio_source_configure.ini","r");
	if(fp == NULL)
	{
		perror("fialed open configure file!!!\n");
	}
	i=0;
	while(fgets(buff_rtsp_address,sizeof(buff_rtsp_address),fp))
	{
		if(strstr(buff_rtsp_address,"rtsp://"))
		{
			ret = strlen(buff_rtsp_address);
			strncpy(rtsp_address[i],buff_rtsp_address,ret-1);
			//printf("rsssss%s\n",rtsp_address[i]);
			i++;
		}
	}
	
	
    // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();			   //创建基础环境类  目前此类不是特别清楚  需后续再做研究
    UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);  //创建调度类  目前此类不是特别清楚  需后续再做研究

    // We need at least one "rtsp://" URL argument:

	for(int i = 0; i< VDEC_DECODE_CHN; i++)
	{

		openURL(*env, "mediaClient", rtsp_address[i]);								//创建rtsp客户端
	}
	
    // All subsequent activity takes place within the event loop:
    env->taskScheduler().doEventLoop(&eventLoopWatchVariable);			//进入事件循环	              所有  后续活动都在事件循环中进行  eventLoopWatchVariable==0才会进入事件循环
    // This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.

    return 0;

    // If you choose to continue the application past this point (i.e., if you comment out the "return 0;" statement above),		//执行1次？
    // and if you don't intend to do anything more with the "TaskScheduler" and "UsageEnvironment" objects,
    // then you can also reclaim the (small) memory used by these objects by uncommenting the following code:
    /*
      env->reclaim(); env = NULL;
      delete scheduler; scheduler = NULL;
    */
}

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class StreamClientState
{
public:
    StreamClientState();
    virtual ~StreamClientState();

public:
    MediaSubsessionIterator* iter;
    MediaSession* session;
    MediaSubsession* subsession;
    TaskToken streamTimerTask;
    double duration;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient: public RTSPClient
{
public:
    static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
                                    int verbosityLevel = 0,
                                    char const* applicationName = NULL,
                                    portNumBits tunnelOverHTTPPortNum = 0);

protected:
    ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
                  int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
    virtual ~ourRTSPClient();

public:
    StreamClientState scs;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink: public MediaSink
{
public:
    static DummySink* createNew(UsageEnvironment& env,
                                MediaSubsession& subsession, // identifies the kind of data that's being received
                                char const* streamId = NULL); // identifies the stream itself (optional)

private:
    DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
    // called only by "createNew()"
    virtual ~DummySink();

    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                           struct timeval presentationTime, unsigned durationInMicroseconds);

private:
    // redefined virtual functions:
    virtual Boolean continuePlaying();

private:
    u_int8_t* fReceiveBuffer;
    MediaSubsession& fSubsession;
    char* fStreamId;
};

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

static unsigned rtspClientCount = 0; // Counts how many streams (i.e., "RTSPClient"s) are currently in use.  计算当前多少个rtsp码流

void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL)
{
    // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
    // to receive (even if more than stream uses the same "rtsp://" URL).
    RTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
    if (rtspClient == NULL)
    {
        env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
        return;
    }

    ++rtspClientCount;

    // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
    // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
    // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
    rtspClient->sendDescribeCommand(continueAfterDESCRIBE);

}


// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString)
{




    do
    {
        UsageEnvironment& env = rtspClient->envir(); // alias
        StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

        if (resultCode != 0)
        {
            env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
            delete[] resultString;
            break;
        }

        char* const sdpDescription = resultString;
        env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

        // Create a media session object from this SDP description:
        scs.session = MediaSession::createNew(env, sdpDescription);
        delete[] sdpDescription; // because we don't need it anymore
        if (scs.session == NULL)
        {
            env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
            break;
        }
        else if (!scs.session->hasSubsessions())
        {
            env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
            break;
        }

        // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
        // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
        // (Each 'subsession' will have its own data source.)
        scs.iter = new MediaSubsessionIterator(*scs.session);
        setupNextSubsession(rtspClient);
        return;
    }
    while (0);

    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP False

void setupNextSubsession(RTSPClient* rtspClient)
{



    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    scs.subsession = scs.iter->next();
    if (scs.subsession != NULL)
    {
        if (!scs.subsession->initiate())
        {
            env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
            setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
        }
        else
        {
            env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
            if (scs.subsession->rtcpIsMuxed())
            {
                env << "client port " << scs.subsession->clientPortNum();
            }
            else
            {
                env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
            }
            env << ")\n";

            // Continue setting up this subsession, by sending a RTSP "SETUP" command:
            rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
        }
        return;
    }

    // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
    if (scs.session->absStartTime() != NULL)
    {
        // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
        //特殊情况：流通过“绝对”时间编制索引，因此发送适当的“ PLAY”命令
        rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
    }
    else
    {
        scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
        rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
    }
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString)
{



    do
    {
        UsageEnvironment& env = rtspClient->envir(); // alias
        StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

        if (resultCode != 0)
        {
            env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
            break;
        }

        env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
        if (scs.subsession->rtcpIsMuxed())
        {
            env << "client port " << scs.subsession->clientPortNum();
        }
        else
        {
            env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
        }
        env << ")\n";

        // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
        // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
        // after we've sent a RTSP "PLAY" command.)

        scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url());
        // perhaps use your own custom "MediaSink" subclass instead
        if (scs.subsession->sink == NULL)
        {
            env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
                << "\" subsession: " << env.getResultMsg() << "\n";
            break;
        }

        env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
        scs.subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession
        scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
                                           subsessionAfterPlaying, scs.subsession);
        // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
        if (scs.subsession->rtcpInstance() != NULL)
        {
            scs.subsession->rtcpInstance()->setByeWithReasonHandler(subsessionByeHandler, scs.subsession);
        }
    }
    while (0);
    delete[] resultString;

    // Set up the next subsession, if any:
    setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString)
{



    Boolean success = False;

    do
    {
        UsageEnvironment& env = rtspClient->envir(); // alias
        StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

        if (resultCode != 0)
        {
            env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
            break;
        }

        // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
        // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
        // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
        // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
        //设置要在流的预期持续时间结束时处理的计时器（如果流尚未使用RTCP“ BYE”发出信号通知其结束）。
        //这是可选的。 相反，如果您想保持流处于活动状态（例如，以便以后可以在其中“搜索”并执行另一个RTSP“播放”），
        //则可以忽略此代码。 （或者，如果您不想接收整个流，则可以将此计时器设置为较短的值。）
        //cout << "scs.duration: " << scs.duration <<endl;
        if (scs.duration > 0)
        {
            //在流的预期持续时间之后，要延迟的秒数。 （这是可选的。）
            unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
            scs.duration += delaySlop;
            unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
            scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
            //cout << "scs.duration: " << scs.duration <<" uSecsToDelay: " << uSecsToDelay<< endl;
        }


        env << *rtspClient << "Started playing session";
        if (scs.duration > 0)
        {
            env << " (for up to " << scs.duration << " seconds)";
        }
        env << "...\n";

        success = True;
    }
    while (0);
    delete[] resultString;

    if (!success)
    {
        // An unrecoverable error occurred with this stream.
        shutdownStream(rtspClient);
    }
}


// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData)
{



    MediaSubsession* subsession = (MediaSubsession*)clientData;
    RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

    // Begin by closing this subsession's stream:
    Medium::close(subsession->sink);
    subsession->sink = NULL;

    // Next, check whether *all* subsessions' streams have now been closed:
    MediaSession& session = subsession->parentSession();
    MediaSubsessionIterator iter(session);
    while ((subsession = iter.next()) != NULL)
    {
        if (subsession->sink != NULL) return; // this subsession is still active
    }

    // All subsessions' streams have now been closed, so shutdown the client:
    //现在，所有子会话的流都已关闭，因此请关闭客户端
    shutdownStream(rtspClient);

}

void subsessionByeHandler(void* clientData, char const* reason)
{



    MediaSubsession* subsession = (MediaSubsession*)clientData;
    RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
    UsageEnvironment& env = rtspClient->envir(); // alias

    env << *rtspClient << "Received RTCP \"BYE\"";
    if (reason != NULL)
    {
        env << " (reason:\"" << reason << "\")";
        delete[] (char*)reason;
    }
    env << " on \"" << *subsession << "\" subsession\n";

    // Now act as if the subsession had closed:
    subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData)
{




    ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
    StreamClientState& scs = rtspClient->scs; // alias

    scs.streamTimerTask = NULL;

    // Shut down the stream:
    shutdownStream(rtspClient);

}

void shutdownStream(RTSPClient* rtspClient, int exitCode)
{




    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    // First, check whether any subsessions have still to be closed:
    if (scs.session != NULL)
    {
        Boolean someSubsessionsWereActive = False;
        MediaSubsessionIterator iter(*scs.session);
        MediaSubsession* subsession;

        while ((subsession = iter.next()) != NULL)
        {
            if (subsession->sink != NULL)
            {
                Medium::close(subsession->sink);
                subsession->sink = NULL;

                if (subsession->rtcpInstance() != NULL)
                {
                    subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
                }

                someSubsessionsWereActive = True;
            }
        }

        if (someSubsessionsWereActive)
        {
            // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
            // Don't bother handling the response to the "TEARDOWN".
            rtspClient->sendTeardownCommand(*scs.session, NULL);
        }
    }

    env << *rtspClient << "Closing the stream.\n";
    Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

    if (--rtspClientCount == 0)
    {
        // The final stream has ended, so exit the application now.
        // (Of course, if you're embedding this code into your own application, you might want to comment this out,
        // and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
        exit(exitCode);
    }
}


// Implementation of "ourRTSPClient":

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
                                        int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
{



    return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
                             int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
    : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1)
{



}

ourRTSPClient::~ourRTSPClient()
{

}


// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
    : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0)
{



}

StreamClientState::~StreamClientState()
{



    delete iter;
    if (session != NULL)
    {
        // We also need to delete "session", and unschedule "streamTimerTask" (if set)
        UsageEnvironment& env = session->envir(); // alias

        env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
        Medium::close(session);
    }
}


// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 30000000

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
{



    return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
    : MediaSink(env),
      fSubsession(subsession)
{
	//printf("sssssssssssssssssss1 %s\n",streamId);
    fStreamId = strDup(streamId);
	//printf("sssssssssssssssssss %s\n",fStreamId);
    fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
}

DummySink::~DummySink()
{
    delete[] fReceiveBuffer;
    delete[] fStreamId;
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                  struct timeval presentationTime, unsigned durationInMicroseconds)
{



    DummySink* sink = (DummySink*)clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);

}

// If you don't want to see debugging output for each received frame, then comment out the following line:
//#define DEBUG_PRINT_EACH_RECEIVED_FRAME 0

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
  // We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
  if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
  envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
  if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
  char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
  sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
  envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
  if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
    envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
  }
#ifdef DEBUG_PRINT_NPT
  envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
  envir() << "\n";
#endif
#if 1

	char bufname[128];
	for(int i = 0; i < VDEC_DECODE_CHN; i++)
	{
		//int i = 0,j = 0;
		u_int8_t start_code[4] = {0x00,0x00,0x00,0x01};
		
		//printf("asd mediumName=%s  protocolName=%s controlPath=%s fStreamId=%s rtsp_address[i]=%s iii=%d\n",fSubsession.mediumName(),fSubsession.protocolName(),fSubsession.controlPath(),fStreamId,rtsp_address[i],i);
		//strcpy(bufname,fStreamId);
		//bufname[strlen(bufname)-1] = '\0';
		//printf("%s,%s,sizeof(fStreamId)=%d,sizeof(rtsp_address[i])=%d frameSize=%d\n",fStreamId,rtsp_address[i],strlen(fStreamId),strlen(rtsp_address[i]),frameSize);
		if((strstr(fSubsession.mediumName(),"video"))&& (strstr(fStreamId,rtsp_address[i]))) 
		{
			//printf("%s\n",fStreamId);
			if(firststream[i] == 1) 
			{
				unsigned int num,i;
				SPropRecord *sps = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(),num);
				if(sps == NULL)
				{
					printf("Failed sps parseSPropParameterSets!!\n");
				}	
				pthread_mutex_lock(&mutex);
				readDataToBuff(4,fifo->recv_buf[i],start_code,fifo->rindex[i],fifo->windex[i],fifo,i);
				readDataToBuff(sps[0].sPropLength,fifo->recv_buf[i],sps[0].sPropBytes,fifo->rindex[i],fifo->windex[i],fifo,i);
				readDataToBuff(4,fifo->recv_buf[i],start_code,fifo->rindex[i],fifo->windex[i],fifo,i);			
				readDataToBuff(sps[1].sPropLength,fifo->recv_buf[i],sps[1].sPropBytes,fifo->rindex[i],fifo->windex[i],fifo,i);	
				pthread_mutex_unlock(&mutex);
				//printf("write writerindex[0]=%8d\n",4+4+sps[0].sPropLength+sps[1].sPropLength);
				firststream[i] = 0;
				delete[] sps;
			}

			if(firststream[i] == 0)
			{	
				if(fifo->bck_not_show[i] == -1)
				{
					pthread_mutex_lock(&mutex);
                    //帧头在readDataBuff填入
					readDataToBuff(frameSize,fifo->recv_buf[i],fReceiveBuffer,fifo->rindex[i],fifo->windex[i],fifo,i);
					pthread_mutex_unlock(&mutex);
				}
				else if(fifo->bck_not_show[i] == 1)
				{
					pthread_mutex_lock(&mutex);
					fifo->windex[i] = 0;
					fifo->rindex[i] = 0;
					pthread_mutex_unlock(&mutex);
				}
			}		
		}		
	}
#endif 		
#if 0		
  //保存成文件

int  i = 0;
char test[VDEC_DECODE_CHN][32] = {"test1.264","test2.264","test3.264","test4.264","test5.264","test6.264","test7.264","test8.264","test9.264"};
for(i = 0; i < VDEC_DECODE_CHN; i++)
{
		//int i = 0,j = 0;
	u_int8_t start_code[4] = {0x00,0x00,0x00,0x01};
	if((!strcmp(fSubsession.mediumName(),"video"))&& (strstr(fStreamId,rtsp_address[i]))) 
	{	
		
  //保存成文件

		 if((firststream[i] )&& (i==0))
		 {
			 unsigned int num;
			 SPropRecord *sps = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), num);
			 // For H.264 video stream, we use a special sink that insert start_codes:
			 //struct timeval tv= {0,0};
			 unsigned char start_code[4] = {0x00, 0x00, 0x00, 0x01};
			 FILE *fp = fopen("test.264", "a+b");
			 if(fp)
			 {
				 fwrite(start_code, 4, 1, fp);
				 fwrite(sps[0].sPropBytes, sps[0].sPropLength, 1, fp);
				 fwrite(start_code, 4, 1, fp);
				 fwrite(sps[1].sPropBytes, sps[1].sPropLength, 1, fp);
				 fclose(fp);
				 fp = NULL;
			 }
			 delete [] sps;
			 firststream[i] = 0;
		 }

		 unsigned char *pbuf = fReceiveBuffer;
		 unsigned char head[4] = {0x00, 0x00, 0x00, 0x01};
		 if(i==0)
		 {
		 FILE *fp = fopen("test.264", "a+b");
		 if(fp)
		 {
			 fwrite(head, 4, 1, fp);
			 fwrite(fReceiveBuffer, frameSize, 1, fp);
			 fclose(fp);
			 printf("11,%d  %d \n",i,frameSize);
			 fp = NULL;
		 }
		 }
	
	} 
}	
#endif 		
		
    continuePlaying();
}

Boolean DummySink::continuePlaying()
{
    if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  return True;
}
