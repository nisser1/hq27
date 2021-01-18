#ifndef __GPIO_H__

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

/* mmap addr operation */
#define WRITE_LOG_ERROR	printf
#define WRITE_LOG_INFO printf
#define PAGE_SIZE 0x1000
#define PAGE_SIZE_MASK (~(0xfff))
typedef struct tag_MMAP_Node
{
	unsigned long Start_P;
	unsigned long Start_V;
	unsigned long length;
	unsigned long refcount;
	struct tag_MMAP_Node * next;
}TMMAP_Node_t;

#define DEBUG 1
#ifdef DEBUG
#define dbg_printf(fmt, args...) printf(fmt, ##args)
#define dbg_perror(msg) (perror(msg))
#else
#define dbg_printf(fmt, args...)
#define dbg_perror(msg)
#endif

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define MAX_INPUT 255
#define GPIO_SET 1
#define GPIO_GET 0

#define GPIO_IN 0
#define GPIO_OUT 1

/* four led control */
#define LED1_NUM 2*8 + 0
#define LED2_NUM 14*8 + 0
#define LED3_NUM 14*8 + 1
#define LED4_NUM 14*8 + 2
#define GPIO2_0 0x1F000030
#define GPIO14_0 0x1F000124
#define GPIO14_1 0x1F000128
#define GPIO14_2 0x1F000114
struct ztGpioPrivate{
   int Gpio_fd;
   volatile unsigned int *LED1_Map;
   volatile unsigned int *LED2_Map;
   volatile unsigned int *LED3_Map;
   volatile unsigned int *LED4_Map;
};

int ztInitGpioMap(struct ztGpioPrivate *Private);
int ztReleaseGpioMap(struct ztGpioPrivate *Private);
int ztGpioExport(int number);
int ztGpioUnexport(int number);
int ztGpioSetDir(int number, int dir);
int ztGpioGetDir(int number, char* dir);
/******************************************************************************
  Function:       ztGpioSetLevel
  Description:    set the gpio level
  Input:          number    	--  gpio number integer value. eg.gpio3_7 =
3*32+7 level			--  set gpio level. 1: high; 0: low. Output:
  Return:         int			--  return the result
  Others:         NONE
*******************************************************************************/
int ztGpioSetLevel(int number, int level);
int ztGpioGetLevel(int number, char* level);

#endif  // __GPIO_H__
