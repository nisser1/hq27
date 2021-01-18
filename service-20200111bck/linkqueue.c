#include <stdio.h>
#include <stdlib.h>
#include "linkqueue.h"



//创建一个空的队列
LINK_QUEUE *linkqueue_create()
{
	//把操作队列的这两个指针在堆区开辟空间
	LINK_QUEUE *lq = (LINK_QUEUE *)malloc(sizeof(LINK_QUEUE));

	//申请一个头结点的空间，标识队列为空
	lq->front = lq->rear = (linknode_t *)malloc(sizeof(linknode_t));
	
	//初始化结构体
	lq->front->next = NULL;

	return lq;
}

//判断队列是否为空
int linkqueue_empty(LINK_QUEUE *lq)
{
	return lq->front == lq->rear ? 1 : 0;
}

//入队
void linkqueue_input(LINK_QUEUE *lq, unsigned char *recvdata)
{
	linknode_t *temp = (linknode_t *)malloc(sizeof(linknode_t));
	int i = 0;
	for(i=0; i<8; i++)
	{
		temp->data[i] = recvdata[i];
	}
	temp->next = NULL;
	
	//将新插入的结点插入到rear的后面
	lq->rear->next = temp;
	//将rear指向最后一个结点（新插入的结点）
	lq->rear = temp;
	return ;
}

unsigned char* get_linkqueue_data(LINK_QUEUE *lq)
{
	if(linkqueue_empty(lq))
		return NULL;
	return lq->front->next->data;
}

//出队
int linkqueue_output(LINK_QUEUE *lq)
{
	if(linkqueue_empty(lq))
	{
		printf("linkqueue is empty\n");
		return (int)-1;
	}

	linknode_t *temp = lq->front->next;
	lq->front->next = temp->next;

	//int value = temp->data;

	free(temp);
	temp = NULL;

	//当最后一个有数据的结点删除之后，需要将rear指向头结点，接着可以执行入队操作
	if(lq->front->next == NULL)
	{
		lq->rear = lq->front;
	}
	
	return 0;
}


























#if 0
LINK_IP_INFO *linklist_create()
{
	LINK_IP_INFO *list = (LINK_IP_INFO *)malloc(sizeof(LINK_IP_INFO));
	list->next =NULL;
	return list;
}
/*
//判断链表是否为空
int linklist_empty(LINK_IP_INFO *ls)
{
	ls->next==NULL:
}*/
//删除不需要回复的节点
void linklist_del(LINK_IP_INFO *ls)
{
	LINK_IP_INFO *lstmp;
	while(ls->next != NULL)
	{
		ls = ls->next;	
		if(ls->reference_count != 0)
		{
			ls->reference_count--;
		}
		else
		{
			//在指定位置做删除操作
			lstmp = ls->next;
			ls = lstmp;
			lstmp = NULL;
			free(lstmp);
		}
	}
	return ;
}
//如果该IP已经存在那么索引+1  如果没有就做尾插
//返回值 非0 代表原有链表对应地址引用索引 0代表新增
int linklist_insert(LINK_IP_INFO *ls,char *ip_addr)
{
	while(ls->next != NULL)
	{
		ls = ls->next;
		if(strcmp(ls->ipaddr,ip_addr) == 0)
		{
			ls->reference_count++;
			return ls->reference_count;
		}
	}
	LINK_IP_INFO *temp = (LINK_IP_INFO *)malloc(sizeof(LINK_IP_INFO));	
	ls->next = temp;
	return 0;
}
//获取链表首部数据
unsigned char* get_linklist_data(LINK_IP_INFO *ls)
{
	
}


#endif








