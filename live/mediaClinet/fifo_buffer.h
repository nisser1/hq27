#ifndef	 __FIFO_BUUFER_H__
#define  __FIFO_BUUFER_H__
#include <pthread.h>
typedef unsigned char u_int8_t;	
//#define BUF_SIZE 12441600
#define BUF_SIZE 24883200
#define BCK_NOTSHOW_CHN 9
#define MAX_VOCHN 10
#define VDEC_DECODE_CHN 9
/*typedef struct vdec_buf{ 
	unsigned int rindex;
	unsigned int windex;
	unsigned char recv_buf[24883200];	
}VDEC_BUF;*/

typedef struct message_info{
	unsigned int fd[9];	
	unsigned int rindex[9];
	unsigned int windex[9];
	unsigned char recv_buf[9][BUF_SIZE];
	int pos[2];
	int bck_not_show[BCK_NOTSHOW_CHN];
	int windexEndPos[9];
	int fd_key;
	int fd_iic;
}MESSAGE;


extern pthread_mutex_t mutex;
extern pthread_cond_t cond ;

//VDEC_BUF


int create_fifo(char *path);
int open_fifo(int *fd);
int read_fifo(unsigned char *recv_from_live555_data, int fd,int read_size);
int write_fifo(u_int8_t *send_to_vdec_data,int fd,int write_size);

//int malloc_space(VDEC_BUF *fifo);

int indexSubtract(int a,int b);
int indexAdd(int a,int b);
int buffDataNum(int rindex,int windex);
int buffFreeSpace(int rindex,int windex);
int buffFull(int rindex,int windex);
int buffNull(int rindex,int windex);
int readDataToBuff(int nread, unsigned char * dest_buff, unsigned char *src_buf,int rindex,int windex,MESSAGE *msg,int dataindex);
#endif 
