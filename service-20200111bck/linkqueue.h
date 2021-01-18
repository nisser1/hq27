#ifndef __LINKQUEUE_H_
#define __LINKQUEUE_H_
#include "service.h"


//创建一个空的队列
LINK_QUEUE *linkqueue_create();
//判断队列是否为空
int linkqueue_empty(LINK_QUEUE *lq);
//入队
void linkqueue_input(LINK_QUEUE *lq, unsigned char *recvdata);
//出队
int linkqueue_output(LINK_QUEUE *lq);
//获取队列首部数据
unsigned char* get_linklist_data(LINK_QUEUE *lq);


/*
LINK_LIST *linklist_create();
//判断链表是否为空
int linklist_empty(LINK_LIST *ls);
//删除
void linklist_input(LINK_LIST *ls, unsigned char *recvdata);
//插入
datatype_t linklist_output(LINK_LIST *ls);
//获取链表首部数据
unsigned char* get_linklist_data(LINK_LIST *ls);*/
#endif