#ifndef __DO_SWITCH__H_
#define __DO_SWITCH__H_
#include "sample_comm.h"
#include "fifo_buffer.h"
#include "hh_osd.h"
#include "sample_vdec.h"
#define PRINTF_PRT(fmt...)   \
do {\
    printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)
extern int previous_mode_all;
extern int current_vdeio_display_mode; //==9标识9屏   ==4 标识四屏  ==1 标识大屏
extern int who_in_pos9;
extern int cmd_confirm;
extern int witch_mode_4;
static int led_ctl;
static int previous_tow_mode = 190;
void *Media_Common_Receive_thread(void *arg);
int response_command(int cmd,MESSAGE *msg);
void signal_iic_handler(int signum);
void signal_key_handler(int signum);
void *set_light_thread(void *arg);
#endif
