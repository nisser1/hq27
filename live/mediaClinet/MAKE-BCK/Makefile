INCLUDES = -I../UsageEnvironment/include -I../groupsock/include -I../liveMedia/include -I../BasicUsageEnvironment/include
PREFIX = /home/ema/share_nfs/media
LIBDIR = $(PREFIX)/lib

# Default library filename suffixes for each library that we link with.  The "config.*" file might redefine these later.
#将四个库包含进来   目前加入我们自己的库文件  一共五个
#libliveMedia_LIB_SUFFIX = $(LIB_SUFFIX)
#libBasicUsageEnvironment_LIB_SUFFIX = $(LIB_SUFFIX)
#libUsageEnvironment_LIB_SUFFIX = $(LIB_SUFFIX)
#libgroupsock_LIB_SUFFIX = $(LIB_SUFFIX)
#libLiveVdeclink_LIB_SUFFIX = $(LIB_SUFFIX)
##### Change the following for your environment:
CROSS_COMPILE?=		aarch64-himix100-linux-
COMPILE_OPTS =		$(INCLUDES) -I/usr/local/include  -I. -O2 -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DLOCALE_NOT_USED
C =			c
C_COMPILER =		$(CROSS_COMPILE)gcc -fPIC
C_FLAGS =		$(COMPILE_OPTS)
CPP =			cpp
CPLUSPLUS_COMPILER =	$(CROSS_COMPILE)g++ 		
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall -DBSD=1 -fPIC -fpermissive  
OBJ =			o
LINK =			$(CROSS_COMPILE)g++ -o   
LINK_OPTS =
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		$(CROSS_COMPILE)ar cr 
LIBRARY_LINK_OPTS =	$(LINK_OPTS)
LIB_SUFFIX =			a
LIBS_FOR_CONSOLE_APPLICATION = -lssl -lcrypto -lpthread -lstdc++
LIBS_FOR_GUI_APPLICATION =
EXE =
##### End of variables to change
NAME = libmediaClient
#MEDIA_CLIENT = test$(EXE)

USAGE_ENVIRONMENT_LIB = $(NAME).$(LIB_SUFFIX)
ALL = $(USAGE_ENVIRONMENT_LIB)
all:	$(ALL)

OBJS = testRTSPClient.$(OBJ) RecvThread.$(OBJ) RecvLinklist.$(OBJ)

$(USAGE_ENVIRONMENT_LIB): $(OBJS)
	$(LIBRARY_LINK)$@ $(LIBRARY_LINK_OPTS) $(OBJS)

.$(C).$(OBJ):
	$(C_COMPILER) -c $(C_FLAGS) $<       

.$(CPP).$(OBJ):
	$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

#test1.$(CPP): 			url.h	testcpp.h	
testRTSPClient.$(CPP):	 RecvLinklist.h RecvThread.h fifo_buffer.h
RecvLinklist.$(C):		RecvLinklist.h
RecvThread.$(C):		RecvThread.h	
fifo_buffer.$(C):		fifo_buffer.h	
clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~	
	
	
install: install1 $(INSTALL2)
install1: $(USAGE_ENVIRONMENT_LIB)
	  install -d $(DESTDIR)$(PREFIX)/include/mediaClient $(DESTDIR)$(LIBDIR)
	  install -m 644 ./*.h $(DESTDIR)$(PREFIX)/include/mediaClient
	  install -m 644 $(USAGE_ENVIRONMENT_LIB) $(DESTDIR)$(LIBDIR)
install_shared_libraries: $(USAGE_ENVIRONMENT_LIB)
	  ln -fs $(NAME).$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/$(NAME).$(SHORT_LIB_SUFFIX)
	  ln -fs $(NAME).$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/$(NAME).so	
	
	
	
	
	
	


##### Any additional, platform-specific rules come here:
