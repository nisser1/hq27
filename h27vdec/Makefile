CC	= aarch64-himix100-linux-gcc
CPP	= aarch64-himix100-linux-g++
LINK	= aarch64-himix100-linux-g++

INCLUDE = -I./include/\
	-I/usr/local/include/\
	-I../../live555-rtsp/include/mediaClient\
	-I../../live555-rtsp/include/BasicUsageEnvironment\
	-I../../live555-rtsp/include/groupsock\
	-I../../live555-rtsp/include/liveMedia\
	-I../../live555-rtsp/include/UsageEnvironment\
	-I./osd

LIBS = ./lib/libmpi.a\
	./lib/libhdmi.a\
	./lib/libdpu_rect.a\
	./lib/libdpu_match.a\
	./lib/libhiavslut.a\
	./lib/libpos_query.a\
	./lib/libmotionfusion.a\
	./lib/lib_hiae.a\
	./lib/libisp.a\
	./lib/lib_hidehaze.a\
	./lib/lib_hidrc.a\
	./lib/lib_hildci.a\
	./lib/libsns_imx477.a\
	./lib/libsns_imx290.a\
	./lib/libsns_imx334.a\
	./lib/libsns_imx334_slave.a\
	./lib/libsns_imx277_slvs.a\
	./lib/lib_hiawb.a\
	./lib/libVoiceEngine.a\
	./lib/libupvqe.a\
	./lib/libdnvqe.a\
	./lib/libaacenc.a\
	./lib/libaacdec.a\
	./lib/libsecurec.a -lpthread -lm -ldl -lstdc++
LIV555LIBS=../../live555-rtsp/lib/libmediaClient.a\
	../../live555-rtsp/lib/libliveMedia.a\
	../../live555-rtsp/lib/libgroupsock.a\
	../../live555-rtsp/lib/libBasicUsageEnvironment.a\
	../../live555-rtsp/lib/libUsageEnvironment.a

CCFLAGS = -Dhi3559av100 -DSENSOR0_TYPE=HT706_XMC_BT1120_30FPS_12BIT  -DSENSOR1_TYPE=SONY_IMX334_MIPI_8M_30FPS_12BIT -DSENSOR2_TYPE=SONY_IMX334_MIPI_8M_30FPS_12BIT\
		-DSENSOR3_TYPE=SONY_IMX334_MIPI_8M_30FPS_12BIT -DSENSOR4_TYPE=SONY_IMX277_SLVS_8M_30FPS_12BIT -DSENSOR5_TYPE=SONY_IMX277_SLVS_8M_30FPS_12BIT\
		-DSENSOR6_TYPE=SONY_IMX277_SLVS_8M_30FPS_12BIT -DSENSOR7_TYPE=SONY_IMX277_SLVS_8M_30FPS_12BIT -DHI_RELEASE -DHI_XXXX -DISP_V2 -DHI_ACODEC_TYPE_INNER\
		-fno-aggressive-loop-optimizations -ffunction-sections -ldl -fdata-sections 

CPPFLAGS = -Dhi3559av100 -DSENSOR0_TYPE=SONY_IMX334_MIPI_8M_30FPS_12BIT  -DSENSOR1_TYPE=SONY_IMX334_MIPI_8M_30FPS_12BIT -DSENSOR2_TYPE=SONY_IMX334_MIPI_8M_30FPS_12BIT\
		-DSENSOR3_TYPE=SONY_IMX334_MIPI_8M_30FPS_12BIT -DSENSOR4_TYPE=SONY_IMX277_SLVS_8M_30FPS_12BIT -DSENSOR5_TYPE=SONY_IMX277_SLVS_8M_30FPS_12BIT\
		-DSENSOR6_TYPE=SONY_IMX277_SLVS_8M_30FPS_12BIT -DSENSOR7_TYPE=SONY_IMX277_SLVS_8M_30FPS_12BIT -DHI_RELEASE -DHI_XXXX -DISP_V2 -DHI_ACODEC_TYPE_INNER\
		-fno-aggressive-loop-optimizations -ffunction-sections -ldl -fdata-sections -fpermissive  

CSRCS = $(wildcard *.c ./osd/*.c)
CPPSRCS = $(wildcard *.cpp ./osd/*.cpp)
OBJS = $(CSRCS:.c=.o) $(CPPSRCS:.cpp=.o)
TARGET=hq27_vdec

all:$(TARGET)

$(TARGET) : $(OBJS)
	$(LINK) $^ $(CPPFLAGS) -o $@ $(LIBS) $(LIV555LIBS) -lssl -lcrypto


%.o:%.c
	$(CC) $< $(CCFLAGS) -c -o $@ $(INCLUDE)


%.o:%.cpp
	$(CPP) $< $(CPPFLAGS) -c -o $@ $(INCLUDE)


clean:
	rm *.o hq27_vdec
