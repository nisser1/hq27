#ifndef	 __RECVLINKLIST_H__
#define  __RECVLINKLIST_H__

typedef unsigned char u_int8_t;
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000
typedef struct node{
	u_int8_t data[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
	struct node *next;
}linklist;

linklist *create_linklist();
void insert_head_linklist(u_int8_t *frame,linklist *link);
void insert_tail_linklist(u_int8_t *frame,linklist *link);
int delete_head_linlist(linklist *link);
#endif 
