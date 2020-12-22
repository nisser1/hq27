#include <RecvLinklist.h>
#include <stdio.h>
#include <string.h>
linklist *create_linklist()
{
	linklist *tmp = (linklist *)malloc(sizeof(linklist));
	printf("syjwrite linklist size = %d\n",sizeof(linklist));
	tmp->next = NULL;
	return tmp;
}
void insert_head_linklist( u_int8_t *frame,linklist *link)
{
	linklist *tmp = (linklist *)malloc(sizeof(linklist));
	strcpy(tmp->data,frame);
	tmp->next = link->next;
	link->next = tmp;
	return ;
}
void insert_tail_linklist( u_int8_t *frame,linklist *link)
{
	printf("sizeof(linklist) %d \n",sizeof(linklist));
	linklist *tmp = (linklist *)malloc(sizeof(linklist));
	strcpy(tmp->data,frame);
	while(link->next != NULL)
	{
		link = link->next;
	}
	link->next = tmp;
	tmp->next = NULL;
	return ;
}
int delete_head_linlist(linklist *link)
{
	if(link->next == NULL )
	{
		return 2;
	}
	linklist *tmp;
	//char *data = tmp->data;
	tmp = link->next;
	link->next = tmp->next;
	free(tmp);
	tmp = NULL;
	return 0;
}
linklist *get_head_data(linklist *link)
{
	if(link->next == NULL )
	{
		return NULL;
	}
	return link->next;
}





